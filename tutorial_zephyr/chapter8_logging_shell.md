# Chapter 8: Logging i Shell

[← Spis treści](README.md)

**Cel rozdziału:** zastąpić `printf`/`printk` systemem logowania z poziomami i
modułami oraz używać shella jako interaktywnej konsoli do debugowania na płytce
(własne komendy i wbudowane komendy kernela).

---

## 1. `printk`, `printf` czy `LOG_*`?

| Mechanizm | Cechy | Kiedy |
|---|---|---|
| `printk` | Prosty, bez formatowania zmiennoprzecinkowego, działa też z ISR, bardzo wcześnie | Szybka diagnostyka, ścieżki awaryjne |
| `printf` | Z biblioteki C (picolibc), cięższy | Gdy potrzebujesz pełnego formatowania |
| **`LOG_*`** | Poziomy, moduły, znacznik czasu, wyłączanie w czasie kompilacji, **tryb odroczony** | Standard w prawdziwej aplikacji |

Logowanie jest wydajniejsze od `printf`, bo w trybie odroczonym wywołanie tylko
**zapisuje dane do bufora**, a formatowanie i wysyłanie na UART robi osobny wątek.

## 2. Logowanie

```
# prj.conf
CONFIG_LOG=y
```

```c
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);       // nazwa modułu i jego poziom

int main(void)
{
	uint8_t buf[] = { 0xDE, 0xAD, 0xBE, 0xEF };

	LOG_INF("start, uptime=%u ms", k_uptime_get_32());
	LOG_DBG("debug: %d", 1);
	LOG_WRN("ostrzezenie");
	LOG_ERR("blad, kod=%d", -5);
	LOG_HEXDUMP_INF(buf, sizeof(buf), "bufor:");

	return 0;
}
```

Wynik (QEMU):

```
[00:00:00.000,000] <inf> app: start, uptime=0 ms
[00:00:00.000,000] <dbg> app.main: debug: 1
[00:00:00.000,000] <wrn> app: ostrzezenie
[00:00:00.000,000] <err> app: blad, kod=-5
[00:00:00.000,000] <inf> app: bufor:
                              de ad be ef                                      |....
```

Format linii: `[czas] <poziom> moduł: treść`. Dla `LOG_DBG` dopisywana jest
też nazwa funkcji (`app.main`).

**Poziomy** (od najważniejszego): `LOG_ERR` < `LOG_WRN` < `LOG_INF` < `LOG_DBG`.
Poziom modułu ustawiasz w `LOG_MODULE_REGISTER(nazwa, poziom)`. Wywołania
poniżej poziomu są **usuwane w czasie kompilacji**, więc nie kosztują nic.

W kolejnych plikach tego samego modułu użyj:

```c
LOG_MODULE_DECLARE(app);        // bez ponownej rejestracji
```

### Domyślna konfiguracja (odczytana z `.config`)

| Symbol | Wartość | Znaczenie |
|---|---|---|
| `LOG_MODE_DEFERRED` | y | Logi trafiają do bufora, wysyła je osobny wątek |
| `LOG_DEFAULT_LEVEL` | 3 (`INF`) | Domyślny poziom modułów bez jawnego poziomu |
| `LOG_BACKEND_UART` | y | Logi wychodzą na konsolę UART (po włączeniu shella ten backend znika, patrz sekcja 4) |
| `LOG_BUFFER_SIZE` | 1024 | Bufor logów [B]. Przepełnienie = utrata komunikatów |
| `LOG_PROCESS_THREAD_STACK_SIZE` | 768 | Stos wątku przetwarzającego logi |
| `LOG_PRINTK` | y | Przy włączonym `LOG` także `printk` idzie przez system logowania |

Tryby: **deferred** (domyślny, szybki w miejscu wywołania), **immediate**
(`CONFIG_LOG_MODE_IMMEDIATE=y`: wysyła od razu w kontekście wywołującego, wolniejszy,
ale kolejność jest natychmiastowa, przydatny przy debugowaniu awarii) i
**minimal** (najmniejszy narzut pamięci).

> Uwaga: w trybie odroczonym logi z przerwania mogą pojawić się **trochę później**
> niż `printk` z tego samego miejsca. Jeśli w trakcie debugowania zawieszenia
> logi „urywają się”, przełącz na tryb immediate.

## 3. Shell

Shell to interaktywna konsola w tym samym kanale co logi. Dla Twojej płytki
`zephyr,shell-uart = &usart3`, więc korzystasz z tego samego wirtualnego portu COM.

```
# prj.conf
CONFIG_SHELL=y
CONFIG_KERNEL_SHELL=y        # wbudowane komendy "kernel ..."
CONFIG_DEVICE_SHELL=y        # komenda "device list"
CONFIG_THREAD_NAME=y         # nazwy wątków na listach
CONFIG_INIT_STACKS=y         # statystyki zużycia stosu
```

### Własne komendy

```c
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

static int cmd_hello(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "Czesc, %s!", argc > 1 ? argv[1] : "Zephyr");
	return 0;
}

#if DT_NODE_EXISTS(DT_ALIAS(led0))
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static int cmd_led(const struct shell *sh, size_t argc, char **argv)
{
	if (!gpio_is_ready_dt(&led)) {
		shell_error(sh, "LED niegotowy");
		return -ENODEV;
	}
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	if (strcmp(argv[1], "on") == 0) {
		gpio_pin_set_dt(&led, 1);
	} else if (strcmp(argv[1], "off") == 0) {
		gpio_pin_set_dt(&led, 0);
	} else {
		shell_error(sh, "uzycie: led <on|off>");
		return -EINVAL;
	}
	return 0;
}
SHELL_CMD_ARG_REGISTER(led, NULL, "led <on|off>", cmd_led, 2, 0);
#endif

SHELL_CMD_ARG_REGISTER(hello, NULL, "hello [imie]", cmd_hello, 1, 1);

int main(void)
{
	return 0;	/* cała praca odbywa się w wątku shella */
}
```

`SHELL_CMD_ARG_REGISTER(nazwa, podkomendy, opis, funkcja, wymagane_arg, opcjonalne_arg)`.
Shell sam sprawdza liczbę argumentów. Funkcja komendy zwraca `0` albo kod błędu.
Ten kod buduje się dla `nucleo_h743zi` (komenda `led` dostępna, bo jest alias
`led0`) oraz dla QEMU (bez komendy `led`).

### Wbudowane komendy – prawdziwa sesja (QEMU)

```
uart:~$ hello Grzegorz
Czesc, Grzegorz!
uart:~$ kernel version
Zephyr version 4.5.99
uart:~$ kernel thread list
Scheduler: 55 since last call
Threads:
*0x200000d0 shell_uart
	options: 0x0, priority: 14 timeout: 0
	state: queued, entry: 0x2331
	stack size 2048, unused 1096, usage 952 / 2048 (46 %)

 0x20000258 idle
	options: 0x1, priority: 15 timeout: 0
	state: , entry: 0x7d51
	stack size 256, unused 192, usage 64 / 256 (25 %)

uart:~$ kernel thread stacks
0x200000d0 shell_uart   (real size 2048):	unused 1096	usage  952 / 2048 (46 %)
0x20000258 idle         (real size  256):	unused  192	usage   64 /  256 (25 %)
0x20001390 IRQ 00       (real size 2048):	unused 1824	usage  224 / 2048 (10 %)
uart:~$ kernel uptime
Uptime: 8590 ms
uart:~$ device list
devices:
- uart@4000e000 (READY)
  DT node labels: uart2
...
```

> ⚠️ **Zmiana nazw komend.** W starszych poradnikach znajdziesz
> `kernel threads` i `kernel stacks`. W tej wersji (4.5.99) to
> **`kernel thread list`** i **`kernel thread stacks`**; stare nazwy dają
> `threads: command not found`. Zawsze sprawdź `kernel` (lista podkomend)
> i `help` w swojej wersji.

Co widać na liście wątków:
- wątek shella (`shell_uart`) ma priorytet **14** i stos 2048 B,
- wątek *idle* ma priorytet **15**,
- **brak wątku `main`**: ten program zwrócił z `main()` i wątek się zakończył,
  a shell nadal działa,
- stos przerwań (`IRQ 00`) jest wymieniony osobno, wraz ze zużyciem.

Listy z `kernel thread stacks` pozwalają **bez debuggera** dobrać rozmiary
stosów (patrz Chapter 10).

### Koszt pamięci

Dla `nucleo_h743zi` budowa z shellem i komendami kernela zajęła **51 924 B
Flash i 8,7 KB RAM**, a blinky z `printf` 20 996 B Flash i 4,7 KB RAM. Shell to
wygoda, ale kosztuje: wyłącz go w wersji produkcyjnej.

## 4. Logi i shell razem

Gdy włączysz `LOG` **i** `SHELL`, system logowania **zmienia backend**. Zweryfikowano
na buildzie z `CONFIG_LOG=y` i `CONFIG_SHELL=y`:

```
# CONFIG_LOG_BACKEND_UART is not set      <- osobny backend UART zniknął
CONFIG_SHELL_LOG_BACKEND=y                <- logi idą przez shella
CONFIG_LOG_MODE_DEFERRED=y
```

Logi są więc wypisywane **przez shella** (nad linią wpisywanej komendy), a
shell dba o poprawne odtworzenie linii, którą właśnie wpisujesz:

```
[00:00:01.010,000] <inf> app: log z main: 1
[00:00:02.020,000] <inf> app: log z main: 2
uart:~$ kernel version
Zephyr version 4.5.99
```

Koszt takiej kombinacji (shell + logowanie + komendy kernela) w QEMU to
53 364 B Flash i 11 KB RAM.

Komenda shella `log` do zmiany poziomów w czasie działania wymaga dodatkowo
`CONFIG_LOG_CMDS=y` (domyślnie wyłączone, mimo że `LOG_RUNTIME_FILTERING` włącza
się samo razem z backendem shella).

## 🔧 Różnica względem FreeRTOS

| FreeRTOS | Zephyr |
|---|---|
| Brak wbudowanego logowania (własny `printf`, SEGGER RTT, ...) | `LOG_*` w standardzie, poziomy, moduły, tryb odroczony |
| Konsola CLI jako osobny dodatek (FreeRTOS+CLI) | Wbudowany shell z komendami kernela, urządzeń i własnymi |
| Lista zadań przez `vTaskList` (wymaga konfiguracji) | `kernel thread list` / `kernel thread stacks` |
| Statystyki stosu przez `uxTaskGetStackHighWaterMark` | `kernel thread stacks`, `k_thread_stack_space_get` |

---

## ✅ Zadania

1. Zbuduj przykład z logowaniem. Zmień poziom modułu na `LOG_LEVEL_WRN`.
   Które linie znikają i czy trafiają do pliku wynikowego?
2. Dodaj drugi moduł (`LOG_MODULE_REGISTER(sensor, LOG_LEVEL_INF)`)
   w osobnym pliku i zobacz, jak wygląda prefiks w logach.
3. Włącz `CONFIG_LOG_MODE_IMMEDIATE=y`, porównaj rozmiar Flash i opóźnienie
   `LOG_INF` (zmierz czas przez `k_cycle_get_32()`).
4. Zbuduj shell i wgraj na płytkę. Wykonaj `hello`, `led on`, `led off`,
   `kernel thread list`, `kernel thread stacks`, `device list`.
5. Dodaj własną komendę `uptime2`, która wypisuje czas w sekundach
   (`k_uptime_get() / 1000`).
6. Odczytaj, ile Flash/RAM kosztuje shell na Twojej płytce, porównując
   `west build` z `CONFIG_SHELL=y` i bez.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 9 – Sterowniki i peryferia płytki](chapter9_drivers_peripherals.md)
