# Chapter 11: C++ w Zephyrze

[← Spis treści](README.md)

**Cel rozdziału:** używać C++ (także C++17/20) w aplikacji Zephyra na
NUCLEO-H743ZI: włączyć odpowiednią bibliotekę, napisać RAII dla muteksu
jądra i wykorzystać `std::span`, `std::optional`, concepts i `constexpr` bez
wyjątków i RTTI.

> Ten rozdział łączy kurs Zephyra z tech talkiem
> [C++11 → C++20](../TOTORIAL.MD). Tam znajdziesz uzasadnienie, dlaczego
> `span`, `optional`, concepts czy `constexpr` warto stosować w embedded. Tu
> sprawdzamy, jak je uruchomić na Zephyrze.

---

## 1. Domyślnie: C++11 i minimalna biblioteka

Po włączeniu `CONFIG_CPP=y` dostajesz **C++11** (domyślny wybór standardu) i
**minimalną bibliotekę C++**, bez STL. Żeby korzystać z `<optional>`, `<span>`,
`<array>`, `<concepts>` itd., włącz:

```
# prj.conf
CONFIG_CPP=y
CONFIG_STD_CPP20=y                 # wybór standardu (STD_CPP11/14/17/20/23)
CONFIG_REQUIRES_FULL_LIBCPP=y      # pełna biblioteka standardowa C++ (libstdc++)
```

Z `.config` zbudowanego przykładu:

```
CONFIG_STD_CPP_VERSION=202002      # = __cplusplus dla C++20
CONFIG_GLIBCXX_LIBCPP=y            # libstdc++ z toolchaina (GCC 14.3 w SDK 1.0.1)
CONFIG_PICOLIBC=y                  # biblioteka C
# CONFIG_CPP_EXCEPTIONS is not set # wyjątki wyłączone
# CONFIG_CPP_RTTI is not set       # RTTI wyłączone
```

Kompilator dostaje flagi `-std=c++20 -fno-exceptions -fno-rtti`.

### Typowy błąd: `<span>` bez pełnej biblioteki

Z samym `CONFIG_CPP=y` i `CONFIG_STD_CPP20=y`:

```
fatal error: span: No such file or directory
```

Standard jest ustawiony poprawnie, ale **minimalna** biblioteka C++ nie ma
nagłówka. Rozwiązanie: `CONFIG_REQUIRES_FULL_LIBCPP=y`.

## 2. Pełny przykład

```c++
#include <zephyr/kernel.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>

/* RAII dla k_mutex: odpowiednik std::lock_guard. */
class MutexGuard {
public:
	explicit MutexGuard(k_mutex &m) : m_(m)
	{
		k_mutex_lock(&m_, K_FOREVER);
	}
	~MutexGuard()
	{
		k_mutex_unlock(&m_);
	}
	MutexGuard(const MutexGuard &) = delete;
	MutexGuard &operator=(const MutexGuard &) = delete;

private:
	k_mutex &m_;
};

K_MUTEX_DEFINE(counter_lock);
static int counter;

static void increment()
{
	MutexGuard guard(counter_lock);	/* odblokowanie nastąpi przy każdym wyjściu z funkcji */
	++counter;
}

/* C++17/20: brak wyniku jako std::optional, bufor jako std::span. */
static std::optional<std::uint8_t> hex_digit(char c)
{
	if (c >= '0' && c <= '9') {
		return static_cast<std::uint8_t>(c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return static_cast<std::uint8_t>(c - 'a' + 10);
	}
	return std::nullopt;
}

static std::uint32_t checksum(std::span<const std::uint8_t> data)
{
	std::uint32_t sum = 0;

	for (const auto b : data) {
		sum += b;
	}
	return sum;
}

template <std::unsigned_integral T>
constexpr T double_it(T value)
{
	return static_cast<T>(value * 2);
}
static_assert(double_it(std::uint8_t{21}) == 42);

int main()
{
	increment();
	increment();
	printk("counter = %d, __cplusplus = %ld\n", counter, static_cast<long>(__cplusplus));

	if (const auto d = hex_digit('b')) {
		printk("hex_digit('b') = %u\n", static_cast<unsigned>(*d));
	}
	printk("hex_digit('z') ma wartosc: %d\n", hex_digit('z').has_value());

	std::array<std::uint8_t, 4> frame{1, 2, 3, 4};
	printk("checksum = %u, pierwsze 2 bajty = %u\n", static_cast<unsigned>(checksum(frame)),
	       static_cast<unsigned>(checksum(std::span(frame).first(2))));
	return 0;
}
```

Plik nazwij `main.cpp` i wskaż go w `CMakeLists.txt`:

```cmake
target_sources(app PRIVATE src/main.cpp)
```

Wynik (QEMU, ten sam kod zbudował się też dla `nucleo_h743zi`):

```
counter = 2, __cplusplus = 202002
hex_digit('b') = 11
hex_digit('z') ma wartosc: 0
checksum = 10, pierwsze 2 bajty = 3
```

Co tu jest warte uwagi:

| Element | Komentarz |
|---|---|
| `K_MUTEX_DEFINE` w C++ | Makra Zephyra działają w pliku `.cpp` |
| `MutexGuard` | RAII: nie zapomnisz odblokować muteksu, także przy wcześniejszym `return` |
| `std::optional` | Zamiast wartości magicznych. Bez alokacji i bez wyjątków |
| `std::span` | Widok na bufor (`std::array`, tablica) zamiast pary wskaźnik + rozmiar |
| `std::unsigned_integral` | Concept z C++20 ogranicza szablon do typów całkowitych bez znaku |
| `static_assert` + `constexpr` | Sprawdzenie w czasie kompilacji |
| `int main()` | `main` w C++ wystarczy bez `extern "C"` |

### Koszt

| Build dla `nucleo_h743zi` | Flash | RAM |
|---|---:|---:|
| Blinky w C (z `printf`) | 20 996 B | 4 672 B |
| Przykład C++20 powyżej (tylko `printk`) | 21 700 B | 4 672 B |

Funkcje C++20 z przykładu (optional, span, concepts, constexpr) to w
dużej mierze koszty w czasie kompilacji, a nie w czasie działania, i w
tym małym programie nie widać znaczącego narzutu. **Uwaga:** to nie jest
porównanie 1:1 (blinky używa cięższego `printf`, przykład C++ tylko `printk`)
i dotyczy jednego małego programu. Przy swojej bazie kodu mierz własny wynik
(`rom_report`, Chapter 10).

## 3. Co wyłączone, a co ważne w embedded

- **Wyjątki i RTTI są domyślnie wyłączone** (`-fno-exceptions -fno-rtti`).
  `std::optional::value()` i `std::get<T>` rzucają wyjątki, więc
  używaj `*opt` po sprawdzeniu, `value_or()` lub `std::get_if`.
- **Sterta jądra jest wyłączona** (`CONFIG_HEAP_MEM_POOL_SIZE=0` na tej płytce).
  Dynamiczna alokacja wymaga jawnej konfiguracji, a w embedded i tak
  preferuj alokację statyczną (obiekty globalne, `std::array`).
- **Obiekty globalne i `static`** są konstruowane przy starcie systemu, **jeszcze
  przed bannerem Zephyra i przed `main()`**. Sprawdzony wynik dla globalnego
  obiektu z konstruktorem drukującym tekst:

  ```
  konstruktor obiektu globalnego
  *** Booting Zephyr OS build v4.5.0-2813-gf8da8ea37050 ***
  main
  ```

  Konstruktor działa więc zanim system jest w pełni uruchomiony, a to ogranicza,
  co można w nim bezpiecznie robić (np. nie wywołuj w nim sterowników, które
  jeszcze się nie zainicjowały). Uważaj też na kolejność inicjalizacji
  obiektów między plikami.
- W tym kursie do synchronizacji używamy prymitywów Zephyra (`k_mutex`,
  `k_sem`) w opakowaniu RAII, jak `MutexGuard`.

## 4. Kiedy C, a kiedy C++

| Użyj C++ | Zostań przy C |
|---|---|
| Logika aplikacji, parsery, algorytmy (`span`, `optional`, concepts) | Cienka warstwa nad sterownikami i makrami Zephyra |
| RAII dla zasobów (mutex, blokady, GPIO) | Kod współdzielony z innymi projektami w C |
| Szablony i `constexpr` do generowania tablic i kontroli w kompilacji | Gdy zespół i narzędzia (np. analiza statyczna) wymagają C |

## 🔧 Różnica względem FreeRTOS + C++

| FreeRTOS + C++ | Zephyr + C++ |
|---|---|
| Sam wybierasz flagi kompilatora i bibliotekę | `CONFIG_CPP`, `CONFIG_STD_CPP20`, `CONFIG_REQUIRES_FULL_LIBCPP` |
| Wyjątki/RTTI zależą od Twojego Makefile/CMake | Domyślnie wyłączone (`-fno-exceptions -fno-rtti`) |
| Opakowania RAII pisane od zera | Te same, ale nad `k_mutex`/`k_sem` |
| STL zależy od toolchaina | Pełna biblioteka dostępna po jednej opcji |

---

## ✅ Zadania

1. Zbuduj przykład C++ dla `nucleo_h743zi`. Zmień standard na `CONFIG_STD_CPP17=y`.
   Co przestaje się kompilować i dlaczego (podpowiedź: `<span>`, concepts)?
2. Usuń `CONFIG_REQUIRES_FULL_LIBCPP=y` i zapisz błąd. Co mówi o wyborze biblioteki?
3. Dodaj do `MutexGuard` wersję z timeoutem (`k_mutex_lock(&m, K_MSEC(10))`)
   i metodę `bool locked() const`.
4. Napisz klasę `Led` opakowującą `gpio_dt_spec` (konstruktor sprawdza
   `gpio_is_ready_dt`, metody `on()`, `off()`, `toggle()`).
5. Dodaj do przykładu `std::span<const uint8_t>` parsera ramki z nagłówkiem
   i sumą kontrolną (zobacz sekcję o `span` w [tech talku](../TOTORIAL.MD)).
6. Porównaj rozmiar Flash blinky w C i jego wersji w C++ (`main.cpp`).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie kursu

Po ukończeniu wszystkich rozdziałów powinieneś:
- [ ] Umieć zbudować, wgrać i uruchomić aplikację na NUCLEO-H743ZI (`west`)
- [ ] Rozumieć trzy filary: CMake, Kconfig, Devicetree
- [ ] Czytać DTS płytki, pisać overlay i znać aliasy `led0`, `sw0`, `pwm-led0`
- [ ] Konfigurować funkcje przez `prj.conf` i rozpoznawać błędy Kconfig
- [ ] Tworzyć wątki i rozumieć **odwrócone** priorytety względem FreeRTOS
- [ ] Wybierać mutex, semafor, `k_msgq`, `k_fifo`, `k_event` do właściwego zadania
- [ ] Stosować wzorzec *deferred processing* (`k_work`) zamiast pracy w ISR
- [ ] Używać logowania i shella do diagnostyki
- [ ] Korzystać z peryferiów przez device model i dodać czujnik przez DT
- [ ] Diagnozować przepełnienie stosu i czytać raport awarii
- [ ] Używać C++20 w aplikacji Zephyra

**Gratulacje.** Naturalny następny krok: zbudować mały projekt łączący
wątek czujnika, `k_msgq`, przetwarzanie w `k_work` i komendy shella na
prawdziwej płytce.
