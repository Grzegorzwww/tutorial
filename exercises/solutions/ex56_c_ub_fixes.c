#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int midpoint(int lo, int hi) {
    /* roznica liczona bez znaku: hi - lo moze nie miescic sie w int (np. INT_MIN..INT_MAX) */
    return lo + (int)(((unsigned)hi - (unsigned)lo) / 2u);
}

uint32_t rotl32(uint32_t x, unsigned n) {
    n &= 31u;
    return (x << n) | (x >> ((32u - n) & 31u));
}

uint32_t abs_diff_i32(int32_t a, int32_t b) {
    return a >= b ? (uint32_t)a - (uint32_t)b : (uint32_t)b - (uint32_t)a;
}

bool mul_overflows_u32(uint32_t a, uint32_t b) {
    return b != 0 && a > UINT32_MAX / b;
}

int sign_extend_12(uint16_t raw) {
    raw &= 0x0FFFu;
    return (raw & 0x800u) ? (int)raw - 4096 : (int)raw;
}

uint32_t read_u32_be(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | (uint32_t)p[3];
}

void drop_front(uint8_t *buf, size_t n, size_t k) {
    if (k >= n) {
        memset(buf, 0, n);
        return;
    }
    memmove(buf, buf + k, n - k);
    memset(buf + (n - k), 0, k);
}

bool is_aligned(const void *p, size_t align) {
    return ((uintptr_t)p & (align - 1)) == 0;
}

int main(void) {
    assert(midpoint(2, 8) == 5);
    assert(midpoint(-10, -4) == -7);
    assert(midpoint(INT_MAX - 2, INT_MAX) == INT_MAX - 1);
    assert(midpoint(INT_MIN, INT_MIN + 2) == INT_MIN + 1);

    assert(rotl32(0x80000001u, 1) == 0x00000003u);
    assert(rotl32(0x12345678u, 0) == 0x12345678u);
    assert(rotl32(0x12345678u, 32) == 0x12345678u);
    assert(rotl32(0x12345678u, 8) == 0x34567812u);
    assert(rotl32(0x12345678u, 40) == 0x34567812u);

    assert(abs_diff_i32(3, 10) == 7 && abs_diff_i32(10, 3) == 7);
    assert(abs_diff_i32(INT32_MIN, INT32_MAX) == UINT32_MAX);
    assert(abs_diff_i32(INT32_MAX, INT32_MIN) == UINT32_MAX);

    assert(!mul_overflows_u32(65535, 65537));
    assert(mul_overflows_u32(65536, 65536));
    assert(!mul_overflows_u32(0, UINT32_MAX) && !mul_overflows_u32(UINT32_MAX, 1));

    assert(sign_extend_12(0x000) == 0);
    assert(sign_extend_12(0x7FF) == 2047);
    assert(sign_extend_12(0x800) == -2048);
    assert(sign_extend_12(0xFFF) == -1);

    uint8_t raw[] = {0xEE, 0x11, 0x22, 0x33, 0x44};
    assert(read_u32_be(raw + 1) == 0x11223344u);        /* niewyrownany adres */

    uint8_t buf[] = {1, 2, 3, 4, 5, 6};
    drop_front(buf, 6, 2);
    assert(memcmp(buf, (uint8_t[]){3, 4, 5, 6, 0, 0}, 6) == 0);
    drop_front(buf, 6, 10);
    assert(memcmp(buf, (uint8_t[]){0, 0, 0, 0, 0, 0}, 6) == 0);

    _Alignas(8) uint8_t aligned[16];
    assert(is_aligned(aligned, 8) && is_aligned(aligned + 4, 4));
    assert(!is_aligned(aligned + 1, 2) && !is_aligned(aligned + 4, 8));

    puts("OK");
    return 0;
}
