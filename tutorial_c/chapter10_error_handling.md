# Chapter 10: Obsługa błędów

[← Spis treści](README.md)

**Cel rozdziału:** C nie ma wyjątków ani destruktorów. Każdy błąd
trzeba jawnie zwrócić, sprawdzić i – co najtrudniejsze – po błędzie
**posprzątać** wszystko, co zostało już przydzielone.

---

## 1. Konwencje zwracania błędów

| Konwencja | Przykład | Uwagi |
|---|---|---|
| `0` = sukces, `<0` = kod błędu | jądro Linux (`-EINVAL`), Zephyr | najczęstsza w embedded |
| `-1` + `errno` | POSIX (`open`, `read`) | błąd w zmiennej globalnej (per wątek) |
| `NULL` = błąd | `malloc`, `fopen` | brak informacji *jaki* błąd |
| `bool` + parametr wyjściowy | `bool parse(const char *, int *out)` | czytelne dla prostych funkcji |
| wartość specjalna | `EOF`, `SIZE_MAX` | łatwo pomylić z poprawnym wynikiem |

Własny typ błędu daje czytelność i ostrzeżenia z `switch`:
```c
typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARG,
    STATUS_NO_MEMORY,
    STATUS_IO,
    STATUS_TIMEOUT,
} Status;
```

GCC/Clang potrafią wymusić sprawdzenie wyniku:
```c
__attribute__((warn_unused_result)) Status flash_write(uint32_t addr, const void *d, size_t n);
/* C23: [[nodiscard]] */
```

## 2. `errno`

```c
#include <errno.h>
#include <string.h>

FILE *f = fopen("config.txt", "r");
if (!f) {
    fprintf(stderr, "fopen: %s\n", strerror(errno));   /* np. "No such file or directory" */
}
```
Reguły:
- `errno` jest ustawiane **tylko przy błędzie** – sukces go nie zeruje.
  Funkcje jak `strtol` wymagają `errno = 0;` przed wywołaniem.
- Kolejne wywołanie biblioteki (nawet `printf`) może nadpisać `errno` –
  zapamiętaj je od razu: `int err = errno;`.

## 3. Sprzątanie po błędzie: problem

```c
int process(void) {
    char *a = malloc(100);
    if (!a) return -1;
    char *b = malloc(200);
    if (!b) { free(a); return -1; }
    FILE *f = fopen("x", "r");
    if (!f) { free(b); free(a); return -1; }   /* każdy kolejny krok - coraz dłuższa lista */
    /* ... */
    fclose(f); free(b); free(a);
    return 0;
}
```
Zduplikowany kod sprzątania łatwo zaktualizować w jednym miejscu, a
zapomnieć w innym → wycieki.

## 4. Wzorzec `goto cleanup`

```c
int process(void) {
    int rc = -1;
    char *a = NULL, *b = NULL;
    FILE *f = NULL;

    a = malloc(100);
    if (!a) goto cleanup;
    b = malloc(200);
    if (!b) goto cleanup;
    f = fopen("x", "r");
    if (!f) goto cleanup;

    /* ... właściwa praca ... */
    rc = 0;

cleanup:
    if (f) fclose(f);
    free(b);              /* free(NULL) jest bezpieczne */
    free(a);
    return rc;
}
```
Jedno wyjście, jedna lista sprzątania, zasoby zwalniane w odwrotnej
kolejności. To **zalecany** styl w jądrze Linux, w CPython, w OpenSSL i
w większości dojrzałych projektów w C. `goto` w przód do etykiety
sprzątania nie ma nic wspólnego z "kodem spaghetti".

Wariant z wieloma etykietami (bez sprawdzania `NULL`):
```c
    a = malloc(100);
    if (!a) goto err;
    b = malloc(200);
    if (!b) goto err_free_a;
    return 0;

err_free_a:
    free(a);
err:
    return -1;
```

## 5. Asercje: błędy programisty vs błędy środowiska

```c
#include <assert.h>

void uart_send(const uint8_t *data, size_t len) {
    assert(data != NULL || len == 0);   /* błąd programisty: nie powinno się zdarzyć */
    if (!uart_ready()) return;          /* błąd środowiska: może się zdarzyć, obsłuż */
}
```
- `assert` znika przy `-DNDEBUG` – nigdy nie umieszczaj w nim kodu z
  efektami ubocznymi (`assert(init() == 0)` ✗).
- W firmware zamiast `abort()` zwykle własne `ASSERT`, które loguje
  plik/linię i resetuje urządzenie lub zatrzymuje się w debuggerze (`__BKPT()`).

## 6. Testowanie ścieżek błędu

Ścieżki błędu wykonują się rzadko – i dlatego są najsłabiej
przetestowane. Technika: **wstrzykiwanie błędów** przez podmienialny
alokator / HAL:

```c
typedef void *(*AllocFn)(size_t);
static int fail_after = -1;            /* -1 = nigdy nie zawodź */

void *test_alloc(size_t n) {
    if (fail_after == 0) return NULL;
    if (fail_after > 0) fail_after--;
    return malloc(n);
}
/* test: dla fail_after = 0, 1, 2, ... sprawdź, że funkcja zwraca błąd
   i nic nie wycieka */
```

---

## ⚠️ Pułapki

- Ignorowanie wartości zwracanej (`fclose`, `snprintf`, `write` też mogą zawieść).
- Sprawdzanie `errno` bez wcześniejszego wyzerowania lub bez sprawdzenia,
  że funkcja zgłosiła błąd.
- `if (rc = -1)` zamiast `if (rc == -1)` – GCC ostrzega przy `-Wall`.
- Mieszanie konwencji (`0` = sukces w jednej funkcji, `1` = sukces w drugiej).

---

## ✅ Zadania

1. Przepisz funkcję z sekcji 3 na wzorzec `goto cleanup` i dodaj
   czwarty zasób.
2. Wywołaj `fopen` na nieistniejącym pliku i wypisz `errno` oraz `strerror(errno)`.
3. Ćwiczenie praktyczne: **ex60** (`goto cleanup` + wstrzykiwanie
   błędów alokacji).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Koniec kursu.** Wróć do [spisu treści](README.md) albo przećwicz
wszystko w [../exercises](../exercises).
