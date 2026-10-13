# Chapter 4: Użytkownicy, uprawnienia i udev

[← Spis treści](README.md)

**Cel rozdziału:** Zrozumieć model uprawnień w Linuksie oraz mechanizm
`udev`, który dynamicznie tworzy i konfiguruje pliki urządzeń w `/dev`
w momencie podłączenia/odłączenia sprzętu. To wiedza wprost
wykorzystywana przy podłączaniu czujników, adapterów USB-serial i
innego sprzętu do urządzenia embedded.

---

## 1. Użytkownicy i grupy

```bash
id
```
Pokazuje Twój UID (numeryczny identyfikator użytkownika), GID (grupa
główna) oraz listę wszystkich grup, do których należysz.

```bash
groups
whoami
```
`groups` – sama lista grup; `whoami` – nazwa bieżącego użytkownika.

Konfiguracja użytkowników/grup mieszka w zwykłych plikach tekstowych:
```bash
cat /etc/passwd   # lista użytkowników: nazwa:x:UID:GID:opis:home:shell
cat /etc/group    # lista grup i ich członków
```

> 💡 Root (UID 0) omija większość sprawdzeń uprawnień – to konto
> "administratora". W embedded Linux często pracuje się jako root
> (proste obrazy Buildroot), ale w produkcyjnych systemach dąży się do
> uruchamiania usług jako zwykły użytkownik z minimalnym zestawem grup.

---

## 2. Uprawnienia do plików

```bash
ls -l /etc/hostname
```
```
-rw-r--r-- 1 root root 12 Sep 10 10:00 /etc/hostname
```

Rozbicie pierwszej kolumny `-rw-r--r--`:
- Znak 1: typ pliku (`-` zwykły, `d` katalog, `l` symlink, `c` urządzenie znakowe, `b` urządzenie blokowe)
- Znaki 2-4: uprawnienia **właściciela** (`rw-` = read, write, brak execute)
- Znaki 5-7: uprawnienia **grupy** (`r--` = tylko read)
- Znaki 8-10: uprawnienia **pozostałych** (`r--` = tylko read)

```bash
chmod 644 plik.txt     # rw-r--r-- : właściciel rw, grupa r, inni r
chmod 755 skrypt.sh    # rwxr-xr-x : właściciel rwx, grupa i inni rx
chmod +x skrypt.sh     # dodaj prawo wykonywania wszystkim (skrót zamiast liczenia oktalnie)
```
Notacja oktalna: każda cyfra to suma `r=4, w=2, x=1` dla kolejno
właściciela/grupy/innych. `644` = właściciel rw(4+2=6), grupa r(4),
inni r(4).

```bash
chown alice plik.txt          # zmień właściciela na alice
chown alice:developers plik.txt  # zmień właściciela i grupę jednocześnie
```

```bash
umask
```
Maska domyślnych uprawnień odejmowana przy tworzeniu nowych plików
(np. `umask 022` sprawia, że nowe pliki dostają `644`, a nie `666`).

### Uprawnienia specjalne (krótko)

- **setuid** (`chmod u+s`) – program wykonuje się z uprawnieniami
  właściciela pliku, nie użytkownika który go uruchomił (klasyczny
  przykład: `/usr/bin/passwd` działa jako root, żeby móc zmienić
  `/etc/shadow`).
- **setgid** (`chmod g+s`) – analogicznie dla grupy; na katalogu:
  nowe pliki dziedziczą grupę katalogu.
- **sticky bit** (`chmod +t`) – na katalogu (np. `/tmp`) oznacza, że
  tylko właściciel pliku może go usunąć, mimo że katalog jest
  zapisywalny dla wszystkich.

---

## 3. Pliki urządzeń w `/dev`

```bash
ls -l /dev/ | head -30
```

Zwróć uwagę na pierwszą literę i dwie liczby zamiast rozmiaru pliku:
```
crw-rw---- 1 root dialout 188, 0 Sep 10 10:00 /dev/ttyUSB0
brw-rw---- 1 root disk     8, 0 Sep 10 10:00 /dev/sda
```
- `c` = **character device** (urządzenie znakowe) – dane czytane/pisane
  strumieniowo, bajt po bajcie, bez buforowania blokowego. Typowe dla
  UART, I2C, GPIO.
- `b` = **block device** (urządzenie blokowe) – dane w blokach o
  stałym rozmiarze, z buforowaniem/cache. Typowe dla dysków, kart SD.
- Dwie liczby (`188, 0`) to **major i minor number** – major identyfikuje
  sterownik obsługujący to urządzenie, minor rozróżnia konkretne
  instancje (np. `ttyUSB0`, `ttyUSB1` mają ten sam major, różny minor).

> 💡 To jest właśnie "spoiwo" między sterownikiem w jądrze a plikiem w
> `/dev` – gdy robisz `open("/dev/ttyUSB0")`, jądro po (major, minor)
> znajduje właściwy sterownik i przekierowuje do niego wszystkie
> operacje `read`/`write`/`ioctl`.

---

## 4. `udev` – dynamiczne zarządzanie `/dev`

Dawniej pliki w `/dev` były tworzone statycznie, ręcznie, na zapas dla
każdego możliwego urządzenia. Dziś `udev` (usługa działająca w
userspace) nasłuchuje **uevents** wysyłanych przez jądro za każdym
razem, gdy sprzęt się pojawia/znika (hotplug), i dynamicznie
tworzy/usuwa odpowiednie pliki w `/dev`, nadaje im uprawnienia, a
opcjonalnie tworzy dodatkowe symlinki (np. stałą nazwę niezależną od
kolejności podłączania).

```bash
udevadm monitor
```
Zostaw uruchomione i podłącz dowolne urządzenie USB (pendrive, adapter
USB-serial) w drugim terminalu – zobaczysz na żywo zdarzenia `UEVENT`
(z jądra) i `EVENT` (przetworzone przez `udevd`).

```bash
udevadm info -a -n /dev/ttyUSB0
```
Pełne drzewo atrybutów urządzenia widziane przez `udev` – producenta,
model, ścieżkę w sysfs, numer seryjny. Bardzo przydatne do pisania
własnych reguł (żeby dopasować regułę tylko do *tego* urządzenia, nie
każdego USB-serial).

### Reguły udev

Reguły to pliki tekstowe `.rules` czytane w kolejności alfabetycznej z
kilku katalogów (późniejsze mogą nadpisywać wcześniejsze):
- `/lib/udev/rules.d/` – reguły dostarczone przez system/pakiety (nie
  edytuj tutaj – nadpiszą się przy aktualizacji)
- `/etc/udev/rules.d/` – reguły własne administratora (tu dodawaj swoje)

**Przykładowa reguła** – nadaj grupę `dialout` i uprawnienia `0660`
każdemu adapterowi FTDI USB-serial (typowy przypadek: żeby zwykły
użytkownik mógł otwierać port bez `sudo`):

```
# /etc/udev/rules.d/99-ftdi-serial.rules
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", GROUP="dialout", MODE="0660"
```

**Przykładowa reguła** – stała nazwa symlinku niezależna od kolejności
podłączenia (bardzo przydatne, gdy masz kilka adapterów USB-serial
podłączonych naraz do docelowego urządzenia embedded):

```
# /etc/udev/rules.d/98-my-device.rules
SUBSYSTEM=="tty", ATTRS{serial}=="AB0C1234", SYMLINK+="ttyMyDevice"
```
Po podłączeniu urządzenia z tym numerem seryjnym pojawi się dodatkowo
`/dev/ttyMyDevice` wskazujący na aktualny `/dev/ttyUSBx`.

**Po dodaniu/zmianie reguły:**
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger      # ponownie "odpal" reguły dla już podłączonych urządzeń
```

---

## ✅ Zadania

1. Sprawdź `ls -l /etc/shadow` – dlaczego zwykły użytkownik nie może
   go odczytać, mimo że root może? Jakie uprawnienia widzisz?
2. Podłącz dowolne urządzenie USB i sprawdź co pojawiło się w
   `udevadm monitor`. Zanotuj przynajmniej 2 linie z wyjścia.
3. Znajdź plik reguł udev odpowiedzialny za nadanie grupy `dialout`
   urządzeniom szeregowym w Twoim systemie (podpowiedź: przeszukaj
   `/lib/udev/rules.d/` i `/etc/udev/rules.d/` pod kątem `tty` lub
   `dialout`).
4. Napisz własną regułę udev nadającą Twojemu urządzeniu USB-serial
   (jeśli masz) uprawnienia `0666` i przetestuj ją (`reload-rules` +
   `trigger`, potem odłącz/podłącz urządzenie).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie

Po tym rozdziale powinieneś:
- [ ] Rozumieć model użytkownik/grupa/inni i notację `rwx`
- [ ] Umieć zmieniać uprawnienia i właściciela plików (`chmod`, `chown`)
- [ ] Rozumieć różnicę character vs. block device oraz rolę major/minor number
- [ ] Rozumieć jak `udev` dynamicznie zarządza `/dev` na podstawie uevents z jądra
- [ ] Umieć napisać i przetestować własną regułę udev

**Następny rozdział:** [Chapter 5 – Kompilacja jądra](chapter5_kernel_compilation.md)
