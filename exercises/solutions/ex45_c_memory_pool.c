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
    for (size_t i = 0; i + 1 < BLOCK_COUNT; ++i) p->blocks[i].next = &p->blocks[i + 1];
    p->blocks[BLOCK_COUNT - 1].next = NULL;
    p->free_list = &p->blocks[0];
    p->available = BLOCK_COUNT;
}

void *pool_alloc(Pool *p) {
    Block *b = p->free_list;
    if (b == NULL) return NULL;
    p->free_list = b->next;
    --p->available;
    return b;
}

bool pool_free(Pool *p, void *ptr) {
    uintptr_t start = (uintptr_t)&p->blocks[0];
    uintptr_t end = (uintptr_t)&p->blocks[BLOCK_COUNT];
    uintptr_t addr = (uintptr_t)ptr;
    if (addr < start || addr >= end || (addr - start) % sizeof(Block) != 0) return false;

    Block *b = ptr;
    b->next = p->free_list;
    p->free_list = b;
    ++p->available;
    return true;
}

size_t pool_available(const Pool *p) {
    return p->available;
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
