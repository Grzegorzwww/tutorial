# Chapter 4: Napisy

[← Spis treści](README.md)

**Cel rozdziału:** w C napis to po prostu tablica `char` zakończona
bajtem `'\0'`. Nie ma długości, nie ma kontroli granic – stąd najwięcej
podatności bezpieczeństwa w historii C. Rozdział pokazuje, jak pisać
operacje na napisach, które nie wychodzą poza bufor.

---

## 1. Napis = bufor + terminator

```c
char a[] = "abc";        /* tablica 4 bajtów: 'a','b','c','\0' - modyfikowalna */
const char *b = "abc";   /* wskaźnik na literał w pamięci tylko do odczytu */
char c[8] = "abc";       /* reszta wypełniona zerami */
char d[3] = "abc";       /* C: dozwolone, ale BEZ '\0' - to już nie jest napis! */

strlen(a);               /* 3 - liczy do '\0', bez niego */
sizeof(a);               /* 4 - rozmiar tablicy z '\0' */
```

**Każda** funkcja tworząca napis musi zostawić miejsce na `'\0'`:
bufor dla N znaków ma rozmiar N + 1.

## 2. Funkcje, których nie używać (lub używać bardzo ostrożnie)

| Funkcja | Problem | Zamiast |
|---|---|---|
| `gets` | brak limitu – usunięta w C11 | `fgets(buf, sizeof buf, stdin)` |
| `strcpy` | nie zna rozmiaru celu | `snprintf(dst, size, "%s", src)` |
| `strcat` | nie zna rozmiaru celu | własne `str_append` z limitem |
| `sprintf` | nie zna rozmiaru celu | `snprintf` |
| `strncpy` | **nie gwarantuje `'\0'`**, dopełnia zerami | `snprintf` lub własna funkcja |
| `strtok` | modyfikuje wejście, stan globalny (nie re-entrant) | `strtok_r` / ręczne parsowanie |
| `atoi` | brak wykrywania błędów, UB przy przepełnieniu | `strtol` |

```c
char dst[4];
strncpy(dst, "abcdef", sizeof dst);   /* dst = 'a','b','c','d' - BEZ '\0'! */
```

## 3. `snprintf` – szwajcarski scyzoryk

```c
char buf[16];
int n = snprintf(buf, sizeof buf, "T=%d.%02d C", 23, 5);

/* n = liczba znaków, które BY zapisano przy nieograniczonym buforze */
if (n < 0)                    { /* błąd kodowania */ }
else if ((size_t)n >= sizeof buf) { /* wynik został obcięty */ }
```
`snprintf` **zawsze** kończy wynik `'\0'` (dla rozmiaru > 0) i nigdy nie
pisze poza bufor. Wartość zwracana pozwala wykryć obcięcie.

Bezpieczne dopisywanie w pętli:
```c
size_t used = 0;
for (int i = 0; i < count; ++i) {
    int w = snprintf(buf + used, sizeof buf - used, "%02X ", data[i]);
    if (w < 0 || (size_t)w >= sizeof buf - used) break;   /* brak miejsca */
    used += (size_t)w;
}
```

## 4. Parsowanie liczb: `strtol` / `strtoul`

```c
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

bool parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s) return false;                  /* brak cyfr */
    if (*end != '\0') return false;              /* śmieci za liczbą: "12abc" */
    if (errno == ERANGE) return false;           /* poza zakresem long */
    if (v < INT_MIN || v > INT_MAX) return false;/* poza zakresem int */
    *out = (int)v;
    return true;
}
```
Pułapka: `strtoul("-1", ...)` **nie zgłasza błędu** – zwraca `ULONG_MAX`
(negacja w arytmetyce bez znaku). Minus trzeba odrzucić samemu.

## 5. Funkcje `<ctype.h>` i znak

```c
char c = '\xE9';           /* 'é' w Latin-1, jako signed char = -23 */
isalpha(c);                /* UB! argument musi być EOF albo wartością unsigned char */
isalpha((unsigned char)c); /* poprawnie */
```

## 6. Parsowanie w miejscu (bez kopiowania)

Częsty wzorzec w protokołach tekstowych (AT-komendy, NMEA, konfiguracja
`klucz=wartość`): zamieniasz separatory na `'\0'` i zapisujesz wskaźniki
na początki pól. Dane nie są kopiowane, ale **bufor musi żyć tak długo
jak wskaźniki**.

```c
/* "GPS,52.1,21.0" -> fields = {"GPS", "52.1", "21.0"} */
size_t split_in_place(char *s, char sep, char **fields, size_t max) {
    size_t n = 0;
    if (max == 0) return 0;
    fields[n++] = s;
    for (; *s; ++s) {
        if (*s == sep) {
            *s = '\0';
            if (n == max) break;
            fields[n++] = s + 1;
        }
    }
    return n;
}
```

---

## ⚠️ Pułapki

- Pomyłka o 1 przy rozmiarze bufora (`char buf[strlen(s)]` – brak miejsca na `'\0'`).
- `strlen` w warunku pętli: `for (i = 0; i < strlen(s); i++)` liczy długość
  w każdym obrocie – O(n²).
- Porównanie napisów przez `==` porównuje **adresy**, nie zawartość – używaj `strcmp`.
- `printf(user_input)` – podatność *format string*; zawsze `printf("%s", user_input)`.

---

## ✅ Zadania

1. Pokaż na przykładzie, że `strncpy` nie dodaje `'\0'`, gdy źródło jest
   dłuższe niż bufor.
2. Sprawdź, co zwraca `snprintf(NULL, 0, "%d", 123456)` – do czego to
   można wykorzystać?
3. Ćwiczenia praktyczne: **ex47** (bezpieczne operacje na napisach),
   **ex48** (parsowanie liczb z kontrolą błędów).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 5 – struct, union, enum](chapter5_structs_unions_enums.md)
