# Chapter 5: Kompilacja jądra

[← Spis treści](README.md)

**Cel rozdziału:** Poznać strukturę źródeł jądra Linux, proces jego
konfiguracji i budowania, na razie natywnie na Ubuntu (x86). To
przygotowanie pod Część 2 kursu, gdzie zrobimy to samo dla ARM
(BeagleBone) przez cross-kompilację, a docelowo w ramach Buildroot/Yocto.

---

## 1. Po co w ogóle kompilować jądro samodzielnie?

- **Włączenie/wyłączenie sterowników** – gotowe jądro z dystrybucji ma
  wkompilowane setki sterowników "na wszelki wypadek"; w embedded
  chcesz mieć tylko te potrzebne (mniejszy rozmiar, szybszy boot).
- **Wsparcie dla nietypowego sprzętu** – Twoja własna płytka z custom
  device tree, niestandardowy czujnik bez gotowego sterownika w
  dystrybucji.
- **Debugowanie/nauka** – możliwość dodania `printk()` w kodzie
  sterownika i zobaczenia efektu w `dmesg`.
- **Bezpieczeństwo/aktualizacje** – łatanie CVE zanim producent SoC
  wyda oficjalny obraz.

---

## 2. Struktura źródeł jądra

Po pobraniu źródeł (patrz sekcja 4) zobaczysz m.in.:

| Katalog | Zawartość |
|---|---|
| `arch/` | Kod specyficzny dla architektury: `arch/arm/`, `arch/x86/`, `arch/arm64/` |
| `drivers/` | Sterowniki urządzeń, pogrupowane wg typu: `drivers/usb/`, `drivers/net/`, `drivers/gpio/` |
| `fs/` | Implementacje systemów plików: `fs/ext4/`, `fs/vfat/`, `fs/proc/` |
| `net/` | Stos sieciowy |
| `kernel/` | Rdzeń: scheduler, zarządzanie procesami, sygnały |
| `mm/` | Zarządzanie pamięcią (memory management) |
| `include/` | Nagłówki współdzielone |
| `Documentation/` | Oficjalna dokumentacja (bardzo obszerna i wiarygodna) |
| `.config` | **Twoja** konfiguracja – co ma być wkompilowane/jako moduł/wyłączone |

---

## 3. Konfiguracja: `Kconfig` i `.config`

Każda opcja jądra (np. obsługa konkretnego sterownika) jest opisana w
plikach `Kconfig` rozsianych po drzewie źródeł. Efektem wyborów jest
płaski plik `.config` z liniami typu:
```
CONFIG_USB_SERIAL=m
CONFIG_USB_SERIAL_FTDI_SIO=m
CONFIG_EXT4_FS=y
# CONFIG_SOUND is not set
```
Trzy możliwe wartości:
- `y` – wkompilowane na stałe do obrazu jądra
- `m` – zbudowane jako moduł (`.ko`), ładowany później przez `modprobe`
- (brak / `is not set`) – funkcja wyłączona całkowicie

```bash
make menuconfig
```
Interaktywny, tekstowy interfejs (ncurses) do przeglądania i
zmieniania `.config` po kategoriach, z wbudowaną wyszukiwarką opcji
(`/`) i podpowiedziami (`help`) wyjaśniającymi co dana opcja robi.

```bash
make defconfig                  # domyślna konfiguracja dla bieżącej architektury
make ARCH=arm bcm2835_defconfig # przykład konfiguracji domyślnej dla konkretnej platformy
```

---

## 4. Pozyskanie źródeł jądra (na Ubuntu, x86)

```bash
uname -r
```
Wersja aktualnie działającego jądra – punkt odniesienia.

```bash
apt source linux-image-$(uname -r)
```
Pobiera źródła jądra dokładnie odpowiadające paczce zainstalowanej w
Twoim Ubuntu (wygodne, bo `.config` startowy będzie zgodny z tym co
faktycznie działa).

```bash
zcat /proc/config.gz > myconfig 2>/dev/null || echo "config.gz niedostępny"
```
Wiele dystrybucji (w tym Ubuntu) kompiluje jądro z opcją
`CONFIG_IKCONFIG_PROC`, która wystawia aktualnie używaną konfigurację
przez `/proc/config.gz` – wygodny sposób, by zobaczyć dokładnie z
czym zbudowane jest Twoje **działające teraz** jądro, bez pobierania
źródeł.

Alternatywnie, źródła "z pierwszej ręki" (nowsza wersja niż w
dystrybucji):
```bash
git clone --depth 1 --branch v6.6 https://github.com/torvalds/linux.git
```

---

## 5. Budowanie (natywnie, na Ubuntu x86 – tylko dla ćwiczenia procesu)

```bash
cd linux/
make olddefconfig      # dopasuj istniejący .config do nowszej wersji źródeł (odpowiada na nowe pytania domyślnie)
make -j$(nproc)        # właściwa kompilacja, równolegle na wszystkich rdzeniach
```
`$(nproc)` zwraca liczbę dostępnych wątków procesora – budowanie jądra
to setki tysięcy plików `.c`, więc zrównoleglenie robi ogromną różnicę
w czasie budowania.

```bash
make modules_install    # zainstaluj zbudowane moduły do /lib/modules/<wersja>/
sudo make install       # zainstaluj obraz jądra do /boot/, zaktualizuj bootloader (GRUB)
```

> ⚠️ Ten krok na Ubuntu podmienia realnie działające jądro systemu –
> na maszynie deweloperskiej rób to ostrożnie (najlepiej w maszynie
> wirtualnej), żeby nie zablokować sobie bootowania.

---

## 6. Zapowiedź: budowanie pod ARM (BeagleBone) – cross-kompilacja

Tego nie robimy jeszcze w tym rozdziale (to zadanie Części 2 kursu z
Buildroot/Yocto), ale warto znać nazwy, bo pojawią się wielokrotnie:

```bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make bb.org_defconfig     # konfiguracja domyślna dla rodziny BeagleBone
make -j$(nproc) uImage dtbs modules
```
- `ARCH=arm` – mówi systemowi budowania, że celujemy w inną
  architekturę niż ta, na której kompilujemy (Ubuntu = x86_64).
- `CROSS_COMPILE=arm-linux-gnueabihf-` – prefiks **cross-toolchaina**:
  zamiast zwykłego `gcc`, użyty zostanie `arm-linux-gnueabihf-gcc`,
  który generuje kod maszynowy ARM, nie x86.
- `uImage` – format obrazu jądra rozumiany przez U-Boot.
- `dtbs` – kompilacja plików Device Tree (`.dts` → `.dtb`) opisujących
  sprzęt konkretnej płytki (patrz Chapter 1 i 2).

---

## ✅ Zadania

1. Pobierz `.config` swojego działającego jądra (`/proc/config.gz`
   lub `apt source`) i wypisz 3 opcje związane z obsługą USB
   (szukaj `CONFIG_USB_*`) wraz z ich wartością (`y`/`m`).
2. Uruchom `make menuconfig` (na pobranych źródłach) i znajdź opcję
   odpowiadającą za wsparcie systemu plików `ext4`. Jaka jest jej
   aktualna wartość?
3. Wyjaśnij własnymi słowami różnicę między `y` a `m` w `.config` –
   kiedy wybrałbyś jedno, a kiedy drugie, budując jądro pod urządzenie
   embedded z ograniczoną pamięcią flash?
4. (Bonus, koncepcyjnie) Dlaczego do budowania jądra dla BeagleBone
   (ARM) na Ubuntu (x86_64) potrzebny jest osobny **cross-toolchain**,
   a nie zwykły `gcc`?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie Części 1 kursu

Po ukończeniu rozdziałów 1-5 powinieneś:
- [ ] Rozumieć strukturę FHS i wiedzieć gdzie szukać czego w systemie plików
- [ ] Umieć czytać stan jądra/sprzętu przez `/proc` i `/sys`
- [ ] Rozumieć podział kernel space / user space oraz rolę syscalli
- [ ] Rozumieć proces bootowania: ROM → U-Boot → jądro → systemd
- [ ] Umieć zarządzać procesami i sygnałami, w tym obsługiwać je w skryptach (`trap`)
- [ ] Rozumieć model uprawnień plików oraz rolę `udev` w zarządzaniu `/dev`
- [ ] Wiedzieć jak pozyskać, skonfigurować i zbudować jądro Linux

**Następny etap:** Część 2 – Systemy budowania obrazów (Buildroot,
Yocto Project) i cross-kompilacja pod ARM (BeagleBone). Zostanie
dodana po ukończeniu i omówieniu Części 1.
