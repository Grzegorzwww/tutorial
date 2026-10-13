# Chapter 4: `constexpr` / `consteval` / `constinit`

[← Spis treści](README.md)

W C++11 `constexpr` istniało, ale było mocno ograniczone: funkcja
`constexpr` mogła mieć w praktyce tylko jedno wyrażenie `return`
(żadnych pętli, żadnych lokalnych zmiennych z przypisaniem). C++14
poluzowało te zasady (pętle, wiele instrukcji, lokalne zmienne), a
C++20 idzie znacznie dalej i dodaje dwa nowe słowa kluczowe:
`consteval` i `constinit`.

---

## 1. Przypomnienie: jak dziś wygląda `constexpr` (dzięki C++14/17/20)

```cpp
constexpr int factorial(int n) {
    int result = 1;               // C++11: NIEDOZWOLONE - lokalna zmienna z pętlą
    for (int i = 2; i <= n; ++i) { // C++11: NIEDOZWOLONE - pętla
        result *= i;
    }
    return result;
}

static_assert(factorial(5) == 120);   // policzone w czasie kompilacji, zero kosztu w runtime
```
W C++11 ta sama funkcja musiałaby być zapisana rekurencyjnie przez
jedno wyrażenie: `return n <= 1 ? 1 : n * factorial(n - 1);` – działa,
ale znacznie mniej czytelne dla bardziej złożonej logiki.

**Kluczowa cecha `constexpr`:** funkcja **może** być wykonana w czasie
kompilacji (jeśli argumenty są znane w compile-time), ale **może też**
być wywołana normalnie w runtime (jeśli argumenty są dynamiczne) –
kompilator sam decyduje.

```cpp
int runtime_n = get_from_uart();      // wartość nieznana w compile-time
int result = factorial(runtime_n);    // ta sama funkcja - wykona się w runtime, normalnie
```

## 2. C++20: `constexpr` obejmuje więcej – wirtualne funkcje, `try/catch`, `std::string`/`std::vector`

```cpp
struct Shape {
    constexpr virtual double area() const = 0;   // C++11/17: wirtualne NIE mogło być constexpr
    constexpr virtual ~Shape() = default;
};

struct Circle : Shape {
    constexpr Circle(double r) : radius(r) {}
    constexpr double area() const override { return 3.14159 * radius * radius; }
    double radius;
};

constexpr double compute() {
    Circle c(2.0);
    return c.area();   // polimorfizm wykonany w compile-time!
}

static_assert(compute() > 12.0);
```

`std::string` i `std::vector` mają teraz `constexpr` konstruktory i
metody (od C++20) – można ich **ograniczenie** używać w kodzie
`constexpr`, o ile cały obiekt jest w pełni skonstruowany i zniszczony
w obrębie tej samej ewaluacji compile-time (nie da się "wynieść"
`constexpr std::vector` jako stałą globalną z alokacją na stercie –
alokacja musi zniknąć przed końcem oceny w compile-time).

## 3. `consteval` – funkcja, która **musi** wykonać się w compile-time

```cpp
consteval int square(int x) {
    return x * x;
}

constexpr int a = square(5);   // OK - wartość znana w compile-time

int runtime_val = get_from_uart();
// int b = square(runtime_val);   // BŁĄD KOMPILACJI - consteval wymaga compile-time ZAWSZE
```
Różnica względem `constexpr`: `consteval` (tzw. "immediate function")
**gwarantuje**, że funkcja nigdy nie trafi do binarki jako kod
wykonywany w runtime – jeśli nie da się jej policzyć w compile-time,
dostajesz błąd kompilacji, a nie "po cichu" wolniejszy kod w runtime.

> 💡 **Praktyczne zastosowanie w embedded:** wymuszenie, żeby np.
> tablica przeliczeniowa (lookup table) do kalibracji ADC albo tabela
> CRC była **zawsze** obliczona w compile-time i wylądowała w pamięci
> Flash/ROM jako stała, nigdy nie licząc się od nowa przy starcie
> urządzenia.

```cpp
consteval uint32_t crc_table_entry(uint8_t index) {
    uint32_t crc = index;
    for (int i = 0; i < 8; ++i) {
        crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
    }
    return crc;
}

constexpr std::array<uint32_t, 256> make_crc_table() {
    std::array<uint32_t, 256> table{};
    for (int i = 0; i < 256; ++i) {
        table[i] = crc_table_entry(static_cast<uint8_t>(i));
    }
    return table;
}

constexpr auto CRC_TABLE = make_crc_table();   // cała tabela 256 elementów w compile-time!
```

## 4. `constinit` – gwarancja inicjalizacji statycznej (bez "static init order fiasco")

Problem, który rozwiązuje: zmienne globalne z nietrywialnym
konstruktorem są normalnie inicjalizowane **dynamicznie**, przy
starcie programu, w nieokreślonej kolejności między jednostkami
kompilacji (tzw. *static initialization order fiasco* – klasyczny,
podstępny bug, gdy jedna zmienna globalna używa innej, jeszcze
niezainicjalizowanej).

```cpp
// źle (potencjalny fiasco) - inicjalizacja dynamiczna w runtime, kolejność niepewna:
std::string g_prefix = compute_prefix();

// C++20 - constinit WYMUSZA inicjalizację statyczną (w compile-time), błąd kompilacji jeśli się nie da:
constinit int g_counter = 100;

constexpr const char* compute_name() { return "sensor_01"; }
constinit const char* g_name = compute_name();   // OK - obliczalne w compile-time
```
> ⚠️ `constinit` **nie** oznacza `const` – zmienna nadal może być
> modyfikowana w runtime później. Gwarantuje tylko sposób
> *inicjalizacji* (statyczny, przed uruchomieniem `main()`, bez
> zależności od kolejności innych zmiennych globalnych).

```cpp
constinit int g_boot_counter = 0;   // OK, wartość początkowa policzona statycznie

void on_boot() {
    g_boot_counter++;   // OK - można modyfikować w runtime, to nie jest const
}
```

---

## 🔧 Różnica względem C++11

| C++11 | C++20 |
|---|---|
| `constexpr` = tylko jedno wyrażenie `return` | `constexpr` = pętle, lokalne zmienne, `try/catch`, wirtualne funkcje |
| Brak sposobu, by *wymusić* compile-time (funkcja mogła "po cichu" wykonać się w runtime) | `consteval` gwarantuje wykonanie wyłącznie w compile-time |
| Zmienne globalne z konstruktorem = ryzyko "static init order fiasco" | `constinit` wymusza bezpieczną inicjalizację statyczną, sprawdzaną przez kompilator |

---

## ✅ Zadania

1. Napisz `consteval` funkcję `is_power_of_two(unsigned n)` i użyj jej
   do `static_assert` na kilku wartościach. Spróbuj wywołać ją z
   wartością znaną dopiero w runtime (np. wczytaną z `std::cin`) –
   zaobserwuj błąd kompilacji.
2. Zbuduj `constexpr` tablicę przeliczeniową `sin()` dla kątów
   0-90 stopni (np. 91-elementowa tablica `float`), obliczoną w
   compile-time (podpowiedź: własna implementacja szeregu Taylora,
   bo `std::sin` z `<cmath>` nie jest `constexpr` w C++20).
3. Zamień jakąś swoją globalną zmienną z nietrywialnym konstruktorem
   (albo napisz przykład) na `constinit` – sprawdź czy kompilator to
   zaakceptuje, czy zgłosi błąd (bo inicjalizacja nie jest obliczalna
   statycznie).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 5 – Nowa składnia: `<=>`, designated initializers, template lambdas](chapter5_comparisons_and_syntax.md)
