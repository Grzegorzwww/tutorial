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
    return p | flags;
}

unsigned perm_clear(unsigned p, unsigned flags) {
    return p & ~flags;
}

bool perm_has(unsigned p, unsigned flags) {
    return (p & flags) == flags;
}

int perm_count(unsigned p) {
    return !!(p & PERM_READ) + !!(p & PERM_WRITE) + !!(p & PERM_EXEC);
}

void perm_to_string(unsigned p, char out[4]) {
    out[0] = (p & PERM_READ) ? 'r' : '-';
    out[1] = (p & PERM_WRITE) ? 'w' : '-';
    out[2] = (p & PERM_EXEC) ? 'x' : '-';
    out[3] = '\0';
}

bool perm_from_string(const char *s, unsigned *out) {
    static const char letters[3] = {'r', 'w', 'x'};
    static const unsigned flags[3] = {PERM_READ, PERM_WRITE, PERM_EXEC};
    if (strlen(s) != 3) return false;

    unsigned p = 0;
    for (int i = 0; i < 3; ++i) {
        if (s[i] == letters[i]) p |= flags[i];
        else if (s[i] != '-') return false;
    }
    *out = p;
    return true;
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
