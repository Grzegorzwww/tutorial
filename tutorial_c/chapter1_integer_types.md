# Chapter 1: Typy całkowite i ich pułapki

[← Spis treści](README.md)

**Cel rozdziału:** C ma zaskakująco skomplikowane reguły dla zwykłych
liczb całkowitych. Większość błędów w obliczeniach na rejestrach,
licznikach czasu i buforach wynika z trzech rzeczy: nieznanego rozmiaru
typu, promocji całkowitych i mieszania `signed` z `unsigned`.

---

## 1. Rozmiary – nie zgaduj, używaj `<stdint.h>`

Standard gwarantuje tylko minimalne zakresy: `int` ma **co najmniej**
16 bitów (na AVR ma 16, na ARM i x86 – 32), `long` co najmniej 32 (na
Linux x86-64 – 64, na Windows – 32).

```c
#include <stdint.h>
#include <stddef.h>

uint8_t  reg8;      /* dokładnie 8 bitów, bez znaku */
int16_t  temp;      /* dokładnie 16 bitów, ze znakiem */
uint32_t ticks;
size_t   len;       /* rozmiar obiektu / indeks tablicy (wynik sizeof) */
intptr_t addr;      /* liczba mieszcząca wskaźnik */

#include <inttypes.h>
printf("%" PRIu32 " %zu\n", ticks, len);   /* przenośne formaty printf */
```

**Zasada:** w kodzie embedded i protokołach używaj typów `uintN_t`/`intN_t`;
`int` zostaw dla małych liczników, gdzie rozmiar nie ma znaczenia.

## 2. Promocje całkowite (integer promotion)

Przed każdą operacją arytmetyczną typy **mniejsze niż `int`** (`char`,
`short`, `uint8_t`, `uint16_t` na platformie 32-bit) są zamieniane na
`int`.

```c
uint8_t a = 200, b = 100;
uint8_t sum = a + b;      /* a+b liczone jako int = 300, potem obcięte do 44 */
int     big = a + b;      /* 300 - nie ma przepełnienia! */

uint8_t x = 0x0F;
if (~x == 0xF0) { }       /* NIGDY prawda: ~x to ~(int)15 = 0xFFFFFFF0, nie 0xF0 */
if ((uint8_t)~x == 0xF0) { }   /* poprawnie */

uint16_t u = 0xFFFF;
uint32_t r = u << 16;     /* u -> int (ze znakiem!), 0xFFFF << 16 przepełnia int -> UB */
uint32_t ok = (uint32_t)u << 16;
```

## 3. Zwykłe konwersje arytmetyczne: `signed` vs `unsigned`

Gdy w jednym wyrażeniu spotkają się `int` i `unsigned int`, **`int`
jest zamieniany na `unsigned`**:

```c
int      a = -1;
unsigned b = 1;
if (a < b) { /* NIE wykona się: -1 staje się 0xFFFFFFFF */ }

size_t len = 0;
for (size_t i = len - 1; i >= 0; --i) { }   /* nieskończona pętla: size_t zawsze >= 0,
                                                a len - 1 = SIZE_MAX */
```

GCC ostrzega o tym przy `-Wextra` (`-Wsign-compare`). **Nie wyciszaj
tych ostrzeżeń rzutowaniem, dopóki nie wiesz, dlaczego się pojawiły.**

## 4. Przepełnienie: `unsigned` się zawija, `signed` to UB

```c
uint8_t  u = 255;  u++;          /* 0 - zdefiniowane: arytmetyka modulo 2^N */
int32_t  s = INT32_MAX;  s++;    /* NIEZDEFINIOWANE ZACHOWANIE */
```

UB oznacza, że kompilator **zakłada, że to się nigdy nie zdarzy** i
optymalizuje pod to założenie:

```c
int will_overflow(int x) {
    return x + 1 < x;    /* GCC -O2 kompiluje to do "return 0;" */
}
```

Poprawne sprawdzenie przepełnienia – **przed** operacją:
```c
#include <limits.h>
#include <stdbool.h>

bool safe_add(int a, int b, int *out) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) return false;
    *out = a + b;
    return true;
}
/* GCC/Clang: bool overflow = __builtin_add_overflow(a, b, &out); */
```

## 5. Liczniki czasu i zawijanie (klasyka embedded)

```c
uint32_t now = HAL_GetTick();
if (now - start >= timeout_ms) { }    /* POPRAWNIE nawet po przekręceniu licznika */
if (now >= start + timeout_ms) { }    /* BŁĄD w okolicy przepełnienia */
```
Odejmowanie `unsigned` liczy różnicę modulo 2^32, więc daje właściwy
wynik nawet, gdy `now` "przekręcił się" przez zero.

## 6. Rzutowania i obcinanie

```c
int32_t  big = 70000;
int16_t  small = (int16_t)big;   /* wartość spoza zakresu: implementation-defined */
uint16_t usmall = (uint16_t)big; /* zdefiniowane: 70000 mod 65536 = 4464 */

double d = 3.99;
int    i = (int)d;               /* 3 - obcięcie w stronę zera, nie zaokrąglenie */
int    bad = (int)1e20;          /* poza zakresem int: UB */
```

---

## ⚠️ Pułapki

- `char` może być `signed` (x86) lub `unsigned` (ARM) – nie używaj gołego
  `char` do danych binarnych, tylko `uint8_t`.
- `abs(INT_MIN)` to UB – `-INT_MIN` nie mieści się w `int`.
- `1 << 31` na 32-bitowym `int` to UB (przesunięcie do bitu znaku);
  pisz `1u << 31` albo `UINT32_C(1) << 31`.
- Przesunięcie o liczbę bitów `>=` szerokość typu (`x << 32` dla
  `uint32_t`) to UB – na ARM i x86 daje różne wyniki.
- `sizeof` zwraca `size_t` – porównania z `int` dają ostrzeżenia `-Wsign-compare`.

---

## ✅ Zadania

1. Sprawdź `sizeof(int)`, `sizeof(long)`, `sizeof(void*)` na Windows i
   Linuxie (lub BeagleBone) – czym się różnią?
2. Skompiluj funkcję `will_overflow` z `-O0` i `-O2`, obejrzyj asembler
   (`gcc -S`) – czy porównanie zniknęło?
3. Ćwiczenie praktyczne: **ex42** (bezpieczna arytmetyka, saturacja,
   porównania ze znakiem i bez).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 1a – Reprezentacja liczb i konwersje w bitach](chapter1a_integer_representation.md)
