#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct Bad {
    uint8_t a;
    uint32_t b;
    uint8_t c;
    uint16_t d;
};

struct Good {
    uint32_t b;
    uint16_t d;
    uint8_t a;
    uint8_t c;
};

struct Telemetry {
    uint16_t id;
    uint32_t timestamp;
    int16_t temp_centi;
    uint8_t flags;
};

#define TELEMETRY_WIRE_SIZE 9

void write_u16_le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

void write_u32_le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

size_t telemetry_pack(const struct Telemetry *t, uint8_t *out, size_t out_size) {
    if (out_size < TELEMETRY_WIRE_SIZE) return 0;
    write_u16_le(&out[0], t->id);
    write_u32_le(&out[2], t->timestamp);
    write_u16_le(&out[6], (uint16_t)t->temp_centi);
    out[8] = t->flags;
    return TELEMETRY_WIRE_SIZE;
}

bool telemetry_unpack(const uint8_t *in, size_t len, struct Telemetry *out) {
    if (len < TELEMETRY_WIRE_SIZE) return false;
    out->id = read_u16_le(&in[0]);
    out->timestamp = read_u32_le(&in[2]);
    uint16_t raw_temp = read_u16_le(&in[6]);
    out->temp_centi = raw_temp > INT16_MAX ? (int16_t)(raw_temp - 65536) : (int16_t)raw_temp;
    out->flags = in[8];
    return true;
}

bool telemetry_equals(const struct Telemetry *a, const struct Telemetry *b) {
    return a->id == b->id && a->timestamp == b->timestamp && a->temp_centi == b->temp_centi &&
           a->flags == b->flags;
}

int main(void) {
    printf("sizeof(struct Bad) = %lu, sizeof(struct Good) = %lu\n",
           (unsigned long)sizeof(struct Bad), (unsigned long)sizeof(struct Good));
    assert(sizeof(struct Good) == 8);
    assert(offsetof(struct Good, b) == 0);

    uint8_t buf[4];
    write_u16_le(buf, 0x1234);
    assert(buf[0] == 0x34 && buf[1] == 0x12);
    assert(read_u16_le(buf) == 0x1234);
    write_u32_le(buf, 0xA1B2C3D4u);
    assert(buf[0] == 0xD4 && buf[1] == 0xC3 && buf[2] == 0xB2 && buf[3] == 0xA1);
    assert(read_u32_le(buf) == 0xA1B2C3D4u);

    struct Telemetry t = {.id = 0x0102, .timestamp = 0x0A0B0C0Du, .temp_centi = -1234, .flags = 0x80};
    uint8_t wire[16];
    memset(wire, 0xEE, sizeof wire);
    assert(telemetry_pack(&t, wire, sizeof wire) == TELEMETRY_WIRE_SIZE);
    const uint8_t expected[TELEMETRY_WIRE_SIZE] = {0x02, 0x01, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E, 0xFB, 0x80};
    assert(memcmp(wire, expected, sizeof expected) == 0);
    assert(wire[9] == 0xEE);                                  /* nic poza ramka */
    assert(telemetry_pack(&t, wire, 8) == 0);

    struct Telemetry back = {0};
    assert(telemetry_unpack(expected, sizeof expected, &back));
    assert(back.temp_centi == -1234);
    assert(telemetry_equals(&t, &back));
    assert(!telemetry_unpack(expected, 8, &back));

    back.flags = 0;
    assert(!telemetry_equals(&t, &back));

    puts("OK");
    return 0;
}
