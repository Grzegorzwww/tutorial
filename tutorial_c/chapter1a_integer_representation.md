# Chapter 1a: Reprezentacja liczb i konwersje – jak to działa w bitach

[← Spis treści](README.md)

**Cel rozdziału:** zobaczyć, co *fizycznie* dzieje się z bitami, gdy
rzutujesz, rozszerzasz, dodajesz i porównujesz liczby ze znakiem i bez
znaku. Wszystkie wyniki w tym rozdziale zostały sprawdzone programem
(GCC, `int` = 32 bity) – możesz je odtworzyć programem z sekcji 10.

**Jedna myśl, która porządkuje cały rozdział:**

> W pamięci nie ma "liczb ze znakiem" i "liczb bez znaku". Są tylko
> **ciągi bitów**. Typ mówi kompilatorowi, **jak te bity interpretować**
> i **jakich instrukcji użyć** (np. do porównania).

---

## 1. Ten sam wzór bitów, dwie interpretacje

Weźmy 16 bitów: `1001 1100 0100 0000` (hex `0x9C40`).

```text
bit:     15   14   13  12  11  10   9   8   7   6   5   4   3   2   1   0
wartość: 1    0    0   1   1   1    0   0   0   1   0   0   0   0   0   0
```

**Jako `uint16_t`** – każdy bit ma wagę dodatnią: $2^{15}, 2^{14}, \dots, 2^0$.
$$0x9C40 = 32768 + 4096 + 2048 + 1024 + 64 = 40000$$

**Jako `int16_t`** (kod uzupełnień do dwóch, *two's complement*) – tylko
najstarszy bit (bit 15, "bit znaku") ma wagę **ujemną** $-2^{15}$:
$$0x9C40 = -32768 + (4096 + 2048 + 1024 + 64) = -32768 + 7232 = -25536$$

Zapamiętaj prosty wzór dla N-bitowego słowa, gdy najstarszy bit jest ustawiony:

$$\text{wartość ze znakiem} = \text{wartość bez znaku} - 2^N$$

Sprawdzenie: $40000 - 65536 = -25536$. ✓

### Przykład 8-bitowy – łatwo zobaczyć wszystko naraz

| Bity | `uint8_t` | `int8_t` |
|---|---|---|
| `0000 0000` | 0 | 0 |
| `0000 0101` | 5 | 5 |
| `0111 1111` | 127 | 127 |
| `1000 0000` | 128 | **-128** |
| `1111 1011` | 251 | **-5** |
| `1111 1111` | 255 | **-1** |

Zakresy: `uint8_t` to $0 \dots 2^8-1 = 255$, `int8_t` to $-2^7 \dots 2^7-1 = -128 \dots 127$.
Ogólnie dla $N$ bitów: bez znaku $0 \dots 2^N-1$, ze znakiem $-2^{N-1} \dots 2^{N-1}-1$.

### Dlaczego akurat kod uzupełnień do dwóch?

Bo **dodawanie jest takie samo** dla obu interpretacji – procesor ma
jedną instrukcję `ADD`. Negacja to "odwróć bity i dodaj 1":

```text
 5  = 0000 0101
~5  = 1111 1010
+1  = 1111 1011  = -5   (bo 5 + (-5) = 1 0000 0000, a dziewiąty bit jest odrzucany)
```

Stąd `-1` to zawsze **same jedynki** (`0xFF`, `0xFFFF`, `0xFFFFFFFF`).

---

## 2. Rzutowanie na węższy typ: zostają MŁODSZE bity

To najważniejsza poprawka do intuicji. Przykład z Twojego pytania:

```c
int32_t  big    = 70000;
int16_t  small  = (int16_t)big;
uint16_t usmall = (uint16_t)big;
```

`70000` w 32 bitach to `0x00011170`:

```text
         starsze 16 bitów        młodsze 16 bitów
        ┌─────────────────┐     ┌─────────────────┐
big  =  │ 0000 0000 0000 0001 │ │ 0001 0001 0111 0000 │   = 0x0001 1170
        └─────────────────┘     └─────────────────┘
              ODRZUCONE                ZOSTAJE
```

**Zawężenie do 16 bitów = odcięcie starszych bitów i zachowanie młodszych.**
(Procesor po prostu "bierze dolną połowę rejestru".)

To samo co: $70000 \bmod 65536 = 4464 = 0x1170$.

Teraz dwie interpretacje **tych samych** 16 bitów `0x1170`:

| | bity | wartość |
|---|---|---|
| `usmall` (`uint16_t`) | `0001 0001 0111 0000` | 4464 |
| `small` (`int16_t`) | `0001 0001 0111 0000` | 4464 |

Tutaj obie wartości są takie same, bo najstarszy bit (bit 15) wynosi
`0` – nie ma co "odejmować $2^{16}$". **Różnicę widać dopiero, gdy
młodsze 16 bitów zaczyna się od jedynki:**

```text
      int32_t        bity (hex)   (int16_t)    (uint16_t)
        70000     0x00011170          4464          4464   ← bit 15 = 0, brak różnicy
        40000     0x00009C40        -25536         40000   ← bit 15 = 1
        32768     0x00008000        -32768         32768
        65535     0x0000FFFF            -1         65535
           -1     0xFFFFFFFF            -1         65535
       -70000     0xFFFEEE90         -4464         61072
```

Przy `-70000`: młodsze 16 bitów to `0xEE90`; jako bez znaku `61072`, jako
ze znakiem $61072 - 65536 = -4464$.

### Co mówi standard, a co robi kompilator

| Konwersja | Standard C | W praktyce (GCC, Clang, MSVC) |
|---|---|---|
| na typ **bez** znaku, wartość spoza zakresu | **zdefiniowana**: wynik $\bmod\ 2^N$ | odcięcie młodszych N bitów |
| na typ **ze** znakiem, wartość spoza zakresu | *implementation-defined* (kompilator ma to opisać w dokumentacji) | to samo: te same młodsze N bitów, tylko zinterpretowane ze znakiem |

Dlatego w komentarzu z tutorialu stoi: `(uint16_t)` – "zdefiniowane",
`(int16_t)` – "implementation-defined". Wszystkie popularne kompilatory
robią to przewidywalnie, ale **nie jest to gwarancja standardu**
(a konwersja liczby zmiennoprzecinkowej poza zakres to już prawdziwe UB).

---

## 3. Rzutowanie na szerszy typ: rozszerzanie

Węższa liczba musi wypełnić nowe, starsze bity. Są dwa sposoby i **typ
źródłowy** decyduje, który zostanie użyty:

- **rozszerzenie zerami** (*zero extension*) – dla źródła **bez znaku**,
- **rozszerzenie znakiem** (*sign extension*) – dla źródła **ze znakiem**:
  nowe bity kopiują dawny bit znaku.

```text
int8_t  -1   = 1111 1111
 → int32_t   = 1111 1111 1111 1111 1111 1111 1111 1111  = -1      (sign extension)

uint8_t 255  = 1111 1111
 → int32_t   = 0000 0000 0000 0000 0000 0000 1111 1111  = 255     (zero extension)
```

Wynik z programu: `int8 -1 -> int32 -1`, `uint8 0xFF -> int32 255`.

**Uwaga na dwa kroki naraz.** Gdy zmieniasz i szerokość, i znakowość,
liczy się kolejność:

```c
int8_t   b = -1;               /* 0xFF */
uint32_t x = (uint32_t)b;      /* 0xFFFFFFFF  - najpierw wartość -1, potem mod 2^32 */
uint32_t y = (uint8_t)b;       /* 0x000000FF  - najpierw obcięcie do uint8_t (=255), potem rozszerzenie zerami */
```

Reguła zapamiętania: **konwersja zachowuje wartość, jeśli się mieści**;
jeśli nie – liczy się modulo $2^N$ typu docelowego (bez znaku) albo
robi to, co zdefiniuje kompilator (ze znakiem).

### Klasyczny błąd: bajty z bufora

```c
char buf[2] = { (char)0xFF, 0x01 };      /* char bywa ze znakiem! */
uint16_t v = buf[0] | (buf[1] << 8);
/* oczekiwane 0x01FF, a wychodzi 0xFFFF: buf[0] = -1 -> int -1 = 0xFFFFFFFF,
   OR z czymkolwiek zostawia same jedynki */

uint8_t ubuf[2] = { 0xFF, 0x01 };
uint16_t ok = (uint16_t)(ubuf[0] | (ubuf[1] << 8));   /* 0x01FF */
```
**Dane binarne zawsze trzymaj w `uint8_t`**, nigdy w gołym `char`.

---

## 4. Ta sama szerokość, inna znakowość: bity się nie zmieniają

```c
int32_t  a = -1;             /* 0xFFFFFFFF */
uint32_t b = (uint32_t)a;    /* 0xFFFFFFFF = 4294967295  - te same bity, inna interpretacja */
int32_t  c = (int32_t)0x80000000u;   /* -2147483648  (implementation-defined, ale tak działa wszędzie) */
```
To jedyny przypadek, gdzie rzutowanie nie robi **nic** w maszynie – zmienia
się tylko to, jak kompilator traktuje wartość dalej (np. jakiej instrukcji
użyje do porównania).

---

## 5. Dodawanie i odejmowanie: ten sam układ, inne skutki

Procesor wykonuje jedno dodawanie na bitach. Różni się to, co z wynikiem
robi język:

```c
uint8_t a = 200, b = 100;
uint8_t s = (uint8_t)(a + b);   /* 300 = 0x12C -> odcięte do 0x2C = 44  (wynik: 44)  */
```
W arytmetyce bez znaku wynik jest zawsze liczony **modulo $2^N$**:
$$200 + 100 = 300 \equiv 300 - 256 = 44 \pmod{256}$$
Takie "zawijanie" jest **zdefiniowane** i jest podstawą np. liczników
czasu (rozdział 1, sekcja 5).

Odejmowanie bez znaku, gdy odjemnik jest większy:
```c
uint32_t d = 3u - 5u;   /* 4294967294 = 2^32 - 2, a nie -2 */
```

Dla **liczb ze znakiem** przekroczenie zakresu (np. `INT32_MAX + 1`) to
**niezdefiniowane zachowanie**: kompilator może założyć, że się nie
zdarzy, i na tym założeniu zoptymalizować kod (patrz rozdział 8).
Sprzęt zrobiłby to samo co dla `unsigned` (zawinięcie do wartości
ujemnej), ale **język tego nie gwarantuje**.

Z tego powodu:

| | przepełnienie | co robić |
|---|---|---|
| `unsigned` | zawija się modulo $2^N$ (zdefiniowane) | jeśli to niechciane – sprawdź przed operacją |
| `signed` | **UB** | **zawsze** sprawdź przed operacją albo licz w szerszym typie |

---

## 6. Porównania: ta sama para bitów, różny wynik

`0xFFFFFFFF` kontra `1`:

```text
jako signed:    -1 <  1   → prawda
jako unsigned:  4294967295 > 1 → fałsz
```
Procesor ma **osobne instrukcje/flagi** do porównań ze znakiem i bez,
więc to typ w wyrażeniu decyduje o wyniku:

```c
int      a = -1;
unsigned b = 1;
if (a < b) { }     /* NIE wykona się. a jest konwertowane na unsigned = 4294967295 */
```

Z programu: `(-1 < 1u) = 0`.

---

## 7. Co się dzieje, gdy typy w wyrażeniu są różne

Kompilator stosuje dwa kroki, zanim cokolwiek policzy.

**Krok 1 – promocje całkowite:** każdy typ **węższy niż `int`**
(`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `char`, `bool`) jest
zamieniany na `int`. (Dla `uint16_t` przy 32-bitowym `int` – zawsze, bo
`int` mieści wszystkie jego wartości.)

**Krok 2 – zwykłe konwersje arytmetyczne:** dwa operandy sprowadza się
do wspólnego typu według reguł (upraszczając):

1. Ta sama znakowość → do szerszego typu.
2. Różna znakowość, a typ **bez** znaku jest co najmniej tak szeroki jak
   typ ze znakiem → **do typu bez znaku**. (To jest źródło pułapek.)
3. Różna znakowość, a typ ze znakiem jest **szerszy** i mieści wszystkie
   wartości typu bez znaku → do typu ze znakiem.

| Wyrażenie | Wspólny typ | Wynik |
|---|---|---|
| `int32_t(-1) < uint32_t(1)` | `uint32_t` (reguła 2) | **fałsz** – `-1` staje się 4294967295 |
| `int64_t(-1) < uint32_t(1)` | `int64_t` (reguła 3) | **prawda** – `int64_t` mieści wszystkie wartości `uint32_t` |
| `uint8_t + uint8_t` | `int` (promocja) | `200 + 100 = 300` – bez zawijania, dopiero zapis do `uint8_t` obcina |
| `int16_t * int16_t` | `int` | bez przepełnienia w szerokości 16 bitów |

### Pułapka: mnożenie `uint16_t` przy 32-bitowym `int`

```c
uint16_t a = 65535, b = 65535;
uint32_t p = a * b;            /* a, b -> int; 65535*65535 = 4294836225 > INT_MAX -> UB! */
uint32_t ok = (uint32_t)a * b; /* jeden operand rzutowany jawnie przed mnożeniem */
```
Wygląda na czystą arytmetykę bez znaku, a tak naprawdę jest to
przepełnienie typu **ze znakiem**.

### Zasada praktyczna

**Rzutuj na szerszy typ PRZED operacją, nie po niej:**
```c
uint32_t total = (uint32_t)a * b;      // ✓
uint32_t bad   = (uint32_t)(a * b);    // ✗ rzutowanie za późno - mnożenie już się wykonało na int
```

---

## 8. Przesunięcia i dzielenie

**Przesunięcie w prawo:**
```c
uint32_t u = 0xFFFFFFF0u;
u >> 2;      /* 0x3FFFFFFC  – "logiczne": z lewej wchodzą ZERA */

int32_t  s = -16;
s >> 2;      /* -4          – "arytmetyczne": z lewej wchodzi bit znaku (implementation-defined, tak robi GCC) */
```
Przesunięcie w lewo liczby ze znakiem, gdy wynik nie mieści się w typie
albo liczba jest ujemna – **UB**. Do operacji bitowych (maski, flagi,
rejestry) **używaj typów bez znaku**.

**Dzielenie i reszta** (C99+) **obcinają w stronę zera**:
```c
-7 / 2   == -3        // nie -4
-7 % 2   == -1        // znak reszty = znak dzielnej
```
Zawsze zachodzi `(a / b) * b + (a % b) == a`. Matematyczne $\bmod$ z
poprzednich sekcji (zawsze nieujemne) to co innego niż operator `%` dla
liczb ujemnych.

---

## 9. Rozszerzenie znaku "ręcznie" – przykład z ADC

Przetwornik 12-bitowy zwraca wartość w `uint16_t` w zakresie
`0..0xFFF`, gdzie `0x800..0xFFF` oznaczają liczby ujemne (kod U2 na 12 bitach).
Trzeba "przenieść" bit znaku z pozycji 11 na pozycje 12-31:

```c
int sign_extend_12(uint16_t raw) {
    raw &= 0x0FFF;
    return (raw & 0x800) ? (int)raw - 4096 : (int)raw;   /* wzór: wartość - 2^N */
}
/* 0x7FF -> 2047,  0x800 -> -2048,  0xFFF -> -1 */
```
Dokładnie wzór z sekcji 1 – ($\text{wartość} - 2^{12}$). Ćwiczenie
praktyczne: **ex56** (`sign_extend_12`).

---

## 10. Eksperyment do samodzielnego uruchomienia

```c
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    int32_t vals[] = {70000, 40000, 32768, 65535, -1, -70000};
    for (int i = 0; i < 6; ++i) {
        printf("%7" PRId32 "  0x%08" PRIX32 "  int16=%6d  uint16=%5u\n",
               vals[i], (uint32_t)vals[i],
               (int)(int16_t)vals[i], (unsigned)(uint16_t)vals[i]);
    }
    return 0;
}
```
Dodaj własne wartości i **zanim uruchomisz – przewidź wynik**. Pomaga
zapisać je szesnastkowo i patrzeć tylko na młodsze 4 cyfry hex.

### Kompilator pomoże, jeśli mu pozwolisz

```bash
gcc -std=c11 -Wall -Wextra -Wconversion -Wsign-conversion plik.c
```
- `-Wconversion` – ostrzega o niejawnym **zawężaniu** (`int32_t` → `int16_t`).
- `-Wsign-conversion` – ostrzega o niejawnej zmianie znakowości.
- `-Wsign-compare` (w `-Wextra`) – ostrzega o porównaniu `signed` z `unsigned`.
- Jawne rzutowanie wycisza ostrzeżenie: rób to **świadomie**, tylko tam,
  gdzie wiesz, że obcięcie jest zamierzone.

---

## ⚠️ Pułapki – krótka lista

- Zawężanie zachowuje **młodsze** bity; starsze są tracone bez ostrzeżenia.
- Rozszerzanie liczby ze znakiem kopiuje bit znaku – `int8_t` `-1` → `0xFFFFFFFF`, a nie `0xFF`.
- `char` bywa ze znakiem; dane binarne → `uint8_t`.
- Wyrażenie `signed` vs `unsigned` konwertuje `signed` na `unsigned`.
- Rzutuj na szerszy typ **przed** działaniem, nie po nim.
- `uint16_t * uint16_t` liczy się jako `int` – możliwe UB.
- Przepełnienie `unsigned` – zawinięcie; `signed` – UB.
- Przesunięcia i maski – tylko na typach bez znaku.

---

## ✅ Zadania

Przewidź wynik (hex i dziesiętnie), a potem sprawdź programem:

1. `int32_t x = 100000; int16_t a = (int16_t)x; uint16_t b = (uint16_t)x;`
   – ile wynoszą `a` i `b`? (Podpowiedź: $100000 = 0x186A0$.)
2. `int32_t x = -100; uint8_t a = (uint8_t)x; int8_t b = (int8_t)x;`
3. `int8_t s = -128; int32_t w = s; uint32_t u = (uint32_t)s;`
4. `uint8_t a = 250, b = 10; int c = a + b; uint8_t d = a + b;`
5. Czy `(uint16_t)0xFFFF + 1 == 0` jest prawdą? A `(uint16_t)(0xFFFF + 1) == 0`?
   (Uwaga na promocję do `int`.)
6. `if (sizeof(int) > -1)` – co zostanie wypisane i dlaczego?
7. Ćwiczenia praktyczne: **ex42** (bezpieczna arytmetyka), **ex56**
   (sign extension, rotacje, przepełnienia).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 2 – Wskaźniki i tablice](chapter2_pointers_arrays.md)
