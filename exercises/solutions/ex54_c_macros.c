#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN(MAX((x), (lo)), (hi))
#define SWAP(type, a, b) do { type swap_tmp_ = (a); (a) = (b); (b) = swap_tmp_; } while (0)
#define STR(x) #x
#define XSTR(x) STR(x)
#define BIT(n) (1u << (n))
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

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
Dwa razy, gdy ++i > 0 (raz w porownaniu, raz w wybranej galezi), a raz, gdy ++i <= 0.
Makro wkleja tekst argumentu w kazde miejsce uzycia - stad funkcje static inline
sa bezpieczniejsze dla argumentow z efektami ubocznymi.
*/
