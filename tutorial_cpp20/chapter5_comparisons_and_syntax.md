# Chapter 5: Nowa składnia – `<=>`, designated initializers, template lambdas

[← Spis treści](README.md)

Zestaw mniejszych, ale bardzo często używanych zmian składniowych,
które upraszczają codzienny kod.

---

## 1. Operator `<=>` ("spaceship operator") – jedno porównanie zamiast sześciu

W C++11/17, żeby typ w pełni wspierał porównania, trzeba było pisać
(lub generować makrami) **sześć** operatorów: `==`, `!=`, `<`, `<=`,
`>`, `>=`.

```cpp
// C++11/17 - trzeba napisać wszystkie sześć operatorów ręcznie:
struct Version11 {
    int major, minor, patch;

    bool operator==(const Version11& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator<(const Version11& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    bool operator!=(const Version11& o) const { return !(*this == o); }
    bool operator>(const Version11& o) const { return o < *this; }
    bool operator<=(const Version11& o) const { return !(o < *this); }
    bool operator>=(const Version11& o) const { return !(*this < o); }
};
```

```cpp
// C++20 - kompilator generuje wszystkie porównania z jednej definicji:
#include <compare>

struct Version20 {
    int major, minor, patch;
    auto operator<=>(const Version20&) const = default;   // to wystarczy!
};

Version20 a{1, 2, 3};
Version20 b{1, 3, 0};
bool r1 = (a < b);    // działa
bool r2 = (a == b);   // działa - '=default' generuje też operator==
bool r3 = (a >= b);   // działa
```
`= default` porównuje pola **w kolejności deklaracji**, leksykograficznie
(dokładnie tak jak ręczna implementacja wyżej) – dla większości
struktur to dokładnie to, czego potrzebujesz.

**Własna logika porównania** (gdy trzeba czegoś niestandardowego):
```cpp
#include <compare>

struct Temperature {
    double celsius;
    std::partial_ordering operator<=>(const Temperature& o) const {
        return celsius <=> o.celsius;   // delegacja do wbudowanego <=> dla double
    }
};
```
Trzy możliwe typy zwracane przez `<=>`, w zależności od semantyki typu:
- `std::strong_ordering` – zawsze da się jednoznacznie porównać (np. `int`)
- `std::weak_ordering` – porównywalne, ale równość `<=>` niekoniecznie oznacza identyczność (np. porównanie case-insensitive stringów)
- `std::partial_ordering` – porównanie może nie istnieć (np. `NaN` dla `float`/`double`)

## 2. Designated initializers – inicjalizacja pól po nazwie

```cpp
struct Config {
    int baud_rate;
    int data_bits;
    bool parity;
};

// C++11: trzeba pamiętać KOLEJNOŚĆ pól, łatwo o pomyłkę:
Config c11{9600, 8, false};

// C++20: jawnie po nazwie - czytelne, mniej podatne na błąd:
Config c20{.baud_rate = 9600, .data_bits = 8, .parity = false};

// Można pominąć pola z końca (dostają wartość domyślną / zero-init):
Config c20_partial{.baud_rate = 115200};   // data_bits = 0, parity = false
```
> ⚠️ **Ograniczenie:** pola muszą być podane **w kolejności
> deklaracji** w strukturze – nie da się "przeskakiwać" ani
> reorganizować kolejności (inaczej niż w czystym C99, gdzie
> designated initializers są bardziej elastyczne).

## 3. Template lambdas – jawne parametry szablonu w lambdzie

```cpp
// C++14: generic lambda z auto, ale bez dostępu do "T" jako nazwy typu:
auto lambda14 = [](auto x, auto y) { return x + y; };
// Problem: jeśli chcesz np. static_assert na typie x, nie masz jego nazwy.

// C++20: jawny parametr szablonu <T> w lambdzie:
auto lambda20 = []<typename T>(T x, T y) {
    static_assert(std::is_arithmetic_v<T>, "Wymagany typ arytmetyczny");
    return x + y;
};

lambda20(2, 3);       // OK - T = int
lambda20(2.5, 1.5);   // OK - T = double
// lambda20(2, 2.5);  // BŁĄD - oba argumenty muszą być tym samym T (int vs double)
```
Zwróć uwagę: `auto x, auto y` (C++14) dedukuje **niezależnie** typ dla
każdego parametru – `lambda(2, 2.5)` zadziałałoby (x=int, y=double).
Wersja `<typename T>(T x, T y)` (C++20) wymusza **ten sam** typ dla
obu parametrów – często dokładnie to jest zamierzone zachowanie.

**Praktyczny przykład – lambda operująca na buforze o znanym typie elementu:**
```cpp
auto sum_buffer = []<typename T>(std::span<const T> data) {
    T total{};
    for (const T& x : data) total += x;
    return total;
};

std::array<int, 5> buf{1, 2, 3, 4, 5};
int total = sum_buffer(std::span<const int>(buf));
```

## 4. `char8_t` – dedykowany typ dla UTF-8

```cpp
char8_t utf8_char = u8'A';        // C++11/17: u8'A' miało typ 'char'
const char8_t* utf8_str = u8"tekst";   // teraz osobny typ, nie myli się z 'char' zwykłym
```
Zmiana głównie porządkująca (odróżnienie surowych bajtów UTF-8 od
zwykłego `char` platformowego) – rzadko krytyczna w embedded, ale
dobrze wiedzieć, że `u8"..."` zmienił typ i stary kod mieszający
`char*` z `u8"..."` może przestać się kompilować po przejściu na
C++20.

---

## 🔧 Różnica względem C++11

| C++11 | C++20 |
|---|---|
| 6 operatorów porównania pisanych ręcznie | `auto operator<=>(const T&) const = default;` generuje wszystkie |
| Inicjalizacja przez pozycję: `Config{9600, 8, false}` | Inicjalizacja przez nazwę: `Config{.baud_rate = 9600, ...}` |
| `[](auto x, auto y)` – niezależna dedukcja typów | `[]<typename T>(T x, T y)` – wymuszony wspólny typ, dostęp do nazwy `T` |
| `u8"..."` ma typ `const char*` | `u8"..."` ma dedykowany typ `const char8_t*` |

---

## ✅ Zadania

1. Weź strukturę z co najmniej 3 polami reprezentującą coś z Twojego
   dotychczasowego kodu embedded (np. ramkę danych, konfigurację UART)
   i dodaj jej `operator<=>(...) const = default`. Sprawdź, że `<`,
   `==`, `>=` działają.
2. Przepisz inicjalizację jakiejś swojej struktury konfiguracyjnej na
   designated initializers.
3. Napisz template lambdę `[]<typename T>(std::span<const T> data) { ... }`
   zwracającą największy element bufora, i przetestuj ją na
   `std::array<int, N>` oraz `std::array<float, N>`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 6 – Coroutines i Modules (przegląd)](chapter6_coroutines_modules_overview.md)
