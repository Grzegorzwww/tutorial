# Chapter 3: Linux i systemy operacyjne

[← Spis treści](README.md)

---

## Procesy, wątki, kernel vs user space

**P: Czym różni się proces od wątku?**
- Proces ma **własną** przestrzeń adresową (pamięć) – procesy są od
  siebie izolowane, komunikacja wymaga jawnych mechanizmów IPC.
- Wątki (w ramach jednego procesu) **współdzielą** przestrzeń
  adresową (pamięć, deskryptory plików) – łatwiejsza komunikacja
  (współdzielone zmienne), ale wymaga jawnej synchronizacji (mutex,
  bo inaczej wyścig danych/data race).
- Koszt: utworzenie procesu (`fork()`) jest droższe niż utworzenie
  wątku (`pthread_create`), bo trzeba skopiować/przygotować całą nową
  przestrzeń adresową (choć `fork()` używa copy-on-write, więc nie
  jest to aż tak drogie jak mogłoby się wydawać).

**P: Wyjaśnij różnicę między kernel space a user space.**
- (Pełne wyjaśnienie z przykładami: [tutorial/chapter2_kernel_basics.md](../tutorial/chapter2_kernel_basics.md))
- Krótko: kernel space ma pełny dostęp do sprzętu i pamięci wszystkich
  procesów, user space jest izolowany i musi prosić jądro o wszystko
  przez syscalle. Rozdzielenie chroni system przed błędami w
  aplikacjach (crash aplikacji ≠ crash całego systemu).

**P: Co to jest syscall i podaj 3 przykłady.**
- Ściśle zdefiniowane wejście do jądra z user space: `open()`,
  `read()`/`write()`, `fork()`, `execve()`, `ioctl()`, `mmap()`,
  `socket()`. (Szczegóły: [tutorial/chapter2_kernel_basics.md](../tutorial/chapter2_kernel_basics.md))

**P: Co to jest proces zombie i jak go "posprzątać"?**
- Proces, który się zakończył, ale jego kod wyjścia nie został jeszcze
  odczytany przez rodzica (`wait()`/`waitpid()`). Zajmuje tylko wpis w
  tabeli procesów. Sprzątanie: rodzic musi wywołać `wait()` (samemu
  albo mieć poprawny handler `SIGCHLD`). Jeśli rodzic się zakończy bez
  tego, zombie jest "adoptowany" przez `init`/`systemd` (PID 1), który
  go sprząta.

**P: Co się dzieje przy `fork()` – co dziedziczy proces potomny?**
- Kopia (logicznie) całej przestrzeni adresowej rodzica
  (copy-on-write – fizyczna kopia strony pamięci następuje dopiero
  przy zapisie), kopie otwartych deskryptorów plików (wskazujące na
  te same pliki/sockety), ale **własny, unikalny PID**. Zwraca 0 w
  procesie potomnym, PID dziecka w rodzicu, -1 przy błędzie.

---

## Sygnały i IPC

**P: Wymień kilka mechanizmów IPC (Inter-Process Communication) w Linuksie.**
- **Pipes** (`|` w shellu, `pipe()` w kodzie) – jednokierunkowy
  strumień bajtów między powiązanymi procesami (np. rodzic-dziecko).
- **Named pipes / FIFO** – jak pipe, ale ma nazwę w systemie plików,
  mogą go używać niepowiązane procesy.
- **Sockets** (w tym Unix domain sockets) – dwukierunkowa
  komunikacja, także między procesami na różnych maszynach (TCP/UDP).
- **Shared memory** (`shmget`/`mmap` z `MAP_SHARED`) – najszybszy
  mechanizm (brak kopiowania danych przez jądro), ale wymaga własnej
  synchronizacji (semafor/mutex w pamięci współdzielonej).
- **Sygnały** – proste, asynchroniczne powiadomienia (patrz Chapter 3
  kursu Linux: [tutorial/chapter3_processes_signals.md](../tutorial/chapter3_processes_signals.md)).
- **Message queues** – kolejki komunikatów zarządzane przez jądro.

**P: Czym różni się `SIGTERM` od `SIGKILL` – dlaczego oba istnieją?**
- (Szczegóły: [tutorial/chapter3_processes_signals.md](../tutorial/chapter3_processes_signals.md))
- Krótko: `SIGTERM` to prośba, którą proces może obsłużyć/zignorować
  (posprzątać zasoby przed zamknięciem); `SIGKILL` jest realizowany
  bezpośrednio przez jądro, proces nie ma nad nim kontroli – ostatnia
  deska ratunku, gdy proces się zawiesił.

---

## Boot process i systemd

**P: Opisz krótko proces bootowania systemu Linux na urządzeniu
embedded (np. na SoC typu BeagleBone/Raspberry Pi).**
- (Pełny opis z diagramem: [tutorial/chapter2_kernel_basics.md](../tutorial/chapter2_kernel_basics.md))
- Krótko: ROM bootloader (w krzemie SoC) → bootloader drugiego stopnia
  (U-Boot) → ładowanie jądra + Device Tree do RAM → jądro startuje,
  montuje rootfs → uruchomienie PID 1 (`systemd` lub `init`) → start
  usług.

**P: Co to jest Device Tree i po co jest potrzebny?**
- Opis sprzętu (jakie układy, na jakich pinach/adresach, z jakimi
  parametrami) w formacie tekstowym (`.dts`) kompilowanym do binarnego
  `.dtb`, ładowany przez bootloader razem z jądrem. Pozwala na
  **jedno** jądro obsługujące wiele różnych płytek z tym samym SoC –
  różnica jest tylko w pliku `.dtb`, bez rekompilacji jądra.

**P: Co robi `systemd` i czym różni się od starszego SysV init?**
- `systemd` to PID 1 – pierwszy proces user space, zarządza
  uruchamianiem/zatrzymywaniem usług (unit files), obsługuje
  zależności między usługami, uruchamia je równolegle (szybszy boot
  niż sekwencyjne skrypty SysV), oferuje `journald` (scentralizowane
  logowanie), zarządzanie cgroups.

---

## Sterowniki i przestrzeń urządzeń

**P: Czym różni się character device od block device?**
- (Szczegóły: [tutorial/chapter4_permissions_udev.md](../tutorial/chapter4_permissions_udev.md))
- Character device – strumień bajtów bez buforowania blokowego (UART,
  I2C, GPIO). Block device – dane w blokach o stałym rozmiarze, z
  cache (dyski, karty SD).

**P: Co robi `udev` i dlaczego jest potrzebny zamiast statycznych
plików w `/dev`?**
- Nasłuchuje zdarzeń z jądra (uevents) przy podłączaniu/odłączaniu
  sprzętu i dynamicznie tworzy/usuwa pliki w `/dev`, nadaje im
  uprawnienia wg reguł – pozwala na hotplug (USB) bez ręcznego
  tworzenia węzłów urządzeń na sztywno.

**P: Jak w Linuksie steruje się prostym urządzeniem (np. diodą LED)
bez pisania sterownika w C?**
```bash
echo 1 > /sys/class/leds/mydevice/brightness
```
Sterownik jądra wystawia interfejs przez `sysfs` – zapis do pliku
wywołuje odpowiednią funkcję sterownika. (Szczegóły:
[tutorial/chapter1_filesystem_proc_sys.md](../tutorial/chapter1_filesystem_proc_sys.md))

**P: Co to jest moduł jądra i czym różni się od kodu wkompilowanego
na stałe?**
- Moduł (`.ko`) to fragment kodu jądra dociągany dynamicznie
  (`insmod`/`modprobe`), bez potrzeby przekompilowania całego jądra
  ani restartu systemu. Kod wkompilowany na stałe (`y` w `.config`)
  jest zawsze obecny w obrazie jądra od momentu bootu.

---

## Pamięć i zarządzanie zasobami w Linuksie

**P: Co to jest pamięć wirtualna i po co jest potrzebna?**
- Każdy proces widzi własną, ciągłą przestrzeń adresową (np.
  0x0000...0xFFFF na 32-bit), niezależną od fizycznego układu RAM.
  Jądro (przy pomocy MMU) tłumaczy adresy wirtualne na fizyczne –
  pozwala to izolować procesy od siebie (proces A nie może
  bezpośrednio odczytać pamięci procesu B) oraz używać więcej pamięci
  "logicznej" niż fizycznie dostępna (swap).

**P: Czym różni się `mmap()` od zwykłego `read()`/`write()` na pliku?**
- `mmap()` mapuje plik bezpośrednio w przestrzeń adresową procesu –
  dostęp do danych pliku wygląda jak dostęp do zwykłej pamięci
  (`buf[i]`), bez jawnego kopiowania przez bufory `read()`/`write()`.
  Przydatne przy dużych plikach/danych współdzielonych między
  procesami, albo przy dostępie do pamięci urządzenia (np. `/dev/mem`
  w embedded do bezpośredniego mapowania rejestrów peryferiów z
  poziomu user space – choć to podejście ma ograniczenia bezpieczeństwa).

---

## ✅ Szybki self-test

1. Wyjaśnij różnicę między procesem a wątkiem – kiedy wybrałbyś jedno,
   a kiedy drugie w projekcie?
2. Co to jest proces zombie i jak mu zapobiec?
3. Opisz jedną sytuację z Twojej pracy (np. w Somfy), gdzie
   wykorzystałeś/mógłbyś wykorzystać IPC albo sygnały.
4. Dlaczego jedno jądro Linux może działać na wielu różnych płytkach
   z tym samym SoC bez rekompilacji?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje odpowiedzi)
```

---

**Następny rozdział:** [Chapter 4 – Protokoły komunikacyjne i narzędzia](chapter4_protocols_and_tools.md)
