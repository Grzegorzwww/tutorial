/* Zadanie 24 (C): wskazniki na funkcje
 *
 *   add / sub / mul:   proste dzialania (typ binary_op)
 *   apply:             wywoluje op(a, b)
 *   map_in_place:      zastepuje kazdy element arr[i] wartoscia f(arr[i])
 *   reduce:            op(op(op(init, arr[0]), arr[1]), ...)
 *   calc:              dispatch przez tablice struktur {symbol, funkcja}, bez if/switch.
 *                      Zapisuje wynik do *result i zwraca 0; dla nieznanego symbolu
 *                      zwraca -1 i NIE zmienia *result.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef int (*binary_op)(int, int);
typedef int (*unary_op)(int);

int add(int a, int b) {
    /* TODO */
    return 0;
}

int sub(int a, int b) {
    /* TODO */
    return 0;
}

int mul(int a, int b) {
    /* TODO */
    return 0;
}

int apply(binary_op op, int a, int b) {
    /* TODO */
    return 0;
}

void map_in_place(int *arr, size_t n, unary_op f) {
    /* TODO */
}

int reduce(const int *arr, size_t n, int init, binary_op op) {
    /* TODO */
    return 0;
}

int calc(char symbol, int a, int b, int *result) {
    /* TODO: static const struct { char symbol; binary_op op; } table[] = { ... }; */
    return -1;
}

static int square(int x) { return x * x; }
static int negate(int x) { return -x; }

int main(void) {
    assert(add(2, 3) == 5 && sub(2, 3) == -1 && mul(2, 3) == 6);
    assert(apply(add, 4, 5) == 9);
    assert(apply(mul, 4, 5) == 20);

    int v[] = {1, 2, 3, 4};
    map_in_place(v, 4, square);
    assert(v[0] == 1 && v[1] == 4 && v[2] == 9 && v[3] == 16);
    map_in_place(v, 2, negate);
    assert(v[0] == -1 && v[1] == -4 && v[2] == 9);

    int w[] = {1, 2, 3, 4};
    assert(reduce(w, 4, 0, add) == 10);
    assert(reduce(w, 4, 1, mul) == 24);
    assert(reduce(w, 3, 100, sub) == 94);
    assert(reduce(w, 0, 42, add) == 42);

    int r = 123;
    assert(calc('+', 2, 3, &r) == 0 && r == 5);
    assert(calc('-', 2, 3, &r) == 0 && r == -1);
    assert(calc('*', 4, 3, &r) == 0 && r == 12);
    r = 777;
    assert(calc('/', 4, 2, &r) == -1 && r == 777);

    puts("OK");
    return 0;
}
