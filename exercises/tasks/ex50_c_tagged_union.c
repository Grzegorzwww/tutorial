/* Zadanie 50 (C, rozdzial 5): unia z tagiem (tagged union)
 *
 *   value_int / value_float / value_str:  "konstruktory" ustawiajace tag i pole unii
 *   value_to_string:  "int:42", "float:1.50" (2 miejsca po przecinku), "str:abc";
 *                     zwraca wynik snprintf
 *   value_equals:     te same typy i rowne wartosci (napisy przez strcmp)
 *   value_as_int:     true i *out dla VAL_INT, inaczej false (*out bez zmian)
 *   float_bits:       bity float jako uint32_t - przez memcpy, bez rzutowania wskaznikow
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum { VAL_INT, VAL_FLOAT, VAL_STR } ValueType;

typedef struct {
    ValueType type;
    union {
        int32_t i;
        float f;
        const char *s;
    } as;
} Value;

Value value_int(int32_t i) {
    Value v = {0};
    /* TODO */
    return v;
}

Value value_float(float f) {
    Value v = {0};
    /* TODO */
    return v;
}

Value value_str(const char *s) {
    Value v = {0};
    /* TODO */
    return v;
}

int value_to_string(const Value *v, char *buf, size_t size) {
    /* TODO: switch po v->type */
    return -1;
}

bool value_equals(const Value *a, const Value *b) {
    /* TODO */
    return false;
}

bool value_as_int(const Value *v, int32_t *out) {
    /* TODO */
    return false;
}

uint32_t float_bits(float f) {
    /* TODO */
    return 0;
}

int main(void) {
    char buf[32];
    Value i = value_int(42), f = value_float(1.5f), s = value_str("abc");

    assert(i.type == VAL_INT && i.as.i == 42);
    assert(f.type == VAL_FLOAT && f.as.f == 1.5f);
    assert(s.type == VAL_STR && strcmp(s.as.s, "abc") == 0);

    assert(value_to_string(&i, buf, sizeof buf) == 6 && strcmp(buf, "int:42") == 0);
    value_to_string(&f, buf, sizeof buf);
    assert(strcmp(buf, "float:1.50") == 0);
    value_to_string(&s, buf, sizeof buf);
    assert(strcmp(buf, "str:abc") == 0);

    char other[] = "abc";
    Value s2 = value_str(other);
    assert(value_equals(&s, &s2));                /* rozne adresy, ta sama tresc */
    Value i2 = value_int(42), i3 = value_int(43);
    assert(value_equals(&i, &i2) && !value_equals(&i, &i3));
    assert(!value_equals(&i, &f));

    int32_t out = -1;
    assert(value_as_int(&i, &out) && out == 42);
    out = -1;
    assert(!value_as_int(&f, &out) && out == -1);

    assert(float_bits(1.0f) == 0x3F800000u);
    assert(float_bits(-2.0f) == 0xC0000000u);
    assert(float_bits(0.0f) == 0);

    puts("OK");
    return 0;
}
