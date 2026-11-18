# Chapter 8: Niezdefiniowane zachowanie, `volatile`, `restrict`

[← Spis treści](README.md)

**Cel rozdziału:** zrozumieć, co standard C pozwala kompilatorowi zrobić
z Twoim kodem. Kod z niezdefiniowanym zachowaniem (UB) "działa" z `-O0`,
psuje się z `-O2` i zachowuje się inaczej na innej architekturze.
`volatile` i `restrict` to dwa słowa kluczowe, które mówią kompilatorowi,
czego **nie** może założyć (lub co może).

---

## 1. Trzy kategorie "niezdefiniowania"

| Kategoria | Znaczenie | Przykład |
|---|---|---|
| *implementation-defined* | zależy od kompilatora, ale **udokumentowane** | `sizeof(int)`, przesunięcie w prawo liczby ujemnej |
| *unspecified* | jedna z kilku możliwości, bez dokumentacji | kolejność obliczania argumentów funkcji |
| *undefined* (UB) | **wszystko** może się stać, program nie ma sensu | przepełnienie `int`, `*NULL`, wyjście poza tablicę |

## 2. Kompilator zakłada, że UB nie występuje

```c
int get(int *p) {
    int v = *p;            /* dereferencja: kompilator wnioskuje, że p != NULL */
    if (p == NULL) return -1;   /* ...więc ten warunek może USUNĄĆ */
    return v;
}
```
To nie jest błąd kompilatora – to poprawna optymalizacja. W 2009 ten
wzorzec był przyczyną luki w jądrze Linux (`tun_chr_poll`).

## 3. Najczęstsze UB w embedded

```c
int32_t a = INT32_MAX; a + 1;          /* przepełnienie signed */
uint32_t x = 1; x << 32;               /* przesunięcie >= szerokość typu */
int mid = (lo + hi) / 2;               /* przepełnia dla dużych lo, hi */
int mid_ok = lo + (hi - lo) / 2;       /* poprawnie (dla lo <= hi) */

uint8_t buf[8];
uint32_t v = *(uint32_t *)buf;         /* naruszenie strict aliasing + możliwe złe wyrównanie */
uint32_t v_ok;
memcpy(&v_ok, buf, sizeof v_ok);       /* poprawnie; kompilator wygeneruje jedną instrukcję */

int i = 0;
i = i++ + 1;                           /* dwie modyfikacje bez punktu sekwencji */
arr[i] = i++;                          /* jw. */
```

**Strict aliasing:** kompilator zakłada, że wskaźniki na różne typy
(`uint32_t *` i `float *`) nie wskazują tej samej pamięci. Wyjątek:
`char *` / `unsigned char *` mogą wskazywać na wszystko. Do
reinterpretacji bajtów używaj `memcpy` (lub unii).

**Rotacja bez UB:**
```c
uint32_t rotl32(uint32_t x, unsigned n) {
    n &= 31;
    return (x << n) | (x >> ((32 - n) & 31));   /* dla n == 0 brak przesunięcia o 32 */
}
```

## 4. `volatile` – "ta pamięć może zmienić się bez Twojej wiedzy"

Bez `volatile` kompilator może trzymać wartość w rejestrze i nie czytać
jej ponownie z pamięci:

```c
uint32_t flag = 0;                 /* ustawiana w przerwaniu */

void wait(void) {
    while (flag == 0) { }          /* -O2: odczyt raz, potem pętla nieskończona */
}

volatile uint32_t flag_ok = 0;     /* każdy odczyt i zapis faktycznie trafia do pamięci */
```

`volatile` jest **potrzebne** dla:
- rejestrów sprzętowych (`*(volatile uint32_t *)0x40021018`),
- zmiennych współdzielonych z przerwaniem,
- zmiennych modyfikowanych po `setjmp`.

`volatile` **nie** zapewnia:
- **atomowości** – `counter++` na `volatile` to nadal odczyt, dodanie i zapis;
  przerwanie w środku zgubi inkrementację,
- **kolejności względem zwykłych zmiennych** ani barier pamięci między
  rdzeniami – do tego służą `<stdatomic.h>` (C11) i bariery (`__DMB()`),
- synchronizacji wątków – na systemie z wątkami używaj mutexów/atomics.

```c
/* sekcja krytyczna w pętli głównej (Cortex-M) */
__disable_irq();
uint32_t snapshot = shared_counter;   /* odczyt 64-bit lub kilku zmiennych naraz */
shared_counter = 0;
__enable_irq();
```

## 5. `restrict` – obietnica braku aliasingu

```c
void add_arrays(int *restrict dst, const int *restrict a, const int *restrict b, size_t n) {
    for (size_t i = 0; i < n; ++i) dst[i] = a[i] + b[i];
}
```
`restrict` mówi: "przez czas życia tego wskaźnika do tych danych nie
dostaniesz się żadnym innym wskaźnikiem". Kompilator może wtedy
wektoryzować pętlę bez sprawdzania nakładania się buforów. Złamanie
obietnicy (wywołanie z nakładającymi się buforami) to UB. Przykład
w bibliotece standardowej: `memcpy` ma `restrict` (bufory nie mogą się
nakładać), `memmove` – nie.

## 6. Narzędzia

```bash
gcc -O2 -Wall -Wextra -fsanitize=undefined prog.c   # UBSan: przepełnienia, przesunięcia, wyrównanie
gcc -O2 -Wall -Wextra -fsanitize=address prog.c     # ASan: pamięć
gcc -fanalyzer prog.c                               # analiza statyczna GCC 10+
cppcheck --enable=all prog.c
```

---

## ⚠️ Pułapki

- "U mnie działa" nie dowodzi braku UB – zmiana flag optymalizacji,
  wersji kompilatora lub architektury może to zmienić.
- `volatile` jako zamiennik mutexa / atomika.
- Rzutowanie `uint8_t *` na `uint32_t *` przy parsowaniu ramek.
- `memcpy` na nakładających się buforach – użyj `memmove`.

---

## ✅ Zadania

1. Skompiluj przykład z pętlą `while (flag == 0)` z `-O2 -S` bez i z
   `volatile` – porównaj asembler.
2. Uruchom (Linux/WSL) program z `x << 32` i przepełnieniem `int` z
   `-fsanitize=undefined` – co raportuje UBSan?
3. Ćwiczenie praktyczne: **ex56** (przepisanie funkcji z UB na
   poprawne odpowiedniki).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 9 – Wzorce embedded](chapter9_embedded_patterns.md)
