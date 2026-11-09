/* Zadanie 21 (C): wskazniki i arytmetyka wskaznikow
 *
 *   swap_int:            zamienia wartosci wskazywane przez a i b
 *   reverse_in_place:    odwraca tablice w miejscu (uzyj dwoch wskaznikow: z lewej i z prawej)
 *   find_max:            wskaznik NA najwiekszy element (pierwszy przy remisie) albo NULL gdy n == 0
 *   count_less:          ile elementow z zakresu [begin, end) jest mniejszych niz limit
 *   rotate_left_by_one:  {1,2,3,4} -> {2,3,4,1}
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

void swap_int(int *a, int *b) {
    /* TODO */
}

void reverse_in_place(int *arr, size_t n) {
    /* TODO */
}

int *find_max(int *arr, size_t n) {
    /* TODO */
    return NULL;
}

size_t count_less(const int *begin, const int *end, int limit) {
    /* TODO */
    return 0;
}

void rotate_left_by_one(int *arr, size_t n) {
    /* TODO */
}

int main(void) {
    int a = 1, b = 2;
    swap_int(&a, &b);
    assert(a == 2 && b == 1);

    int r[] = {1, 2, 3, 4, 5};
    reverse_in_place(r, 5);
    assert(r[0] == 5 && r[1] == 4 && r[2] == 3 && r[3] == 2 && r[4] == 1);
    int r4[] = {1, 2, 3, 4};
    reverse_in_place(r4, 4);
    assert(r4[0] == 4 && r4[3] == 1);
    reverse_in_place(r4, 0);

    int m[] = {3, 9, 4, 9, 1};
    int *max = find_max(m, 5);
    assert(max == &m[1] && *max == 9);
    *max = 0;
    assert(m[1] == 0);
    assert(find_max(m, 0) == NULL);

    int c[] = {3, 1, 4, 1, 0};
    assert(count_less(c, c + 5, 2) == 3);
    assert(count_less(c + 2, c + 4, 2) == 1);
    assert(count_less(c, c, 100) == 0);

    int rot[] = {1, 2, 3, 4};
    rotate_left_by_one(rot, 4);
    assert(rot[0] == 2 && rot[1] == 3 && rot[2] == 4 && rot[3] == 1);
    int one[] = {7};
    rotate_left_by_one(one, 1);
    assert(one[0] == 7);
    rotate_left_by_one(one, 0);

    puts("OK");
    return 0;
}
