/* Zadanie 49 (C, rozdzial 5): uklad struktur i serializacja ramki
 *
 * 1) struct Good: te same pola co struct Bad, ale w kolejnosci minimalizujacej padding
 *    (sizeof(struct Good) == 8 na typowych platformach 32/64-bit).
 * 2) Funkcje little-endian (najmlodszy bajt pierwszy) - tylko przesuniecia i maski.
 * 3) Format ramki Telemetry na "przewodzie" (9 bajtow, bez paddingu):
 *      [0..1] id (LE)  [2..5] timestamp (LE)  [6..7] temp_centi (LE, U2)  [8] flags
 *    telemetry_pack:    0 gdy out_size < 9, inaczej 9
 *    telemetry_unpack:  false gdy len < 9
 *    telemetry_equals:  porownanie pole po polu (nie memcmp - padding!)
 */
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

struct Good {          /* TODO: przestaw pola */
    uint8_t a;
    uint32_t b;
    uint8_t c;
    uint16_t d;
};

struct Telemetry {
    uint16_t id;
    uint32_t timestamp;
    int16_t temp_centi;
    uint8_t flags;
};

#define TELEMETRY_WIRE_SIZE 9

void write_u16_le(uint8_t *p, uint16_t v) {
    /* TODO */
}

uint16_t read_u16_le(const uint8_t *p) {
    /* TODO */
    return 0;
}

void write_u32_le(uint8_t *p, uint32_t v) {
    /* TODO */
}

uint32_t read_u32_le(const uint8_t *p) {
    /* TODO */
    return 0;
}

size_t telemetry_pack(const struct Telemetry *t, uint8_t *out, size_t out_size) {
    /* TODO */
    return 0;
}

bool telemetry_unpack(const uint8_t *in, size_t len, struct Telemetry *out) {
    /* TODO */
    return false;
}

bool telemetry_equals(const struct Telemetry *a, const struct Telemetry *b) {
    /* TODO */
    return false;
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
