#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint32_t set_bit(uint32_t x, unsigned n) { return x | ((uint32_t)1 << n); }

uint32_t clear_bit(uint32_t x, unsigned n) { return x & ~((uint32_t)1 << n); }

uint32_t toggle_bit(uint32_t x, unsigned n) { return x ^ ((uint32_t)1 << n); }

bool test_bit(uint32_t x, unsigned n) { return (x >> n) & 1u; }

unsigned popcount(uint32_t x) {
    unsigned count = 0;
    while (x) {
        x &= x - 1;  /* zeruje najmlodszy ustawiony bit */
        ++count;
    }
    return count;
}

bool is_power_of_two(uint32_t x) { return x != 0 && (x & (x - 1)) == 0; }

static uint32_t field_mask(unsigned width) {
    return width >= 32 ? 0xFFFFFFFFu : (((uint32_t)1 << width) - 1);
}

uint32_t extract_field(uint32_t x, unsigned pos, unsigned width) {
    return (x >> pos) & field_mask(width);
}

uint32_t insert_field(uint32_t x, unsigned pos, unsigned width, uint32_t value) {
    uint32_t mask = field_mask(width);
    return (x & ~(mask << pos)) | ((value & mask) << pos);
}

uint32_t swap_bytes32(uint32_t x) {
    return (x >> 24) | ((x >> 8) & 0x0000FF00u) | ((x << 8) & 0x00FF0000u) | (x << 24);
}

uint8_t reverse_bits8(uint8_t x) {
    uint8_t out = 0;
    for (int i = 0; i < 8; ++i) {
        out = (uint8_t)((out << 1) | (x & 1u));
        x >>= 1;
    }
    return out;
}

int main(void) {
    assert(set_bit(0, 3) == 0x8);
    assert(set_bit(0x8, 3) == 0x8);
    assert(set_bit(0, 31) == 0x80000000u);
    assert(clear_bit(0xFF, 0) == 0xFE);
    assert(clear_bit(0, 5) == 0);
    assert(toggle_bit(0x5, 0) == 0x4);
    assert(toggle_bit(0x5, 1) == 0x7);
    assert(test_bit(0x10, 4) && !test_bit(0x10, 3));
    assert(test_bit(0x80000000u, 31));

    assert(popcount(0) == 0);
    assert(popcount(0xFFFFFFFFu) == 32);
    assert(popcount(0xF0F0) == 8);
    assert(popcount(0x80000001u) == 2);

    assert(is_power_of_two(1) && is_power_of_two(64) && is_power_of_two(0x80000000u));
    assert(!is_power_of_two(0) && !is_power_of_two(12) && !is_power_of_two(0xFFFFFFFFu));

    assert(extract_field(0xABCD1234u, 8, 8) == 0x12);
    assert(extract_field(0xABCD1234u, 0, 4) == 0x4);
    assert(extract_field(0xABCD1234u, 28, 4) == 0xA);
    assert(extract_field(0xABCD1234u, 0, 32) == 0xABCD1234u);

    assert(insert_field(0, 4, 4, 0xF) == 0xF0);
    assert(insert_field(0xFFFFFFFFu, 8, 8, 0) == 0xFFFF00FFu);
    assert(insert_field(0, 0, 4, 0xFF) == 0xF);
    assert(insert_field(0x12345678u, 0, 32, 0xDEADBEEFu) == 0xDEADBEEFu);

    assert(swap_bytes32(0x11223344u) == 0x44332211u);
    assert(swap_bytes32(0x000000FFu) == 0xFF000000u);

    assert(reverse_bits8(0x01) == 0x80);
    assert(reverse_bits8(0x0F) == 0xF0);
    assert(reverse_bits8(0x16) == 0x68);

    puts("OK");
    return 0;
}
