/* Zadanie 48 (C, rozdzial 4): parsowanie liczb z kontrola bledow
 *
 * Wszystkie funkcje przy bledzie zwracaja false i NIE zmieniaja *out.
 * Blad to: brak cyfr, smieci za liczba, wartosc poza zakresem typu.
 *   parse_i32:      dziesietnie, opcjonalny znak (strtol/strtoll + errno + endptr)
 *   parse_u8:       0..255; minus to blad (strtoul sam go NIE odrzuca!)
 *   parse_hex_u32:  szesnastkowo, opcjonalny prefiks 0x/0X, minus to blad
 *   parse_bool:     "1"/"true"/"on" oraz "0"/"false"/"off", bez rozrozniania wielkosci liter
 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool parse_i32(const char *s, int32_t *out) {
    /* TODO */
    return false;
}

bool parse_u8(const char *s, uint8_t *out) {
    /* TODO */
    return false;
}

bool parse_hex_u32(const char *s, uint32_t *out) {
    /* TODO */
    return false;
}

bool parse_bool(const char *s, bool *out) {
    /* TODO */
    return false;
}

int main(void) {
    int32_t i = 7;
    assert(parse_i32("42", &i) && i == 42);
    assert(parse_i32("-17", &i) && i == -17);
    assert(parse_i32("2147483647", &i) && i == INT32_MAX);
    assert(parse_i32("-2147483648", &i) && i == INT32_MIN);
    i = 7;
    assert(!parse_i32("2147483648", &i) && i == 7);
    assert(!parse_i32("99999999999999999999", &i) && i == 7);
    assert(!parse_i32("12a", &i) && i == 7);
    assert(!parse_i32("", &i) && i == 7);
    assert(!parse_i32("-", &i) && i == 7);

    uint8_t u = 7;
    assert(parse_u8("0", &u) && u == 0);
    assert(parse_u8("255", &u) && u == 255);
    u = 7;
    assert(!parse_u8("256", &u) && u == 7);
    assert(!parse_u8("-1", &u) && u == 7);
    assert(!parse_u8(" -1", &u) && u == 7);
    assert(!parse_u8("1.5", &u) && u == 7);

    uint32_t h = 7;
    assert(parse_hex_u32("ff", &h) && h == 0xFF);
    assert(parse_hex_u32("0xDEADBEEF", &h) && h == 0xDEADBEEFu);
    assert(parse_hex_u32("0XFFFFFFFF", &h) && h == UINT32_MAX);
    h = 7;
    assert(!parse_hex_u32("0x100000000", &h) && h == 7);
    assert(!parse_hex_u32("0x", &h) && h == 7);
    assert(!parse_hex_u32("xyz", &h) && h == 7);
    assert(!parse_hex_u32("-1", &h) && h == 7);

    bool b = false;
    assert(parse_bool("TRUE", &b) && b);
    assert(parse_bool("off", &b) && !b);
    assert(parse_bool("1", &b) && b);
    assert(parse_bool("False", &b) && !b);
    b = true;
    assert(!parse_bool("yes", &b) && b);
    assert(!parse_bool("", &b) && b);
    assert(!parse_bool("truex", &b) && b);

    puts("OK");
    return 0;
}
