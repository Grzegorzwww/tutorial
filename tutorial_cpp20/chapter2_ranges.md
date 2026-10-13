# Chapter 2: Ranges – biblioteka zakresów

[← Spis treści](README.md)

**Problem, który rozwiązują:** algorytmy z `<algorithm>` w C++11
zawsze operowały na parze iteratorów (`begin`, `end`) i zazwyczaj
wymagały tymczasowego kontenera na wynik pośredni, jeśli chciałeś
złożyć kilka operacji (np. najpierw przefiltrować, potem
przekształcić). To prowadziło do kaskad tymczasowych `std::vector`.

**Ranges** (`<ranges>`) pozwalają operować bezpośrednio na "zakresie"
(czymkolwiek co ma `begin()`/`end()`) i **składać** operacje
(*views*) w potok, bez alokowania kontenerów pośrednich – obliczenia są
leniwe (lazy), wykonują się dopiero przy faktycznej iteracji.

---

## 1. Stary sposób (C++11/17, `<algorithm>`)

```cpp
#include <algorithm>
#include <vector>

std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

std::vector<int> even;
std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(even),
             [](int n) { return n % 2 == 0; });

std::vector<int> squared;
std::transform(even.begin(), even.end(), std::back_inserter(even),
               [](int n) { return n * n; });
```
Dwa tymczasowe wektory (`even`, potem nadpisany `squared` – tu nawet
błędnie użyty ten sam kontener) tylko po to, żeby przefiltrować i
przekształcić dane.

## 2. Nowy sposób – `views` i operator `|`

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = numbers
                | std::views::filter([](int n) { return n % 2 == 0; })
                | std::views::transform([](int n) { return n * n; });

    for (int n : result) {
        std::cout << n << " ";   // 4 16 36 64 100
    }
    std::cout << "\n";
}
```
Operator `|` czyta się jak potok (pipe) z basha: "weź `numbers`,
przefiltruj, przekształć". **Żadna** pośrednia kolekcja nie jest
tworzona – `result` to lekki obiekt-widok, filtr i transformacja
wykonują się dopiero podczas pętli `for`, element po elemencie
(leniwa ewaluacja).

## 3. Najczęściej używane views

```cpp
#include <ranges>
namespace rv = std::views;

std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

for (int n : v | rv::take(3))        std::cout << n << " ";  // 1 2 3
std::cout << "\n";

for (int n : v | rv::drop(7))        std::cout << n << " ";  // 8 9 10
std::cout << "\n";

for (int n : v | rv::reverse)        std::cout << n << " ";  // 10 9 8 ... 1
std::cout << "\n";

for (auto [idx, val] : v | rv::enumerate) {   // C++23 w praktyce w niektórych implementacjach;
    // w czystym C++20 enumerate nie istnieje - patrz uwaga niżej
}

for (int n : rv::iota(1, 6))         std::cout << n << " ";  // 1 2 3 4 5 (bez potrzeby kontenera!)
std::cout << "\n";
```
> ⚠️ **Uwaga:** `std::views::enumerate` i kilka innych wygodnych views
> (`zip`, `chunk`) to dodatki z **C++23**, nie C++20. W czystym C++20
> masz: `filter`, `transform`, `take`, `take_while`, `drop`,
> `drop_while`, `reverse`, `iota`, `join`, `split`, `common`, `all`.
> Warto to wiedzieć, żeby nie szukać w dokumentacji C++20 czegoś, co
> pojawiło się dopiero rok później.

## 4. Ranges-owe wersje algorytmów (bez iteratorów `begin`/`end` osobno)

```cpp
#include <algorithm>
#include <ranges>
#include <vector>

std::vector<int> v{5, 3, 1, 4, 2};

std::ranges::sort(v);                          // zamiast std::sort(v.begin(), v.end())
auto it = std::ranges::find(v, 3);              // zamiast std::find(v.begin(), v.end(), 3)
bool any_even = std::ranges::any_of(v, [](int n) { return n % 2 == 0; });
```
Krócej, mniej miejsc na literówkę (`v.begin(), v.end()` vs. `v.begin(), v.begin()` – klasyczny błąd).

## 5. Composowanie własnego "pipeline" na realnych danych

```cpp
#include <ranges>
#include <vector>
#include <string>
#include <iostream>

struct Measurement {
    std::string sensor_name;
    double value;
    bool valid;
};

int main() {
    std::vector<Measurement> readings{
        {"temp1", 23.5, true},
        {"temp2", -999.0, false},   // odczyt błędny
        {"temp3", 24.1, true},
        {"temp4", 22.8, true},
    };

    auto valid_values = readings
        | std::views::filter([](const Measurement& m) { return m.valid; })
        | std::views::transform([](const Measurement& m) { return m.value; });

    double sum = 0;
    int count = 0;
    for (double v : valid_values) {
        sum += v;
        ++count;
    }
    std::cout << "Średnia z poprawnych odczytów: " << (sum / count) << "\n";
}
```

---

## 🔧 Różnica względem C++11

| C++11/17 (`<algorithm>`) | C++20 (`<ranges>`) |
|---|---|
| `std::copy_if(v.begin(), v.end(), out, pred)` | `v \| std::views::filter(pred)` |
| Wymaga tymczasowego kontenera dla każdego kroku | Operacje leniwe, brak alokacji pośrednich |
| Kolejność argumentów: `begin, end, ...` (łatwo pomylić) | Jeden argument: cały kontener/zakres |
| Trudno składać wiele kroków czytelnie | Naturalna kompozycja przez `|` |

---

## ✅ Zadania

1. Masz `std::vector<int> v{1..20}`. Za pomocą jednego "potoku" views
   wybierz liczby podzielne przez 3, podnieś każdą do kwadratu, i
   wypisz tylko pierwsze 3 wyniki (`filter` + `transform` + `take`).
2. Przepisz pętlę `for (int i = 0; i < 10; ++i)` na `for (int i : std::views::iota(0, 10))`.
3. Użyj `std::ranges::sort` i `std::ranges::find` zamiast wersji z
   jawnymi iteratorami na dowolnym `std::vector<int>`.
4. (Bonus, koncepcyjnie) Dlaczego leniwa ewaluacja (obliczanie dopiero
   przy iteracji) może być korzystna w systemie embedded z
   ograniczonym RAM, w porównaniu do tworzenia pośrednich `std::vector`?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 3 – std::span](chapter3_span.md)
