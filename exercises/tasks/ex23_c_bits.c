/* Zadanie 23 (C): operacje bitowe (klasyka embedded)
 *
 * Nie uzywaj __builtin_* ani bibliotek. Numeracja bitow od 0 (LSB).
 *   set_bit / clear_bit / toggle_bit / test_bit
 *   popcount:            liczba jedynek w slowie
 *   is_power_of_two:     0 NIE jest potega dwojki
 *   extract_field:       `width` bitow od pozycji `pos` (1 <= width <= 32, pos + width <= 32)
 *   insert_field:        wstawia `value` (obcieta do `width` bitow) w pole [pos, pos+width)
 *   swap_bytes32:        0x11223344 -> 0x44332211
 *   reverse_bits8:       0b00010110 -> 0b01101000
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint32_t set_bit(uint32_t x, unsigned n) {
    /* TODO */
    return x;
}

uint32_t clear_bit(uint32_t x, unsigned n) {
    /* TODO */
    return x;
}

uint32_t toggle_bit(uint32_t x, unsigned n) {
    /* TODO */
    return x;
}

bool test_bit(uint32_t x, unsigned n) {
    /* TODO */
    return false;
}

unsigned popcount(uint32_t x) {
    /* TODO */
    return 0;
}

bool is_power_of_two(uint32_t x) {
    /* TODO */
    return false;
}

uint32_t extract_field(uint32_t x, unsigned pos, unsigned width) {
    /* TODO */
    return 0;
}

uint32_t insert_field(uint32_t x, unsigned pos, unsigned width, uint32_t value) {
    /* TODO */
    return x;
}

uint32_t swap_bytes32(uint32_t x) {
    /* TODO */
    return x;
}

uint8_t reverse_bits8(uint8_t x) {
    /* TODO */
    return x;
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
