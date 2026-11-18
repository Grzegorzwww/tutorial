# Chapter 6: Wskaźniki na funkcje i callbacki

[← Spis treści](README.md)

**Cel rozdziału:** w C nie ma klas ani wirtualnych metod, ale jest
wskaźnik na funkcję – i to wystarcza do budowy callbacków, sterowników z
wymiennym backendem, tablic dispatch i generycznych algorytmów.

---

## 1. Składnia i `typedef`

```c
int add(int a, int b) { return a + b; }

int (*op)(int, int) = add;     /* wskaźnik na funkcję (int, int) -> int */
int r = op(2, 3);              /* wywołanie; (*op)(2, 3) - to samo */

typedef int (*BinaryOp)(int, int);   /* bez typedef deklaracje szybko stają się nieczytelne */
BinaryOp ops[] = { add };

/* funkcja przyjmująca callback: */
void for_each(int *arr, size_t n, void (*fn)(int *));
```

## 2. Callback z kontekstem: `void *ctx`

Gołe `void (*fn)(int)` nie pozwala przekazać żadnego stanu (C nie ma
lambd z przechwytywaniem). Standardowe rozwiązanie – dodatkowy argument
`void *` przekazywany bez zmian do callbacku:

```c
typedef void (*EventHandler)(int event, void *ctx);

typedef struct {
    EventHandler fn;
    void *ctx;
} Subscriber;

/* użytkownik */
typedef struct { int count; } Stats;

static void count_events(int event, void *ctx) {
    Stats *s = ctx;            /* w C void* konwertuje się bez rzutowania */
    s->count++;
}

Stats stats = {0};
Subscriber sub = { count_events, &stats };
sub.fn(42, sub.ctx);           /* stats.count == 1 */
```
Ten wzorzec znajdziesz wszędzie: `pthread_create`, `qsort_r`, timery
FreeRTOS (`pvTimerGetTimerID`), callbacki HAL (`huart->pRxBuffPtr`),
Zephyr (`k_work` + `CONTAINER_OF`).

## 3. "Interfejs" – struktura wskaźników na funkcje

Odpowiednik klasy abstrakcyjnej z C++. Tak zbudowany jest np. VFS w
jądrze Linux (`struct file_operations`):

```c
typedef struct {
    int  (*init)(void *self);
    int  (*write)(void *self, const uint8_t *data, size_t len);
    void (*deinit)(void *self);
} StreamOps;

typedef struct {
    const StreamOps *ops;      /* wskaźnik na "vtable" - wspólny dla wszystkich instancji */
    void *self;                /* dane konkretnej implementacji */
} Stream;

int stream_write(Stream *s, const uint8_t *d, size_t n) {
    return s->ops->write(s->self, d, n);   /* kod nie wie, czy to UART, USB czy plik */
}
```
W testach na PC podmieniasz `ops` na wersję zapisującą do pamięci – bez
`#ifdef` w logice.

## 4. Tablica dispatch zamiast długiego `switch`

```c
typedef int (*CommandFn)(const char *args);

static const struct {
    const char *name;
    CommandFn   fn;
} commands[] = {
    { "help",   cmd_help },
    { "reboot", cmd_reboot },
    { "led",    cmd_led },
};

int dispatch(const char *name, const char *args) {
    for (size_t i = 0; i < sizeof commands / sizeof commands[0]; ++i) {
        if (strcmp(commands[i].name, name) == 0) return commands[i].fn(args);
    }
    return -1;
}
```
Tablica `static const` trafia do Flash. Dodanie komendy = jedna linijka.

## 5. `qsort` i `bsearch` – generyczne algorytmy przez `void *`

```c
#include <stdlib.h>

typedef struct { int id; const char *name; } Item;

static int cmp_by_id(const void *a, const void *b) {
    const Item *x = a, *y = b;
    return (x->id > y->id) - (x->id < y->id);   /* -1, 0, 1 bez ryzyka przepełnienia */
}

Item items[] = { {3, "c"}, {1, "a"}, {2, "b"} };
qsort(items, 3, sizeof items[0], cmp_by_id);

Item key = { .id = 2 };
Item *found = bsearch(&key, items, 3, sizeof items[0], cmp_by_id);   /* wymaga posortowanej tablicy */
```

**Pułapka komparatora:** `return x->id - y->id;` przepełnia `int` dla
dużych wartości o różnych znakach (np. `INT_MIN - 1`). Wzorzec `(a > b) - (a < b)`
jest bezpieczny.

Komparator z kilkoma kluczami – porównuj kolejne pola, aż któreś się różni:
```c
static int cmp_by_age_then_name(const void *a, const void *b) {
    const Person *x = a, *y = b;
    if (x->age != y->age) return (x->age > y->age) - (x->age < y->age);
    return strcmp(x->name, y->name);
}
```

## 6. `CONTAINER_OF` – od pola do struktury

Gdy callback dostaje wskaźnik na **pole** wewnątrz Twojej struktury
(częste w jądrze Linux i Zephyr), odzyskujesz wskaźnik na całość:

```c
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct { int led_pin; struct k_work work; } LedCtx;

void work_handler(struct k_work *w) {
    LedCtx *ctx = CONTAINER_OF(w, LedCtx, work);
}
```

---

## ⚠️ Pułapki

- Wywołanie `NULL`-owego wskaźnika na funkcję → crash. Sprawdzaj
  opcjonalne callbacki przed wywołaniem.
- Rzutowanie wskaźnika na funkcję na inny typ funkcji i wywołanie → UB.
- `void *` i wskaźnik na funkcję to **nie** to samo – standard C nie
  gwarantuje konwersji między nimi (POSIX tak, dla `dlsym`).
- Callback wywoływany z przerwania musi być krótki i nie może używać
  funkcji blokujących.

---

## ✅ Zadania

1. Zbuduj prosty "interfejs" `Logger` z dwiema implementacjami
   (`stdout` i bufor w pamięci) i jedną funkcją `log_msg(Logger*, const char*)`.
2. Sprawdź, co zwraca komparator `a - b` dla `a = INT_MIN`, `b = 1`.
3. Ćwiczenia praktyczne: **ex52** (event bus z `void *ctx`),
   **ex53** (`qsort`/`bsearch` z własnymi komparatorami).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 7 – Preprocesor](chapter7_preprocessor.md)
