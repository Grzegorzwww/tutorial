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
    v.type = VAL_INT;
    v.as.i = i;
    return v;
}

Value value_float(float f) {
    Value v = {0};
    v.type = VAL_FLOAT;
    v.as.f = f;
    return v;
}

Value value_str(const char *s) {
    Value v = {0};
    v.type = VAL_STR;
    v.as.s = s;
    return v;
}

int value_to_string(const Value *v, char *buf, size_t size) {
    switch (v->type) {
        case VAL_INT: return snprintf(buf, size, "int:%ld", (long)v->as.i);
        case VAL_FLOAT: return snprintf(buf, size, "float:%.2f", (double)v->as.f);
        case VAL_STR: return snprintf(buf, size, "str:%s", v->as.s);
    }
    return -1;
}

bool value_equals(const Value *a, const Value *b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VAL_INT: return a->as.i == b->as.i;
        case VAL_FLOAT: return a->as.f == b->as.f;
        case VAL_STR: return strcmp(a->as.s, b->as.s) == 0;
    }
    return false;
}

bool value_as_int(const Value *v, int32_t *out) {
    if (v->type != VAL_INT) return false;
    *out = v->as.i;
    return true;
}

uint32_t float_bits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    return u;
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
