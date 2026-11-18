# Chapter 10: Debugowanie i typowe błędy

[← Spis treści](README.md)

**Cel rozdziału:** diagnozować problemy z pamięcią stosu, awarie (fatal
error), konfiguracją i budowaniem oraz wiedzieć, jak czytać raporty pamięci
i uruchamiać debugger na NUCLEO-H743ZI. Na końcu: lista typowych błędów z
całego kursu i pytania rekrutacyjne.

---

## 1. Stos: ile mam i ile zużywam

Stos to najczęstsze źródło tajemniczych awarii. W Zephyrze każdy wątek ma
**stały, statyczny** stos podany w bajtach, więc sam musisz go dobrać.

### Narzędzie 1: `k_thread_stack_space_get`

```
CONFIG_INIT_STACKS=y         # wypełnia stos wzorcem, żeby dało się zmierzyć zużycie
CONFIG_THREAD_STACK_INFO=y   # (na Twojej płytce domyślnie włączone)
```

Załóżmy, że `worker_tid` to wątek zdefiniowany przez
`K_THREAD_DEFINE(worker_tid, STACK_SIZE, worker, NULL, NULL, NULL, 5, 0, 0)` z
`STACK_SIZE` równym 1024:

```c
size_t unused;

if (k_thread_stack_space_get(worker_tid, &unused) == 0) {
	printk("worker: wolne %u z %d B stosu\n", (unsigned)unused, STACK_SIZE);
}
```

Wynik: `worker: wolne 772 z 1024 B stosu`. To zmierzone **maksymalne** zużycie
(*high-water mark*), analogiczne do `uxTaskGetStackHighWaterMark` z FreeRTOS.

### Narzędzie 2: Thread Analyzer

```
CONFIG_THREAD_ANALYZER=y
CONFIG_THREAD_ANALYZER_USE_PRINTK=y
CONFIG_THREAD_ANALYZER_AUTO=y
CONFIG_THREAD_ANALYZER_AUTO_INTERVAL=5     # sekundy, dozwolony zakres 5..3600
CONFIG_THREAD_NAME=y
CONFIG_INIT_STACKS=y
CONFIG_THREAD_STACK_INFO=y
```

Cyklicznie wypisuje zużycie stosu i CPU każdego wątku (QEMU):

```
Thread analyze:
 worker_tid          : STACK: unused  772 usage  252 / 1024 ( 24 %); CPU:   0 %
                     : Total CPU cycles used: 0
 thread_analyzer     : STACK: unused  600 usage  424 / 1024 ( 41 %); CPU:  56 %
 hungry_tid          : STACK: unused  460 usage   52 /  512 ( 10 %); CPU:   0 %
 idle                : STACK: unused  204 usage   52 /  256 ( 20 %); CPU:   0 %
 main                : STACK: unused  772 usage  252 / 1024 ( 24 %); CPU:  18 %
 ISR0                : STACK: unused 1876 usage  172 / 2048 (  8 %); CPU:   0 %
```

(wartości `CPU` w QEMU są tylko orientacyjne, bo symulator nie odwzorowuje czasu).
Na płytce zobaczysz realne. Dodatkowo `CONFIG_THREAD_ANALYZER_AUTO_INTERVAL=1`
kończy się błędem budowania (poniżej minimum 5, patrz Chapter 3).

### Narzędzie 3: shell

`kernel thread stacks` (Chapter 8) pokazuje to samo bez zmian w kodzie.

**Zasada doboru:** zmierz maksimum w testach obciążeniowych i dodaj **zapas
(np. 25–50%)**. Uważaj na rekurencję, duże tablice lokalne i `printf`, który
potrafi zużyć sporo stosu.

## 2. Przepełnienie stosu – prawdziwy przypadek

Wątek z **512 B** stosu i lokalną tablicą **600 B**:

```c
static void hungry(void *p1, void *p2, void *p3)
{
	volatile uint8_t big[600];	/* więcej niż cały stos wątku (512 B) */

	for (int i = 0; i < 600; i++) {
		big[i] = (uint8_t)i;
	}
	printk("hungry: bufor wypelniony\n");
	k_msleep(100);
}
K_THREAD_DEFINE(hungry_tid, 512, hungry, NULL, NULL, NULL, 5, 0, 1000);
```

Wynik w QEMU (z `CONFIG_STACK_SENTINEL=y`):

```
hungry: bufor wypelniony
r0/a1:  0x00000002  r1/a2:  0xf0f0f0f0  r2/a3:  0x20000738
r3/a4:  0x00000002 r12/ip:  0x00000000 r14/lr:  0x00001a37
 xpsr:  0x11000000
Faulting instruction address (r15/pc): 0x0000145c
>>> ZEPHYR FATAL ERROR 2: Stack overflow on CPU 0
Current thread: 0x20000050 (hungry_tid)
Halting system
```

Jak to czytać:

| Linia | Znaczenie |
|---|---|
| `ZEPHYR FATAL ERROR 2` | Kod `K_ERR_STACK_CHK_FAIL` (0 = wyjątek CPU, 1 = spurious IRQ, 2 = **stack check fail**, 3 = oops, 4 = panic) |
| `Stack overflow on CPU 0` | Przyczyna |
| **`Current thread: ... (hungry_tid)`** | **Winowajca.** Nazwa wątku (przy `CONFIG_THREAD_NAME=y`) |
| `Faulting instruction address (r15/pc)` | Adres do zamiany na linię kodu |
| `Halting system` | Domyślna reakcja, system się zatrzymuje |

Adres z `pc` zamieniasz na funkcję narzędziem z SDK:

```powershell
arm-zephyr-eabi-addr2line -e build\zephyr\zephyr.elf -f -p 0x145c
# z_check_stack_sentinel at ...\zephyr\kernel\thread.c:355
```

> ⚠️ **Niuans.** Mechanizm `STACK_SENTINEL` działa tak: najniższe słowo stosu
> trzyma wartość kontrolną, a funkcja `z_check_stack_sentinel` porównuje ją **przy
> przełączaniu kontekstu**. Jeśli ją nadpisano, zgłasza `K_ERR_STACK_CHK_FAIL`.
> Dlatego adres w `pc` wskazuje funkcję, która **wykryła** przepełnienie, a nie tę,
> która je spowodowała. Szukaj winowajcy w linii `Current thread:`, a nie w `pc`.
> (Natomiast `CONFIG_INIT_STACKS` wypełnia stos wzorcem `0xAA`, po którym
> `k_thread_stack_space_get` rozpoznaje, ile stosu nie zostało użyte.)

### Metody wykrywania na Twojej płytce

| Metoda | Konfiguracja | Jak działa |
|---|---|---|
| **MPU (sprzętowa)** | `CONFIG_HW_STACK_PROTECTION=y` | **Domyślnie włączone** w defconfig `nucleo_h743zi`. Strefa ochronna przy końcu stosu, dostęp do niej wywołuje wyjątek |
| **Sentinel (programowa)** | `CONFIG_STACK_SENTINEL=y` | Sprawdza wartość kontrolną przy przełączeniu kontekstu. Wykrywa **po fakcie** |

Powyższy przykład uruchomiono w QEMU (`qemu_cortex_m3`, bez MPU), stąd
programowy sentinel. Na Twojej płytce z MPU przepełnienie powinno zostać
zgłoszone **wcześniej**, jako wyjątek ochrony pamięci, więc komunikat będzie inny
niż pokazany. Sprawdź to w zadaniu 2. Zawsze szukaj linii `Current thread:`.

## 3. Debugger (GDB przez OpenOCD)

Dla tej płytki domyślnym narzędziem debugowania jest OpenOCD (z `runners.yaml`):

```powershell
west debug                     # buduje, wgrywa i otwiera GDB
west attach                    # dołącza do działającego programu
```

Aby kod był czytelny w debuggerze (zmienne nie są „zoptymalizowane”),
dodaj do `prj.conf`:

```
CONFIG_DEBUG_OPTIMIZATIONS=y        # kompilacja z niską optymalizacją
CONFIG_DEBUG_THREAD_INFO=y          # informacje o wątkach dla GDB
```

W GDB (z symbolami `zephyr.elf`) przydatne są: `break main`, `info threads`,
`bt`, `monitor reset halt`. Wątki Zephyra widać na liście wątków po włączeniu
`DEBUG_THREAD_INFO`.

## 4. Asserty

```
CONFIG_ASSERT=y        # domyślnie WYŁĄCZONE na Twojej płytce
```

```c
__ASSERT(ptr != NULL, "wskaznik nie moze byc NULL");
```

Przy porażce system zatrzymuje się z komunikatem (plik, linia). Włącz w
fazie rozwoju, wyłącz w produkcji. To odpowiednik `configASSERT`.

## 5. Raporty pamięci

```powershell
west build -d build\blinky -t rom_report     # co zajmuje Flash
west build -d build\blinky -t ram_report     # co zajmuje RAM
```

Raport rozbija zajętość na pliki i funkcje (drzewo z procentami). Przykładowo
w blinky widać `__l_vfprintf` (**1158 B**, czyli `printf` z picolibc) oraz
`__udivmoddi4` (700 B). Podsumowanie po każdym buildzie:

```
FLASH:       20996 B         2 MB      1.00%
RAM:          4672 B       512 KB      0.89%
```

Także plik `zephyr.map` mówi dokładnie, co gdzie trafiło.

## 6. Katalog typowych błędów (z całego kursu)

Wszystkie poniższe komunikaty zostały **odtworzone** na tym środowisku.

| Objaw | Przyczyna | Rozwiązanie | Rozdział |
|---|---|---|---|
| `west: unknown command "build"` | `west` uruchomiony poza workspace | Uruchom z wnętrza workspace lub ustaw `ZEPHYR_BASE` | 0 |
| `west flash` nie działa | Domyślny programator to STM32CubeProgrammer | `west flash --runner openocd` | 0 |
| `DT_N_ALIAS_led0_... undeclared` | Brak aliasu/węzła w tej płytce | Dodaj alias w overlayu lub zmień płytkę | 2 |
| `devicetree error: 'xyz' ... not declared in 'properties:'` | Literówka we właściwości DT | Sprawdź binding YAML | 2 |
| `Aborting due to Kconfig warnings` | Literówka w symbolu lub zły zakres | Popraw `prj.conf` | 3 |
| `was assigned the value 'y' but got the value 'n'` | Niespełniona zależność (build przechodzi!) | Przeczytaj listę zależności, włącz węzeł DT/symbole | 3 |
| `undefined reference to z_impl_k_...` | Brak symbolu Kconfig (np. `CONFIG_EVENTS`) | Dodaj symbol do `prj.conf` | 3, 6 |
| `__device_dts_ord_<N> undeclared` | Węzeł wyłączony lub bez sterownika | `status = "okay"`, włącz sterownik | 9 |
| `fatal error: span: No such file` | C++ bez pełnej biblioteki | `CONFIG_REQUIRES_FULL_LIBCPP=y` | 11 |
| `threads: command not found` | Zmiana nazw komend shella | `kernel thread list` | 8 |
| `ZEPHYR FATAL ERROR 2: Stack overflow` | Za mały stos wątku | Zwiększ stos, zmierz zużycie | 10 |

## 7. Błędy projektowe (bez komunikatu kompilatora)

| Pułapka | Dlaczego groźna |
|---|---|
| **Odwrócone priorytety** względem FreeRTOS | Wątek „ważny” dostaje liczbę wyższą, a więc **niższy** priorytet |
| Pętla bez blokowania w wątku o priorytecie ≥ innych | Głodzi wątki o niższym priorytecie. `main` ma 0, więc wątki 5 nigdy nie ruszą |
| Długi handler w `sysworkq` | Kolejka jest kooperacyjna (-1). Blokuje inne prace i wątki |
| Blokujące wywołanie w ISR (`k_msleep`, `k_mutex_lock`, `k_sem_take` z timeoutem) | Awaria lub zakleszczenie. W ISR tylko wywołania nieblokujące |
| Callback `k_timer` traktowany jak wątek | Działa w ISR, nie wolno tam blokować |
| `k_event_wait(..., reset=true, ...)` | Zgubione zdarzenia wysłane przed czekaniem. Używaj `_safe` |
| Brak `device_is_ready()` | Urządzenie bez sterownika/czujnika = nieokreślone zachowanie |
| `uart_poll_out` na konsoli | Miesza się z `printk`/logami |
| Wskaźnik do zmiennej lokalnej w `k_fifo` / `k_work` | Element nie żyje, gdy odbiorca po niego sięga |
| Założenie, że tick = 1 ms | Na tej płytce tick to 100 µs (10 000 Hz). Używaj `K_MSEC(...)` |
| Wątek `main` zakończony | To normalne. Reszta systemu działa dalej |
| Ignorowanie kodów zwrotnych | Funkcje zwracają ujemne errno. Sprawdzaj wynik |

---

## 8. Pytania rekrutacyjne (z krótkimi odpowiedziami)

1. **Czym Zephyr różni się od FreeRTOS?**
   Zephyr to cała platforma: jądro, sterowniki z jednolitym API, system budowania
   (CMake + Kconfig + Devicetree), podsystemy (logging, shell, sieć, USB). FreeRTOS to
   samo jądro. W Zephyrze sprzęt opisuje drzewo urządzeń, a nie kod.
2. **Wymień trzy filary konfiguracji.**
   CMake (jak budować), Kconfig (jakie funkcje włączyć), Devicetree (jaki jest
   sprzęt i jak połączony).
3. **Czym jest alias, a czym `chosen`?**
   Alias (`led0`) to stała nazwa węzła, której używa kod aplikacji. `chosen`
   wskazuje urządzenie pełniące rolę systemową (konsola, shell, SRAM).
4. **Jak działają priorytety wątków?**
   Mniejsza liczba = wyższy priorytet. Ujemne są kooperacyjne (nie wywłaszczane
   przez wątki), nieujemne wywłaszczalne. *Idle* ma najniższy.
5. **Dlaczego nie robić pracy w ISR i co jest alternatywą?**
   ISR ma być krótki, bez blokowania. Zlecasz pracę przez `k_work_submit` /
   `k_sem_give` / `k_event_post`, a wątek wykonuje ją w normalnym kontekście
   (*deferred interrupt processing*).
6. **Mutex vs semafor w Zephyrze?**
   `k_mutex` ma właściciela, jest rekurencyjny i dziedziczy priorytet. Chroni dane.
   `k_sem` nie ma właściciela, służy do sygnalizacji i zliczania. Mutex nie
   działa w ISR, a `k_sem_give` tak.
7. **Jak sprawdzić, ile stosu zużywa wątek?**
   `kernel thread stacks` w shellu, Thread Analyzer lub `k_thread_stack_space_get`
   (wymaga `CONFIG_INIT_STACKS`).
8. **Czym różni się `k_timer` od `k_work_delayable`?**
   Callback `k_timer` działa w ISR, a `k_work_delayable` wykonuje się w wątku
   kolejki pracy i może blokować.
9. **Co daje `west build -p always`?**
   Czyści katalog budowania (*pristine*). Konieczne przy zmianie płytki i
   po niektórych zmianach konfiguracji.
10. **Dlaczego `K_THREAD_DEFINE` alokuje statycznie?**
    Pamięć jest znana w czasie kompilacji (bez fragmentacji sterty), widoczna
    w raporcie pamięci i bezpieczna dla systemów o ograniczonych zasobach.

---

## ✅ Zadania

1. Uruchom Thread Analyzer w swoim projekcie i zapisz zużycie stosu
   `main`, wątku shella i wątków własnych. Który ma najmniejszy zapas?
2. Wywołaj kontrolowane przepełnienie stosu jak w sekcji 2 na swojej
   płytce (z `HW_STACK_PROTECTION`). Jak wygląda komunikat w porównaniu z QEMU?
3. Zamień adres z `pc` na linię kodu przez `arm-zephyr-eabi-addr2line`.
4. Uruchom `west build -t rom_report` dla blinky. Ile zajmuje `printf`?
   Zamień `printf` na `printk` i zmierz zmianę.
5. Włącz `CONFIG_ASSERT=y` i wywołaj `__ASSERT(0, "test")`. Jak wygląda raport?
6. Uruchom `west debug`, ustaw `break main`, wykonaj `info threads`.
7. Odtwórz co najmniej cztery błędy z katalogu z sekcji 6 i zapisz
   dokładne komunikaty.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 11 – C++ w Zephyrze](chapter11_cpp.md)
