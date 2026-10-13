# Chapter 0: Pomost C++14/17 – co mogłeś przegapić

[← Spis treści](README.md)

Jeśli pracowałeś głównie w C++11 na embedded, prawdopodobnie ominęły
Cię też przydatne dodatki z C++14 i C++17. Ten rozdział to szybki
przegląd – bez tego niektóre przykłady w C++20 (dalsze rozdziały) będą
wyglądać dziwnie.

---

## C++14 (drobne, ale wygodne)

**Zwracany typ `auto` z dedukcją (nie tylko dla lambd):**
```cpp
auto add(int a, int b) {   // C++11 wymagałoby: auto add(int a, int b) -> int
    return a + b;
}
```

**Zmienne szablonowe i generic lambdas:**
```cpp
template<typename T>
constexpr T pi = T(3.1415926535897932385);

auto lambda = [](auto x, auto y) { return x + y; };  // C++11: lambda musiała mieć jawne typy
```

**Literały binarne i separator cyfr** (bardzo przydatne przy pracy z rejestrami!):
```cpp
constexpr uint8_t mask = 0b1010'1010;   // C++11: tylko 0xAA, bez separatora
constexpr uint32_t big = 1'000'000;
```

---

## C++17 (tu jest już sporo mięsa)

**`if constexpr` – kompilacja warunkowa wewnątrz szablonu:**
```cpp
template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;          // ta gałąź istnieje tylko dla typów całkowitych
    } else {
        return value;
    }
}
```
W C++11 do tego potrzebne było `std::enable_if` i przeciążanie funkcji
– znacznie mniej czytelne. To jedna z najważniejszych zmian pod kątem
pisania generycznego kodu embedded (np. sterowniki obsługujące różne
szerokości rejestrów).

**Structured bindings – rozpakowywanie struktur/par/tuple:**
```cpp
std::map<int, std::string> m;
for (const auto& [key, value] : m) {   // C++11: auto it = ...; it->first; it->second;
    std::cout << key << " = " << value << "\n";
}

struct Point { int x; int y; };
Point p{1, 2};
auto [x, y] = p;   // rozpakowanie pól struktury
```

**`std::optional<T>` – wartość, która może nie istnieć, bez wskaźników/wyjątków:**
```cpp
#include <optional>

std::optional<int> parse_number(const std::string& s) {
    if (s.empty()) return std::nullopt;
    return std::stoi(s);
}

if (auto result = parse_number("42")) {
    std::cout << "Wartość: " << *result << "\n";
}
```
W embedded to naturalny zamiennik wzorca "zwróć -1 / 0xFFFF jako
błąd" – bez poświęcania wartości z prawidłowego zakresu na kod błędu.

**`std::variant<T...>` – bezpieczna unia (type-safe union):**
```cpp
#include <variant>

std::variant<int, float, std::string> v = 42;
v = 3.14f;
std::visit([](auto&& val) { std::cout << val << "\n"; }, v);
```
Zamiennik surowego `union` z C – jądro pamięta, który typ jest
aktualnie aktywny, więc nie ma UB przy odczycie złego pola.

**CTAD – Class Template Argument Deduction:**
```cpp
std::pair p{1, 2.5};          // C++11: std::pair<int, double> p{1, 2.5};
std::vector v{1, 2, 3};       // kompilator sam wydedukuje std::vector<int>
```

**`std::string_view` – nie-właścicielski widok na string:**
```cpp
#include <string_view>

void log(std::string_view msg) {   // nie kopiuje, nie alokuje - istotne w embedded
    std::cout << msg << "\n";
}
log("literał");                    // działa bez konstruowania std::string
log(std::string("zmienna"));       // też działa, bez kopii danych
```
To jest duchowy poprzednik `std::span` z C++20 (Chapter 3) – ten sam
pomysł ("widok" zamiast właściciela danych), ale tylko dla tekstu.

**Inline variables – definicja zmiennej globalnej w nagłówku bez ODR violation:**
```cpp
// w pliku .hpp, dołączanym do wielu jednostek kompilacji:
inline constexpr int kBufferSize = 256;   // C++11: potrzebny osobny .cpp z definicją
```

---

## ✅ Zadania

1. Przepisz funkcję używającą `std::enable_if` (jeśli masz taką w
   starym kodzie) na wersję z `if constexpr`. Jeśli nie masz przykładu
   pod ręką, napisz funkcję `to_string_debug(T value)`, która dla
   typów arytmetycznych zwraca `std::to_string(value)`, a dla innych
   (np. `bool`) zwraca `"true"/"false"` – zaimplementuj to przez
   `if constexpr`.
2. Napisz funkcję `find_first_negative(const std::vector<int>&)`
   zwracającą `std::optional<int>` (indeks pierwszego elementu
   ujemnego albo `std::nullopt`). Porównaj to z tym, jak zrobiłbyś to
   w C++11 (zwracając np. `-1` jako "brak").
3. Zamień pętlę `for (auto it = m.begin(); it != m.end(); ++it)` po
   `std::map<std::string, int>` na wersję ze structured bindings.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 1 – Concepts](chapter1_concepts.md)
