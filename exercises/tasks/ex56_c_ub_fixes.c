/* Zadanie 56 (C, rozdzial 8): usuwanie niezdefiniowanego zachowania
 *
 * Przy kazdej funkcji jest "naiwna" wersja z UB. Napisz poprawna.
 *   midpoint:          srodek [lo, hi] (lo <= hi) bez przepelnienia; naiwnie: (lo + hi) / 2
 *   rotl32:            rotacja w lewo o n bitow (n dowolne, liczone mod 32);
 *                      naiwnie: (x << n) | (x >> (32 - n))  -> UB dla n == 0 i n >= 32
 *   abs_diff_i32:      |a - b| jako uint32_t; naiwnie: abs(a - b) -> przepelnienie
 *   mul_overflows_u32: czy a * b nie miesci sie w uint32_t (sprawdzenie PRZED mnozeniem)
 *   sign_extend_12:    12-bitowa wartosc U2 z ADC (0..0xFFF) -> int; 0x800..0xFFF to liczby ujemne
 *   read_u32_be:       big-endian z bufora pod DOWOLNYM adresem;
 *                      naiwnie: *(uint32_t *)p -> zle wyrownanie + strict aliasing + endianness
 *   drop_front:        usuwa pierwsze k bajtow bufora n-bajtowego, przesuwajac reszte
 *                      na poczatek i zerujac koniec; naiwnie memcpy (nakladajace sie bufory!)
 *   is_aligned:        czy wskaznik jest wyrownany do `align` (potega dwojki)
 */
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int midpoint(int lo, int hi) {
    /* TODO */
    return 0;
}

uint32_t rotl32(uint32_t x, unsigned n) {
    /* TODO */
    return 0;
}

uint32_t abs_diff_i32(int32_t a, int32_t b) {
    /* TODO */
    return 0;
}

bool mul_overflows_u32(uint32_t a, uint32_t b) {
    /* TODO */
    return false;
}

int sign_extend_12(uint16_t raw) {
    /* TODO */
    return 0;
}

uint32_t read_u32_be(const uint8_t *p) {
    /* TODO */
    return 0;
}

void drop_front(uint8_t *buf, size_t n, size_t k) {
    /* TODO (k >= n -> wyzeruj caly bufor) */
}

bool is_aligned(const void *p, size_t align) {
    /* TODO */
    return false;
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
