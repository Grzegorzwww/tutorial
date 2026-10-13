# Chapter 1: Concepts – ograniczanie szablonów

[← Spis treści](README.md)

**Problem, który rozwiązują:** w C++11/14/17, gdy szablon wymagał
konkretnych właściwości typu (np. "musi być liczbą całkowitą", "musi
mieć metodę `.size()`"), jedynym narzędziem było SFINAE
(`std::enable_if`, `decltype` w kontekście nieudanej dedukcji) –
działające, ale skrajnie nieczytelne, z okropnymi komunikatami błędów
na stronę tekstu.

**Concepts** to nazwane, czytelne wymagania na typy, sprawdzane przez
kompilator **przed** próbą instancjacji szablonu – błędy są krótkie i
konkretne ("typ X nie spełnia koncepcji Y"), zamiast strony
nieczytelnego błędu z wnętrza implementacji szablonu.

---

## 1. Podstawowa składnia

```cpp
#include <concepts>
#include <iostream>

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
T add(T a, T b) {
    return a + b;
}

int main() {
    std::cout << add(2, 3) << "\n";       // OK - int spełnia Numeric
    std::cout << add(2.5, 1.5) << "\n";   // OK - double spełnia Numeric
    // add(std::string("a"), std::string("b"));  // BŁĄD KOMPILACJI - czytelny komunikat
}
```

Bez concepts, wywołanie `add` na `std::string` w C++11 skompilowałoby
się (bo `std::string` ma `operator+`), tylko przy okazji dałoby
zaskakujący wynik. Z concept `Numeric` kompilator odrzuca to **na
etapie doboru przeciążenia**, z jasnym komunikatem.

## 2. Standardowe koncepcje (biblioteka `<concepts>`)

Nie musisz definiować wszystkiego sam – biblioteka standardowa daje
gotowy zestaw:

```cpp
#include <concepts>

template<std::integral T>          // tylko typy całkowite (int, uint8_t, size_t...)
T double_it(T x) { return x * 2; }

template<std::floating_point T>    // tylko float/double/long double
T half_it(T x) { return x / 2; }

template<std::copyable T>          // typ musi być kopiowalny
void store(T value) { /* ... */ }

template<typename T>
requires std::equality_comparable<T>   // musi wspierać operator==
bool are_equal(T a, T b) { return a == b; }
```

## 3. Trzy sposoby zapisu tego samego ograniczenia

```cpp
// Wariant 1: koncepcja jako "typ" parametru szablonu
template<std::integral T>
T f1(T x) { return x; }

// Wariant 2: klauzula requires po liście szablonu
template<typename T>
requires std::integral<T>
T f2(T x) { return x; }

// Wariant 3: requires jako "trailing" na końcu deklaracji
template<typename T>
T f3(T x) requires std::integral<T> { return x; }

// Wariant 4 (skrócony): "abbreviated function template" - auto z koncepcją
T f4(std::integral auto x) { return x; }
```
Wszystkie cztery są równoważne – w praktyce Wariant 1 i 4 są najczęściej
spotykane w nowym kodzie ze względu na zwięzłość.

## 4. Własna koncepcja z `requires` – wymagania na wyrażenia

Można wymagać, żeby typ obsługiwał konkretne operacje/metody, nie
tylko sprawdzać nazwane cechy:

```cpp
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template<HasSize T>
void print_size(const T& container) {
    std::cout << "Rozmiar: " << container.size() << "\n";
}

print_size(std::vector<int>{1, 2, 3});   // OK
// print_size(42);                       // BŁĄD - int nie ma .size()
```

`requires(T t) { ... }` to blok, w którym piszesz **przykładowy
kod używający T** – kompilator sprawdza, czy taki kod w ogóle by się
skompilował, nie wykonuje go.

## 5. `requires` do ograniczenia bardziej złożonej logiki (embedded-owy przykład)

```cpp
template<typename T>
concept RegisterType = std::unsigned_integral<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);

template<RegisterType T>
void write_register(volatile T* reg, T value) {
    *reg = value;
}
```
To realny wzorzec z embedded: chcesz, żeby funkcja pisząca do rejestru
akceptowała tylko `uint8_t`/`uint16_t`/`uint32_t`, a nie np. `int` czy
`bool` – z czytelnym błędem kompilacji, gdy ktoś się pomyli.

---

## 🔧 Różnica względem C++11

| C++11 (SFINAE) | C++20 (Concepts) |
|---|---|
| `template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>` | `template<std::integral T>` |
| Błędy kompilacji: strony nieczytelnego tekstu z wnętrza `<type_traits>` | Błędy kompilacji: "constraints not satisfied" z nazwą koncepcji |
| Wymagania rozproszone, trudne do odczytania z sygnatury | Wymagania czytelne wprost w sygnaturze funkcji |
| Trudne do złożenia (`&&`/`||` na `enable_if` to koszmar) | Concepts składają się naturalnie: `Numeric = integral || floating_point` |

---

## ✅ Zadania

1. Napisz koncepcję `ByteLike` spełnianą przez `uint8_t`, `int8_t`,
   `char` i `std::byte` (podpowiedź: sprawdź `sizeof(T) == 1` i
   `std::integral<T>` lub `std::same_as<T, std::byte>`).
2. Napisz funkcję `template<typename T> concept Printable = requires(T t) { std::cout << t; };`
   i użyj jej do ograniczenia szablonowej funkcji `void log_value(Printable auto v)`.
3. Przepisz jakąś swoją starą funkcję szablonową z C++11 używającą
   `std::enable_if` (jeśli pamiętasz taką z pracy) na wersję z concepts.
4. (Bonus) Napisz koncepcję `Container` wymagającą `.begin()`,
   `.end()` i `.size()` jednocześnie – połącz trzy `requires` w jednym
   bloku.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 2 – Ranges](chapter2_ranges.md)
