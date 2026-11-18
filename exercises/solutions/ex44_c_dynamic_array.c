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
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

bool vec_reserve(IntVec *v, size_t new_cap) {
    if (new_cap <= v->cap) return true;
    int *tmp = realloc(v->data, new_cap * sizeof *tmp);
    if (tmp == NULL) return false;   /* v->data nadal wazne */
    v->data = tmp;
    v->cap = new_cap;
    return true;
}

static bool vec_grow_if_full(IntVec *v) {
    if (v->size < v->cap) return true;
    return vec_reserve(v, v->cap ? v->cap * 2 : 4);
}

bool vec_push(IntVec *v, int value) {
    if (!vec_grow_if_full(v)) return false;
    v->data[v->size++] = value;
    return true;
}

bool vec_pop(IntVec *v, int *out) {
    if (v->size == 0) return false;
    *out = v->data[--v->size];
    return true;
}

bool vec_insert(IntVec *v, size_t idx, int value) {
    if (idx > v->size) return false;
    if (!vec_grow_if_full(v)) return false;
    memmove(&v->data[idx + 1], &v->data[idx], (v->size - idx) * sizeof v->data[0]);
    v->data[idx] = value;
    ++v->size;
    return true;
}

void vec_free(IntVec *v) {
    free(v->data);
    vec_init(v);
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
