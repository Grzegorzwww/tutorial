# Chapter 3: Kconfig i `prj.conf`

[← Spis treści](README.md)

**Cel rozdziału:** konfigurować funkcje Zephyra przez Kconfig: pisać
`prj.conf`, znać kolejność nakładania konfiguracji, dodawać własne symbole
i rozpoznawać typowe błędy (literówki, zakresy, brakujące zależności).

---

## 1. Czym jest Kconfig

Kconfig to ten sam system konfiguracji, którego używa jądro Linuksa. Każda
funkcja ma **symbol** (`CONFIG_NAZWA`) o typie `bool`, `int`, `hex` lub
`string`, z opisem, wartością domyślną, **zależnościami** i zakresem.

W `prj.conf` wpisujesz tylko to, co chcesz zmienić względem domyślnych:

```
CONFIG_GPIO=y                      # bool: włącz sterowniki GPIO
CONFIG_MAIN_STACK_SIZE=2048        # int
CONFIG_LOG=y
# CONFIG_ASSERT is not set         # wyłączone (komentarz specjalnego formatu)
```

Wynik trafia do `build\zephyr\.config` (pełna lista wszystkich symboli) oraz
`autoconf.h` (jako `#define CONFIG_...`).

## 2. Skąd bierze się końcowa wartość

Konfiguracja jest **składana z warstw**, a późniejsze nadpisują wcześniejsze:

1. wartości domyślne symboli (z plików `Kconfig` jądra i sterowników),
2. **defconfig płytki** (np. `nucleo_h743zi_defconfig`: MPU, ochrona stosu, UART),
3. **`prj.conf`** aplikacji,
4. pliki dodatkowe: `boards/<płytka>.conf`, `-DEXTRA_CONF_FILE=...`,
5. opcje z linii poleceń: `-DCONFIG_NAZWA=y`.

Przykład dodatkowego pliku przekazanego jawnie:

```powershell
west build -b nucleo_h743zi <aplikacja> -- -DEXTRA_CONF_FILE=debug.conf
```

Zawsze możesz sprawdzić wynik: `grep CONFIG_NAZWA build\zephyr\.config`.

## 3. Edycja interaktywna

```powershell
west build -t menuconfig     # interfejs tekstowy (Windows: pakiet windows-curses)
west build -t guiconfig      # interfejs graficzny
```

Narzędzia pokazują opis, zależności i **to, dlaczego symbol jest niewidoczny**.
Uwaga: zmiany zapisane w `menuconfig` trafiają do katalogu budowania i **giną
przy `-p always`**. Trwałą zmianę wpisz do `prj.conf`.

## 4. Zależności: `depends on` i `select`

Symbole mają zależności. `depends on` oznacza „ten symbol może być włączony
tylko, jeśli tamten jest włączony”, a `select` automatycznie włącza inne
symbole. Przykład: `CONFIG_CPP=y` wybiera (`select`) dodatkowe symbole
potrzebne do obsługi C++.

**Pułapka: niespełniona zależność nie zatrzymuje budowania.** Wpisz w
`prj.conf` sterownik, którego zależności nie są spełnione (tu sterownik I2C
STM32 w budowie dla QEMU):

```
CONFIG_I2C_STM32=y
```

Build się powiedzie, a w logu pojawi się tylko ostrzeżenie:

```
warning: I2C_STM32 (defined at drivers/i2c/Kconfig.stm32:4) was assigned the value 'y' but got the
value 'n'. Check these unsatisfied dependencies: (DT_HAS_ST_STM32_I2C_V1_ENABLED ||
DT_HAS_ST_STM32_I2C_V2_ENABLED) (=n), I2C (=n). ...
```

Symbol **nie trafia do `.config`**, więc sterownik nie istnieje w programie, choć
„włączyłeś” go w `prj.conf`. Przeczytaj w takim komunikacie listę
niespełnionych zależności.

Zwróć uwagę na `DT_HAS_ST_STM32_I2C_V1_ENABLED`: **symbole sterowników zależą od
Devicetree**. Sterownik włącza się, gdy w drzewie urządzeń jest odpowiedni węzeł ze
`status = "okay"`. Dlatego do działania peryferium potrzebujesz obu: węzła w DT
(Chapter 2) i symbolu w Kconfig.

## 5. Wartości domyślne na Twojej płytce

To **prawdziwe wartości** odczytane z `.config` zbudowanego blinky dla
`nucleo_h743zi`:

| Symbol | Wartość | Znaczenie |
|---|---|---|
| `SYS_CLOCK_TICKS_PER_SEC` | **10000** | Tick systemowy 10 kHz (rozdzielczość 100 µs) |
| `SYS_CLOCK_HW_CYCLES_PER_SEC` | 480000000 | Zegar rdzenia 480 MHz |
| `MAIN_STACK_SIZE` | 1024 | Stos wątku `main` [B] |
| `MAIN_THREAD_PRIORITY` | 0 | Priorytet `main` |
| `IDLE_STACK_SIZE` | 320 | Stos wątku *idle* [B] |
| `ISR_STACK_SIZE` | 2048 | Stos przerwań [B] |
| `SYSTEM_WORKQUEUE_STACK_SIZE` | 1024 | Stos kolejki pracy systemowej [B] |
| `SYSTEM_WORKQUEUE_PRIORITY` | **-1** | Kolejka systemowa jest **kooperacyjna** |
| `NUM_PREEMPT_PRIORITIES` | 15 | Priorytety wywłaszczalne: 0..14 |
| `NUM_COOP_PRIORITIES` | 16 | Priorytety kooperacyjne: -16..-1 |
| `TIMESLICING` / `TIMESLICE_SIZE` | y / 20 | Round-robin 20 ms dla równych priorytetów |
| `HW_STACK_PROTECTION` | y | Ochrona stosu przez MPU (w defconfig płytki) |
| `HEAP_MEM_POOL_SIZE` | 0 | Sterta jądra wyłączona (`k_malloc` niedostępny) |
| `LOG`, `ASSERT`, `FPU` | wyłączone | Trzeba włączyć świadomie |
| `THREAD_NAME`, `INIT_STACKS` | wyłączone | Potrzebne do nazw wątków i statystyk stosu |
| `PICOLIBC` | y | Biblioteka C (zamiast newlib) |

> Różnice między płytkami są duże. Ten sam program w QEMU
> (`qemu_cortex_m3`) wydrukował `tick: 100 Hz`, a na Twojej płytce będzie
> 10000 Hz. Nigdy nie zakładaj wartości, tylko sprawdzaj `.config`.

## 6. Własne symbole aplikacji

W katalogu aplikacji możesz dodać plik `Kconfig`:

```
mainmenu "Moja aplikacja"

config APP_BLINK_PERIOD_MS
	int "Okres migania [ms]"
	default 500
	range 50 5000
	help
	  Okres zmiany stanu diody w aplikacji.

config APP_VERBOSE
	bool "Dodatkowe komunikaty diagnostyczne"

source "Kconfig.zephyr"
```

Ostatnia linia `source "Kconfig.zephyr"` jest **obowiązkowa**. Wciąga
wszystkie symbole Zephyra.

W `prj.conf` (lub pliku `.conf`):

```
CONFIG_APP_BLINK_PERIOD_MS=80
CONFIG_APP_VERBOSE=y
```

W kodzie:

```c
#include <zephyr/kernel.h>

int main(void)
{
	printk("okres = %d ms\n", CONFIG_APP_BLINK_PERIOD_MS);

	if (IS_ENABLED(CONFIG_APP_VERBOSE)) {
		printk("tryb verbose wlaczony\n");
	}

#if CONFIG_APP_BLINK_PERIOD_MS < 100
	printk("uwaga: bardzo szybkie miganie\n");
#endif

	printk("stos main: %d B, tick: %d Hz\n", CONFIG_MAIN_STACK_SIZE, CONFIG_SYS_CLOCK_TICKS_PER_SEC);
	return 0;
}
```

Przykładowy wynik (QEMU, Zephyr 4.5.99):

```
okres = 80 ms
tryb verbose wlaczony
uwaga: bardzo szybkie miganie
stos main: 1024 B, tick: 100 Hz
```

Dwa sposoby użycia w kodzie:
- `IS_ENABLED(CONFIG_X)` – zwykły `if`, kompilator usuwa martwy kod, ale
  **obie gałęzie muszą się kompilować**,
- `#if CONFIG_X` / `#ifdef` – usuwa kod z preprocesora.

## 7. Typowe błędy Kconfig (wszystkie odtworzone)

> Nie każde ostrzeżenie Kconfig przerywa budowanie. Poniższe dwa przypadki
> (literówka i zły zakres) kończą się błędem. Niespełniona zależność
> (sekcja 4) to **tylko ostrzeżenie**, więc czytaj log konfiguracji.

### Literówka w nazwie symbolu

`prj.conf`: `CONFIG_GPIOO=y`

```
neg_kc.conf:1: warning: attempt to assign the value 'y' to the undefined symbol GPIOO
error: Aborting due to Kconfig warnings
```

To chroni przed cichym ignorowaniem źle wpisanej opcji, której brak miałby
skutki dopiero w runtime.

### Wartość spoza zakresu

`CONFIG_THREAD_ANALYZER_AUTO_INTERVAL=1` (dozwolony zakres to 5–3600):

```
warning: user value 1 on the int symbol THREAD_ANALYZER_AUTO_INTERVAL ... ignored
         due to being outside the active range ([5, 3600]) -- falling back on defaults
error: Aborting due to Kconfig warnings
```

### Funkcja opcjonalna bez włączonego symbolu

Użycie `k_event_wait()` bez `CONFIG_EVENTS=y` **przechodzi kompilację**, ale
nie przechodzi linkowania:

```
undefined reference to `z_impl_k_event_wait'
undefined reference to `z_impl_k_event_post'
```

Rozpoznasz go po `z_impl_...`: oznacza, że kod wywołuje API, którego
implementacji nie ma w programie, bo odpowiedni symbol Kconfig jest wyłączony.
Rozwiązanie: sprawdź w dokumentacji/nagłówku, jakiego symbolu wymaga ta funkcja
(tu `CONFIG_EVENTS=y`), i dodaj go do `prj.conf`.

| Objaw | Zwykle znaczy |
|---|---|
| `Aborting due to Kconfig warnings` | Literówka w nazwie symbolu albo wartość spoza zakresu |
| `was assigned the value 'y' but got the value 'n'` | Niespełniona zależność (często brakujący węzeł DT lub inny symbol). Build przechodzi! |
| `undefined reference to z_impl_...` | Brak symbolu Kconfig włączającego dany moduł |
| `fatal error: xxx.h: No such file` | Podsystem/biblioteka niewłączone w Kconfig (np. `<span>` w C++, Chapter 11) |

## 🔧 Różnica względem FreeRTOS

| FreeRTOS (`FreeRTOSConfig.h`) | Zephyr (Kconfig) |
|---|---|
| `#define configUSE_...` włączają funkcje jądra | `CONFIG_EVENTS=y`, `CONFIG_LOG=y`, ... dla funkcji i podsystemów |
| Literówka w `#define` = cicho ignorowana opcja | Nieistniejący symbol = błąd budowania |
| Brak zależności między opcjami | `depends on` / `select`, kontrola zakresów |
| Jeden plik nagłówkowy | Warstwy: płytka → `prj.conf` → pliki dodatkowe |
| Konfiguracja wyłącznie w kodzie (`#define`) | Osobne pliki `.conf`, dostępne też dla CMake |

---

## ✅ Zadania

1. Zbuduj aplikację z własnym symbolem `APP_BLINK_PERIOD_MS` (jak w
   sekcji 6). Zmień wartość w `prj.conf` i sprawdź w `.config`.
2. Wpisz `CONFIG_APP_BLINK_PERIOD_MS=10` (poniżej zakresu 50). Jaki jest
   komunikat?
3. Odtwórz błąd `undefined reference to z_impl_k_event_wait`: wywołaj
   `k_event_post` bez `CONFIG_EVENTS=y`, a potem napraw.
4. Otwórz `.config` i sprawdź na swojej płytce: `SYS_CLOCK_TICKS_PER_SEC`,
   `HW_STACK_PROTECTION`, `HEAP_MEM_POOL_SIZE`. Co z nich wynika dla
   `k_msleep(1)` i dla `k_malloc`?
5. Uruchom `west build -t menuconfig` (lub `guiconfig`), znajdź opcję `LOG` i
   przeczytaj jej zależności.
6. Wpisz do `prj.conf` sterownik z niespełnioną zależnością (np.
   `CONFIG_I2C_STM32=y` dla `qemu_cortex_m3`). Czy build się powiódł? Czy symbol
   jest w `.config`? Jakie zależności wymienia ostrzeżenie?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 4 – Wątki i scheduling](chapter4_threads.md)
