#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int matrix_sum(const int (*m)[3], size_t rows) {
    int sum = 0;
    for (size_t r = 0; r < rows; ++r)
        for (size_t c = 0; c < 3; ++c) sum += m[r][c];
    return sum;
}

void transpose_2x3(const int (*src)[3], int (*dst)[2]) {
    for (size_t r = 0; r < 2; ++r)
        for (size_t c = 0; c < 3; ++c) dst[c][r] = src[r][c];
}

bool min_max(const int *data, size_t n, int *out_min, int *out_max) {
    if (n == 0) return false;
    int lo = data[0], hi = data[0];
    for (size_t i = 1; i < n; ++i) {
        if (data[i] < lo) lo = data[i];
        if (data[i] > hi) hi = data[i];
    }
    *out_min = lo;
    *out_max = hi;
    return true;
}

long find_str(const char *const *list, size_t n, const char *s) {
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(list[i], s) == 0) return (long)i;
    }
    return -1;
}

void reverse_ptrs(const char **list, size_t n) {
    if (n < 2) return;
    for (size_t i = 0, j = n - 1; i < j; ++i, --j) {
        const char *tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
    }
}

int main(void) {
    const int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    assert(matrix_sum(m, 2) == 21);
    assert(matrix_sum(m, 1) == 6);
    assert(matrix_sum(m, 0) == 0);

    int t[3][2] = {{0}};
    transpose_2x3(m, t);
    assert(t[0][0] == 1 && t[0][1] == 4);
    assert(t[1][0] == 2 && t[1][1] == 5);
    assert(t[2][0] == 3 && t[2][1] == 6);

    const int data[] = {4, -2, 9, 0};
    int lo = 100, hi = 100;
    assert(min_max(data, 4, &lo, &hi) && lo == -2 && hi == 9);
    lo = hi = 100;
    assert(!min_max(data, 0, &lo, &hi) && lo == 100 && hi == 100);

    const char *names[] = {"uart", "spi", "i2c"};
    char key[] = "spi";                 /* inny adres niz literal w tablicy */
    assert(find_str(names, 3, key) == 1);
    assert(find_str(names, 3, "can") == -1);
    assert(find_str(names, 0, "uart") == -1);

    const char *first = names[0];
    reverse_ptrs(names, 3);
    assert(strcmp(names[0], "i2c") == 0 && strcmp(names[2], "uart") == 0);
    assert(names[2] == first);          /* przesuniety wskaznik, nie skopiowany napis */
    reverse_ptrs(names, 0);

    puts("OK");
    return 0;
}
