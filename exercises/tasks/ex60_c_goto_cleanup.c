/* Zadanie 60 (C, rozdzial 10): goto cleanup i wstrzykiwanie bledow
 *
 * build_report("3,1,2", &out) tworzy napis "count=3 sum=6 max=3".
 * Wymagane alokacje (WYLACZNIE przez test_alloc / test_free):
 *   1) kopia wejscia (do podzielenia po przecinkach)
 *   2) tablica intow na sparsowane liczby
 *   3) bufor wyniku (64 bajty)
 * Kontrakt:
 *   - csv == NULL lub out == NULL      -> ST_INVALID_ARG
 *   - pusty napis, puste pole, smieci  -> ST_PARSE_ERROR  (pole parsuj przez strtol)
 *   - brak pamieci                     -> ST_NO_MEMORY
 *   - przy KAZDYM bledzie: *out bez zmian i wszystkie alokacje zwolnione
 *   - przy sukcesie: *out wskazuje bufor wyniku (zwalnia go wywolujacy), reszta zwolniona
 * Uzyj jednej etykiety `cleanup:` na koncu funkcji.
 */
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
    /* TODO: "ok", "invalid argument", "out of memory", "parse error" */
    return "";
}

Status build_report(const char *csv, char **out) {
    /* TODO */
    return ST_OK;
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
