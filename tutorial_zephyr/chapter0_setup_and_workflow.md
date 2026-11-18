# Chapter 0: Środowisko i workflow

[← Spis treści](README.md)

**Cel rozdziału:** umieć zbudować, wgrać i uruchomić aplikację Zephyra na
NUCLEO-H743ZI, zobaczyć wyjście na konsoli i nie wpaść w typowe pułapki
środowiska (`west` poza workspace, domyślny programator, build dla złej
płytki).

---

## 1. Czym jest Zephyr (w jednym akapicie)

Zephyr to **RTOS + system budowania + zestaw sterowników i podsystemów** (logging,
shell, stos sieciowy, USB, Bluetooth, systemy plików...) rozwijany w ramach
Linux Foundation na licencji Apache-2.0. Różni się od FreeRTOS tym, że nie
dostajesz samego jądra, tylko **całą platformę**: opis sprzętu (Devicetree),
konfigurację funkcji (Kconfig), sterowniki z jednolitym API i narzędzie `west`
do zarządzania repozytoriami i budowaniem.

## 2. Co gdzie leży

Typowy workspace (nazwy z Twojego środowiska):

```
zephyrproject/                  <- katalog nadrzędny
├── .venv/                      <- środowisko Pythona z `west` i zależnościami
├── moja_apka/                  <- Twoja aplikacja (blinky)
└── zephyrproject/              <- WORKSPACE `west` (zawiera .west/)
    ├── .west/config
    ├── zephyr/                 <- jądro, sterowniki, płytki, dokumentacja
    │   └── boards/st/nucleo_h743zi/   <- definicja Twojej płytki
    └── modules/                <- HAL-e i biblioteki (hal_stm32, cmsis, ...)
```

Toolchain (Zephyr SDK) leży osobno: `%USERPROFILE%\zephyr-sdk-1.0.1` i zawiera
GCC dla ARM, OpenOCD, QEMU i inne narzędzia hosta.

> Aplikacja **nie musi** leżeć wewnątrz workspace – ale wtedy trzeba
> pamiętać o pułapce z sekcji 5.

## 3. Podstawowy workflow

Aktywuj środowisko Pythona (raz na sesję PowerShella), żeby `west` był w `PATH`:

```powershell
C:\workspace\zephyrproject\.venv\Scripts\Activate.ps1
cd C:\workspace\zephyrproject\zephyrproject      # wnętrze workspace
```

**Build** (pełna przebudowa, ok. 45 s na tej konfiguracji):

```powershell
west build -b nucleo_h743zi C:\workspace\zephyrproject\moja_apka -d build\blinky -p always
```

Kluczowe opcje:

| Opcja | Znaczenie |
|---|---|
| `-b nucleo_h743zi` | Płytka (identyfikator katalogu w `zephyr/boards`) |
| `-d <katalog>` | Katalog wyjściowy (domyślnie `build`) |
| `-p always` / `-p auto` | Wyczyść katalog budowania (*pristine*) zawsze / gdy trzeba |
| `-- -DEXTRA_CONF_FILE=...` | Argumenty przekazywane do CMake (po `--`) |
| `-t menuconfig` | Cel: edytor Kconfig (na Windows wymaga pakietu `windows-curses`) |
| `-t rom_report` / `ram_report` | Raport zajętości Flash / RAM |

Wynik budowania blinky dla tej płytki:

```
Memory region         Used Size  Region Size  %age Used
           FLASH:       20996 B         2 MB      1.00%
             RAM:        4672 B       512 KB      0.89%
```

**Wgranie i debug:**

```powershell
west flash                       # domyślny programator: STM32CubeProgrammer
west flash --runner openocd      # OpenOCD (jest w Zephyr SDK)
west debug                       # sesja gdb przez OpenOCD
```

**Konsola:** płytka wyprowadza konsolę na **USART3 (PD8/PD9)**, który ST-LINK
udostępnia jako wirtualny port COM (115200 8N1). Znajdź port w Menedżerze
urządzeń i podłącz dowolny terminal (PuTTY, Tera Term albo np. `miniterm`, jeśli
masz zainstalowany pakiet `pyserial`):

```powershell
python -m serial.tools.miniterm COM5 115200       # numer COM zmień na swój
```

Zgodnie z kodem blinky po zresetowaniu płytki powinieneś zobaczyć banner Zephyra
i naprzemienne komunikaty co sekundę:

```
*** Booting Zephyr OS build v4.5.0-2813-gf8da8ea37050 ***
LED state: OFF
LED state: ON
```

## 4. Co powstaje w katalogu budowania

```
build/zephyr/
├── zephyr.elf          <- obraz z symbolami (dla debuggera)
├── zephyr.hex / .bin   <- obrazy do wgrania
├── zephyr.map          <- mapa linkera (co ile zajmuje)
├── .config             <- WYNIKOWA konfiguracja Kconfig
├── zephyr.dts          <- WYNIKOWE drzewo urządzeń (po scaleniu overlayów)
├── runners.yaml        <- jakie programatory obsługuje płytka i który jest domyślny
└── include/generated/zephyr/
    ├── autoconf.h              <- CONFIG_* jako #define
    └── devicetree_generated.h  <- makra DT_* wygenerowane z drzewa urządzeń
```

Do tych plików będziesz zaglądać **bardzo często**, szczególnie do `.config`
i `zephyr.dts` – to źródło prawdy o tym, co faktycznie zostało zbudowane.

## 5. Typowe pułapki środowiska

### Pułapka 1: `west` poza workspace

Uruchomienie z katalogu aplikacji leżącego poza workspace kończy się tak:

```
west: unknown command "build"; do you need to run this inside a workspace?
```

`west` szuka katalogu `.west/` w katalogach nadrzędnych bieżącego katalogu.
Rozwiązania (oba sprawdzone):

```powershell
# a) uruchamiaj west z wnętrza workspace i podawaj ścieżkę do aplikacji
cd C:\workspace\zephyrproject\zephyrproject
west build -b nucleo_h743zi C:\workspace\zephyrproject\moja_apka

# b) ustaw ZEPHYR_BASE i buduj z katalogu aplikacji
$env:ZEPHYR_BASE = "C:\workspace\zephyrproject\zephyrproject\zephyr"
cd C:\workspace\zephyrproject\moja_apka
west build -b nucleo_h743zi .
```

### Pułapka 2: domyślny programator to STM32CubeProgrammer

W pliku `runners.yaml` dla tej płytki widać:

```yaml
runners: [stm32cubeprogrammer, openocd, jlink, pyocd]
flash-runner: stm32cubeprogrammer      # domyślny dla `west flash`
debug-runner: openocd                  # domyślny dla `west debug`
```

Jeśli nie masz zainstalowanego **STM32CubeProgrammer**, użyj
`west flash --runner openocd`.

### Pułapka 3: katalog budowania jest "przypięty" do płytki

Zmiana płytki w istniejącym katalogu bez `-p always` daje dziwne błędy
konfiguracji. Przy przełączaniu płytki zawsze czyść (`-p always`) albo używaj
osobnych katalogów (`-d build\nucleo`, `-d build\qemu`).

### Pułapka 4: pliki `boards/` są filtrowane po nazwie płytki

W `moja_apka\boards\` leży `nrf54h20dk_nrf54h20_cpuppr.overlay` (pozostałość
po przykładowym blinky). Nie ma wpływu na Twoją płytkę – overlay z katalogu
`boards/` jest używany **tylko dla płytki o pasującej nazwie**
(dla Ciebie: `boards/nucleo_h743zi.overlay`).

---

## ✅ Zadania

1. Zbuduj `moja_apka` dla `nucleo_h743zi` w osobnym katalogu `-d build\blinky`,
   wgraj ją i zobacz wyjście konsoli. Który programator został użyty?
2. Otwórz `build\blinky\zephyr\runners.yaml` i sprawdź, jaki jest domyślny
   programator oraz ścieżka do OpenOCD.
3. Zmień `SLEEP_TIME_MS` w `main.c` na 250, zbuduj ponownie (bez `-p`) i wgraj.
   Ile trwała przebudowa w porównaniu z pełnym buildem?
4. Odtwórz błąd `unknown command "build"` uruchamiając `west` z katalogu
   aplikacji, a następnie napraw go przez `ZEPHYR_BASE`.
5. Zbuduj tę samą aplikację dla `qemu_cortex_m3` w osobnym katalogu.
   Dlaczego się nie zbuduje (podpowiedź: aliasy `led0`/`led1`; wróć do tego w
   Chapter 2)?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 1 – Architektura i system budowania](chapter1_architecture_and_build.md)
