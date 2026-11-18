# C++11 → C++20

Tech talk: co realnie zmieniło się w C++ od C++11 i co z tego daje nam w kodzie (także embedded).

> Wszystkie przykłady C++20 były kompilowane przez GCC 15 z `-std=c++20 -Wall -Wextra -Wpedantic`
> (fragmenty C++11 z `-std=c++11`). Sprawdź wsparcie w swoim toolchainie na
> [cppreference – compiler support](https://en.cppreference.com/w/cpp/compiler_support).

## Plan

| # | Temat | Standard | Koszt wdrożenia |
|--:|---|:---:|:---:|
| 1 | `if constexpr` | C++17 | niski |
| 2 | Structured bindings | C++17 | niski |
| 3 | `std::optional` | C++17 | niski |
| 4 | `std::variant` | C++17 | niski/średni |
| 5 | Concepts | C++20 | średni |
| 6 | `constexpr`, `consteval`, `constinit` | C++14 → C++20 | średni |
| 7 | `std::span` | C++20 | niski |
| 8 | Designated initializers | C++20 | niski |
| 9 | Operator `<=>` | C++20 | niski |
| 10 | Ranges | C++20 | średni |
| 11 | `<bit>` | C++20 | niski |
| 12 | `std::jthread` | C++20 | średni |
| 13 | Coroutines | C++20 | wysoki |

Rozdziały 1–4 to C++17. Bez nich nie da się sensownie opowiadać o C++20, a w wielu projektach „C++11”
oznacza po prostu kod, który tych rzeczy jeszcze nie używa.

---

# Część I: fundament z C++17

## 1. `if constexpr`: kod wybierany podczas kompilacji (C++17)

### C++11

```cpp
// C++11
template <typename T>
typename std::enable_if<std::is_enum<T>::value,
                        typename std::underlying_type<T>::type>::type
to_raw(T value)
{
    return static_cast<typename std::underlying_type<T>::type>(value);
}

template <typename T>
typename std::enable_if<!std::is_enum<T>::value, T>::type
to_raw(T value)
{
    return value;
}
```

### C++17

```cpp
// C++17
template <typename T>
auto to_raw(T value)
{
    if constexpr (std::is_enum_v<T>) {
        return static_cast<std::underlying_type_t<T>>(value);
    } else {
        return value;
    }
}

enum class Mode : std::uint8_t { Off, On };

auto a = to_raw(Mode::On);   // std::uint8_t
auto b = to_raw(5);          // int
```

**Jak to działa?**  
Warunek `if constexpr` musi być znany w czasie kompilacji. Gałąź, która nie jest wybrana, **nie jest
instancjonowana** dla danego `T`. Dlatego `std::underlying_type_t<T>` nie powoduje błędu dla `int`,
choć dla `int` nie ma sensu. W zwykłym `if` ta gałąź i tak musiałaby się skompilować. Typ zwracany
(`auto`) jest wyprowadzany tylko z gałęzi, która została wybrana.

**Dlaczego lepiej?**
- jedna funkcja zamiast dwóch przeciążeń wykluczających się przez `enable_if`,
- logika czyta się od góry do dołu jak zwykły kod,
- niewybrana gałąź nie trafia do pliku wynikowego.

**Embedded:** typowe zastosowanie to jeden szablon do zapisu pól rejestrów, które mają różne typy (`enum class`,
liczby 8-, 16-, 32-bitowe).

**Uwaga:** `if constexpr` działa tylko w szablonie i tylko dla warunku znanego w czasie kompilacji.

---

## 2. Structured bindings: rozpakowanie wyniku (C++17)

### C++11

```cpp
// C++11
Status status;                // zmienne muszą istnieć wcześniej
std::uint32_t value;
std::tie(status, value) = read_value();
```

### C++17

```cpp
// C++17
std::pair<Status, std::uint32_t> read_value();

const auto [status, value] = read_value();
```

```cpp
struct Point { int x; int y; };

Point p{1, 2};
const auto& [x, y] = p;       // x i y odnoszą się do p.x i p.y, bez kopii

std::map<int, std::string> names{{1, "uart"}};
for (const auto& [id, name] : names) {   // zamiast it->first / it->second
    use(id, name);
}
```

**Jak to działa?**  
Kompilator wprowadza nazwy dla elementów pary, krotki, tablicy lub struktury z publicznymi polami.
`const auto&` oznacza, że nazwy są referencjami do istniejącego obiektu. Z samym `auto` obiekt
jest kopiowany.

**Dlaczego lepiej?**
- zmienne mogą być `const` i nie trzeba ich wcześniej deklarować,
- nazwy `status` i `value` zamiast `.first` i `.second`,
- działa z własnymi strukturami, więc funkcja może zwrócić wiele wartości bez parametrów wyjściowych (`T*`).

**Embedded:** zwracanie `{status, dane}` zamiast wskaźnika wyjściowego (`Status read(uint32_t* out)`).

**Uwaga:** w C++17 nie można przechwytywać takich nazw w lambdzie. C++20 to dopuszcza.

---

## 3. `std::optional`: brak wyniku bez wartości magicznej (C++17)

### C++11

```cpp



// C++11
int find_channel(std::uint16_t id)
{
    return id == 42 ? 3 : -1;     // -1 oznacza "brak"
}



```

### C++17

```cpp



// C++17
std::optional<std::size_t> find_channel(std::uint16_t id)
{
    if (id == 42) {
        return 3;
    }
    return std::nullopt;
}
if (const auto channel = find_channel(42)) {
    enable_channel(*channel);
}
const std::size_t ch = find_channel(7).value_or(0);   // wartość domyślna



```

**Jak to działa?**  
`std::optional<T>` przechowuje albo wartość typu `T`, albo informację, że wartości nie ma.
Wartość leży **wewnątrz** obiektu. Nie ma alokacji na stercie. Rozmiar to mniej więcej
`sizeof(T)` plus flaga (z wyrównaniem).

**Dlaczego lepiej?**
- cała dziedzina `T` zostaje dostępna dla poprawnych wartości, bo „brak” nie zajmuje liczby takiej jak `-1` czy `0xFFFF`,
- sygnatura funkcji mówi, że wyniku może nie być, a `if (auto x = ...)` ułatwia sprawdzenie,
- nie wymaga wyjątków ani `new`.

**Embedded:** zamiennik `-1`, `0xFFFF`, `nullptr` czy osobnego parametru `bool* ok`.

**Uwagi:**
- `*opt` na pustym `optional` to UB, więc najpierw sprawdź, czy wartość istnieje,
- `opt.value()` rzuca `std::bad_optional_access`, więc przy `-fno-exceptions` należy go unikać.

---

## 4. `std::variant`: bezpieczna unia (C++17)

### C++11

```cpp


// C++11
struct SensorState {
    enum class Kind : std::uint8_t { Idle, Reading, Fault } kind;
    union {
        std::uint16_t raw;     // ważne tylko dla Reading
        std::uint8_t  code;    // ważne tylko dla Fault
    };
};



```

### C++17

```cpp



// C++17
struct Idle {};
struct Reading { std::uint16_t raw; };
struct Fault   { std::uint8_t code; };

using SensorState = std::variant<Idle, Reading, Fault>;

template <typename... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

void report(const SensorState& state)
{
    std::visit(overloaded{
        [](Idle)             { std::puts("idle"); },
        [](const Reading& r) { std::printf("raw=%u\n", unsigned{r.raw}); },
        [](const Fault& f)   { std::printf("fault=%u\n", unsigned{f.code}); },
    }, state);
}

SensorState s = Reading{512};
if (const auto* r = std::get_if<Reading>(&s)) {   // nullptr, gdy aktywny jest inny typ
    use(r->raw);
}



```

**Jak to działa?**  
`std::variant` przechowuje dokładnie jeden z podanych typów i pamięta, który jest aktywny.
Dane leżą w obiekcie (bez sterty), a rozmiar to największa alternatywa plus indeks.
`std::visit` wywołuje właściwą lambdę. `overloaded` to powszechny pomocnik, który skleja
kilka lambd w jeden obiekt wywoływalny. W C++17 potrzebuje jeszcze deduction guide, a w C++20 już nie.

**Dlaczego lepiej?**
- w wersji z `union` łatwo odczytać pole, które nie jest aktywne (UB), a tu `get_if` zwróci `nullptr`,
- stan i dane to jeden obiekt, więc nie da się ich rozsynchronizować,
- `visit` wymaga obsługi **wszystkich** alternatyw. Dodanie nowego stanu spowoduje błąd kompilacji
  w każdym miejscu, które go nie obsługuje. Przy `switch` na `enum` łatwo to przeoczyć.

**Embedded:** maszyny stanów, zdarzenia z różnymi danymi, wynik operacji `Ok | Error`.

**Uwaga:** `std::get<T>` przy złym typie rzuca wyjątek, więc przy `-fno-exceptions` używaj `get_if` lub `visit`.

---

# Część II: C++20

## 5. Concepts: wymagania na typy w interfejsie (C++20)

### C++11

```cpp


// C++11
template <typename T,
          typename = typename std::enable_if<
              std::is_unsigned<T>::value && (sizeof(T) <= 4)>::type>
void write_register(volatile T* reg, T value);



```

### C++20

```cpp


// C++20
template <typename T>
concept RegisterValue =
    std::unsigned_integral<T> && !std::same_as<T, bool> && (sizeof(T) <= 4);
template <RegisterValue T>
void write_register(volatile T* reg, T value)
{
    *reg = value;
}
volatile std::uint32_t reg = 0;
write_register(&reg, std::uint32_t{1});    // OK
// write_register(&reg, -1);               // błąd 



```

**Jak to działa?**  
`concept` to nazwany warunek sprawdzany w czasie kompilacji. Kompilator sprawdza go przed wejściem
do ciała szablonu. Komunikat wskazuje, **który** warunek nie jest spełniony (`requires unsigned_integral<T>`),
a nie dziesiątki linii z wnętrza `<type_traits>`.

> `std::unsigned_integral<bool>` jest prawdziwe, bo `bool` to typ całkowity bez znaku.
> Dlatego jawnie go wykluczamy.

### Concept jako interfejs sterownika

```cpp
// C++20
template <typename T>
concept ByteSink = requires(T t, std::span<const std::uint8_t> data) {
    { t.write(data) } -> std::same_as<bool>;
};

bool send_frame(ByteSink auto& sink, std::span<const std::uint8_t> frame)
{
    return sink.write(frame);
}

struct Uart { bool write(std::span<const std::uint8_t>) { return true; } };

Uart uart;
send_frame(uart, frame);          // OK
// send_frame(42, frame);         // błąd: int nie spełnia ByteSink
```

**Dlaczego lepiej?**
- wymagania są widoczne w sygnaturze i mają nazwę,
- komunikaty błędów są krótkie,
- concepts można łączyć (`&&`, `||`), co w `enable_if` szybko robi się nieczytelne,
- `ByteSink` działa jak interfejs, ale **bez `virtual`**: bez vtable i bez wywołania przez wskaźnik, a wywołanie można zinlinować.

**Embedded:** typ `Uart`, `Spi` i `MockUart` w testach mogą spełniać ten sam kontrakt bez wspólnej klasy bazowej.

**Uwaga:** concept sprawdza **typ**, a nie wartość. Zakres argumentu (np. numer kanału) nadal trzeba sprawdzić osobno.

---

## 6. `constexpr`, `consteval`, `constinit`: więcej pracy przed uruchomieniem (C++11 → C++20)

### C++11

```cpp



// C++11: funkcja constexpr to praktycznie jedno wyrażenie return
constexpr unsigned factorial(unsigned n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}



```

### C++17 / C++20

```cpp
// C++17: pętle i zmienne lokalne (od C++14), constexpr std::array (od C++17)



constexpr auto make_lookup()
{
    std::array<std::uint16_t, 8> table{};
    for (unsigned i = 0; i < table.size(); ++i) {
        table[i] = static_cast<std::uint16_t>(i * i);
    }
    return table;
}

constexpr auto lookup = make_lookup();
static_assert(lookup[3] == 9);          // sprawdzone przez kompilator


```

```cpp
// C++20: consteval – wywołanie MUSI być wykonane w czasie kompilacji


void invalid_baud_rate();               // celowo bez definicji i bez constexpr

consteval std::uint32_t baud_divisor(std::uint32_t clk, std::uint32_t baud)
{
    if (baud == 0) {
        invalid_baud_rate();            // dojście tutaj = błąd kompilacji
    }
    return clk / baud;
}
constexpr auto divisor = baud_divisor(48'000'000, 115'200);   // 416
// constexpr auto bad  = baud_divisor(48'000'000, 0);         // błąd kompilacji
// auto x = baud_divisor(48'000'000, runtime_baud);           // błąd: wartość nie jest stałą



```

```cpp
// C++20: constinit – zmienna modyfikowalna, ale inicjalizowana stałą
constinit std::uint32_t boot_counter = baud_divisor(10, 2);
```

**Jak to działa?**

| Słowo | Znaczenie |
|---|---|
| `constexpr` | może być wyliczone w czasie kompilacji (jeśli argumenty są stałe), ale może też działać w runtime |
| `consteval` | **musi** być wyliczone w czasie kompilacji (każde wywołanie z wartością z runtime'u to błąd) |
| `constinit` | gwarantuje inicjalizację stałą, ale zmienna pozostaje modyfikowalna |

C++20 dokłada jeszcze m.in. `constexpr` `std::vector`/`std::string` (w obrębie obliczenia w czasie kompilacji),
`virtual` w `constexpr` i `std::is_constant_evaluated()`.

**Dlaczego lepiej?**
- błędna konfiguracja (np. dzielnik baudrate) wychodzi **przy kompilacji**, a nie na sprzęcie,
- tablice lookup liczy kompilator, zamiast generatorów w osobnym skrypcie lub kodu uruchamianego przy starcie,
- `constinit` usuwa problem kolejności inicjalizacji obiektów globalnych (*static initialization order fiasco*).

**Embedded:** tablice CRC/sinus, dzielniki zegarów, rozmiary buforów, walidacja konfiguracji pinów.

**Uwagi:**
- zamiast `throw` użyliśmy wywołania niekonstexprowej funkcji, bo `throw` nie kompiluje się z `-fno-exceptions` (typowe dla embedded),
- `constexpr` tablica zwykle trafia do pamięci Flash (`.rodata`), ale zależy to od toolchaina. Warto sprawdzić plik map.

---

## 7. `std::span`: wskaźnik i rozmiar jako jeden obiekt (C++20)

### C++11

```cpp



// C++11
void send(const std::uint8_t* data, std::size_t size);

void handler(std::uint8_t* packet)
{
    send(packet, sizeof(packet));   // BŁĄD: sizeof wskaźnika (4 lub 8), nie bufora
}


```

### C++20

```cpp


// C++20
void send(std::span<const std::uint8_t> data);

std::uint8_t packet[8]{};
send(packet);                            // rozmiar wyprowadzony z typu tablicy
send(std::span(packet).first(4));        // tylko pierwsze 4 bajty

std::array<std::uint8_t, 8> arr{};
std::vector<std::uint8_t> vec(8);
send(arr);                               // ta sama funkcja przyjmie array...
send(vec);                               // ...i vector



```

```cpp



void parse(std::span<const std::uint8_t> frame)
{
    auto header  = frame.first(2);       // widoki na fragmenty tej samej pamięci
    auto payload = frame.subspan(2);
}


```

```cpp


// Rozmiar znany w czasie kompilacji
void send4(std::span<const std::uint8_t, 4> data);

std::array<std::uint8_t, 4> four{};
send4(four);                             // OK
// send4(arr);                           // błąd kompilacji: array<..., 8> to nie 4 bajty



```

**Jak to działa?**  
`std::span<T>` to lekki, **niewłasnościowy** widok na ciągły fragment pamięci: wskaźnik plus rozmiar
(przy rozmiarze statycznym zwykle sam wskaźnik). Nie kopiuje danych ani nie alokuje.
`span<const T>` oznacza dostęp tylko do odczytu.

**Dlaczego lepiej?**
- dwa powiązane parametry (`ptr`, `size`) to jeden obiekt, więc nie da się ich pomylić ani rozsynchronizować,
- jedna funkcja przyjmuje tablicę C, `std::array` i `std::vector`,
- `first`, `last` i `subspan` ułatwiają parsowanie ramek bez ręcznej arytmetyki na wskaźnikach,
- koszt jest taki jak przy przekazaniu wskaźnika i rozmiaru.

**Embedded:** granica API sterownika (UART, SPI, DMA), parsery ramek, bufory odbiorcze.

**Uwagi:**
- `span` **nie przedłuża życia** bufora. Widok na zmienną lokalną nie może jej przeżyć,
- `operator[]` domyślnie **nie sprawdza zakresu**. `span` daje jawny rozmiar, ale nie jest automatycznym zabezpieczeniem.

---

## 8. Designated initializers: pola z nazwami (C++20)

### C++11

```cpp
// C++11
struct UartConfig {
    std::uint32_t baud;
    std::uint8_t  data_bits;
    bool          parity;
    std::uint8_t  stop_bits;
};

UartConfig cfg = {115200, 8, false, 1};     // co oznacza false? a 1?
```

### C++20

```cpp
// C++20
struct UartConfig {
    std::uint32_t baud      = 115'200;
    std::uint8_t  data_bits = 8;
    bool          parity    = false;
    std::uint8_t  stop_bits = 1;
};

UartConfig a{.baud = 9'600, .parity = true};                  // reszta: wartości domyślne
UartConfig b{.baud = 115'200, .data_bits = 8, .parity = false, .stop_bits = 2};
```

**Jak to działa?**  
W inicjalizacji agregatu można wskazać pola po nazwie (`.pole = wartość`). Pominięte pola dostają
wartość domyślną z deklaracji lub są inicjalizowane zerem. Domyślne wartości pól agregatu (`= 115'200`) są dozwolone od C++14.

**Reguły (to nie jest pełna swoboda jak w C99):**
- kolejność musi odpowiadać deklaracji (`{.stop_bits = 2, .baud = 9'600}` to błąd kompilacji),
- nie można mieszać zapisu pozycyjnego i nazwanego,
- działa dla agregatów (struktury bez konstruktorów, bez ukrytych pól).

**Dlaczego lepiej?**
- kod mówi, co ustawia, bez zaglądania do definicji struktury,
- dodanie pola w środku struktury nie zmienia znaczenia istniejących inicjalizacji (przy zapisie pozycyjnym mogłoby to po cichu przesunąć wartości),
- pomijanie pól daje kontrolowane wartości domyślne.

**Embedded:** struktury konfiguracyjne peryferiów (UART, SPI, timer, GPIO) i deskryptory, np. tablice pinów lub zadań.

---

## 9. Operator `<=>`: porównania bez boilerplate'u (C++20)

### C++11

```cpp
// C++11: ręcznie napisane operatory (tu tylko dwa z sześciu)
struct Version {
    std::uint8_t major, minor, patch;
};

inline bool operator==(const Version& a, const Version& b)
{
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

inline bool operator<(const Version& a, const Version& b)
{
    return std::tie(a.major, a.minor, a.patch) <
           std::tie(b.major, b.minor, b.patch);
}
// ...plus !=, >, <=, >=
```

### C++20

```cpp
// C++20
struct Version {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t patch;

    auto operator<=>(const Version&) const = default;
};

static_assert(Version{1, 2, 3} <  Version{1, 3, 0});
static_assert(Version{1, 2, 3} == Version{1, 2, 3});
static_assert(Version{2, 0, 0} >= Version{1, 9, 9});
```

**Jak to działa?**  
`= default` generuje porównanie **po kolei po polach, w kolejności deklaracji** (porządek leksykograficzny).
Kompilator dodatkowo deklaruje `==`, a `<`, `>`, `<=`, `>=` i `!=` są wyprowadzane z wyniku `<=>` (a `!=` z `==`).
Typ wyniku (`strong_ordering`, `partial_ordering` dla `float`) jest dobierany automatycznie.

**Dlaczego lepiej?**
- jedna linia zamiast sześciu operatorów,
- kod nie rozjedzie się przy dodaniu pola. W ręcznym `operator<` łatwo zapomnieć o nowym polu,
- gotowe na `constexpr`, więc działa też w `static_assert`.

**Embedded:** wersje firmware i protokołów, klucze w tablicach, kolejność zdarzeń lub zadań.

**Uwaga:** kolejność pól w strukturze **jest** kolejnością porównania. Zmiana kolejności pól zmienia wynik.

---

## 10. Ranges: opisujemy operację, nie pętlę (C++20)

### C++11

```cpp
// C++11
for (int v : samples) {
    if (v >= threshold) {
        process(v * 2);
    }
}
```

### C++20

```cpp
// C++20
std::array<int, 8> samples{3, 12, 7, 20, 1, 15, 9, 30};
const int threshold = 10;

auto output = samples
    | std::views::filter([threshold](int v) { return v >= threshold; })
    | std::views::transform([](int v) { return v * 2; });

for (int v : output) {
    process(v);
}
```

```cpp
// Algorytmy przyjmują cały zakres, nie parę iteratorów
std::ranges::sort(samples);                  // zamiast sort(begin, end)
auto it = std::ranges::find(samples, 20);

for (int v : std::views::iota(0, 5)) { }     // 0,1,2,3,4
for (int v : samples | std::views::take(3)) { }   // pierwsze 3 elementy
```

**Jak to działa?**  
Widok (`view`) to lekki opis operacji na zakresie. `filter` i `transform` są **leniwe**: nic się
nie liczy, dopóki nie zaczniesz iterować, a każdy element przechodzi przez cały potok po kolei.
Nie powstaje pośredni kontener. Operator `|` łączy kolejne etapy.

**Dlaczego lepiej?**
- kod opisuje „co” (filtruj, potem przemnóż), a nie „jak” (indeksy, `if`, kontener tymczasowy),
- brak pośrednich buforów, które w embedded kosztują RAM,
- algorytmy `std::ranges::*` nie wymagają pary `begin/end`, a mają bardziej czytelne komunikaty błędów (concepts).

**Embedded:** filtrowanie i skalowanie próbek z ADC, przetwarzanie ramek.

**Uwagi:**
- widok **nie jest właścicielem** danych źródłowych. Widok na zmienną lokalną nie może jej przeżyć,
- lambda przechwycona przez widok też musi przeżyć widok (`[threshold]` kopiuje wartość),
- szablony zwiększają czas kompilacji, a niekiedy rozmiar kodu. Prosta pętla bywa lepsza, więc **mierz**.

---

## 11. `<bit>`: standardowe operacje bitowe (C++20)

### C++11

```cpp
// C++11
bool one_bit = value != 0 && (value & (value - 1)) == 0;   // potęga dwójki

std::uint32_t raw;
std::memcpy(&raw, &f, sizeof raw);                         // bity float jako uint32_t
// albo __builtin_popcount(...) / __builtin_clz(...), czyli rozszerzenia GCC/Clang
```

### C++20

```cpp
// C++20
constexpr std::uint32_t value = 0b0010'1100;

static_assert(!std::has_single_bit(value));     // czy to potęga dwójki
static_assert(std::popcount(value) == 3);       // liczba ustawionych bitów
static_assert(std::bit_width(value) == 6);      // ile bitów zajmuje liczba
static_assert(std::countr_zero(value) == 2);    // zera od LSB
static_assert(std::countl_zero(value) == 26);   // zera od MSB
static_assert(std::bit_ceil(value) == 64);      // najbliższa potęga 2 >= value
static_assert(std::rotl(std::uint8_t{0b1000'0001}, 1) == 0b0000'0011);
```

```cpp
// bit_cast: reinterpretacja bitów, także w czasie kompilacji
constexpr float temperature = 23.5f;
constexpr auto raw = std::bit_cast<std::uint32_t>(temperature);
static_assert(raw == 0x41BC0000u);

if constexpr (std::endian::native == std::endian::little) {
    // kolejność bajtów znana w czasie kompilacji
}
```

**Jak to działa?**  
Nagłówek `<bit>` daje standardowe odpowiedniki operacji, które pisaliśmy ręcznie albo przez
rozszerzenia kompilatora. `std::bit_cast<To>(from)` kopiuje wzorzec bitowy między typami o tym samym rozmiarze,
które są trywialnie kopiowalne. Warunki są sprawdzane przez kompilator, a wynik można liczyć w `constexpr`.

**Dlaczego lepiej?**
- nazwa funkcji opisuje intencję (`has_single_bit`, a nie `x & (x - 1)`),
- przenośność: ten sam kod w GCC, Clang i MSVC,
- `bit_cast` zastępuje `memcpy`, `union` i `reinterpret_cast` (ten ostatni to UB przez złamanie *strict aliasing*),
- `std::endian` pozwala wybrać kolejność bajtów w czasie kompilacji (przydatne w protokołach).

**Embedded:** analiza flag i rejestrów, kodowanie protokołów, wyrównania buforów, zamiana `float` ↔ `uint32_t`.

**Uwagi:**
- funkcje takie jak `has_single_bit` przyjmują **tylko typy całkowite bez znaku** (`has_single_bit(8)` na `int` się nie skompiluje),
- na rdzeniach z odpowiednimi instrukcjami (np. CLZ, ROR) kompilator **może** je użyć, ale na prostszych rdzeniach wynik to wywołanie funkcji bibliotecznej. Mierz.

---

## 12. `std::jthread`: wątek z automatycznym `join` (C++20)

### C++11

```cpp
// C++11
std::thread worker(run_worker);

do_something_that_may_throw();   // wyjątek => ~thread() => std::terminate()

worker.join();                   // trzeba pamiętać; ta linia może nigdy nie zostać wykonana
```

### C++20

```cpp
// C++20
void run_worker(std::stop_token stop)
{
    while (!stop.stop_requested()) {
        poll_device();
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

int main()
{
    std::jthread worker(run_worker);    // stop_token trafia do funkcji automatycznie
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
}   // destruktor: request_stop() + join()
```

**Jak to działa?**  
`std::jthread` w destruktorze robi `request_stop()`, a następnie `join()`. Jeśli funkcja wątku przyjmuje
`std::stop_token` jako pierwszy argument, dostaje go automatycznie. Zatrzymanie jest **kooperacyjne**:
wątek sam sprawdza `stop_requested()` i się kończy. Nic nie jest przerywane siłą.

**Dlaczego lepiej?**
- nie da się „zapomnieć o `join`”: zniszczenie niedołączonego `std::thread` kończy program przez `std::terminate`,
- bezpieczne przy wyjątkach i wcześniejszym `return`, bo działa RAII,
- standardowy mechanizm zatrzymywania zamiast własnej `std::atomic<bool> stop_flag`.

**Embedded:** głównie **Embedded Linux** i RTOS ze wsparciem `std::thread`. Na bare-metal (np. `arm-none-eabi`)
biblioteka zwykle nie udostępnia `<thread>`. Wtedy zostaje natywne API RTOS-a, np. FreeRTOS.

**Uwaga:** `jthread` nie zastępuje projektu synchronizacji (muteksy, kolejki, priorytety).

---

## 13. Coroutines: asynchroniczny przepływ zapisany liniowo (C++20)

### Przed: callbacki

```cpp
// C++11: kolejne kroki zagnieżdżają się w callbackach
void uart_read(void (*on_byte)(std::uint8_t));

void read_frame()
{
    uart_read([](std::uint8_t start) {
        if (start != 0x7E) {
            return;
        }
        uart_read([](std::uint8_t length) {
            std::printf("frame length=%u\n", unsigned{length});
        });
    });
}
```

### C++20

```cpp
// C++20
Task read_frame()
{
    const auto start = co_await NextByte{};      // zawieś się do czasu nadejścia bajtu
    if (start != 0x7E) {
        co_return;
    }
    const auto length = co_await NextByte{};     // zawieś się ponownie
    std::printf("frame length=%u\n", unsigned{length});
}

// Wywołanie z pętli głównej lub dyspozytora zdarzeń
read_frame();
on_byte(0x7E);   // coroutine rusza dalej
on_byte(5);      // wypisze: frame length=5
```

**Elementy, które trzeba dostarczyć (biblioteka standardowa ich nie ma):**

```cpp
// Typ zwracany przez coroutine
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }   // startuje od razu
        std::suspend_never final_suspend() noexcept { return {}; }     // sprząta po zakończeniu
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

// Obiekt, na który czekamy
struct NextByte {
    static inline std::coroutine_handle<> waiting{};
    static inline std::uint8_t last = 0;

    bool await_ready() const noexcept { return false; }              // czy mamy już wynik?
    void await_suspend(std::coroutine_handle<> h) noexcept { waiting = h; }   // zapamiętaj, kogo wznowić
    std::uint8_t await_resume() const noexcept { return last; }      // wynik dla co_await
};

void on_byte(std::uint8_t b)         // wywołać, gdy dane dotrą
{
    NextByte::last = b;
    if (auto h = std::exchange(NextByte::waiting, {})) {
        h.resume();
    }
}
```

**Jak to działa?**  
`co_await` zawiesza funkcję i oddaje sterowanie wywołującemu. Gdy wynik jest dostępny, ktoś
wywołuje `resume()`, a funkcja wraca dokładnie w to miejsce, z zachowanymi zmiennymi lokalnymi.
Kompilator przekształca coroutine w automat stanów. `promise_type` mówi, jak coroutine startuje, kończy
i co zwraca, a *awaiter* (`await_ready`, `await_suspend`, `await_resume`) opisuje, na co czekamy.

**Dlaczego lepiej?**
- przepływ czyta się od góry do dołu, jak kod synchroniczny, bez zagnieżdżonych callbacków,
- zmienne lokalne żyją między zawieszeniami, więc nie trzeba ręcznie utrzymywać struktury stanu,
- protokoły wieloetapowe (handshake, parsowanie ramek, sekwencje komend) mają prostszy kod niż ręczny automat stanów.

**Embedded:** sekwencje komunikacyjne, parsowanie strumienia bajtów, obsługa timeoutów, a w Embedded Linux sterowniki asynchroniczne.

**Uwagi (najważniejsze dla embedded):**
- ramka coroutine (zmienne lokalne i stan) musi gdzieś leżeć. Domyślnie to `operator new`,
  kompilator czasem to eliduje, ale nie jest to gwarantowane. Można dostarczyć `promise_type::operator new`
  z własnej puli statycznej,
- trzeba samemu zdecydować, **kto** wznawia coroutine. Tu robi to pętla główna, a nie przerwanie (ISR powinno być krótkie),
- przykład ma jednego oczekującego (`static waiting`). Prawdziwa biblioteka musi to uogólnić,
- w wielu zespołach proste automaty stanów pozostają bardziej przewidywalne. Warto sięgnąć po gotowy framework zamiast pisać własny.

---

# Podsumowanie

## Co wdrażać najpierw

| Priorytet | Funkcja | Dlaczego |
|:---:|---|---|
| 1 | `std::span` | od razu poprawia API, bardzo mały koszt |
| 1 | `std::optional` | usuwa wartości magiczne |
| 1 | designated initializers | czytelne struktury konfiguracyjne |
| 1 | `<bit>` | standardowe operacje bitowe zamiast makr i rozszerzeń |
| 2 | `constexpr` / `consteval` + `static_assert` | błędy konfiguracji w czasie kompilacji |
| 2 | concepts | czytelne kontrakty szablonów |
| 2 | `std::variant` | bezpieczniejsze stany i zdarzenia |
| 2 | `<=>` | mniej boilerplate'u |
| 3 | ranges | gdy poprawia czytelność, ale **mierz** rozmiar i czas kompilacji |
| 3 | `jthread` | tylko tam, gdzie jest OS ze wsparciem wątków |
| 3 | coroutines | gdy mamy framework i kontrolę nad pamięcią |

## Wspólny mianownik

Prawie każda z tych zmian robi jedno z trzech:

1. **przesuwa błędy z runtime'u do kompilacji** (`consteval`, concepts, `span` ze stałym rozmiarem, `visit`),
2. **zastępuje konwencje typami** (`optional` zamiast `-1`, `span` zamiast `ptr + size`, `variant` zamiast `union + enum`),
3. **usuwa boilerplate** (`<=>`, designated initializers, structured bindings).

> Nie chodzi o używanie nowej składni wszędzie, lecz o to, żeby kontrakt kodu był widoczny dla kompilatora
> i dla człowieka. W embedded zawsze sprawdź też rozmiar kodu, zużycie RAM i wsparcie toolchaina.

## Pytania do dyskusji

1. Gdzie w naszym kodzie funkcje przyjmują parę `(wskaźnik, rozmiar)`?
2. Gdzie „brak wyniku” jest oznaczony wartością magiczną (`-1`, `0xFFFF`, `nullptr`)?
3. Jakie konfiguracje (baudrate, rozmiary buforów, piny) moglibyśmy sprawdzać w `static_assert`?
4. Czy nasz toolchain wspiera wszystko, czego potrzebujemy (kompilator **i** biblioteka standardowa)?
