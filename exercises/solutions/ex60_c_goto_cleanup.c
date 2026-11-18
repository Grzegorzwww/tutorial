#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { ST_OK, ST_INVALID_ARG, ST_NO_MEMORY, ST_PARSE_ERROR } Status;

/* ---- podmienialny alokator testowy (gotowy) ---- */

static int s_fail_after = -1;   /* -1: nigdy; 0: nastepna alokacja zawiedzie; n: po n udanych */
static int s_live = 0;

static void *test_alloc(size_t n) {
    if (s_fail_after == 0) return NULL;
    if (s_fail_after > 0) --s_fail_after;
    void *p = malloc(n);
    if (p) ++s_live;
    return p;
}

static void test_free(void *p) {
    if (p) --s_live;
    free(p);
}

/* ---- zadanie ---- */

const char *status_to_string(Status s) {
    switch (s) {
        case ST_OK: return "ok";
        case ST_INVALID_ARG: return "invalid argument";
        case ST_NO_MEMORY: return "out of memory";
        case ST_PARSE_ERROR: return "parse error";
    }
    return "unknown";
}

#define REPORT_SIZE 64

Status build_report(const char *csv, char **out) {
    if (csv == NULL || out == NULL) return ST_INVALID_ARG;

    Status st = ST_NO_MEMORY;
    char *copy = NULL;
    long *nums = NULL;
    char *result = NULL;

    size_t len = strlen(csv);
    size_t count = 1;
    for (const char *p = csv; *p; ++p) {
        if (*p == ',') ++count;
    }

    copy = test_alloc(len + 1);
    if (!copy) goto cleanup;
    memcpy(copy, csv, len + 1);

    nums = test_alloc(count * sizeof *nums);
    if (!nums) goto cleanup;

    st = ST_PARSE_ERROR;
    char *field = copy;
    for (size_t i = 0; i < count; ++i) {
        char *comma = strchr(field, ',');
        if (comma) *comma = '\0';
        if (*field == '\0') goto cleanup;

        char *end;
        errno = 0;
        nums[i] = strtol(field, &end, 10);
        if (*end != '\0' || errno == ERANGE) goto cleanup;

        if (comma) field = comma + 1;
    }

    st = ST_NO_MEMORY;
    result = test_alloc(REPORT_SIZE);
    if (!result) goto cleanup;

    long sum = 0;
    long max = nums[0];
    for (size_t i = 0; i < count; ++i) {
        sum += nums[i];
        if (nums[i] > max) max = nums[i];
    }
    snprintf(result, REPORT_SIZE, "count=%lu sum=%ld max=%ld", (unsigned long)count, sum, max);

    *out = result;
    result = NULL;          /* wlasnosc przeszla na wywolujacego */
    st = ST_OK;

cleanup:
    test_free(result);
    test_free(nums);
    test_free(copy);
    return st;
}

int main(void) {
    assert(strcmp(status_to_string(ST_NO_MEMORY), "out of memory") == 0);
    assert(strcmp(status_to_string(ST_PARSE_ERROR), "parse error") == 0);

    char *report = NULL;
    assert(build_report("3,1,2", &report) == ST_OK);
    assert(report && strcmp(report, "count=3 sum=6 max=3") == 0);
    assert(s_live == 1);                         /* zyje tylko wynik */
    test_free(report);
    assert(s_live == 0);

    report = NULL;
    assert(build_report("-5", &report) == ST_OK && strcmp(report, "count=1 sum=-5 max=-5") == 0);
    test_free(report);

    char *untouched = (char *)"sentinel";
    report = untouched;
    assert(build_report(NULL, &report) == ST_INVALID_ARG && report == untouched);
    assert(build_report("1", NULL) == ST_INVALID_ARG);
    assert(build_report("", &report) == ST_PARSE_ERROR && report == untouched);
    assert(build_report("1,,2", &report) == ST_PARSE_ERROR && report == untouched);
    assert(build_report("1,x", &report) == ST_PARSE_ERROR && report == untouched);
    assert(s_live == 0);

    for (int fail = 0; fail < 3; ++fail) {      /* kazda z trzech alokacji po kolei zawodzi */
        s_fail_after = fail;
        report = untouched;
        assert(build_report("4,5,6", &report) == ST_NO_MEMORY);
        assert(report == untouched);
        assert(s_live == 0);                     /* nic nie wycieklo */
    }
    s_fail_after = -1;

    puts("OK");
    return 0;
}
