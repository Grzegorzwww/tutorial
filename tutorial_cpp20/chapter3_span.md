# Chapter 3: `std::span` – bezpieczny widok na bufor

[← Spis treści](README.md)

**To prawdopodobnie najbardziej praktyczna nowość C++20 dla kogoś z
tłem embedded C++11.** Ile razy pisałeś funkcję w stylu:

```cpp
void process_buffer(uint8_t* data, size_t length);   // klasyka C/C++11
```

Ten wzorzec ma problemy: nic nie gwarantuje, że `length` faktycznie
odpowiada rozmiarowi bufora pod `data` – to tylko konwencja, łatwo o
błąd (przekazanie złej długości, wyjście poza bufor). `std::span` to
**nie-właścicielski, lekki widok** na ciągły blok pamięci (tablicę,
`std::array`, `std::vector`, surowy wskaźnik+rozmiar), który **niesie
rozmiar razem z sobą** i pozwala na bezpieczny dostęp indeksowany.

---

## 1. Podstawy

```cpp
#include <span>
#include <array>
#include <vector>
#include <iostream>

void print_all(std::span<const int> data) {   // const int = tylko do odczytu
    for (int x : data) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

int main() {
    int raw_array[] = {1, 2, 3};
    std::array<int, 4> std_array{4, 5, 6, 7};
    std::vector<int> vec{8, 9, 10};

    print_all(raw_array);    // działa - rozmiar wydedukowany automatycznie z tablicy C
    print_all(std_array);    // działa
    print_all(vec);          // działa

    int* ptr = raw_array;
    print_all(std::span<int>(ptr, 3));   // działa też z parą (wskaźnik, rozmiar), jeśli musisz
}
```
Kluczowa różnica z `uint8_t* + size_t`: **jedna funkcja** obsługuje
teraz tablicę C, `std::array`, `std::vector` i surowy wskaźnik – bez
przeciążeń, bez szablonów, bez utraty informacji o rozmiarze po drodze.

## 2. `span` nie kopiuje danych – to tylko widok (podobnie jak `string_view`)

```cpp
#include <span>
#include <vector>

void zero_out(std::span<int> data) {
    for (int& x : data) {
        x = 0;
    }
}

int main() {
    std::vector<int> v{1, 2, 3, 4, 5};
    zero_out(v);   // MODYFIKUJE oryginalny wektor - span to widok, nie kopia
    // v == {0, 0, 0, 0, 0}
}
```
`std::span<int>` (bez `const`) pozwala modyfikować dane pod spodem –
tak jak surowy wskaźnik, ale z rozmiarem i bez arytmetyki wskaźników.

## 3. Statyczny rozmiar (`extent`) – zero narzutu, sprawdzenie w compile-time

```cpp
#include <span>

void process_frame(std::span<uint8_t, 64> frame) {   // MUSI mieć dokładnie 64 bajty
    // kompilator wie, że frame.size() == 64 zawsze - może to wykorzystać do optymalizacji
}

uint8_t buffer[64];
process_frame(buffer);   // OK

uint8_t wrong_size[32];
// process_frame(wrong_size);   // BŁĄD KOMPILACJI - 32 != 64
```
To bardzo przydatne w embedded: bufor ramki protokołu o stałym
rozmiarze, DMA buffer o znanej wielkości – kompilator wymusza zgodność
zamiast liczyć na to, że ktoś nie pomyli się w runtime.

## 4. Podział na podzakresy – bez kopiowania

```cpp
#include <span>
#include <array>
#include <iostream>

int main() {
    std::array<uint8_t, 10> packet{0xAA, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xFF};

    std::span<uint8_t> full(packet);
    std::span<uint8_t> header  = full.first(2);        // pierwsze 2 bajty: [0xAA, 0x01]
    std::span<uint8_t> payload = full.subspan(2, 7);    // 7 bajtów od indeksu 2
    std::span<uint8_t> footer  = full.last(1);          // ostatni bajt: [0xFF]

    std::cout << "Payload size: " << payload.size() << "\n";   // 7
}
```
`first()`, `last()`, `subspan()` zwracają **kolejne widoki** na te same
dane – zero kopiowania, zero alokacji. To dokładnie ten typ operacji,
który wcześniej robiłeś ręcznie przez arytmetykę wskaźników
(`data + offset`, licząc długości samodzielnie) – teraz jest to
bezpieczniejsze i czytelniejsze.

## 5. Reinterpretacja bajtów (`std::as_bytes`, `std::as_writable_bytes`)

```cpp
#include <span>
#include <cstdint>

struct SensorFrame {
    uint32_t timestamp;
    float temperature;
    float humidity;
};

void send_over_uart(std::span<const std::byte> raw_data);   // deklaracja gdzieś indziej

void send_frame(const SensorFrame& frame) {
    std::span<const SensorFrame> one_frame(&frame, 1);
    auto bytes = std::as_bytes(one_frame);   // std::span<const std::byte>, rozmiar = sizeof(SensorFrame)
    send_over_uart(bytes);
}
```
To bezpieczniejszy odpowiednik częstego w embedded
`reinterpret_cast<uint8_t*>(&frame)` – nadal "trik niskopoziomowy", ale
typowany i z jawnie zaznaczonym zamiarem ("chcę na to patrzeć jako na
bajty"), zamiast surowego rzutowania wskaźnika.

---

## 🔧 Różnica względem C++11

| C++11 (`T* + size_t`) | C++20 (`std::span<T>`) |
|---|---|
| `void f(uint8_t* data, size_t len)` | `void f(std::span<uint8_t> data)` |
| Rozmiar i wskaźnik to dwa niezależne parametry – łatwo o niezgodność | Rozmiar "przypięty" do widoku, zawsze spójny |
| Jedna funkcja per typ kontenera, albo szablon | Jedna funkcja obsługuje tablicę C, `array`, `vector`, wskaźnik |
| Podzakres = ręczna arytmetyka (`data + offset, len - offset`) | `subspan()`, `first()`, `last()` – czytelne i bezpieczne |
| Brak informacji o rozmiarze w typie | Opcjonalny statyczny rozmiar w typie (`span<T, N>`) sprawdzany w compile-time |

---

## ✅ Zadania

1. Napisz funkcję `uint32_t checksum(std::span<const uint8_t> data)`
   liczącą prostą sumę kontrolną (np. XOR wszystkich bajtów) i wywołaj
   ją na tablicy C, `std::array` i `std::vector` – bez zmiany sygnatury.
2. Napisz funkcję `void parse_packet(std::span<const uint8_t, 10> packet)`
   wymagającą dokładnie 10-bajtowego pakietu, wewnątrz podziel go na
   `header = first(2)`, `payload = subspan(2, 7)`, `crc = last(1)`.
3. Wyjaśnij własnymi słowami: dlaczego `std::span` **nie** rozwiązuje
   problemu "wiszącego wskaźnika" (dangling) – co się stanie, jeśli
   `span` przeżyje kontener, z którego powstał?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 4 – constexpr/consteval/constinit](chapter4_constexpr_consteval.md)
