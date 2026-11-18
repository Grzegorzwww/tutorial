#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

static int s_live_nodes = 0;

static Node *node_new(int value) {
    Node *n = malloc(sizeof *n);
    if (n) {
        n->value = value;
        n->next = NULL;
        ++s_live_nodes;
    }
    return n;
}

static void node_delete(Node *n) {
    if (n) --s_live_nodes;
    free(n);
}

bool list_push_front(Node **head, int value) {
    Node *n = node_new(value);
    if (!n) return false;
    n->next = *head;
    *head = n;
    return true;
}

bool list_push_back(Node **head, int value) {
    Node *n = node_new(value);
    if (!n) return false;
    Node **pp = head;
    while (*pp) pp = &(*pp)->next;
    *pp = n;
    return true;
}

size_t list_length(const Node *head) {
    size_t n = 0;
    for (; head; head = head->next) ++n;
    return n;
}

bool list_remove(Node **head, int value) {
    for (Node **pp = head; *pp; pp = &(*pp)->next) {
        if ((*pp)->value == value) {
            Node *dead = *pp;
            *pp = dead->next;
            node_delete(dead);
            return true;
        }
    }
    return false;
}

void list_reverse(Node **head) {
    Node *prev = NULL;
    Node *cur = *head;
    while (cur) {
        Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    *head = prev;
}

void list_free(Node **head) {
    Node *cur = *head;
    while (cur) {
        Node *next = cur->next;
        node_delete(cur);
        cur = next;
    }
    *head = NULL;
}

static bool list_equals(const Node *head, const int *expected, size_t n) {
    for (size_t i = 0; i < n; ++i, head = head->next) {
        if (!head || head->value != expected[i]) return false;
    }
    return head == NULL;
}

int main(void) {
    Node *list = NULL;
    assert(list_length(list) == 0);

    assert(list_push_back(&list, 2));
    assert(list_push_back(&list, 3));
    assert(list_push_front(&list, 1));
    assert(list_push_back(&list, 4));
    assert(list_equals(list, (int[]){1, 2, 3, 4}, 4));
    assert(list_length(list) == 4 && s_live_nodes == 4);

    assert(list_remove(&list, 1));                      /* glowa */
    assert(list_remove(&list, 3));                      /* srodek */
    assert(list_remove(&list, 4));                      /* ogon */
    assert(!list_remove(&list, 42));
    assert(list_equals(list, (int[]){2}, 1));
    assert(s_live_nodes == 1);

    for (int i = 3; i <= 5; ++i) assert(list_push_back(&list, i));
    list_reverse(&list);
    assert(list_equals(list, (int[]){5, 4, 3, 2}, 4));

    list_free(&list);
    assert(list == NULL && s_live_nodes == 0);
    list_reverse(&list);
    list_free(&list);

    puts("OK");
    return 0;
}
