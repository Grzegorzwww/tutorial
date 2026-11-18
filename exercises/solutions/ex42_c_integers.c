#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool add_u32_checked(uint32_t a, uint32_t b, uint32_t *out) {
    if (a > UINT32_MAX - b) return false;
    *out = a + b;
    return true;
}

bool add_i32_checked(int32_t a, int32_t b, int32_t *out) {
    if ((b > 0 && a > INT32_MAX - b) || (b < 0 && a < INT32_MIN - b)) return false;
    *out = a + b;
    return true;
}

uint8_t add_sat_u8(uint8_t a, uint8_t b) {
    return a > UINT8_MAX - b ? UINT8_MAX : (uint8_t)(a + b);
}

int16_t add_sat_i16(int16_t a, int16_t b) {
    int sum = a + b;    /* promocja do int: dla int16_t nie moze sie przepelnic */
    if (sum > INT16_MAX) return INT16_MAX;
    if (sum < INT16_MIN) return INT16_MIN;
    return (int16_t)sum;
}

bool less_mixed(int32_t a, uint32_t b) {
    return a < 0 || (uint32_t)a < b;
}

bool timeout_expired(uint32_t now, uint32_t start, uint32_t timeout) {
    return (uint32_t)(now - start) >= timeout;
}

uint32_t abs_u32(int32_t x) {
    return x < 0 ? 0u - (uint32_t)x : (uint32_t)x;
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
