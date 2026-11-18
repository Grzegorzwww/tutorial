/* Zadanie 54 (C, rozdzial 7): bezpieczne makra
 *
 * Zastap definicje-zaslepki poprawnymi makrami:
 *   ARRAY_SIZE(a)           liczba elementow tablicy
 *   MIN(a, b) / MAX(a, b)   z nawiasami wokol argumentow i CALEGO wyrazenia
 *   CLAMP(x, lo, hi)        ogranicza x do [lo, hi] (moze uzywac MIN/MAX)
 *   SWAP(type, a, b)        zamiana wartosci; musi dzialac jako jedna instrukcja
 *                           w if/else bez klamer -> do { ... } while (0)
 *   STR(x) / XSTR(x)        STR robi napis z tekstu argumentu, XSTR najpierw go rozwija
 *   BIT(n)                  maska z jednym bitem, typ unsigned (1u)
 *   CONTAINER_OF(ptr, type, member)  od wskaznika na pole do wskaznika na cala strukture
 *
 * Na koniec odpowiedz w komentarzu ANSWER: ile razy wykona sie ++i w MAX(++i, 0)?
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) ((size_t)0)                 /* TODO */
#define MIN(a, b) 0                               /* TODO */
#define MAX(a, b) 0                               /* TODO */
#define CLAMP(x, lo, hi) 0                        /* TODO */
#define SWAP(type, a, b) do { } while (0)         /* TODO */
#define STR(x) ""                                 /* TODO */
#define XSTR(x) ""                                /* TODO */
#define BIT(n) 0u                                 /* TODO */
#define CONTAINER_OF(ptr, type, member) ((type *)NULL)   /* TODO */

#define FW_VERSION 42

typedef struct {
    int id;
    double reading;
    char tag[8];
} Sensor;

int main(void) {
    int arr[7];
    const char *names[] = {"a", "b", "c"};
    assert(ARRAY_SIZE(arr) == 7);
    assert(ARRAY_SIZE(names) == 3);

    assert(MIN(3, 5) == 3 && MAX(3, 5) == 5);
    assert(10 - MIN(3, 5) == 7);          /* wymaga nawiasow wokol calego wyrazenia */
    assert(2 * MAX(1 + 1, 3) == 6);
    assert(MIN(1 + 2, 2 * 2) == 3);       /* wymaga nawiasow wokol argumentow */

    assert(CLAMP(15, 0, 10) == 10);
    assert(CLAMP(-5, 0, 10) == 0);
    assert(CLAMP(5, 0, 10) == 5);

    int x = 1, y = 2, other = 0;
    if (x < y)
        SWAP(int, x, y);
    else
        other = 1;
    assert(x == 2 && y == 1 && other == 0);

    assert(strcmp(STR(FW_VERSION), "FW_VERSION") == 0);
    assert(strcmp(XSTR(FW_VERSION), "42") == 0);
    assert(strcmp(STR(a + b), "a + b") == 0);

    assert(BIT(0) == 1u && BIT(5) == 32u && BIT(31) == 0x80000000u);

    Sensor s = {7, 21.5, "temp"};
    double *field = &s.reading;
    Sensor *owner = CONTAINER_OF(field, Sensor, reading);
    assert(owner == &s && owner->id == 7);
    char *tag = s.tag;
    assert(CONTAINER_OF(tag, Sensor, tag) == &s);

    puts("OK");
    return 0;
}

/*
ANSWER:
(tu wpisz)
*/
