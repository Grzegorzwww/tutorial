# Chapter 3: Pamięć

[← Spis treści](README.md)

**Cel rozdziału:** w C sam odpowiadasz za każdy bajt. Rozdział omawia
trzy rodzaje pamięci, poprawne użycie `malloc`/`realloc`/`free`, typowe
błędy i alternatywę dla sterty stosowaną w embedded – pule o stałym
rozmiarze bloku.

---

## 1. Trzy rodzaje pamięci

| | Stos | Sterta (`malloc`) | Statyczna |
|---|---|---|---|
| Kiedy przydzielana | wejście do bloku | wywołanie `malloc` | start programu |
| Kiedy zwalniana | wyjście z bloku | `free` | koniec programu |
| Rozmiar | mały (embedded: 1–8 KB) | ograniczony RAM-em | znany przy linkowaniu |
| Koszt | ~0 | wyszukiwanie wolnego bloku | 0 |
| Ryzyko | przepełnienie stosu | wycieki, fragmentacja | brak re-entrancy |

```c
void f(void) {
    uint8_t frame[4096];     /* na stosie - na MCU z 2 KB stosu to crash */
    static uint8_t big[4096];/* w .bss - bezpieczne, ale współdzielone między wywołaniami */
}
```

## 2. `malloc`, `calloc`, `realloc`, `free`

```c
#include <stdlib.h>
#include <string.h>

int *a = malloc(n * sizeof *a);     /* sizeof *a - typ wynika ze zmiennej, nie trzeba go powtarzać */
if (a == NULL) { /* obsłuż brak pamięci */ }

int *z = calloc(n, sizeof *z);      /* wyzerowana pamięć; sprawdza przepełnienie n * size */

free(a);
a = NULL;                           /* chroni przed użyciem po zwolnieniu i podwójnym free */
free(NULL);                         /* dozwolone, nic nie robi */
```

**`realloc` – poprawny wzorzec:**
```c
int *tmp = realloc(a, new_cap * sizeof *a);
if (tmp == NULL) {
    /* a NADAL jest ważne - nie zgubiliśmy danych */
    return -1;
}
a = tmp;
```
`a = realloc(a, ...)` to klasyczny wyciek: przy błędzie `realloc` zwraca
`NULL`, a stary blok zostaje bez wskaźnika.

**Dynamiczna tablica – wzrost geometryczny:** podwajaj pojemność
(`cap = cap ? cap * 2 : 4`), wtedy `push` ma zamortyzowany koszt O(1).
Zwiększanie o 1 element daje O(n²) kopiowania.

## 3. Typowe błędy pamięci

```c
char *s = malloc(5);
strcpy(s, "hello");     /* przepełnienie: "hello" ma 6 bajtów z '\0' */

free(s);
s[0] = 'x';             /* use-after-free */
free(s);                /* double free */

int *p = malloc(10 * sizeof *p);
p = NULL;               /* wyciek: nikt już nie zwolni bloku */

int *q;
*q = 5;                 /* niezainicjalizowany wskaźnik */
```

Narzędzia, które to wykrywają:
```bash
gcc -g -fsanitize=address prog.c && ./a.out   # AddressSanitizer
valgrind --leak-check=full ./a.out            # Valgrind (Linux)
```

## 4. Dlaczego embedded często unika `malloc`

- **Fragmentacja** – po tysiącach alokacji różnej wielkości wolna pamięć
  jest pocięta na kawałki i duży blok "nie wchodzi", choć sumarycznie
  miejsca jest dość.
- **Niedeterministyczny czas** – nie wolno tego wołać z przerwania ani
  w pętli z twardym limitem czasu.
- **Brak sposobu na odzyskanie** – urządzenie ma działać latami bez restartu.

Standardy bezpieczeństwa (MISRA C, część norm automotive/medycznych)
wprost zabraniają dynamicznej alokacji po inicjalizacji.

## 5. Pula bloków o stałym rozmiarze (memory pool)

Statyczna tablica bloków + lista wolnych bloków przechowywana **w samych
wolnych blokach** (nie potrzeba dodatkowej pamięci):

```c
#define BLOCK_SIZE  32
#define BLOCK_COUNT 8

typedef union Block {
    union Block *next;              /* gdy wolny: wskaźnik na następny wolny */
    unsigned char data[BLOCK_SIZE]; /* gdy zajęty: dane użytkownika */
} Block;

static Block pool[BLOCK_COUNT];
static Block *free_list;

void pool_init(void) {
    for (int i = 0; i < BLOCK_COUNT - 1; ++i) pool[i].next = &pool[i + 1];
    pool[BLOCK_COUNT - 1].next = NULL;
    free_list = &pool[0];
}

void *pool_alloc(void) {
    Block *b = free_list;
    if (b) free_list = b->next;
    return b;                       /* O(1), deterministycznie, bez fragmentacji */
}

void pool_free(void *p) {
    Block *b = p;
    b->next = free_list;
    free_list = b;
}
```
Taki mechanizm znajdziesz w FreeRTOS (`heap_1..5`), Zephyr (`k_mem_slab`)
i stosach sieciowych (lwIP `memp`).

## 6. Listy wiązane i własność

Struktury wskaźnikowe (listy, drzewa) wymagają jasnej odpowiedzi:
**kto zwalnia węzeł?** Dobra praktyka – jedna funkcja `*_destroy`,
która zwalnia całą strukturę, i usuwanie węzłów przez wskaźnik do
wskaźnika (bez osobnego przypadku dla głowy listy):

```c
typedef struct Node { int value; struct Node *next; } Node;

void list_remove(Node **head, int value) {
    for (Node **pp = head; *pp; pp = &(*pp)->next) {
        if ((*pp)->value == value) {
            Node *dead = *pp;
            *pp = dead->next;
            free(dead);
            return;
        }
    }
}
```

---

## ⚠️ Pułapki

- `malloc(n * size)` może się przepełnić dla dużych `n` – `calloc` to sprawdza.
- `sizeof(ptr)` zamiast `sizeof(*ptr)` – alokujesz 8 bajtów zamiast struktury.
- Rzutowanie wyniku `malloc` (`(int *)malloc(...)`) w C jest zbędne i
  może ukryć brak `#include <stdlib.h>`.
- Zwolnienie wskaźnika, który nie pochodzi z `malloc` (np. `free(arr + 1)`) to UB.

---

## ✅ Zadania

1. Uruchom przykłady z sekcji 3 z `-fsanitize=address` (Linux/WSL) i
   przeczytaj raport dla każdego błędu.
2. Policz, ile razy kopiowane są elementy przy dodaniu 1000 elementów
   do tablicy rosnącej o 1 vs. podwajanej.
3. Ćwiczenia praktyczne: **ex44** (dynamiczna tablica), **ex45** (pula
   pamięci), **ex46** (lista jednokierunkowa).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 4 – Napisy](chapter4_strings.md)
