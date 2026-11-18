#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* ===================== INTERFEJS (stack.h) ===================== */

typedef struct Stack Stack;

Stack *stack_create(void);
void stack_destroy(Stack *s);
bool stack_push(Stack *s, int value);
bool stack_pop(Stack *s, int *out);
size_t stack_size(const Stack *s);

/* ================== IMPLEMENTACJA (stack.c) ==================== */

#define STACK_CAPACITY 3
#define MAX_STACKS 2

struct Stack {
    int items[STACK_CAPACITY];
    size_t size;
    bool in_use;
};

static struct Stack s_pool[MAX_STACKS];

Stack *stack_create(void) {
    for (size_t i = 0; i < MAX_STACKS; ++i) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof s_pool[i]);
            s_pool[i].in_use = true;
            return &s_pool[i];
        }
    }
    return NULL;
}

void stack_destroy(Stack *s) {
    if (s) s->in_use = false;
}

bool stack_push(Stack *s, int value) {
    if (s->size == STACK_CAPACITY) return false;
    s->items[s->size++] = value;
    return true;
}

bool stack_pop(Stack *s, int *out) {
    if (s->size == 0) return false;
    *out = s->items[--s->size];
    return true;
}

size_t stack_size(const Stack *s) {
    return s->size;
}

/* ======================= UZYTKOWNIK (main.c) ==================== */

int main(void) {
    Stack *a = stack_create();
    Stack *b = stack_create();
    assert(a != NULL && b != NULL && a != b);
    assert(stack_create() == NULL);           /* pula wyczerpana */

    assert(stack_size(a) == 0);
    assert(stack_push(a, 1) && stack_push(a, 2) && stack_push(a, 3));
    assert(!stack_push(a, 4));
    assert(stack_size(a) == 3);

    assert(stack_push(b, 100));
    assert(stack_size(b) == 1);               /* stosy sa niezalezne */

    int out = -1;
    assert(stack_pop(a, &out) && out == 3);
    assert(stack_pop(a, &out) && out == 2);
    assert(stack_pop(a, &out) && out == 1);
    out = -1;
    assert(!stack_pop(a, &out) && out == -1);

    assert(stack_push(a, 7));
    stack_destroy(a);
    stack_destroy(NULL);
    Stack *c = stack_create();
    assert(c != NULL);
    assert(stack_size(c) == 0);               /* ponownie uzyty obiekt jest pusty */

    stack_destroy(b);
    stack_destroy(c);
    assert(stack_create() != NULL && stack_create() != NULL);

    puts("OK");
    return 0;
}
