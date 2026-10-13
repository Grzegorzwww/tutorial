# Chapter 6: Coroutines i Modules – krótki przegląd koncepcyjny

[← Spis treści](README.md)

To dwie **największe** funkcjonalności dodane w C++20 pod względem
zmian w kompilatorze/języku – ale też najciężej strawne i najsłabiej
wspierane przez kompilatory w praktyce (nawet w 2024/2025). Ten
rozdział celowo jest krótki: chodzi o to, żebyś **rozpoznał temat i
wiedział o co chodzi**, gdy natrafisz na niego w kodzie/dokumentacji –
bez wchodzenia w pisanie własnych coroutine types czy migrację
projektu na moduły.

---

## 1. Coroutines – funkcje, które można "zapauzować" i wznowić

**Problem, który rozwiązują:** kod asynchroniczny (np. "poczekaj na
dane z sieci, potem przetwórz") tradycyjnie pisze się przez callbacki
albo maszyny stanów pisane ręcznie – oba podejścia szybko robią się
nieczytelne przy większej złożoności.

Coroutine to funkcja, która może **zawiesić** swoje wykonanie w
środku (zachowując cały swój stan – lokalne zmienne, miejsce w kodzie)
i zostać **wznowiona** później, z dokładnie tego samego miejsca.

Trzy nowe słowa kluczowe:
```cpp
co_await wyrażenie;   // zawieś wykonanie, czekając na wynik "wyrażenia"
co_yield wartość;      // zwróć "wartość" do wywołującego, zawieś się, czekaj na wznowienie
co_return wartość;     // zakończ coroutine, zwracając "wartość"
```

**Konceptualny przykład – generator liczb (wymaga własnego typu `Generator`, patrz niżej):**
```cpp
Generator<int> count_up_to(int n) {
    for (int i = 1; i <= n; ++i) {
        co_yield i;   // "zwróć i, zapamiętaj gdzie jesteśmy, czekaj na kolejne wywołanie"
    }
}

// użycie (koncepcyjnie):
for (int value : count_up_to(5)) {
    std::cout << value << " ";   // 1 2 3 4 5 - ale liczone "na żądanie", nie od razu
}
```

### ⚠️ Ważny szczegół, o którym trzeba wiedzieć

**C++20 nie dostarcza gotowego typu `Generator<T>`** (jak w
przykładzie wyżej) – to tylko **mechanizm językowy** (`co_await`,
`co_yield`, `co_return` + protokół `promise_type`). Żeby faktycznie
napisać coroutine w C++20, musisz albo:
- napisać własny typ "coroutine frame" (`promise_type` z metodami
  `initial_suspend`, `final_suspend`, `yield_value` itd. – sporo
  boilerplate'u), albo
- użyć gotowej biblioteki (np. `cppcoro`), albo
- poczekać/użyć **C++23**, które dodało `std::generator<T>` gotowy do
  użycia bez pisania własnego promise_type.

To jedna z głównych krytyk C++20 coroutines: język dał mechanizm, ale
nie dał "baterii w zestawie" – stąd niska popularność w kodzie
produkcyjnym embedded (dodatkowo: coroutines zwykle **alokują na
stercie** ramkę stanu, co bywa problematyczne przy braku heap albo
przy twardych wymaganiach RTOS/real-time).

**Zapamiętaj tyle:** gdy zobaczysz `co_await`/`co_yield`/`co_return` w
kodzie (np. w bibliotece do async I/O, jak niektóre nowsze frameworki
sieciowe), wiesz że to mechanizm zawieszania/wznawiania funkcji – nie
musisz na tym etapie umieć napisać własnego `promise_type` od zera.

---

## 2. Modules – zamiennik dla `#include` i systemu nagłówków

**Problem, który rozwiązują:** model `#include` istnieje od czasów C
– preprocesor dosłownie wkleja tekst nagłówka w miejsce `#include`,
dla **każdej** jednostki kompilacji z osobna. Przy dużych projektach
to prowadzi do: bardzo długich czasów kompilacji (ten sam nagłówek
parsowany setki razy), problemów z kolejnością include (makra,
include guards), przypadkowego "wycieku" makr między plikami.

**Moduły** to nowy mechanizm: kod jest kompilowany **raz** do binarnej
reprezentacji pośredniej (interface), a inne pliki go `import`-ują
zamiast `#include`-ować tekst źródłowy na nowo.

```cpp
// plik: math_utils.cppm (interfejs modułu)
export module math_utils;   // deklaruje moduł o nazwie "math_utils"

export int add(int a, int b) {   // 'export' = widoczne dla importujących
    return a + b;
}

int internal_helper() { return 42; }   // bez 'export' - prywatne dla modułu
```

```cpp
// plik: main.cpp
import math_utils;   // zamiast #include "math_utils.hpp"

int main() {
    return add(2, 3);
}
```

### ⚠️ Stan wsparcia w praktyce (2024/2025)

Moduły to funkcjonalność, gdzie **wsparcie narzędziowe wciąż dojrzewa**:
- Wymaga nowych flag budowania i zwykle integracji z konkretnym
  systemem budowania (CMake 3.28+ ma eksperymentalne wsparcie).
  Composer/Makefile ręczny bez CMake – bardzo trudne do ogarnięcia.
  ­
- GCC, Clang i MSVC mają **różne** poziomy zgodności i różne detale
  implementacyjne – kod modułowy bywa mniej przenośny między
  kompilatorami niż klasyczny `#include`.
- Biblioteka standardowa jako moduł (`import std;`) to dopiero
  **C++23**, nie C++20.
- Większość istniejących bibliotek (w tym typowych embedded HAL/SDK)
  nadal używa `#include` i długo jeszcze będzie – moduły i nagłówki
  mogą współistnieć w jednym projekcie.

**Zapamiętaj tyle:** moduły to docelowy zamiennik dla `#include`,
głównie po to by przyspieszyć kompilację dużych projektów i
odizolować makra/przestrzenie nazw – ale w 2025 wciąż rzadko spotykane
w praktyce, zwłaszcza w embedded, gdzie toolchainy (arm-none-eabi-gcc,
ESP-IDF, STM32CubeIDE) nie mają jeszcze dojrzałego wsparcia.

---

## 🔧 Różnica względem C++11

| C++11 | C++20 |
|---|---|
| Async kod = callbacki lub ręczne maszyny stanów | Coroutines: `co_await`/`co_yield`/`co_return` (mechanizm, bez gotowej biblioteki) |
| `#include` = tekstowe wklejanie nagłówka, parsowane wielokrotnie | `import` modułu = skompilowany raz interfejs binarny (gdy toolchain wspiera) |

---

## ✅ Zadania (koncepcyjne – to nie jest rozdział do pisania dużo kodu)

1. Wyjaśnij własnymi słowami różnicę między `co_yield` a zwykłym
   `return` w generatorze.
2. Sprawdź (bez konieczności pisania kodu) wersję swojego `g++`/`clang++`
   i poszukaj w dokumentacji, czy Twoja wersja ma jakiekolwiek wsparcie
   dla `import std;` lub modułów w ogóle (`g++ --version`, potem
   dokumentacja GCC "C++20 modules status").
3. Jak sądzisz – dlaczego coroutines (alokacja ramki na stercie z
   domyślną implementacją) mogą być problematyczne na mikrokontrolerze
   bez sterty albo z twardymi wymaganiami czasu rzeczywistego?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 7 – Biblioteka narzędziowa](chapter7_utility_library.md)
