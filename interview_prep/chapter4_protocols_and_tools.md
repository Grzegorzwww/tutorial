# Chapter 4: Protokoły komunikacyjne i narzędzia

[← Spis treści](README.md)

To obszar, gdzie masz już realne doświadczenie (UART/SPI/I2C, MODBUS,
MQTT z Hymon, git/CMake/Jenkins z Somfy) – potraktuj te pytania jako
przygotowanie do **precyzyjnego** nazwania rzeczy, które już umiesz
robić w praktyce.

---

## UART / SPI / I2C

**P: Porównaj UART, SPI i I2C – liczba przewodów, prędkość, topologia.**

| | UART | SPI | I2C |
|---|---|---|---|
| Przewody | 2 (TX, RX) + GND | min. 4 (SCLK, MOSI, MISO, CS per urządzenie) | 2 (SDA, SCL) + GND |
| Zegar | Brak (asynchroniczny, wspólny baudrate) | Wspólny, generowany przez master | Wspólny, generowany przez master |
| Topologia | Punkt-punkt (1 do 1) | Master + wiele slave (osobny CS na każdy) | Master + wiele slave (adresowanie, wspólna magistrala) |
| Prędkość | Typowo do ~1-3 Mbps | Typowo dziesiątki Mbps | Typowo 100kHz-3.4MHz (zależnie od trybu) |
| Typowe użycie | Debug console, GPS, proste modemy | Karty SD, ekrany, czujniki wymagające dużej przepustowości | Czujniki (temperatura, akcelerometr), EEPROM, wiele urządzeń na jednej magistrali |

**P: Jak wykryć i debugować brak odpowiedzi (ACK) na magistrali I2C?**
- Sprawdzić adres urządzenia (typowa pomyłka: adres 7-bit vs. 8-bit z
  bitem R/W).
- Sprawdzić podciągnięcia (pull-up) na SDA/SCL – I2C wymaga
  rezystorów pull-up, linia jest open-drain.
- Logic analyzer / oscyloskop – sprawdzić czy w ogóle widać sygnał
  zegara i czy poziomy napięć są poprawne.
- Sprawdzić, czy urządzenie faktycznie odpowiada na tym adresie
  (skan I2C – `i2cdetect -y 1` na Linuksie).

**P: Co to jest "clock stretching" w I2C?**
- Slave może przytrzymać linię SCL w stanie niskim, sygnalizując
  masterowi "poczekaj, jeszcze nie jestem gotowy odpowiedzieć" – nie
  wszystkie implementacje mastera (zwłaszcza software'owe bit-banging)
  to poprawnie obsługują, co bywa źródłem trudnych do zdiagnozowania
  błędów komunikacji.

---

## MODBUS

**P: Czym różni się MODBUS RTU od MODBUS TCP?**
- MODBUS RTU – transmisja binarna po RS-485/RS-232, adresowanie
  urządzeń po ID slave, CRC do weryfikacji integralności ramki.
- MODBUS TCP – ten sam model danych (rejestry, coile) enkapsulowany w
  pakietach TCP/IP, bez potrzeby CRC (TCP już to zapewnia), zwykle
  port 502.

**P: Jakie są podstawowe typy danych/rejestrów w MODBUS?**
- Coils (1 bit, do odczytu/zapisu – np. sterowanie przekaźnikiem).
- Discrete Inputs (1 bit, tylko do odczytu – np. stan czujnika).
- Holding Registers (16 bit, odczyt/zapis – np. nastawy).
- Input Registers (16 bit, tylko do odczytu – np. odczyty z czujnika).

**P: Jak obsłużyć timeout i brak odpowiedzi od urządzenia slave w
MODBUS RTU?**
- Ustawić rozsądny timeout bazujący na baudrate i maksymalnym
  rozmiarze ramki (standard MODBUS definiuje minimalny czas ciszy
  między ramkami – 3.5 znaku).
- Retry z ograniczoną liczbą prób, potem oznaczenie urządzenia jako
  "offline" zamiast blokowania całej magistrali w nieskończoność.

---

## MQTT

**P: Czym różni się MQTT od zwykłego HTTP/REST przy komunikacji
urządzeń IoT?**
- MQTT: publish/subscribe (broker pośredniczy, urządzenie nie musi
  znać adresu odbiorcy), utrzymywane, długożyjące połączenie (mniejszy
  narzut niż powtarzane handshake'i HTTP), lekki nagłówek (dobre dla
  ograniczonej przepustowości/baterii), wbudowane QoS.
- HTTP/REST: model żądanie-odpowiedź, każde zapytanie to osobne
  połączenie (chyba że keep-alive), zwykle większy narzut nagłówków.

**P: Wyjaśnij poziomy QoS w MQTT (0, 1, 2).**
- QoS 0 – "at most once": wyślij i zapomnij, brak potwierdzenia, może
  się zgubić.
- QoS 1 – "at least once": potwierdzenie (PUBACK), ale wiadomość może
  dotrzeć **więcej niż raz** (retransmisja przy braku ACK) – odbiorca
  musi umieć obsłużyć duplikaty.
- QoS 2 – "exactly once": pełny handshake (4-etapowy), gwarantuje
  dokładnie jedno dostarczenie, ale najwyższy narzut/latencja.

**P: Co to jest "retained message" i "Last Will and Testament" (LWT) w MQTT?**
- Retained message – broker zapamiętuje ostatnią wiadomość na danym
  temacie i wysyła ją natychmiast każdemu nowemu subskrybentowi (np.
  ostatni znany stan urządzenia).
- LWT – wiadomość zdefiniowana przez klienta przy połączeniu, którą
  broker wyśle automatycznie w jego imieniu, jeśli klient rozłączy się
  "nieczysto" (bez normalnego DISCONNECT) – typowe zastosowanie:
  status "online"/"offline" urządzenia.

---

## USB

**P: Czym różni się USB HID od USB CDC (Communication Device Class)?**
- HID (Human Interface Device) – klasa dla klawiatur/myszy/joysticków,
  ale też ogólnego przesyłania małych pakietów danych bez potrzeby
  instalowania dedykowanego sterownika (host ma generyczny sterownik
  HID).
- CDC – klasa emulująca port szeregowy (widoczny w systemie jako
  `/dev/ttyACM*` lub `COMx`) – wygodne do prostej komunikacji, ale
  wymaga (na niektórych systemach) sterownika CDC.

**P: Co to jest enumeracja USB i jakie są jej główne etapy?**
- Proces, w którym host rozpoznaje nowo podłączone urządzenie: reset
  linii, odczyt deskryptorów urządzenia (VID/PID, klasa, konfiguracje),
  przydzielenie adresu (`SetAddress`), wybór konfiguracji, załadowanie
  odpowiedniego sterownika po stronie hosta. Błędy w tym procesie
  (np. `CHECK_ADDR`, "root port reset failed") zwykle wskazują na
  problem elektryczny (zasilanie VBUS, jakość sygnału D+/D-), nie
  software'owy.

---

## Git

**P: Czym różni się `git merge` od `git rebase`?**
- `merge` – tworzy nowy commit scalający (merge commit) z dwoma
  rodzicami, zachowuje pełną, faktyczną historię (łącznie z
  rozgałęzieniami).
- `rebase` – "przekłada" commity z jednej gałęzi na wierzchołek
  drugiej, tworząc **liniową** historię (ale zmienia hashe commitów –
  niebezpieczne na współdzielonych, już wypchniętych gałęziach).

**P: Co robi `git cherry-pick`?**
- Kopiuje pojedynczy, konkretny commit z jednej gałęzi na inną (bez
  scalania całej gałęzi) – przydatne np. do przeniesienia poprawki
  bugfixa na starszą gałąź release.

**P: Jak cofnąć ostatni commit, który już wypchnąłeś (`push`) na
współdzieloną gałąź?**
- `git revert <hash>` – tworzy **nowy** commit odwracający zmiany
  (bezpieczne dla współdzielonej historii, nie zmienia istniejących
  hashy).
- (`git reset --hard` + `push --force` zmienia historię i jest
  ryzykowne na współdzielonych gałęziach – wymaga koordynacji z
  zespołem.)

---

## CMake

**P: Czym różni się `target_link_libraries` z `PUBLIC`/`PRIVATE`/`INTERFACE`?**
- `PRIVATE` – zależność potrzebna tylko do zbudowania **tego** targetu,
  nie propaguje się do innych targetów, które linkują ten target.
- `PUBLIC` – zależność potrzebna zarówno do zbudowania tego targetu,
  jak i propagowana dalej (każdy, kto zlinkuje ten target, dostanie
  też tę zależność).
- `INTERFACE` – zależność niepotrzebna do zbudowania samego targetu,
  ale propagowana dalej (typowe dla bibliotek header-only).

**P: Co to jest "toolchain file" w CMake i po co jest potrzebny przy
cross-kompilacji?**
- Plik `.cmake` ustawiający kompilator (`CMAKE_C_COMPILER`,
  `CMAKE_CXX_COMPILER` wskazujące na cross-toolchain, np.
  `arm-none-eabi-gcc`), architekturę docelową, oraz ścieżki sysroot –
  pozwala zbudować projekt pod inną architekturę niż ta, na której
  uruchomiony jest CMake (np. budowanie pod ARM na maszynie x86).

---

## Debugowanie i narzędzia

**P: Jakie narzędzia użyłbyś do zdiagnozowania, że aplikacja na Linuksie
zawiesza się przy otwieraniu portu szeregowego?**
- `strace` – zobaczyć dokładnie na którym syscallu (`open`, `ioctl`)
  program się blokuje i z jakim `errno`.
- `lsof` – sprawdzić czy port jest już otwarty przez inny proces.
- `dmesg` – sprawdzić czy jądro zgłasza błędy sprzętowe/sterownika.

**P: Czym różni się debugowanie przez JTAG/SWD od debugowania przez
`gdb` na Linuksie (np. `gdbserver`)?**
- JTAG/SWD – debugowanie sprzętowe na poziomie procesora
  (mikrokontroler), pozwala na dostęp do rejestrów/pamięci nawet gdy
  program się zawiesił/nie wystartował, bez udziału systemu
  operacyjnego (bo go zwykle nie ma).
- `gdb`/`gdbserver` na Linuksie – debugowanie procesu w ramach
  działającego systemu operacyjnego, korzysta z syscalli
  (`ptrace()`) do przechwytywania i kontrolowania procesu.

**P: Co robi `valgrind` (memcheck) i jakie ma ograniczenia w embedded?**
- Wykrywa błędy pamięci (wycieki, użycie po zwolnieniu, odczyt
  niezainicjalizowanej pamięci) przez uruchomienie programu w
  emulowanym środowisku. Ograniczenie: wymaga pełnego systemu
  operacyjnego (Linux) i sporego narzutu wydajnościowego – nieprzydatny
  bezpośrednio na "gołym" mikrokontrolerze (choć można testować logikę
  na hoście/desktopie przed wgraniem na target).

---

## ✅ Szybki self-test

1. Wyjaśnij różnicę QoS 1 vs QoS 2 w MQTT i kiedy wybrałbyś które.
2. Opisz sytuację z Twojej pracy w Hymon, gdzie użyłeś MODBUS/MQTT –
   jakie napotkałeś problemy i jak je rozwiązałeś?
3. Czym różni się `git merge` od `git rebase` i kiedy użyłbyś którego?
4. Jak zdiagnozowałbyś, że komunikacja I2C z czujnikiem "czasami" się
   gubi (działa w 95% przypadków)?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje odpowiedzi)
```

---

**Następny rozdział:** [Chapter 5 – Zadania praktyczne](chapter5_practical_tasks.md)
