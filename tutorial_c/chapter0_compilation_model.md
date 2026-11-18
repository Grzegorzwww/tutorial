# Chapter 0: Model kompilacji

[← Spis treści](README.md)

**Cel rozdziału:** zrozumieć, co dzieje się między plikiem `.c` a
programem wykonywalnym. Większość "dziwnych" błędów w C (`undefined
reference`, `multiple definition`, zmienna, która "sama" zmienia
wartość) bierze się z niezrozumienia tego procesu.

---

## 1. Cztery etapy

```text
main.c ──preprocesor──> main.i ──kompilator──> main.s ──asembler──> main.o ─┐
uart.c ──preprocesor──> uart.i ──kompilator──> uart.s ──asembler──> uart.o ─┼─linker──> program
                                                                  libc.a ─┘
```

Każdy etap możesz zatrzymać i obejrzeć wynik:
```bash
gcc -E main.c -o main.i   # tylko preprocesor: rozwinięte #include i makra
gcc -S main.c -o main.s   # do asemblera
gcc -c main.c -o main.o   # do pliku obiektowego
gcc main.o uart.o -o app  # linkowanie
nm main.o                 # symbole: T = zdefiniowany kod, U = niezdefiniowany (szukany u innych)
```

**Kluczowa obserwacja:** kompilator widzi **jeden plik `.c` naraz**
(tzw. *translation unit*, jednostka translacji). Nie wie nic o innych
plikach – zna tylko deklaracje, które dostał przez `#include`. Dopiero
linker łączy odwołania (`U`) z definicjami (`T`).

## 2. Deklaracja a definicja

```c
int add(int a, int b);        /* deklaracja: "taka funkcja gdzieś istnieje" */
int add(int a, int b) {       /* definicja: tu jest jej kod */
    return a + b;
}

extern int g_counter;         /* deklaracja zmiennej (bez pamięci) */
int g_counter = 0;            /* definicja zmiennej (rezerwuje pamięć) */
```

Zasada jednej definicji: deklaracji może być wiele, **definicja jest
dokładnie jedna** w całym programie. Dlatego:

```c
/* config.h */
#ifndef CONFIG_H
#define CONFIG_H

extern int g_log_level;        /* deklaracja w nagłówku - OK */
/* int g_log_level = 0; */     /* definicja w nagłówku - BŁĄD: każdy .c dołączający
                                  ten plik dostanie własną definicję -> "multiple definition" */
#endif

/* config.c */
#include "config.h"
int g_log_level = 2;           /* jedyna definicja */
```

`#ifndef CONFIG_H` / `#define` / `#endif` to *include guard* – chroni
przed dwukrotnym dołączeniem nagłówka **w obrębie jednego** pliku `.c`.
Nie chroni przed definicjami w nagłówku dołączanym do **wielu** plików.

## 3. Linkowanie: `static` i `extern` na poziomie pliku

```c
/* uart.c */
static int rx_errors = 0;          /* widoczna TYLKO w uart.c (internal linkage) */
int uart_baud = 115200;            /* widoczna w całym programie (external linkage) */

static void reset_fifo(void) { }   /* funkcja prywatna modułu */
void uart_init(void) { reset_fifo(); }
```

`static` na poziomie pliku to w C odpowiednik `private` – **domyślnie
używaj `static` dla wszystkiego, czego nie eksportujesz w nagłówku**.
Unikasz kolizji nazw (dwa moduły z funkcją `init`) i dajesz
kompilatorowi więcej swobody w optymalizacji (może np. wkleić funkcję
inline i usunąć oryginał).

## 4. Czas życia zmiennych (storage duration)

| Gdzie zadeklarowana | Czas życia | Inicjalizacja domyślna |
|---|---|---|
| Globalna / `static` w pliku | cały program | zero |
| `static` wewnątrz funkcji | cały program | zero, **raz** przed startem |
| Lokalna (automatyczna) | do końca bloku `{}` | **brak – śmieci!** |
| `malloc` | do `free` | brak (`calloc` zeruje) |

```c
#include <stdio.h>

int next_id(void) {
    static int id = 0;    /* inicjalizowana raz, pamięta wartość między wywołaniami */
    return ++id;
}

int main(void) {
    printf("%d %d %d\n", next_id(), next_id(), next_id());   /* kolejność wywołań niezdefiniowana! */
    int a = next_id();
    int b = next_id();
    printf("%d %d\n", a, b);   /* 4 5 */
}
```

Lokalna zmienna `static` to globalna zmienna "ukryta" w funkcji. W
embedded często używana do liczników, buforów i flag inicjalizacji – ale
pamiętaj, że funkcja z `static` stanem **nie jest re-entrant** (nie
wolno jej wołać jednocześnie z przerwania i pętli głównej).

## 5. Gdzie lądują zmienne w pamięci (mikrokontroler)

```text
.text   – kod i stałe (`const` globalne)          -> Flash
.rodata – literały napisowe, tablice `const`       -> Flash
.data   – globalne z wartością != 0                -> RAM (kopiowane z Flash przy starcie)
.bss    – globalne zerowane / bez inicjalizatora   -> RAM (zerowane przy starcie)
stos    – zmienne lokalne
sterta  – malloc (w embedded często wyłączona)
```

```bash
arm-none-eabi-size firmware.elf   # text / data / bss – sprawdzasz zużycie Flash i RAM
```

Wniosek praktyczny: `const uint8_t table[1024] = {...};` zajmuje Flash,
a bez `const` – zajmie **i Flash (wartości początkowe), i RAM**.

---

## ⚠️ Pułapki

- **Definicja zmiennej w nagłówku** → `multiple definition` przy linkowaniu.
- **Brak prototypu** (C89-owe "implicit int") – GCC daje ostrzeżenie
  `implicit declaration of function`. Traktuj je jak błąd (`-Werror=implicit-function-declaration`).
- **`int f();` to NIE jest funkcja bez argumentów** w C (przed C23) – to
  funkcja o nieokreślonych argumentach. Pisz `int f(void);`.
- **Funkcja zwracająca wskaźnik do zmiennej lokalnej** – po powrocie
  zmienna nie istnieje. Zwracanie wskaźnika do `static` działa, ale
  kolejne wywołanie nadpisze poprzedni wynik.

---

## ✅ Zadania

1. Rozbij dowolny program na `main.c`, `module.c`, `module.h`. Zdefiniuj
   w module zmienną globalną i funkcję prywatną. Sprawdź `nm module.o`:
   jaką literę ma funkcja `static` (`t`) a jaką eksportowana (`T`)?
2. Celowo zdefiniuj zmienną w nagłówku dołączanym do dwóch plików `.c` –
   jaki komunikat daje linker?
3. Ćwiczenie praktyczne: **ex41** (`static`, czas życia, bufor statyczny).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 1 – Typy całkowite](chapter1_integer_types.md)
