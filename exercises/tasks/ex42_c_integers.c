/* Zadanie 42 (C, rozdzial 1): bezpieczna arytmetyka calkowita
 *
 * Bez __builtin_* i bez typow szerszych niz argumenty (cwiczymy sprawdzanie PRZED operacja).
 *   add_u32_checked:  false przy przepelnieniu (*out bez zmian)
 *   add_i32_checked:  false przy przepelnieniu w gore lub w dol (*out bez zmian)
 *   add_sat_u8:       dodawanie z nasyceniem do 255
 *   add_sat_i16:      dodawanie z nasyceniem do zakresu int16_t
 *   less_mixed:       poprawne a < b dla int32_t i uint32_t (-1 < 0u ma byc true!)
 *   timeout_expired:  czy od `start` minelo >= timeout tickow; odporne na przekrecenie licznika
 *   abs_u32:          modul liczby jako uint32_t, poprawny takze dla INT32_MIN
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool add_u32_checked(uint32_t a, uint32_t b, uint32_t *out) {
    /* TODO */
    return false;
}

bool add_i32_checked(int32_t a, int32_t b, int32_t *out) {
    /* TODO */
    return false;
}

uint8_t add_sat_u8(uint8_t a, uint8_t b) {
    /* TODO */
    return 0;
}

int16_t add_sat_i16(int16_t a, int16_t b) {
    /* TODO */
    return 0;
}

bool less_mixed(int32_t a, uint32_t b) {
    /* TODO */
    return false;
}

bool timeout_expired(uint32_t now, uint32_t start, uint32_t timeout) {
    /* TODO */
    return false;
}

uint32_t abs_u32(int32_t x) {
    /* TODO */
    return 0;
}

int main(void) {
    uint32_t u = 7;
    assert(add_u32_checked(1, 2, &u) && u == 3);
    assert(add_u32_checked(UINT32_MAX - 1, 1, &u) && u == UINT32_MAX);
    u = 7;
    assert(!add_u32_checked(UINT32_MAX, 1, &u) && u == 7);

    int32_t s = 7;
    assert(add_i32_checked(-5, 3, &s) && s == -2);
    assert(add_i32_checked(INT32_MAX, INT32_MIN, &s) && s == -1);
    s = 7;
    assert(!add_i32_checked(INT32_MAX, 1, &s) && s == 7);
    assert(!add_i32_checked(INT32_MIN, -1, &s) && s == 7);

    assert(add_sat_u8(200, 50) == 250);
    assert(add_sat_u8(200, 100) == 255);
    assert(add_sat_i16(30000, 10000) == INT16_MAX);
    assert(add_sat_i16(-30000, -10000) == INT16_MIN);
    assert(add_sat_i16(-100, 50) == -50);

    assert(less_mixed(-1, 0));
    assert(less_mixed(5, 6));
    assert(!less_mixed(6, 5));
    assert(less_mixed(INT32_MIN, 0));
    assert(!less_mixed(INT32_MAX, 100));
    assert(less_mixed(INT32_MAX, UINT32_MAX));

    assert(timeout_expired(1100, 1000, 100));
    assert(!timeout_expired(1099, 1000, 100));
    assert(timeout_expired(50, UINT32_MAX - 49, 100));   /* licznik przekrecil sie przez zero */
    assert(!timeout_expired(10, UINT32_MAX - 49, 100));

    assert(abs_u32(5) == 5);
    assert(abs_u32(-5) == 5);
    assert(abs_u32(INT32_MIN) == 2147483648u);

    puts("OK");
    return 0;
}
