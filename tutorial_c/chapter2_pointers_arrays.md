# Chapter 2: Wskaźniki i tablice

[← Spis treści](README.md)

**Cel rozdziału:** uporządkować relację tablica–wskaźnik, czytać
deklaracje z `const`, pracować z tablicami wielowymiarowymi i
projektować funkcje z parametrami wyjściowymi.

---

## 1. Tablica to nie wskaźnik – ale "rozpada się" do wskaźnika

```c
int arr[5] = {1, 2, 3, 4, 5};
int *p = arr;                 /* arr "rozpada się" (decay) do &arr[0] */

sizeof(arr);                  /* 20 - rozmiar całej tablicy */
sizeof(p);                    /* 8 (lub 4) - rozmiar wskaźnika */

p[2] == *(p + 2);             /* indeksowanie to arytmetyka wskaźników */
arr + 1;                      /* adres arr[1] (przesunięcie o sizeof(int) bajtów) */
&arr + 1;                     /* adres ZA całą tablicą (przesunięcie o 20 bajtów) */
```

W parametrze funkcji tablica **zawsze** jest wskaźnikiem – rozmiar ginie:
```c
void f(int a[5]) {            /* to jest dokładnie: void f(int *a) */
    sizeof(a);                /* 8 - NIE 20! GCC ostrzeże: -Wsizeof-array-argument */
}
```
Dlatego w C funkcja przyjmuje **wskaźnik + długość**:
```c
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))   /* działa tylko na prawdziwej tablicy */
int sum(const int *data, size_t n);
sum(arr, ARRAY_SIZE(arr));
```

## 2. `const` przy wskaźnikach – czytaj od prawej do lewej

```c
const int *p1;        /* wskaźnik na const int: nie zmienisz *p1, możesz zmienić p1 */
int const *p2;        /* to samo co wyżej */
int *const p3 = &x;   /* const wskaźnik na int: zmienisz *p3, nie zmienisz p3 */
const int *const p4 = &x;   /* nic nie zmienisz */

/* embedded: rejestr pod stałym adresem */
volatile uint32_t *const GPIOA_ODR = (volatile uint32_t *)0x48000014u;
```

**Zasada API:** parametr, którego funkcja nie modyfikuje, deklaruj jako
`const T *`. To dokumentacja + ochrona + pozwala przekazać dane z Flash.

## 3. Parametry wyjściowe i zwracanie wielu wartości

```c
#include <stdbool.h>

/* zwraca status, wyniki przez wskaźniki */
bool min_max(const int *data, size_t n, int *out_min, int *out_max) {
    if (n == 0) return false;
    *out_min = *out_max = data[0];
    for (size_t i = 1; i < n; ++i) {
        if (data[i] < *out_min) *out_min = data[i];
        if (data[i] > *out_max) *out_max = data[i];
    }
    return true;
}
```
Alternatywa: zwracanie struktury (`struct MinMax { int min, max; }`) –
w C struktury przekazuje się i zwraca **przez wartość**, co dla małych
struktur jest czytelniejsze.

## 4. Wskaźnik do wskaźnika

Potrzebny, gdy funkcja ma **zmienić sam wskaźnik** wywołującego:
```c
#include <stdlib.h>

int alloc_buffer(uint8_t **out, size_t n) {
    *out = malloc(n);
    return *out ? 0 : -1;
}

uint8_t *buf = NULL;
alloc_buffer(&buf, 64);
```
Oraz tablica napisów: `const char *names[] = {"a", "b"};` ma typ
`const char *[2]`, a w funkcji: `const char *const *names`.

## 5. Tablice wielowymiarowe

```c
int m[2][3] = {{1, 2, 3}, {4, 5, 6}};   /* ciągły blok 6 intów, wierszami */

/* w parametrze rozpada się do "wskaźnika na tablicę 3 intów" */
int sum_matrix(const int (*m)[3], size_t rows);
int sum_matrix2(size_t rows, size_t cols, const int m[rows][cols]);   /* VLA w parametrze (C99) */

int *flat = &m[0][0];
flat[1 * 3 + 2] == m[1][2];             /* indeks spłaszczony: wiersz * kolumny + kolumna */
```
`int **` to **coś innego** niż `int[2][3]` – tablica wskaźników do
osobnych wierszy, nie ciągły blok. Nie da się jednego przekazać jako drugiego.

## 6. Arytmetyka wskaźników – granice

```c
int a[4];
int *end = a + 4;     /* OK: wskaźnik "za ostatni" element - wolno go utworzyć i porównać */
*end;                 /* UB: nie wolno go dereferencjonować */
a + 5;                /* UB: nawet samo utworzenie */
```
Odejmowanie wskaźników daje `ptrdiff_t` (liczba elementów) – tylko dla
wskaźników do **tej samej** tablicy.

---

## ⚠️ Pułapki

- `sizeof` na parametrze-tablicy zwraca rozmiar wskaźnika.
- Literał napisowy (`char *s = "abc";`) jest w pamięci tylko do odczytu –
  `s[0] = 'x'` to UB (zwykle crash). Pisz `const char *s` albo `char s[] = "abc";`.
- Niezainicjalizowany wskaźnik to nie `NULL` – to śmieci.
- Zwracanie adresu zmiennej lokalnej – *dangling pointer*.

---

## ✅ Zadania

1. Wypisz `sizeof(arr)`, `sizeof(&arr)`, `sizeof(*&arr)` dla `int arr[10]`
   i wyjaśnij każdy wynik.
2. Napisz deklarację: "wskaźnik na funkcję zwracającą wskaźnik na
   `const char`, przyjmującą `int`". (Podpowiedź: `cdecl.org`.)
3. Ćwiczenie praktyczne: **ex43** (macierze, parametry wyjściowe,
   tablice napisów).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 3 – Pamięć](chapter3_memory.md)
