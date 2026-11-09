# Chapter 1: Filesystem, /proc i /sys

[← Spis treści](README.md)

**Cel rozdziału:** Zrozumieć jak Linux organizuje system plików oraz jak
jądro udostępnia informacje o sobie i o sprzęcie poprzez wirtualne
systemy plików `/proc` i `/sys`. To fundament pod dalsze rozdziały –
sterowniki, moduły jądra i device tree wszystkie "wystawiają się"
właśnie tutaj.

Ćwiczenia wykonujemy równolegle na:
- **Ubuntu** (stacja robocza, x86)
- **BeagleBone Black** (Debian Bullseye, ARM, `ssh debian@192.168.7.2`)

---

## 1. Filesystem Hierarchy Standard (FHS)

W Linuksie obowiązuje jedna, wspólna dla całego systemu hierarchia
katalogów zaczynająca się od `/` (root). W przeciwieństwie do Windows
nie ma osobnych liter dysków – wszystkie nośniki są "podpięte"
(zamontowane) gdzieś w tym jednym drzewie.

| Katalog       | Co tam jest |
|---------------|-------------|
| `/bin`, `/sbin` | Podstawowe programy wykonywalne (dziś często symlink do `/usr/bin`) |
| `/usr`        | Programy i biblioteki instalowane przez system/pakiety |
| `/etc`        | Pliki konfiguracyjne systemu (tekstowe, edytowalne ręcznie) |
| `/var`        | Dane zmienne w czasie: logi (`/var/log`), cache, kolejki |
| `/opt`        | Oprogramowanie firm trzecich instalowane poza menedżerem pakietów |
| `/lib`        | Biblioteki współdzielone (`.so`) potrzebne programom z `/bin` |
| `/dev`        | Pliki urządzeń (interfejs do sprzętu – patrz Rozdział 4) |
| `/proc`       | Wirtualny FS – stan jądra i procesów (opisany niżej) |
| `/sys`        | Wirtualny FS – model urządzeń jądra (opisany niżej) |
| `/home`       | Katalogi domowe użytkowników |
| `/tmp`        | Pliki tymczasowe, czyszczone przy restarcie |
| `/boot`       | Obraz jądra, initramfs, konfiguracja bootloadera |
| `/mnt`, `/media` | Punkty montowania dysków zewnętrznych/tymczasowych |

**Ćwiczenie:**
```bash
ls -la /
man hier          # pełny, oficjalny opis FHS (warto przeczytać raz w całości)
```

> 💡 **Dlaczego to ważne dla Embedded Linux:** w systemach wbudowanych
> często sam budujesz rootfs (Buildroot/Yocto – Część 2 kursu) i musisz
> decydować, co trafia do `/etc`, co do `/usr`, a co powinno być w
> read-only partycji vs. zapisywalnej (`/var`, `/tmp` na tmpfs).

---

## 2. `/proc` – okno do jądra i procesów

`/proc` **nie jest prawdziwym systemem plików na dysku**. To wirtualny
filesystem generowany przez jądro "w locie" – gdy odczytujesz plik w
`/proc`, jądro w tym momencie tworzy jego zawartość z bieżącego stanu
pamięci/struktur wewnętrznych. Nic tam fizycznie nie jest zapisane na
nośniku.

```bash
cat /proc/cpuinfo
```
Pokazuje szczegóły procesora/rdzeni: model, liczbę rdzeni, flagi
(`flags`), częstotliwość. Na ARM zobaczysz inne pola niż na x86
(np. `BogoMIPS`, `Features` zamiast `flags`).

```bash
cat /proc/meminfo
```
Aktualny stan pamięci RAM: ile wolnej, ile w cache, ile w swapie.
Przydatne przy debugowaniu wycieków pamięci na urządzeniu wbudowanym
o ograniczonym RAM.

```bash
cat /proc/uptime
```
Dwie liczby: czas działania systemu od bootu (sekundy) i suma czasu
bezczynności wszystkich rdzeni.

```bash
ls /proc/$$
```
`$$` to zmienna specjalna bash zawierająca PID bieżącego procesu
(czyli samego shella). W `/proc/<PID>/` znajdziesz katalog per-proces
z informacjami jądra o tym konkretnym procesie.

```bash
cat /proc/$$/status
```
Czytelny status procesu: stan (Running/Sleeping), PPID (rodzic), użycie
pamięci (`VmRSS`), liczba wątków, maski sygnałów.

**Dodatkowe, praktyczne pliki w `/proc` (embedded engineer używa ich często):**

```bash
cat /proc/interrupts   # licznik przerwań sprzętowych per-CPU per-urządzenie
                        # - świetne do sprawdzenia czy dany sterownik/IRQ w ogóle "żyje"
cat /proc/modules       # lista aktualnie załadowanych modułów jądra (to samo co lsmod)
cat /proc/mounts        # aktualnie zamontowane systemy plików (żywy odpowiednik /etc/fstab)
cat /proc/version       # wersja jądra, kompilator którym zbudowano, data builda
cat /proc/cmdline       # parametry przekazane jądru przez bootloader (np. root=, console=)
```

**✅ Zadanie 1:**
1. Podaj wynik `cat /proc/cmdline` na BeagleBone – jaki `console=` i
   `root=` zostały przekazane jądru?
2. Znajdź w `/proc/interrupts` linię odpowiadającą UART-owi (szukaj
   `uart` lub `serial`) – ile przerwań się nazbierało?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## 3. `/sys` – model urządzeń jądra (sysfs)

`/sys` to również wirtualny filesystem, ale o innym przeznaczeniu niż
`/proc`. Podczas gdy `/proc` skupia się na procesach i ogólnym stanie
jądra, `/sys` eksportuje **strukturalny model wszystkich urządzeń,
sterowników i magistral** rozpoznanych przez jądro (tzw. *kernel driver
model*). To jest miejsce, przez które współczesny Linux pozwala
sterować sprzętem z poziomu userspace bez pisania kodu C.

```bash
ls /sys/class/
```
Urządzenia pogrupowane funkcjonalnie, niezależnie od tego jaką
magistralą są podłączone – np. `net/` (interfejsy sieciowe), `gpio/`,
`leds/`, `tty/` (porty szeregowe), `pwm/`.

```bash
ls /sys/bus/
```
Urządzenia pogrupowane wg magistrali fizycznej: `i2c/`, `spi/`, `usb/`,
`platform/` (urządzenia "wlutowane na stałe", typowe dla SoC jak w
BeagleBone).

```bash
ls /sys/devices/
```
Surowe, fizyczne drzewo urządzeń tak jak widzi je jądro (topologia
sprzętu) – `/sys/class` i `/sys/bus` to w praktyce symlinki do wpisów
tutaj.

### Na BeagleBone: device tree i realne sterowanie sprzętem

```bash
ssh debian@192.168.7.2

ls /sys/firmware/devicetree/base/
cat /sys/firmware/devicetree/base/model
```
Device Tree to opis sprzętu (jakie układy scalone, na jakich pinach,
z jakimi parametrami) ładowany przez bootloader (U-Boot) i przekazywany
jądru przy starcie – zamiast "wklejać" te informacje na sztywno w kod
jądra jak robi się to np. na mikrokontrolerach. `/sys/firmware/devicetree/base/`
to żywy, aktualnie załadowany opis. Wrócimy do tego dokładniej w
Chapter 5 i w Części 2 kursu (Yocto/Buildroot).

```bash
ls /sys/class/leds/
cat /sys/class/leds/beaglebone:green:usr0/trigger
```
Diody USR0-USR3 na płytce BeagleBone są wystawione jako klasa `leds`.
Plik `trigger` pokazuje, co aktualnie steruje diodą (np. `heartbeat`,
`mmc0` = aktywność karty SD, albo `none` = sterowanie ręczne).

**Ręczne sterowanie diodą (bez linijki kodu w C!):**
```bash
echo none > /sys/class/leds/beaglebone:green:usr0/trigger
echo 1 > /sys/class/leds/beaglebone:green:usr0/brightness
echo 0 > /sys/class/leds/beaglebone:green:usr0/brightness
```
To pokazuje kluczową ideę Linuksa: sterownik w jądrze wystawia plik w
`/sys`, a zwykły `echo` (zapis do pliku) wywołuje w tle odpowiednią
funkcję sterownika. Dokładnie to samo dzieje się z GPIO, PWM,
podświetleniem ekranu itd.

**✅ Zadanie 2:**
1. Co zwróciło `cat /sys/firmware/devicetree/base/model` na BeagleBone?
2. Ile diod LED widzisz w `/sys/class/leds/` i jak się nazywają?
3. Zapal i zgaś diodę `usr0` poleceniami powyżej – zadziałało?
4. (Bonus) Sprawdź `cat /sys/class/leds/beaglebone:green:usr0/max_brightness`
   – co to oznacza w kontekście zakresu wartości do `brightness`?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie

Po tym rozdziale powinieneś:
- [ ] Znać podstawową strukturę FHS i wiedzieć gdzie szukać czego
- [ ] Rozumieć, że `/proc` i `/sys` to wirtualne (nie dyskowe) systemy plików
- [ ] Umieć odczytać stan jądra/procesu przez `/proc`
- [ ] Umieć znaleźć i sterować prostym urządzeniem przez `/sys`
- [ ] Rozumieć ideę Device Tree jako opisu sprzętu ładowanego przy boocie

**Następny rozdział:** [Chapter 2 – Czym jest jądro Linux i jak
współgra z otoczeniem](chapter2_kernel_basics.md)
