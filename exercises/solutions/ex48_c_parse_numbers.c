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
    char *end;
    errno = 0;
    long long v = strtoll(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE) return false;
    if (v < INT32_MIN || v > INT32_MAX) return false;
    *out = (int32_t)v;
    return true;
}

static bool has_minus(const char *s) {
    while (isspace((unsigned char)*s)) ++s;
    return *s == '-';
}

bool parse_u8(const char *s, uint8_t *out) {
    if (has_minus(s)) return false;
    char *end;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v > UINT8_MAX) return false;
    *out = (uint8_t)v;
    return true;
}

bool parse_hex_u32(const char *s, uint32_t *out) {
    if (has_minus(s)) return false;
    const char *digits = s;
    if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X')) digits += 2;
    if (!isxdigit((unsigned char)*digits)) return false;   /* "0x" bez cyfr */

    char *end;
    errno = 0;
    unsigned long long v = strtoull(digits, &end, 16);
    if (*end != '\0' || errno == ERANGE || v > UINT32_MAX) return false;
    *out = (uint32_t)v;
    return true;
}

static bool equals_ignore_case(const char *a, const char *b) {
    for (; *a && *b; ++a, ++b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    }
    return *a == *b;
}

bool parse_bool(const char *s, bool *out) {
    static const char *const yes[] = {"1", "true", "on"};
    static const char *const no[] = {"0", "false", "off"};
    for (size_t i = 0; i < 3; ++i) {
        if (equals_ignore_case(s, yes[i])) { *out = true; return true; }
        if (equals_ignore_case(s, no[i])) { *out = false; return true; }
    }
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
