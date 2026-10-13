# Chapter 6: Pytania sytuacyjne/behawioralne i jak mówić o CV

[← Spis treści](README.md)

Techniczna wiedza to jedno, ale rekruterzy (zwłaszcza HR/tech lead na
pierwszej rozmowie) równie często pytają o doświadczenie zespołowe,
sposób pracy i motywację. Tu przygotowanie "na sucho" robi ogromną
różnicę.

---

## Metoda STAR – jak strukturyzować odpowiedzi

Dla pytań typu "opowiedz o sytuacji, gdy..." używaj struktury:
- **S**ituation – krótki kontekst (jaki projekt, jaka firma/rola)
- **T**ask – jakie było konkretne zadanie/problem
- **A**ction – co **Ty** konkretnie zrobiłeś (nie "zespół", tylko Twój wkład)
- **R**esult – jaki był efekt, najlepiej z liczbą/konkretem

**Przykład zastosowania do Twojego CV (projekt CI/CD w Somfy):**
> "W ramach jednego z projektów w Somfy (**S**) potrzebowaliśmy
> zautomatyzować testy na platformie STM32 uruchamiane przy każdym
> commicie (**T**). Skonfigurowałem Jenkins do automatycznego
> budowania firmware i uruchamiania testów przez Python SDK na
> rzeczywistym sprzęcie podłączonym do agenta CI (**A**). Dzięki temu
> zespół wykrywał regresje w ciągu minut zamiast podczas ręcznych
> testów pod koniec sprintu (**R**)."

---

## Pytania o dotychczasowe doświadczenie (bazując na Twoim CV)

**P: Opowiedz o projekcie z Somfy – jaka była Twoja rola w "redesignie"
platformy?**
- Przygotuj konkretny przykład: jaki mikrokontroler (STM32 M3/M4), co
  dokładnie migrowałeś/rozwijałeś, jakie napotkałeś trudności
  techniczne (np. migracja peryferiów, kompatybilność wsteczna).

**P: Jak wyglądała Twoja praca przy projekcie CI/Testów w Somfy –
jakie były największe wyzwania w konfiguracji Jenkins/Python SDK?**
- Przygotuj: co dokładnie testowałeś (HW-in-the-loop? unit testy?),
  jak radziłeś sobie z niestabilnością testów na prawdziwym sprzęcie
  (częsty problem: flaky testy przez timing, zasilanie, USB).

**P: Opowiedz o projekcie freelance dla Hymon – jak wyglądała
architektura systemu (ESP32 + MQTT + MODBUS + baza danych)?**
- Przygotuj diagram myślowy: Heat Pump → MODBUS → ESP32 → MQTT →
  broker → zapis do bazy. Jakie decyzje architektoniczne podjąłeś
  (np. wybór brokera, format danych, obsługa utraty połączenia
  sieciowego)? Jakie problemy sprzętowe/programistyczne napotkałeś
  (Twoje notatki z tego projektu wspominają o problemach z USB
  host/zasilaniem – to świetny, konkretny przykład do opowiedzenia
  na pytanie "opowiedz o trudnym problemie, który rozwiązałeś").

**P: Pracowałeś zarówno z AVR jak i ARM (STM32) – jakie były
największe różnice w podejściu do programowania tych platform?**
- Przygotuj konkretne różnice: architektura (8-bit vs 32-bit),
  dostępność peryferiów, narzędzia (Atmel Studio/avr-gcc vs
  STM32CubeIDE/HAL), zarządzanie przerwaniami, dostępna pamięć.

**P: Dlaczego chcesz przejść z czystego Embedded C++ w stronę Embedded
Linux?**
- Bądź szczery i konkretny (patrz Twoje własne motywacje z tej
  rozmowy) – np. chęć rozwoju w stronę bardziej złożonych systemów,
  zainteresowanie warstwą systemową (kernel, drivers), naturalny
  kolejny krok po doświadczeniu z Linuksem na Qt/desktop i podstawach
  z BeagleBone.

---

## Pytania o pracę zespołową i proces

**P: Jak wygląda Twój typowy workflow z git w zespole (branching,
code review, CI)?**
- Przygotuj: strategia branchowania (feature branch, trunk-based?),
  jak wygląda pull request/code review w Twoim zespole, czy CI
  (Jenkins) blokuje merge przy nieudanych testach.

**P: Opowiedz o sytuacji, gdy nie zgadzałeś się z decyzją techniczną
kolegi/przełożonego – jak to rozwiązałeś?**
- Metoda STAR. Podkreśl: argumentację merytoryczną, otwartość na
  kompromis, brak eskalacji do konfliktu personalnego.

**P: Jak dokumentujesz swoją pracę? (Masz doświadczenie z Polarion)**
- Przygotuj konkretny przykład: jak wyglądał proces tworzenia
  dokumentacji protokołu/funkcji aplikacji w Polarion, po co to było
  potrzebne (śledzalność wymagań, audyt, compliance w branży
  przemysłowej/automatyki).

**P: Jak radzisz sobie z pracą w międzynarodowym, rozproszonym
zespole?** (masz to wprost w CV – Somfy, zespoły międzynarodowe)
- Przygotuj przykład komunikacji mimo różnic strefy czasowej/języka,
  ewentualnie jak wykorzystujesz język angielski/hiszpański w pracy.

---

## Pytania o motywację i oczekiwania

**P: Dlaczego akurat nasza firma/ta oferta?**
- Zrób research przed rozmową: czym zajmuje się firma, jaki mają
  produkt/stack technologiczny, dlaczego to pasuje do Twojej ścieżki
  (Embedded Linux).

**P: Jakie są Twoje oczekiwania finansowe?**
- Sprawdź realne widełki rynkowe dla Embedded Linux Engineer w
  Polsce/regionie przed rozmową (różnią się znacząco między
  lokalizacjami i modelem pracy zdalnej/hybrydowej). Podaj widełki, nie
  sztywną liczbę, z zaznaczeniem że jesteś elastyczny w zależności od
  całego pakietu (benefity, zdalna praca, rozwój).

**P: Gdzie widzisz siebie za 2-3 lata?**
- Połącz z planem rozwoju z tej rozmowy: pogłębienie Embedded Linux
  (Yocto/Buildroot, kernel drivers), być może rozwój w stronę
  architektury systemów czy roli bardziej senior/lead.

**P: Jakie masz pytania do nas?** (zawsze zadawaj pytania!)
- Przygotuj 3-4 pytania, np.:
  - "Jaki system budowania obrazu Linux używacie – Yocto, Buildroot,
    czy własne rozwiązanie?"
  - "Jak wygląda proces wdrażania aktualizacji OTA na urządzeniach w
    terenie?"
  - "Jaki jest typowy cykl rozwoju – ile czasu poświęca się na
    utrzymanie istniejącego kodu vs. nowe projekty?"
  - "Jak wygląda współpraca między zespołem embedded a zespołem
    tworzącym np. aplikację mobilną/chmurową (jeśli produkt to IoT)?"

---

## Częste błędy podczas rozmowy (czego unikać)

- Mówienie "zespół zrobił" zamiast "ja zrobiłem X, a zespół Y" –
  rekruter chce poznać **Twój** wkład.
- Zbyt ogólne odpowiedzi ("pracowałem z mikrokontrolerami") bez
  konkretów (jaki model, jaki protokół, jaki problem rozwiązałeś).
- Krytykowanie poprzednich pracodawców/kolegów.
- Brak pytań na koniec rozmowy – sugeruje brak zaangażowania.
- Nieprzygotowanie konkretnego przykładu trudnego bugu i tego, jak go
  zdiagnozowałeś (bardzo częste pytanie: "opowiedz o najtrudniejszym
  problemie, jaki rozwiązałeś" – przygotuj **jeden, dobrze przemyślany
  przykład** zawczasu, np. properly ten z ESP32 USB Host/VBUS z Twoich
  własnych notatek – to świetna historia: symptomy, systematyczna
  diagnoza warstwa po warstwie, hipoteza, weryfikacja, rozwiązanie).

---

## ✅ Zadanie przygotowawcze

Zanim pójdziesz na rozmowę, przygotuj **na piśmie** (nie tylko w
głowie) po jednym konkretnym przykładzie (metoda STAR) na każde z
poniższych:
1. Najtrudniejszy bug/problem techniczny, jaki rozwiązałeś.
2. Sytuacja, w której musiałeś się szybko nauczyć nowej technologii.
3. Konflikt/nieporozumienie w zespole i jak je rozwiązałeś.
4. Decyzja architektoniczna, za którą byłeś odpowiedzialny i dlaczego
   ją podjąłeś (np. wybór technologii w projekcie Hymon).
5. Sytuacja, w której popełniłeś błąd – co się stało i czego się
   nauczyłeś.

**Notatki / Twoje przygotowane historie:**
```
(tu wpisz swoje przygotowane odpowiedzi STAR)
```
