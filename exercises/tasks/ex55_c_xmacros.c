/* Zadanie 55 (C, rozdzial 7): X-macros
 *
 * Enumy ErrorCode i SensorId sa juz wygenerowane z list ponizej (wzor do nasladowania).
 * Twoje zadanie - wygeneruj z TYCH SAMYCH list (bez przepisywania wartosci recznie):
 *   error_names[]     tablica napisow indeksowana kodem bledu
 *   error_to_string   napis dla kodu; dla kodu spoza zakresu "unknown"
 *   error_from_string kod dla napisu; false gdy nie znaleziono (*out bez zmian)
 *   sensor_names[] i sensor_scales[]   z SENSOR_LIST
 *   sensor_scaled     raw * skala czujnika
 *
 * Sprawdz: dodanie nowej linii do ERROR_LIST nie powinno wymagac zadnej innej zmiany.
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define ERROR_LIST(X)                  \
    X(ERR_OK, "ok")                    \
    X(ERR_TIMEOUT, "timeout")          \
    X(ERR_CRC, "crc mismatch")         \
    X(ERR_NOMEM, "out of memory")      \
    X(ERR_BUSY, "busy")

typedef enum {
#define X(code, text) code,
    ERROR_LIST(X)
#undef X
    ERR_COUNT
} ErrorCode;

#define SENSOR_LIST(X)                 \
    X(TEMP, "temp", 0.1f)              \
    X(HUMIDITY, "humidity", 0.5f)      \
    X(PRESSURE, "pressure", 2.0f)

typedef enum {
#define X(id, name, scale) SENSOR_##id,
    SENSOR_LIST(X)
#undef X
    SENSOR_COUNT
} SensorId;

static const char *const error_names[] = {
    "TODO"   /* TODO: wygeneruj przez ERROR_LIST */
};

static const char *const sensor_names[] = {
    "TODO"   /* TODO */
};

static const float sensor_scales[] = {
    0.0f     /* TODO */
};

const char *error_to_string(int code) {
    /* TODO */
    return "";
}

bool error_from_string(const char *s, ErrorCode *out) {
    /* TODO */
    return false;
}

float sensor_scaled(SensorId id, int raw) {
    /* TODO */
    return 0.0f;
}

int main(void) {
    assert(ERR_COUNT == 5);
    assert(ARRAY_SIZE(error_names) == ERR_COUNT);
    assert(strcmp(error_to_string(ERR_OK), "ok") == 0);
    assert(strcmp(error_to_string(ERR_CRC), "crc mismatch") == 0);
    assert(strcmp(error_to_string(ERR_BUSY), "busy") == 0);
    assert(strcmp(error_to_string(-1), "unknown") == 0);
    assert(strcmp(error_to_string(ERR_COUNT), "unknown") == 0);

    ErrorCode e = ERR_OK;
    assert(error_from_string("out of memory", &e) && e == ERR_NOMEM);
    e = ERR_OK;
    assert(!error_from_string("nope", &e) && e == ERR_OK);

    assert(ARRAY_SIZE(sensor_names) == SENSOR_COUNT);
    assert(ARRAY_SIZE(sensor_scales) == SENSOR_COUNT);
    assert(strcmp(sensor_names[SENSOR_HUMIDITY], "humidity") == 0);
    assert(sensor_scaled(SENSOR_PRESSURE, 500) == 1000.0f);
    assert(sensor_scaled(SENSOR_HUMIDITY, 90) == 45.0f);

    puts("OK");
    return 0;
}
