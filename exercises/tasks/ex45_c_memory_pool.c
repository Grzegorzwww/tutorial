/* Zadanie 45 (C, rozdzial 3): pula blokow o stalym rozmiarze (bez malloc)
 *
 * Lista wolnych blokow jest przechowywana W SAMYCH wolnych blokach (union Block).
 *   pool_init:       wszystkie bloki wolne, polaczone w liste
 *   pool_alloc:      zwraca wolny blok albo NULL; O(1)
 *   pool_free:       oddaje blok; zwalniany jest jako pierwszy do ponownego przydzialu (LIFO).
 *                    false dla wskaznika spoza puli lub nie na poczatku bloku (wtedy nic nie zmienia)
 *   pool_available:  liczba wolnych blokow
 *
 * Uwaga: tego zadania nie da sie w 100% zabezpieczyc przed podwojnym free - to swiadome
 * uproszczenie (w prawdziwych pulach dodaje sie np. bitmape zajetosci).
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 16
#define BLOCK_COUNT 4

typedef union Block {
    union Block *next;
    unsigned char data[BLOCK_SIZE];
} Block;

typedef struct {
    Block blocks[BLOCK_COUNT];
    Block *free_list;
    size_t available;
} Pool;

void pool_init(Pool *p) {
    /* TODO */
}

void *pool_alloc(Pool *p) {
    /* TODO */
    return NULL;
}

bool pool_free(Pool *p, void *ptr) {
    /* TODO: sprawdz zakres adresow i wyrownanie do poczatku bloku (uintptr_t / roznica wskaznikow) */
    return false;
}

size_t pool_available(const Pool *p) {
    /* TODO */
    return 99;
}

int main(void) {
    Pool pool;
    pool_init(&pool);
    assert(pool_available(&pool) == BLOCK_COUNT);

    void *b[BLOCK_COUNT];
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        b[i] = pool_alloc(&pool);
        assert(b[i] != NULL);
        memset(b[i], i + 1, BLOCK_SIZE);   /* blok mozna w calosci zapisac */
    }
    for (int i = 0; i < BLOCK_COUNT; ++i)
        for (int j = i + 1; j < BLOCK_COUNT; ++j) assert(b[i] != b[j]);
    assert(pool_alloc(&pool) == NULL);
    assert(pool_available(&pool) == 0);
    assert(((unsigned char *)b[2])[BLOCK_SIZE - 1] == 3);

    assert(pool_free(&pool, b[1]));
    assert(pool_available(&pool) == 1);
    assert(pool_alloc(&pool) == b[1]);      /* LIFO */

    assert(pool_free(&pool, b[0]) && pool_free(&pool, b[3]));
    assert(pool_alloc(&pool) == b[3]);
    assert(pool_alloc(&pool) == b[0]);

    int outside = 0;
    assert(!pool_free(&pool, &outside));
    assert(!pool_free(&pool, (unsigned char *)b[2] + 1));
    assert(pool_available(&pool) == 0);

    puts("OK");
    return 0;
}
