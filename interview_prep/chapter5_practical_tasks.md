# Chapter 5: Zadania praktyczne

[← Spis treści](README.md)

Konkretne zadania kodowe, jakie może dać rekruter na rozmowę o
stanowisko Embedded C++ / Embedded Linux Engineer – od klasyków
low-level po zadania bardziej systemowe. Rozwiązuj **z klawiaturą w
ręku**. Rozwiązania/hinty są pod każdym zadaniem – zakryj je najpierw.

---

## Zadanie 1: Bufor cykliczny (ring buffer)

**Treść:** Zaimplementuj bufor cykliczny o stałym rozmiarze
(np. do buforowania bajtów z przerwania UART) z operacjami `push`,
`pop`, `is_full`, `is_empty`. Bez alokacji dynamicznej.

```cpp
template<typename T, size_t N>
class RingBuffer {
public:
    bool push(const T& item);
    bool pop(T& out);
    bool is_full() const;
    bool is_empty() const;
private:
    T buffer_[N];
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};
```

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
template<typename T, size_t N>
bool RingBuffer<T, N>::push(const T& item) {
    if (is_full()) return false;
    buffer_[head_] = item;
    head_ = (head_ + 1) % N;
    ++count_;
    return true;
}

template<typename T, size_t N>
bool RingBuffer<T, N>::pop(T& out) {
    if (is_empty()) return false;
    out = buffer_[tail_];
    tail_ = (tail_ + 1) % N;
    --count_;
    return true;
}

template<typename T, size_t N>
bool RingBuffer<T, N>::is_full() const { return count_ == N; }

template<typename T, size_t N>
bool RingBuffer<T, N>::is_empty() const { return count_ == 0; }
```

**Pytanie dodatkowe rekrutera:** "A co jeśli `push` dzieje się w ISR, a
`pop` w pętli głównej – co trzeba zmienić?" Odpowiedź: `count_` musi
być modyfikowany atomowo/z wyłączonymi przerwaniami przy dostępie z
obu kontekstów, albo użyj osobnych wskaźników head/tail bez wspólnego
licznika (klasyczny lock-free single-producer/single-consumer ring
buffer – `is_full`/`is_empty` liczone z `head_`/`tail_` bezpośrednio).
</details>

---

## Zadanie 2: Odwracanie listy jednokierunkowej

**Treść:** Odwróć listę jednokierunkową iteracyjnie (bez rekurencji –
w embedded rekurencja o nieznanej głębokości to ryzyko przepełnienia
stosu).

```cpp
struct Node {
    int value;
    Node* next;
};

Node* reverse(Node* head);
```

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
Node* reverse(Node* head) {
    Node* prev = nullptr;
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;   // zapamiętaj przed nadpisaniem
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;   // nowa głowa listy
}
```
Złożoność: O(n) czasowa, O(1) pamięciowa (iteracyjnie – ważne w
embedded, gdzie rekurencyjna wersja O(n) na stosie byłaby problemem).
</details>

---

## Zadanie 3: Bit manipulation – ustawianie/czyszczenie/sprawdzanie bitu w rejestrze

**Treść:** Napisz funkcje do ustawiania, czyszczenia, przełączania
(toggle) i sprawdzania konkretnego bitu w rejestrze 32-bit, bez
wpływania na pozostałe bity.

```cpp
void set_bit(volatile uint32_t& reg, uint8_t bit_num);
void clear_bit(volatile uint32_t& reg, uint8_t bit_num);
void toggle_bit(volatile uint32_t& reg, uint8_t bit_num);
bool check_bit(uint32_t reg, uint8_t bit_num);
```

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
void set_bit(volatile uint32_t& reg, uint8_t bit_num) {
    reg |= (1u << bit_num);
}
void clear_bit(volatile uint32_t& reg, uint8_t bit_num) {
    reg &= ~(1u << bit_num);
}
void toggle_bit(volatile uint32_t& reg, uint8_t bit_num) {
    reg ^= (1u << bit_num);
}
bool check_bit(uint32_t reg, uint8_t bit_num) {
    return (reg & (1u << bit_num)) != 0;
}
```
**Pytanie dodatkowe:** "Co się stanie, jeśli `set_bit` i `clear_bit`
na tym samym rejestrze są wywoływane z różnych kontekstów (main loop i
ISR) bez ochrony?" Odpowiedź: operacja `reg |= ...` to w rzeczywistości
odczyt-modyfikacja-zapis (read-modify-write) – **nie jest atomowa**.
Przerwanie między odczytem a zapisem może spowodować utratę zmiany z
drugiego kontekstu (race condition). Rozwiązanie: sekcja krytyczna
(wyłączenie przerwań na czas operacji) albo instrukcje bitband/atomic
set/clear jeśli dostępne na danej architekturze.
</details>

---

## Zadanie 4: Endianness – konwersja i wykrywanie

**Treść:** Napisz funkcję zamieniającą kolejność bajtów 32-bitowej
liczby (big-endian ↔ little-endian) bez używania funkcji bibliotecznych
typu `ntohl`. Napisz też funkcję wykrywającą endianness bieżącej
platformy w runtime.

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
uint32_t swap_endian(uint32_t val) {
    return ((val & 0x000000FF) << 24) |
           ((val & 0x0000FF00) << 8)  |
           ((val & 0x00FF0000) >> 8)  |
           ((val & 0xFF000000) >> 24);
}

bool is_little_endian() {
    uint32_t test = 1;
    return *reinterpret_cast<uint8_t*>(&test) == 1;
    // Alternatywa C++20 bez UB: std::endian::native == std::endian::little (patrz <bit>)
}
```
**Kontekst embedded:** to realny problem przy parsowaniu ramek
protokołów sieciowych (big-endian, "network byte order") na
mikrokontrolerze little-endian (typowe ARM Cortex-M) – MODBUS TCP i
wiele innych protokołów przemysłowych używa big-endian.
</details>

---

## Zadanie 5: Naprawa race condition (code review)

**Treść:** Poniższy kod ma bug związany z wielowątkowością/przerwaniami.
Znajdź go i zaproponuj poprawkę.

```cpp
bool data_ready = false;
uint8_t shared_buffer[64];

// wywoływane z ISR:
void on_uart_rx_complete() {
    // ... kopiowanie danych do shared_buffer ...
    data_ready = true;
}

// wywoływane w pętli głównej:
void main_loop() {
    while (true) {
        if (data_ready) {
            process(shared_buffer);
            data_ready = false;
        }
    }
}
```

<details>
<summary>Hint / rozwiązanie</summary>

**Bug:** `data_ready` nie jest `volatile` – kompilator (przy
optymalizacji) może "podnieść" odczyt `data_ready` poza pętlę
`while(true)`, zakładając że nic go nie zmienia (z jego punktu
widzenia w tej funkcji nic go nie modyfikuje), co skutkuje pętlą,
która nigdy nie zauważy zmiany ustawionej w ISR.

**Poprawka:**
```cpp
volatile bool data_ready = false;
```
**Dodatkowy problem do wychwycenia:** nawet z `volatile`, jeśli
`process()` trwa długo i w międzyczasie nadejdzie kolejne przerwanie
UART nadpisujące `shared_buffer` w trakcie przetwarzania – wyścig
danych na samym buforze, nie tylko na fladze. W realnym kodzie
potrzebny podwójny bufor (double buffering) albo wyłączenie przerwań
UART na czas kopiowania danych z bufora do przetworzenia.
</details>

---

## Zadanie 6: Prosty parser protokołu (ramka z sumą kontrolną)

**Treść:** Zaimplementuj parser prostej ramki: `[0xAA][LEN][DATA...][CRC]`,
gdzie `LEN` to długość `DATA` w bajtach, `CRC` to XOR wszystkich
bajtów `DATA`. Funkcja powinna zwrócić `std::optional<std::vector<uint8_t>>`
z danymi, albo `std::nullopt` jeśli ramka jest niepoprawna (zły
nagłówek albo zła suma kontrolna).

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
#include <optional>
#include <vector>
#include <cstdint>
#include <span>

std::optional<std::vector<uint8_t>> parse_frame(std::span<const uint8_t> raw) {
    if (raw.size() < 3) return std::nullopt;              // min: header + len + crc
    if (raw[0] != 0xAA) return std::nullopt;               // zły nagłówek

    uint8_t len = raw[1];
    if (raw.size() != static_cast<size_t>(2 + len + 1)) return std::nullopt;  // niezgodna długość

    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; ++i) {
        crc ^= raw[2 + i];
    }
    if (crc != raw[2 + len]) return std::nullopt;          // zła suma kontrolna

    return std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 2 + len);
}
```
To dobre zadanie do pokazania znajomości `std::span`/`std::optional`
(C++17/20) – jeśli rekruter zapyta "a jak zrobiłbyś to w C++11?",
odpowiedź: `const uint8_t* + size_t` zamiast `span`, kod błędu/wskaźnik
`nullptr` zamiast `optional`.
</details>

---

## Zadanie 7 (systemowe/Linux): Znajdź proces zużywający najwięcej pamięci

**Treść:** Napisz skrypt bash, który wypisze 5 procesów zużywających
najwięcej pamięci RSS.

<details>
<summary>Hint / rozwiązanie</summary>

```bash
ps aux --sort=-%mem | head -6
```
Albo bez polegania na formacie `ps aux` (bardziej przenośnie):
```bash
ps -eo pid,comm,rss --sort=-rss | head -6
```
**Pytanie dodatkowe:** "Co to jest RSS a co VSZ?" – RSS (Resident Set
Size) to faktycznie zajmowana fizyczna pamięć RAM; VSZ (Virtual Size)
to cała zarezerwowana przestrzeń adresowa procesu (może być znacznie
większa niż faktycznie używana fizyczna pamięć, np. przez mapowania
bibliotek współdzielonych).
</details>

---

## Zadanie 8 (live coding, whiteboard-style): Debounce przycisku

**Treść:** Napisz funkcję `bool button_pressed()` wywoływaną cyklicznie
(np. co 10ms z timera), która zwraca `true` dokładnie raz na jedno
fizyczne naciśnięcie przycisku, z uwzględnieniem drgań styków
(debounce), bez blokowania (`sleep`).

<details>
<summary>Hint / rozwiązanie</summary>

```cpp
bool button_pressed() {
    static uint8_t stable_count = 0;
    static bool last_reported_state = false;
    constexpr uint8_t DEBOUNCE_THRESHOLD = 5;   // 5 * 10ms = 50ms stabilizacji

    bool raw_state = read_gpio_pin();   // true = wciśnięty

    if (raw_state == last_reported_state) {
        stable_count = 0;
        return false;
    }

    ++stable_count;
    if (stable_count >= DEBOUNCE_THRESHOLD) {
        last_reported_state = raw_state;
        stable_count = 0;
        return raw_state;   // zdarzenie tylko przy przejściu w stan "wciśnięty"
    }
    return false;
}
```
Kluczowa idea: licz kolejne, **spójne** próbki zamiast reagować na
pierwszą zmianę – dopiero po utrzymaniu się nowego stanu przez X
kolejnych odczytów uznajemy zmianę za prawdziwą.
</details>

---

## Wskazówki ogólne do live-codingu na rozmowie

1. **Mów na głos, co myślisz** – rekruterzy oceniają proces myślenia,
   nie tylko finalny kod.
2. **Pytaj o brzegowe przypadki** zanim zaczniesz pisać (pusty bufor?
   `nullptr`? maksymalny rozmiar?) – pokazuje dojrzałość inżynierską.
3. **Nazywaj złożoność** (Big-O) swojego rozwiązania, nawet jeśli nikt
   nie pyta wprost.
4. Jeśli utkniesz – zacznij od najgłupszego, działającego rozwiązania
   (brute force), potem optymalizuj. Lepsze działające O(n²) niż
   niedokończone O(n).

---

**Następny rozdział:** [Chapter 6 – Pytania sytuacyjne i jak mówić o CV](chapter6_behavioral_and_cv.md)
