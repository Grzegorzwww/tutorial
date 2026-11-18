# Chapter 7: Preprocesor

[← Spis treści](README.md)

**Cel rozdziału:** preprocesor to prosty mechanizm podmiany tekstu
uruchamiany **przed** kompilacją. Nie zna typów, zasięgów ani priorytetów
operatorów – stąd jego siła i najbardziej podstępne błędy. Rozdział
pokazuje, jak pisać makra bezpiecznie i kiedy w ogóle ich nie pisać.

---

## 1. Podstawy

```c
#include <stdio.h>      /* nagłówek systemowy (ścieżki -I i systemowe) */
#include "uart.h"       /* nagłówek projektu (najpierw katalog bieżący) */

#define BUFFER_SIZE 64
#define DEBUG 1

#if DEBUG
#  define LOG(msg) printf("[%s:%d] %s\n", __FILE__, __LINE__, msg)
#else
#  define LOG(msg) ((void)0)
#endif
```
Podgląd wyniku preprocesora: `gcc -E plik.c | less`.

## 2. Nawiasy, nawiasy, nawiasy

```c
#define SQUARE_BAD(x) x * x
SQUARE_BAD(1 + 2)        /* 1 + 2 * 1 + 2 = 5, nie 9 */

#define SQUARE(x) ((x) * (x))   /* każdy argument i całe wyrażenie w nawiasach */
```

## 3. Wielokrotne obliczanie argumentów

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int i = 5;
int m = MAX(i++, 3);     /* i++ wykona się DWA razy: i == 7, m == 6 */
```
Makro nie jest funkcją – argument jest wklejany tyle razy, ile występuje.
Jeśli to możliwe, używaj **`static inline` funkcji** – są tak samo
szybkie, a mają sprawdzanie typów i normalną semantykę argumentów:
```c
static inline int max_int(int a, int b) { return a > b ? a : b; }
```

## 4. Makra wielolinijkowe: `do { ... } while (0)`

```c
#define SWAP_BAD(a, b) int t = a; a = b; b = t;

if (x > y) SWAP_BAD(x, y);   /* tylko pierwsza instrukcja jest pod if-em! */

#define SWAP(a, b) do { int swap_tmp_ = (a); (a) = (b); (b) = swap_tmp_; } while (0)

if (x > y) SWAP(x, y);       /* jedna instrukcja; średnik na końcu pasuje naturalnie */
else       do_other();      /* z samymi klamrami {} tu byłby błąd składni */
```

## 5. `#` i `##` – stringifikacja i sklejanie

```c
#define STR(x)  #x               /* STR(foo) -> "foo" */
#define XSTR(x) STR(x)           /* najpierw rozwija makro, potem robi napis */
#define VERSION 42
STR(VERSION)                     /* "VERSION" */
XSTR(VERSION)                    /* "42" */

#define CONCAT(a, b) a##b
int CONCAT(reg_, ctrl) = 0;      /* int reg_ctrl = 0; */

#define ASSERT_EQ(a, b) \
    do { if ((a) != (b)) printf("FAIL: %s != %s\n", #a, #b); } while (0)
```

## 6. Przydatne makra standardowe w projektach

```c
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(n)        (1u << (n))
#define UNUSED(x)     ((void)(x))
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

_Static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");   /* C11, w czasie kompilacji */
```

## 7. X-macros – jedna lista, wiele wygenerowanych struktur

Problem: masz enum kodów błędów i tablicę ich nazw. Dodanie kodu wymaga
edycji w dwóch miejscach – i prędzej czy później się rozjadą.

```c
#define ERROR_LIST(X)                  \
    X(ERR_OK,      "ok")               \
    X(ERR_TIMEOUT, "timeout")          \
    X(ERR_CRC,     "crc mismatch")     \
    X(ERR_NOMEM,   "out of memory")

typedef enum {
#define X(code, text) code,
    ERROR_LIST(X)
#undef X
    ERR_COUNT                           /* liczba kodów - automatycznie */
} ErrorCode;

static const char *const error_names[] = {
#define X(code, text) [code] = text,
    ERROR_LIST(X)
#undef X
};

_Static_assert(ARRAY_SIZE(error_names) == ERR_COUNT, "names out of sync");
```
Dodajesz jedną linijkę w `ERROR_LIST` – enum, tablica napisów i licznik
aktualizują się same. Tak buduje się tablice komend, rejestrów, stanów
automatu czy pól konfiguracji.

## 8. Kompilacja warunkowa – z umiarem

```c
#if defined(STM32F4)
#  include "stm32f4xx.h"
#elif defined(__linux__)
#  include "linux_hal.h"
#else
#  error "Unsupported platform"
#endif
```
Dużo `#ifdef` w środku logiki = kod, którego część nigdy nie jest
kompilowana w Twojej konfiguracji (i się psuje). Lepiej: osobne pliki
`.c` per platforma z tym samym nagłówkiem, wybierane w systemie budowania.

---

## ⚠️ Pułapki

- Brak nawiasów wokół argumentów i całego wyrażenia.
- Argumenty z efektami ubocznymi (`i++`, wywołania funkcji) w makrach.
- Średnik na końcu `#define` (`#define N 10;`) – wkleja się wszędzie.
- Makro zasłania funkcję o tej samej nazwie (`#define min ...` psuje
  nagłówki C++ i Windows).
- Nazwy zaczynające się od `_` + wielka litera lub `__` są zarezerwowane
  dla implementacji – nie używaj ich w include guardach (`_UART_H_` ✗, `UART_H` ✓).

---

## ✅ Zadania

1. Uruchom `gcc -E` na pliku z X-macro i zobacz, co wygenerował preprocesor.
2. Napisz makro `LOG_ERR(fmt, ...)` dopisujące `__FILE__:__LINE__` i
   używające `__VA_ARGS__`.
3. Ćwiczenia praktyczne: **ex54** (bezpieczne makra), **ex55** (X-macros).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 8 – UB, volatile, restrict](chapter8_ub_volatile.md)
