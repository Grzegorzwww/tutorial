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
#define X(code, text) [code] = text,
    ERROR_LIST(X)
#undef X
};

static const char *const sensor_names[] = {
#define X(id, name, scale) [SENSOR_##id] = name,
    SENSOR_LIST(X)
#undef X
};

static const float sensor_scales[] = {
#define X(id, name, scale) [SENSOR_##id] = scale,
    SENSOR_LIST(X)
#undef X
};

_Static_assert(ARRAY_SIZE(error_names) == ERR_COUNT, "error_names out of sync");

const char *error_to_string(int code) {
    if (code < 0 || code >= ERR_COUNT) return "unknown";
    return error_names[code];
}

bool error_from_string(const char *s, ErrorCode *out) {
    for (int i = 0; i < ERR_COUNT; ++i) {
        if (strcmp(error_names[i], s) == 0) {
            *out = (ErrorCode)i;
            return true;
        }
    }
    return false;
}

float sensor_scaled(SensorId id, int raw) {
    return (float)raw * sensor_scales[id];
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
