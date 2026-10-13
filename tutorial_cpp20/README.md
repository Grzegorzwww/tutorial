# C++20 dla programisty Embedded C++11 – Tutorial

Kurs skupia się **wyłącznie na tym, co zmieniło się / doszło** do C++
od czasów C++11 (z krótkim pomostem przez C++14/17, żeby C++20 miało
sens), a nie na powtarzaniu rzeczy, które już znasz (klasy, szablony
podstawowe, `auto`, lambdy, smart pointery, move semantics – to
zakładam jako znane).

**Środowisko ćwiczeń:** Ubuntu, GCC/Clang z pełnym wsparciem biblioteki
standardowej (desktop). Kompilacja:
```bash
g++ -std=c++20 -Wall -Wextra plik.cpp -o program
# lub
clang++ -std=c++20 -stdlib=libc++ -Wall -Wextra plik.cpp -o program
```
> Sprawdź wersję: `g++ --version` – potrzebujesz **GCC 10+** dla
> większości tego kursu, **GCC 13+** dla `std::format` (Chapter 8).
> Clang: **13+** zalecane, `<format>` bywa niepełny w starszych.

Każdy rozdział to osobny plik z: opisem koncepcyjnym, kodem
przykładowym (kompilowalnym), notatką "co to zmienia względem C++11" i
zadaniami.

## Spis treści

0. [Pomost C++14/17 – co mogłeś przegapić](chapter0_bridge_cpp14_17.md) –
   `if constexpr`, structured bindings, `std::optional`/`variant`,
   CTAD, inline variables, `std::string_view`.
1. [Concepts – ograniczanie szablonów](chapter1_concepts.md) – nazwane
   wymagania na typy w szablonach zamiast nieczytelnego SFINAE.
2. [Ranges – biblioteka zakresów](chapter2_ranges.md) – kompozycja
   operacji na kolekcjach bez tymczasowych kontenerów, składnia `|`.
3. [`std::span` – bezpieczny widok na bufor](chapter3_span.md) –
   zamiennik pary `(wskaźnik, rozmiar)` typowej w kodzie embedded.
4. [`constexpr`/`consteval`/`constinit`](chapter4_constexpr_consteval.md) –
   mocniejsze obliczenia w czasie kompilacji.
5. [Nowa składnia: `<=>`, designated initializers, template lambdas](chapter5_comparisons_and_syntax.md)
6. [Coroutines i Modules – krótki przegląd koncepcyjny](chapter6_coroutines_modules_overview.md) –
   świadomie płytko, temat ciężki i rzadko potrzebny w embedded.
7. [Biblioteka narzędziowa: `<bit>`, `std::format`, `jthread` i inne](chapter7_utility_library.md)

## Jak korzystać

- Kompiluj i uruchamiaj **każdy** przykład sam – ten kurs zakłada
  praktykę, nie samo czytanie.
- Sekcja "🔧 Różnica względem C++11" w każdym rozdziale to najważniejszy
  fragment – tam jest sedno tego, co się zmieniło.
- Zadania (✅) rozwiązuj pisząc kod, nie tylko w głowie.
