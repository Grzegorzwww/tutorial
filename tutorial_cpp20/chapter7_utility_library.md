# Chapter 7: Biblioteka narzędziowa – `<bit>`, `std::format`, `jthread` i inne

[← Spis treści](README.md)

Zbiór mniejszych, ale praktycznych dodatków do biblioteki standardowej
w C++20 – wiele z nich bezpośrednio przydatnych w kodzie embedded
(operacje bitowe, wątki z automatycznym join).

---

## 1. `<bit>` – operacje bitowe bez UB i bez własnych implementacji

W C++11 typowe operacje bitowe pisało się ręcznie (i często niepoprawnie
pod kątem *undefined behavior* przy przesunięciach czy aliasingu typów).

```cpp
#include <bit>
#include <cstdint>
#include <iostream>

uint32_t x = 0b0000'0000'0000'0000'0000'0000'0010'1100;

std::popcount(x);          // liczba ustawionych bitów (ile jedynek) -> 4
std::countl_zero(x);       // liczba zer wiodących (od MSB) -> 26
std::countr_zero(x);       // liczba zer końcowych (od LSB) -> 2
std::has_single_bit(x);    // czy x jest potęgą dwójki? -> false
std::bit_width(x);         // ile bitów potrzeba by zapisać x -> 6
std::bit_ceil(x);          // najmniejsza potęga 2 >= x
std::bit_floor(x);         // największa potęga 2 <= x

uint32_t rotated = std::rotl(x, 4);   // rotacja bitowa w lewo o 4 (zamiennik ręcznego (x << 4) | (x >> 28))
```
To dokładnie ten rodzaj operacji, który wcześniej pisało się ręcznie z
pomocą magicznych wzorów albo intrinsics kompilatora (`__builtin_clz`,
`__builtin_popcount` w GCC) – teraz są **przenośne i standardowe**.

**`std::bit_cast` – bezpieczna reinterpretacja bitów (zamiennik `reinterpret_cast`/`union`/`memcpy`):**
```cpp
#include <bit>
#include <cstdint>

float f = 3.14f;
uint32_t bits = std::bit_cast<uint32_t>(f);   // reinterpretuj bity float jako uint32_t

// Stary sposób (C++11), z potencjalnym UB przy złym użyciu:
uint32_t bits_old;
std::memcpy(&bits_old, &f, sizeof(f));   // poprawny, ale mniej czytelny sposób w C++11
// LUB (UB, unikać!): uint32_t bits_ub = *reinterpret_cast<uint32_t*>(&f);
```
`std::bit_cast` wymaga, żeby oba typy miały ten sam rozmiar i były
trywialnie kopiowalne – kompilator sprawdza to w compile-time, a
implementacja jest równie wydajna jak `memcpy` (często wygenerowana
jako pojedyncza instrukcja, bez faktycznego kopiowania pamięci).

## 2. `std::format` – bezpieczny typowo `printf`

```cpp
#include <format>
#include <iostream>

int sensor_id = 3;
double temperature = 23.456;

std::string msg = std::format("Sensor {}: {:.2f} C", sensor_id, temperature);
std::cout << msg << "\n";   // "Sensor 3: 23.46 C"
```
W porównaniu do `printf("Sensor %d: %.2f C", sensor_id, temperature)`:
- **Type-safe** – nie ma niezgodności specyfikatora z typem (`%d` na
  `double` to w `printf` niezdefiniowane zachowanie; `std::format`
  wykrywa to w compile-time).
- Składnia `{}` z opcjonalnym formatowaniem `{:.2f}` (podobna do
  Pythona) zamiast zapamiętywania dziesiątek specyfikatorów `%`.

> ⚠️ **Uwaga praktyczna dla embedded:** `<format>` wymaga stosunkowo
> nowego kompilatora (pełne wsparcie: **GCC 13+**, wcześniejsze GCC 10-12
> mają tylko częściowe/eksperymentalne wsparcie) oraz typowo większego
> rozmiaru kodu wynikowego niż `printf` – na mikrokontrolerze z
> ograniczoną pamięcią Flash warto to zmierzyć, zanim zastąpisz
> istniejący, sprawdzony `printf`/`snprintf`.

## 3. `std::jthread` – wątek z automatycznym join i kooperacyjnym anulowaniem

```cpp
#include <thread>   // jthread też jest w <thread>
#include <stop_token>
#include <chrono>
#include <iostream>

void worker(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::cout << "Pracuję...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Zatrzymano.\n";
}

int main() {
    std::jthread t(worker);   // automatycznie dostaje stop_token

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // brak jawnego t.join()!
}   // <- tutaj jthread automatycznie: wysyła stop request I robi join() w destruktorze
```
Dwie różnice względem `std::thread` (C++11):
1. **Destruktor automatycznie robi `join()`** – klasyczny bug z
   `std::thread` to zapomnienie o `join()`/`detach()`, co kończy się
   wywołaniem `std::terminate()`. `jthread` eliminuje tę klasę błędów.
2. **Kooperacyjne anulowanie** przez `std::stop_token`/`std::stop_source`
   – standardowy sposób poproszenia wątku o zakończenie, bez własnych
   flag `std::atomic<bool> should_stop`.

> Uwaga: to nadal wątki systemu operacyjnego (Linux/RTOS z wątkami) –
> nieprzydatne na "gołym" mikrokontrolerze bez OS, ale bardzo przydatne
> w Embedded Linux (Twój kolejny kierunek!) czy aplikacjach pod RTOS z
> wrapperem C++ na wątki.

## 4. `std::atomic_ref` – atomowe operacje na zwykłej zmiennej

```cpp
#include <atomic>

int shared_counter = 0;   // zwykła, "gołą" zmienna int - NIE std::atomic<int>

void increment() {
    std::atomic_ref<int> atomic_view(shared_counter);
    atomic_view.fetch_add(1);   // atomowa operacja na ISTNIEJĄCEJ zmiennej
}
```
W C++11 atomowość wymagała, żeby zmienna **od początku** była
zadeklarowana jako `std::atomic<int>`. `std::atomic_ref` pozwala
"nałożyć" atomowość na zwykłą zmienną, którą już masz (np. pole w
strukturze współdzielonej z kodem C, gdzie nie możesz zmienić typu
pola) – bez zmiany layoutu/ABI struktury.

## 5. Drobne, ale przydatne dodatki

```cpp
#include <numbers>
double circumference = 2 * std::numbers::pi * radius;   // std::numbers::pi zamiast własnego #define M_PI

#include <array>
auto arr = std::to_array({1, 2, 3, 4});   // std::array<int, 4> - rozmiar wydedukowany, bez podawania <int, 4>

#include <numeric>
int mid = std::midpoint(10, 20);       // 15 - bez ryzyka przepełnienia jak w (a+b)/2 dla dużych liczb
double lerp_val = std::lerp(0.0, 10.0, 0.5);   // interpolacja liniowa -> 5.0

#include <source_location>
void log_error(const std::string& msg,
               std::source_location loc = std::source_location::current()) {
    std::cout << loc.file_name() << ":" << loc.line() << " - " << msg << "\n";
}
// wywołanie log_error("coś poszło nie tak") automatycznie wypełni plik/linię wywołania
// - zamiennik makr __FILE__/__LINE__ znany z C, ale typowany i bez preprocesora
```

---

## 🔧 Różnica względem C++11

| C++11 | C++20 |
|---|---|
| `__builtin_popcount`/ręczne wzory bitowe (niestandardowe) | `<bit>`: `std::popcount`, `std::countl_zero`, `std::rotl`, przenośne i standardowe |
| `reinterpret_cast`/`memcpy` do reinterpretacji bitów | `std::bit_cast` – bezpieczne, sprawdzane w compile-time |
| `printf`/`snprintf` – brak type safety | `std::format` – type-safe, składnia `{}` (koszt: rozmiar binarki, wymaga nowego kompilatora) |
| `std::thread` – ręczny `join()`, ręczne flagi anulowania | `std::jthread` – auto-join w destruktorze, wbudowany `stop_token` |
| `std::atomic<T>` – zmienna musiała być atomowa "od zawsze" | `std::atomic_ref<T>` – atomowość nałożona na istniejącą zmienną |
| `#define M_PI`, ręczne `(a+b)/2` | `std::numbers::pi`, `std::midpoint` (bez przepełnienia) |
| Makra `__FILE__`/`__LINE__` | `std::source_location::current()` – typowane, bez preprocesora |

---

## ✅ Zadania

1. Napisz funkcję zliczającą, ile bitów trzeba zmienić żeby przejść z
   jednej liczby `uint32_t` na drugą (Hamming distance), używając
   `std::popcount` i XOR.
2. Użyj `std::bit_cast`, żeby podejrzeć bitową reprezentację `float`
   jako `uint32_t` dla kilku wartości (`1.0f`, `-1.0f`, `0.0f`) i
   porównaj z tym, czego oczekujesz ze standardu IEEE 754.
3. Napisz mini-przykład z `std::jthread`: wątek w pętli co 500ms
   wypisujący komunikat, zatrzymywany po 2 sekundach przez wyjście
   z zakresu (bez jawnego `join()`/`request_stop()`).
4. (Jeśli masz GCC 13+/Clang z pełnym `<format>`) Zamień jedno swoje
   dotychczasowe wywołanie `printf`/`std::cout <<` na `std::format`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie kursu

Po ukończeniu wszystkich rozdziałów powinieneś:
- [ ] Znać kluczowe dodatki C++14/17, które są fundamentem pod C++20
- [ ] Umieć ograniczać szablony przez `concepts` zamiast SFINAE
- [ ] Umieć składać operacje na kolekcjach przez `ranges`/`views`
- [ ] Używać `std::span` zamiast pary `(wskaźnik, rozmiar)` w nowym kodzie
- [ ] Rozumieć rozszerzone możliwości `constexpr` oraz różnicę względem `consteval`/`constinit`
- [ ] Znać operator `<=>`, designated initializers i template lambdas
- [ ] Rozpoznawać coroutines i moduły w kodzie, nawet bez pisania ich od zera
- [ ] Znać najważniejsze dodatki biblioteki narzędziowej (`<bit>`, `jthread`, `atomic_ref`)

**Gratulacje – to koniec tego kursu C++20.** Naturalny następny krok:
zastosowanie `concepts` i `std::span` w prawdziwym kodzie (np. w
projekcie Embedded Linux z poprzedniego kursu), żeby utrwalić nawyki.
