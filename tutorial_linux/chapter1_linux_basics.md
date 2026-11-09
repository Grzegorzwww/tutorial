# Embedded Linux Engineer – Tutorial

## Chapter 1: Linux jako system – filesystem, /proc, /sys, procesy, uprawnienia

**Cel tygodnia:** Zrozumieć jak Linux organizuje system plików, jak jądro
komunikuje się z userspace przez `/proc` i `/sys`, oraz jak zarządzać
procesami i uprawnieniami. Ćwiczenia wykonujemy równolegle na:
- **Ubuntu** (stacja robocza, x86)
- **BeagleBone Black** (Debian Bullseye, ARM, `ssh debian@192.168.7.2`)

---

### Dzień 1-2: Filesystem Hierarchy Standard (FHS) + `/proc` i `/sys`

**Teoria:** W Linuksie "wszystko jest plikiem". `/proc` to wirtualny
filesystem dający wgląd w stan jądra i procesów w czasie rzeczywistym.
`/sys` to model urządzeń jądra (kernel object / driver model) – tu
znajdziesz reprezentację sterowników, magistral i urządzeń.

**Struktura systemu:**
```bash
ls -la /
man hier          # opis FHS: /bin vs /usr/bin, /etc, /var, /opt, /lib
```

**`/proc` – informacje o jądrze i procesach:**
```bash
cat /proc/cpuinfo
cat /proc/meminfo
cat /proc/uptime
ls /proc/$$       # $$ = PID bieżącego shella - co jądro trzyma per-proces
cat /proc/$$/status
```

**`/sys` – model urządzeń jądra:**
```bash
ls /sys/class/
ls /sys/bus/
ls /sys/devices/
```

**Na BeagleBone (prawdziwy Embedded Linux – device tree i sprzęt):**
```bash
ssh debian@192.168.7.2

# Żywe drzewo urządzeń załadowane przez bootloader
ls /sys/firmware/devicetree/base/
cat /sys/firmware/devicetree/base/model

# Diody LED na płytce sterowane z userspace przez /sys
ls /sys/class/leds/
cat /sys/class/leds/beaglebone:green:usr0/trigger
```

**✅ Zadanie 1:**
1. Co zwróciło `cat /sys/firmware/devicetree/base/model` na BeagleBone?
2. Ile diod LED widzisz w `/sys/class/leds/` i jak się nazywają?
3. (Bonus) Spróbuj zapalić/zgasić diodę ręcznie:
   ```bash
   echo none > /sys/class/leds/beaglebone:green:usr0/trigger
   echo 1 > /sys/class/leds/beaglebone:green:usr0/brightness
   echo 0 > /sys/class/leds/beaglebone:green:usr0/brightness
   ```

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

### Dzień 3: Procesy i sygnały

```bash
ps aux
top            # albo htop jeśli zainstalowany
pstree

kill -l        # lista dostępnych sygnałów
```

**Eksperyment:**
```bash
sleep 100 &
jobs
ps aux | grep sleep     # znajdź PID
kill -SIGTERM <PID>     # sprawdź czy proces zniknął
sleep 100 &
kill -SIGSTOP <PID>     # zatrzymaj proces
kill -SIGCONT <PID>     # wznów proces
kill -SIGKILL <PID>     # zabij na twardo
```

**✅ Zadanie 2:** Opisz różnicę między `SIGTERM`, `SIGKILL` i `SIGSTOP` –
który proces może "złapać" (obsłużyć) i zignorować, a którego nie?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

### Dzień 4: Użytkownicy, uprawnienia, `udev`

```bash
id
groups
ls -l /dev/ | head -30

# udev - dynamiczne zarządzanie urządzeniami
udevadm info -a -n /dev/ttyUSB0   # jeśli masz jakiś adapter USB-serial
udevadm monitor                    # zostaw włączone, podłącz/odłącz USB - obserwuj zdarzenia
```

**✅ Zadanie 3:**
1. Podłącz dowolne urządzenie USB (pendrive, adapter USB-serial) i
   sprawdź co pojawiło się w wyjściu `udevadm monitor`.
2. Znajdź plik reguł udev odpowiedzialny za nadanie uprawnień do portu
   szeregowego (podpowiedź: `/etc/udev/rules.d/`, `/lib/udev/rules.d/`).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

### Dzień 5: Kompilacja jądra (wprowadzenie, na Ubuntu x86)

Na tym etapie chodzi o zrozumienie **procesu**, nie o docelową
architekturę ARM (to zrobimy w Etapie 2 z Yocto/Buildroot i
cross-kompilacją).

```bash
uname -r
zcat /proc/config.gz > myconfig 2>/dev/null || echo "config.gz niedostępny"

# Pobranie źródeł jądra dopasowanych do dystrybucji
apt source linux-image-$(uname -r)
```

**✅ Zadanie 4:** Znajdź plik `.config` swojego aktualnie działającego
jądra (lub pobierz domyślny) i wypisz 3 opcje konfiguracyjne związane
z obsługą USB (`CONFIG_USB_*`).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie Chapter 1

Po tym rozdziale powinieneś:
- [ ] Rozumieć strukturę FHS i wiedzieć gdzie szukać czego w systemie plików
- [ ] Umieć czytać stan jądra/sprzętu przez `/proc` i `/sys`
- [ ] Umieć sterować prostym urządzeniem (LED) z poziomu userspace przez `/sys`
- [ ] Rozumieć podstawowe zarządzanie procesami i sygnałami
- [ ] Rozumieć rolę `udev` w dynamicznym zarządzaniu urządzeniami
- [ ] Wiedzieć jak pozyskać i przejrzeć źródła jądra

**Następny rozdział:** Chapter 2 – Systemy budowania obrazów (Buildroot,
Yocto) i cross-kompilacja pod ARM (BeagleBone).
