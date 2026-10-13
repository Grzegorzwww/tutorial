# Chapter 2: Embedded-specific

[← Spis treści](README.md)

---

## Dostęp do sprzętu, rejestry, `volatile`

**P: Dlaczego dostęp do rejestrów sprzętowych musi być przez `volatile`?**
```cpp
#define STATUS_REG (*(volatile uint32_t*)0x40001000)

while (!(STATUS_REG & 0x01)) { }   // czekaj aż bit gotowości się ustawi
```
Bez `volatile` kompilator (widząc, że pętla "nic nie robi" z jego
punktu widzenia) mógłby odczytać wartość rejestru **raz** i
zoptymalizować pętlę do nieskończonego `while(true)` (bo w kodzie nic
nie modyfikuje tej zmiennej z perspektywy przepływu programu). `volatile`
wymusza odczyt z pamięci przy **każdym** dostępie.

**P: Co to jest memory-mapped I/O?**
- Sposób adresowania peryferiów (UART, GPIO, timery) tak, jakby były
  zwykłą pamięcią – zapis/odczyt pod konkretnym adresem fizycznym
  bezpośrednio steruje sprzętem, zamiast osobnych instrukcji `IN`/`OUT`
  (jak np. w x86). Typowe dla ARM Cortex-M/A.

**P: Jak zapewnić, że dostęp do współdzielonej struktury rejestrów jest
poprawny mimo optymalizacji kompilatora i reorderingu?**
- `volatile` na poziomie zmiennej/struktury (zapobiega optymalizacji
  odczytu/zapisu).
- Bariery pamięci (memory barriers, np. `__DMB()`, `__DSB()` na ARM)
  gdy kolejność zapisów do wielu peryferiów ma znaczenie względem
  siebie nawzajem, nie tylko względem kompilatora.
- Na wielordzeniowych SoC (Embedded Linux): `std::atomic` z
  odpowiednim `memory_order` dla zmiennych współdzielonych między
  rdzeniami/wątkami.

---

## Przerwania (Interrupts / ISR)

**P: Co powinno, a czego nie powinno znaleźć się w Interrupt Service
Routine (ISR)?**
- Powinno: minimalna, szybka reakcja – odczyt/zapis rejestru,
  ustawienie flagi, zapis do kolejki/bufora, ewentualnie obudzenie
  zadania (RTOS).
- Nie powinno: długie obliczenia, alokacja pamięci (`malloc`/`new`),
  blokujące operacje (czekanie na mutex, `printf` do wolnego UART),
  rekurencja – wszystko co wydłuża czas z zablokowanymi przerwaniami
  i psuje responsywność systemu (możliwe pominięcie kolejnego
  przerwania albo przeterminowanie innych zadań real-time).

**P: Co to jest "priority inversion" w kontekście przerwań/RTOS?**
- Sytuacja, gdy zadanie o wysokim priorytecie czeka na zasób
  (mutex) zajęty przez zadanie o niskim priorytecie, a zadanie o
  priorytecie pośrednim (niepotrzebujące tego zasobu) wywłaszcza
  zadanie niskopriorytetowe – efektywnie blokując zadanie
  wysokopriorytetowe na czas nieokreślony. Rozwiązanie: priority
  inheritance/priority ceiling (wspierane np. przez mutexy w FreeRTOS/
  większości RTOS-ów).

**P: Czym różni się przerwanie (interrupt) od pułapki/wyjątku (trap/exception)?**
- Przerwanie – asynchroniczne, generowane przez sprzęt zewnętrzny
  (timer, UART, GPIO), niezwiązane bezpośrednio z aktualnie wykonywaną
  instrukcją.
- Pułapka/wyjątek procesora – synchroniczne, wynik wykonania
  konkretnej instrukcji (np. dzielenie przez zero, dostęp do
  niedozwolonej pamięci – Hard Fault na Cortex-M, `SIGSEGV` na Linux).

**P: Co to jest "debounce" i dlaczego jest potrzebny przy obsłudze
przycisku przez przerwanie GPIO?**
- Mechaniczny przycisk przy naciśnięciu/zwolnieniu generuje wiele
  szybkich, fałszywych przełączeń stanu (drgania styków) zanim
  ustabilizuje się na docelowym poziomie – bez debounce, jedno
  fizyczne naciśnięcie może wygenerować kilka/kilkanaście przerwań.
  Rozwiązanie: opóźnienie programowe (ignoruj kolejne zbocza przez
  X ms) albo filtr RC sprzętowy.

---

## Pamięć na mikrokontrolerze, linker script, bootloader

**P: Co znajduje się w typowym linker script (`.ld`) mikrokontrolera?**
- Definicje regionów pamięci (`FLASH`, `RAM`) z adresami i rozmiarami.
- Sekcje: `.text` (kod, trafia do Flash), `.data` (zainicjalizowane
  zmienne globalne – wartość początkowa w Flash, kopiowana do RAM przy
  starcie), `.bss` (niezainicjalizowane/zerowe zmienne globalne –
  tylko rezerwacja w RAM, zerowana przy starcie), `.rodata` (stałe
  tylko do odczytu), stos i sterta.

**P: Co robi kod startowy (`startup.s`/`Reset_Handler`) zanim wywoła
`main()`?**
- Ustawia wskaźnik stosu (SP) na wartość z wektora przerwań.
- Kopiuje sekcję `.data` z Flash do RAM (wartości początkowe zmiennych
  globalnych).
- Zeruje sekcję `.bss` (zmienne globalne bez jawnej inicjalizacji).
- Wywołuje konstruktory obiektów globalnych C++ (`__libc_init_array`
  lub odpowiednik).
- Dopiero potem skacze do `main()`.

**P: Jaka jest rola bootloadera na mikrokontrolerze?**
- Mały program uruchamiany jako pierwszy po resecie, którego zadaniem
  jest zweryfikowanie/zaktualizowanie właściwej aplikacji (np. przez
  UART/USB/sieć – firmware update) i przekazanie sterowania do
  aplikacji głównej (skok pod odpowiedni adres Flash + ustawienie
  wskaźnika stosu z wektora przerwań aplikacji).

**P: Co to jest watchdog timer i jak się go poprawnie używa?**
- Sprzętowy licznik, który resetuje mikrokontroler, jeśli nie zostanie
  regularnie "nakarmiony" (`feed`/`kick`) przez oprogramowanie w
  określonym czasie – zabezpieczenie przed zawieszeniem się programu
  (np. pętla nieskończona, deadlock). Karmienie watchdoga **nie
  powinno** odbywać się "ślepo" w jednym miejscu (np. w przerwaniu
  timera niezależnie od stanu reszty systemu) – bo wtedy watchdog
  traci sens (nie wykryje zawieszenia głównej pętli).

---

## Stos, sterta i determinizm w embedded

**P: Dlaczego w wielu projektach embedded unika się dynamicznej
alokacji (`malloc`/`new`) po starcie systemu?**
- Fragmentacja sterty przy długim czasie działania (urządzenie
  pracujące miesiącami) może doprowadzić do sytuacji, gdzie mimo
  wystarczającej **sumarycznej** wolnej pamięci, nie ma **ciągłego**
  bloku odpowiedniego rozmiaru – alokacja zawodzi w nieprzewidywalnym
  momencie.
- Czas wykonania `malloc`/`free` jest niedeterministyczny (zależny od
  stanu alokatora) – problematyczne w systemach twardego real-time.
- Typowa praktyka: cała potrzebna pamięć alokowana raz, na starcie
  (statyczne bufory, pule obiektów), zero alokacji w pętli głównej.

**P: Jak wykryć przepełnienie stosu (stack overflow) na mikrokontrolerze?**
- Wypełnienie stosu wzorcem (np. `0xCC`) przy starcie i okresowe
  sprawdzanie "wysokiego wodostanu" (stack watermark) – ile bajtów
  wzorca zostało nadpisanych.
- MPU (Memory Protection Unit) skonfigurowany tak, by region tuż za
  stosem generował Fault przy zapisie.
- W RTOS (FreeRTOS): wbudowana funkcja `uxTaskGetStackHighWaterMark()`.

---

## RTOS – podstawowe koncepcje

**P: Czym różni się system z RTOS od "super pętli" (superloop, bare-metal)?**
- Superloop: jedna nieskończona pętla `while(1)` w `main()`, przerwania
  tylko ustawiają flagi, cała logika sekwencyjna – prostsze, ale
  trudne do skalowania przy wielu niezależnych, czasowo-krytycznych
  zadaniach.
- RTOS: wiele niezależnych zadań (tasks) z przydzielonymi
  priorytetami, wywłaszczany scheduler decyduje który task działa w
  danym momencie, wbudowane mechanizmy synchronizacji (mutex,
  semafor, kolejka) i pomiaru czasu – lepsze przy złożonych systemach
  z wieloma współbieżnymi wymaganiami czasowymi.

**P: Co to jest semafor binarny, a co mutex – czym się różnią mimo
pozornego podobieństwa?**
- Semafor binarny: mechanizm sygnalizacji zdarzenia (np. z ISR do
  taska: "dane gotowe") – może być zwolniony przez inny kontekst niż
  ten, który go zabrał; brak koncepcji "właściciela".
- Mutex: mechanizm wzajemnego wykluczenia dostępu do zasobu – **musi**
  być zwolniony przez ten sam task, który go zabrał; typowo wspiera
  priority inheritance (patrz wyżej), semafor zwykle nie.

---

## ✅ Szybki self-test

1. Wyjaśnij, dlaczego kod w ISR nie powinien wywoływać `malloc()`.
2. Co się stanie, jeśli zapomnisz o `volatile` przy zmiennej flagi
   ustawianej w ISR i odczytywanej w pętli głównej, przy kompilacji
   z optymalizacją `-O2`?
3. Opisz krótko sekwencję zdarzeń od resetu mikrokontrolera do
   wejścia w `main()`.
4. Dlaczego "ślepe" karmienie watchdoga w przerwaniu timera jest
   złą praktyką?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje odpowiedzi)
```

---

**Następny rozdział:** [Chapter 3 – Linux i systemy operacyjne](chapter3_linux_os.md)
