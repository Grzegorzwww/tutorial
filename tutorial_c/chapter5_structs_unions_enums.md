# Chapter 5: `struct`, `union`, `enum`

[← Spis treści](README.md)

**Cel rozdziału:** zrozumieć układ struktur w pamięci (padding,
alignment), poprawnie zamieniać struktury na bajty i z powrotem, używać
unii z tagiem i flag bitowych w `enum`.

---

## 1. Padding i alignment

Procesor najszybciej (a na części ARM – w ogóle tylko) czyta `uint32_t`
spod adresu podzielnego przez 4. Kompilator dodaje więc niewidoczne
bajty wyrównania:

```c
#include <stddef.h>
#include <stdint.h>

struct Bad {          /* offset */
    uint8_t  a;       /* 0 */
                      /* 1..3 padding */
    uint32_t b;       /* 4 */
    uint8_t  c;       /* 8 */
                      /* 9..11 padding (rozmiar musi być wielokrotnością 4) */
};                    /* sizeof = 12 */

struct Good {
    uint32_t b;       /* 0 */
    uint8_t  a;       /* 4 */
    uint8_t  c;       /* 5 */
};                    /* sizeof = 8 */

_Static_assert(offsetof(struct Bad, b) == 4, "layout");
```
**Zasada:** układaj pola od największych do najmniejszych. W tablicy
1000 struktur oszczędzasz 4 KB RAM.

## 2. `packed` – kiedy i dlaczego ostrożnie

```c
struct __attribute__((packed)) Frame {   /* GCC/Clang; MSVC: #pragma pack(push, 1) */
    uint8_t  type;
    uint32_t value;   /* offset 1 - niewyrównany! */
};
```
- Odczyt niewyrównanego pola na Cortex-M0 → *HardFault*; na M3+ działa,
  ale wolniej.
- `uint32_t *p = &frame.value;` – wskaźnik niewyrównany, dalsze użycie to UB
  (GCC ostrzega: `-Waddress-of-packed-member`).
- Kolejność bajtów (endianness) i tak zależy od platformy.

## 3. Serializacja: nie wysyłaj struktury jako bajtów

```c
send(&frame, sizeof frame);   /* ŹLE: padding (losowe bajty), endianness, rozmiar typów */
```
Przenośnie – pole po polu, z jawną kolejnością bajtów:
```c
size_t frame_pack(const struct Frame *f, uint8_t *out) {
    out[0] = f->type;
    out[1] = (uint8_t)(f->value);         /* little-endian: najmłodszy bajt pierwszy */
    out[2] = (uint8_t)(f->value >> 8);
    out[3] = (uint8_t)(f->value >> 16);
    out[4] = (uint8_t)(f->value >> 24);
    return 5;
}

uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
```
Taki kod działa identycznie na każdej platformie i kompilatorze.

## 4. Inicjalizacja i przypisanie struktur

```c
struct Config cfg = { .baud = 115200, .parity = PARITY_NONE };   /* designated init (C99), reszta = 0 */
struct Config copy = cfg;          /* kopiowanie przez przypisanie - działa */
/* cfg == copy */                  /* BŁĄD kompilacji: C nie porównuje struktur */
memcmp(&cfg, &copy, sizeof cfg);   /* RYZYKOWNE: porównuje też śmieci w paddingu */
```
Porównywanie struktur – pole po polu, w osobnej funkcji `*_equals`.

## 5. Unia i unia z tagiem (tagged union)

Unia – wszystkie pola zajmują **tę samą** pamięć; rozmiar = największe pole.

```c
typedef enum { VAL_INT, VAL_FLOAT, VAL_STR } ValueType;

typedef struct {
    ValueType type;            /* "tag" - mówi, które pole unii jest aktywne */
    union {
        int32_t     i;
        float       f;
        const char *s;
    } as;
} Value;

void print_value(const Value *v) {
    switch (v->type) {
        case VAL_INT:   printf("%d\n", (int)v->as.i); break;
        case VAL_FLOAT: printf("%f\n", v->as.f); break;
        case VAL_STR:   printf("%s\n", v->as.s); break;
    }
}
```
To ręczna wersja `std::variant` z C++. Odczyt innego pola niż ostatnio
zapisane (type punning przez unię) jest w C dozwolony, ale wynik zależy
od reprezentacji bitowej – do podglądania bitów `float` lepiej użyć `memcpy`:

```c
uint32_t float_bits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof u);   /* przenośne; kompilator i tak zoptymalizuje do 1 instrukcji */
    return u;
}
```

## 6. `enum` i flagi bitowe

```c
typedef enum {
    PERM_READ  = 1u << 0,
    PERM_WRITE = 1u << 1,
    PERM_EXEC  = 1u << 2,
} Perm;

unsigned perms = PERM_READ | PERM_WRITE;
if (perms & PERM_WRITE) { }
perms &= ~PERM_WRITE;
```
Uwagi:
- W C `enum` to po prostu `int` – kompilator nie pilnuje zakresu, możesz
  przypisać dowolną liczbę.
- Kombinacja flag nie jest wartością enuma – zmienną trzymaj jako
  `unsigned` / `uint32_t`, nie jako `Perm`.
- `switch` po enumie bez `default` + `-Wswitch` = kompilator ostrzeże o
  nieobsłużonej wartości (warto!).

## 7. Pola bitowe (bit-fields)

```c
struct Status {
    unsigned ready : 1;
    unsigned error : 1;
    unsigned mode  : 3;
};
```
Wygodne, ale **kolejność bitów, padding i typ bazowy są
implementation-defined**. Nie używaj pól bitowych do mapowania rejestrów
sprzętowych ani formatów binarnych przesyłanych między platformami –
do tego służą maski i przesunięcia.

---

## ⚠️ Pułapki

- `sizeof(struct)` to nie suma rozmiarów pól.
- `memcmp` / `memcpy` struktury z paddingiem do pliku / sieci – przenosisz śmieci.
- Niezainicjalizowane pola lokalnej struktury (`struct S s;`) zawierają
  śmieci; `struct S s = {0};` zeruje wszystko.
- Unia bez tagu – nikt nie wie, które pole jest aktualne.

---

## ✅ Zadania

1. Wypisz `sizeof` i `offsetof` każdego pola dla `struct Bad` i
   `struct Good`. Spróbuj też na ARM (BeagleBone).
2. Zapisz `struct Bad` przez `memcpy` do tablicy bajtów zainicjalizowanej
   wartością `0xAA` i wypisz ją – które bajty to padding?
3. Ćwiczenia praktyczne: **ex49** (layout i serializacja ramki),
   **ex50** (unia z tagiem), **ex51** (flagi bitowe w `enum`).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 6 – Wskaźniki na funkcje](chapter6_function_pointers.md)
