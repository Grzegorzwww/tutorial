/* Zadanie 25 (C): bufor cykliczny (ring buffer) na bajty
 *
 * Pojemnosc stala (RB_CAPACITY). Struktura jest gotowa: head to indeks
 * najstarszego elementu, count - liczba elementow.
 * Indeks zapisu = (head + count) % RB_CAPACITY.
 *
 *   rb_init:      zeruje bufor
 *   rb_push:      false, gdy pelny (bufor sie nie zmienia)
 *   rb_pop:       false, gdy pusty (*out sie nie zmienia); usuwa najstarszy element
 *   rb_count / rb_is_full / rb_is_empty
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RB_CAPACITY 4

typedef struct {
    uint8_t data[RB_CAPACITY];
    size_t head;
    size_t count;
} RingBuffer;

void rb_init(RingBuffer *rb) {
    /* TODO */
}

bool rb_push(RingBuffer *rb, uint8_t value) {
    /* TODO */
    return false;
}

bool rb_pop(RingBuffer *rb, uint8_t *out) {
    /* TODO */
    return false;
}

size_t rb_count(const RingBuffer *rb) {
    /* TODO */
    return 99;
}

bool rb_is_full(const RingBuffer *rb) {
    /* TODO */
    return false;
}

bool rb_is_empty(const RingBuffer *rb) {
    /* TODO */
    return false;
}

int main(void) {
    RingBuffer rb;
    rb_init(&rb);
    assert(rb_is_empty(&rb) && !rb_is_full(&rb) && rb_count(&rb) == 0);

    uint8_t out = 0xEE;
    assert(!rb_pop(&rb, &out));
    assert(out == 0xEE);

    for (uint8_t i = 1; i <= 4; ++i) assert(rb_push(&rb, i));
    assert(rb_is_full(&rb) && rb_count(&rb) == 4);
    assert(!rb_push(&rb, 99));
    assert(rb_count(&rb) == 4);

    assert(rb_pop(&rb, &out) && out == 1);
    assert(rb_pop(&rb, &out) && out == 2);
    assert(rb_count(&rb) == 2);

    assert(rb_push(&rb, 5));
    assert(rb_push(&rb, 6));  /* zapis "zawija" sie na poczatek tablicy */
    assert(rb_is_full(&rb));

    uint8_t expected[] = {3, 4, 5, 6};
    for (int i = 0; i < 4; ++i) {
        assert(rb_pop(&rb, &out) && out == expected[i]);
    }
    assert(rb_is_empty(&rb));
    assert(!rb_pop(&rb, &out));

    puts("OK");
    return 0;
}
