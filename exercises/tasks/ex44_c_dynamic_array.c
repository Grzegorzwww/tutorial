/* Zadanie 44 (C, rozdzial 3): dynamiczna tablica (malloc/realloc/free)
 *
 *   vec_init:     pusta tablica (data NULL, size 0, cap 0)
 *   vec_reserve:  zapewnia pojemnosc >= new_cap (nie zmniejsza); false przy braku pamieci
 *   vec_push:     dodaje na koniec; gdy brak miejsca - pojemnosc 4, potem podwajanie
 *                 (cap = cap ? cap * 2 : 4)
 *   vec_pop:      zdejmuje ostatni do *out; false gdy pusta (*out bez zmian)
 *   vec_insert:   wstawia na pozycje idx (0..size), przesuwa reszte (memmove); idx > size -> false
 *   vec_free:     zwalnia pamiec i wraca do stanu po vec_init
 *
 * Pamietaj o poprawnym wzorcu realloc: wynik do zmiennej tymczasowej!
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *data;
    size_t size;
    size_t cap;
} IntVec;

void vec_init(IntVec *v) {
    /* TODO */
}

bool vec_reserve(IntVec *v, size_t new_cap) {
    /* TODO */
    return false;
}

bool vec_push(IntVec *v, int value) {
    /* TODO */
    return false;
}

bool vec_pop(IntVec *v, int *out) {
    /* TODO */
    return false;
}

bool vec_insert(IntVec *v, size_t idx, int value) {
    /* TODO */
    return false;
}

void vec_free(IntVec *v) {
    /* TODO */
}

int main(void) {
    IntVec v;
    vec_init(&v);
    assert(v.data == NULL && v.size == 0 && v.cap == 0);

    for (int i = 0; i < 10; ++i) assert(vec_push(&v, i * i));
    assert(v.size == 10 && v.cap == 16);
    assert(v.data[0] == 0 && v.data[3] == 9 && v.data[9] == 81);

    int out = -1;
    assert(vec_pop(&v, &out) && out == 81 && v.size == 9);

    assert(vec_reserve(&v, 100) && v.cap == 100 && v.size == 9);
    assert(vec_reserve(&v, 10) && v.cap == 100);    /* nie zmniejsza */

    IntVec w;
    vec_init(&w);
    assert(vec_insert(&w, 0, 2));
    assert(vec_insert(&w, 0, 1));
    assert(vec_insert(&w, 2, 4));
    assert(vec_insert(&w, 2, 3));
    assert(!vec_insert(&w, 9, 99));
    assert(w.size == 4);
    assert(w.data[0] == 1 && w.data[1] == 2 && w.data[2] == 3 && w.data[3] == 4);

    vec_free(&w);
    assert(w.data == NULL && w.size == 0 && w.cap == 0);
    out = 123;
    assert(!vec_pop(&w, &out) && out == 123);

    vec_free(&v);
    puts("OK");
    return 0;
}
