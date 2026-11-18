/* Zadanie 51 (C, rozdzial 5): flagi bitowe w enum
 *
 *   perm_set / perm_clear:  dodaje / usuwa flagi (moze byc kilka naraz)
 *   perm_has:               true gdy p zawiera WSZYSTKIE flagi z `flags`
 *   perm_count:             liczba ustawionych flag (r/w/x)
 *   perm_to_string:         "rw-", "--x", "---" (out ma miejsce na 4 znaki z '\0')
 *   perm_from_string:       odwrotnie; dokladnie 3 znaki, na pozycji 0 'r' lub '-',
 *                           na 1 'w' lub '-', na 2 'x' lub '-'; inaczej false (*out bez zmian)
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    PERM_READ = 1u << 0,
    PERM_WRITE = 1u << 1,
    PERM_EXEC = 1u << 2,
} Perm;

#define PERM_ALL (PERM_READ | PERM_WRITE | PERM_EXEC)

unsigned perm_set(unsigned p, unsigned flags) {
    /* TODO */
    return p;
}

unsigned perm_clear(unsigned p, unsigned flags) {
    /* TODO */
    return p;
}

bool perm_has(unsigned p, unsigned flags) {
    /* TODO */
    return false;
}

int perm_count(unsigned p) {
    /* TODO */
    return -1;
}

void perm_to_string(unsigned p, char out[4]) {
    /* TODO */
    out[0] = '\0';
}

bool perm_from_string(const char *s, unsigned *out) {
    /* TODO */
    return false;
}

int main(void) {
    unsigned p = 0;
    p = perm_set(p, PERM_READ);
    p = perm_set(p, PERM_WRITE | PERM_EXEC);
    assert(p == PERM_ALL);
    p = perm_clear(p, PERM_WRITE);
    assert(p == (PERM_READ | PERM_EXEC));
    assert(perm_clear(PERM_READ, PERM_WRITE) == PERM_READ);

    assert(perm_has(PERM_ALL, PERM_READ | PERM_WRITE));
    assert(!perm_has(PERM_READ, PERM_READ | PERM_WRITE));
    assert(perm_has(PERM_READ, 0));

    assert(perm_count(0) == 0 && perm_count(PERM_ALL) == 3 && perm_count(PERM_EXEC) == 1);

    char s[4];
    perm_to_string(PERM_READ | PERM_WRITE, s);
    assert(strcmp(s, "rw-") == 0);
    perm_to_string(PERM_EXEC, s);
    assert(strcmp(s, "--x") == 0);
    perm_to_string(0, s);
    assert(strcmp(s, "---") == 0);

    unsigned q = 99;
    assert(perm_from_string("r-x", &q) && q == (PERM_READ | PERM_EXEC));
    assert(perm_from_string("---", &q) && q == 0);
    q = 99;
    assert(!perm_from_string("rw", &q) && q == 99);
    assert(!perm_from_string("rwxa", &q) && q == 99);
    assert(!perm_from_string("wrx", &q) && q == 99);

    puts("OK");
    return 0;
}
