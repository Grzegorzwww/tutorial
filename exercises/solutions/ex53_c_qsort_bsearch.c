#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int age;
    char name[16];
} Person;

static int cmp3(int x, int y) {
    return (x > y) - (x < y);
}

int cmp_int_asc(const void *a, const void *b) {
    return cmp3(*(const int *)a, *(const int *)b);
}

int cmp_int_desc(const void *a, const void *b) {
    return cmp3(*(const int *)b, *(const int *)a);
}

int cmp_person_age_then_name(const void *a, const void *b) {
    const Person *x = a, *y = b;
    if (x->age != y->age) return cmp3(x->age, y->age);
    return strcmp(x->name, y->name);
}

int cmp_person_id(const void *a, const void *b) {
    const Person *x = a, *y = b;
    return cmp3(x->id, y->id);
}

const Person *find_by_id(const Person *people, size_t n, int id) {
    Person key = {.id = id};
    return bsearch(&key, people, n, sizeof *people, cmp_person_id);
}

int cmp_cstr(const void *a, const void *b) {
    const char *const *x = a;
    const char *const *y = b;
    return strcmp(*x, *y);
}

void sort_strings(const char **arr, size_t n) {
    qsort(arr, n, sizeof *arr, cmp_cstr);
}

int main(void) {
    int v[] = {3, INT_MIN, 7, INT_MAX, -1, 0};
    qsort(v, 6, sizeof v[0], cmp_int_asc);
    assert(v[0] == INT_MIN && v[1] == -1 && v[2] == 0 && v[3] == 3 && v[4] == 7 && v[5] == INT_MAX);
    qsort(v, 6, sizeof v[0], cmp_int_desc);
    assert(v[0] == INT_MAX && v[5] == INT_MIN);

    Person people[] = {
        {30, 25, "Zed"},
        {10, 30, "Bob"},
        {20, 25, "Amy"},
        {40, 30, "Ala"},
    };
    qsort(people, 4, sizeof people[0], cmp_person_age_then_name);
    assert(strcmp(people[0].name, "Amy") == 0);
    assert(strcmp(people[1].name, "Zed") == 0);
    assert(strcmp(people[2].name, "Ala") == 0);
    assert(strcmp(people[3].name, "Bob") == 0);

    qsort(people, 4, sizeof people[0], cmp_person_id);
    assert(people[0].id == 10 && people[3].id == 40);
    const Person *p = find_by_id(people, 4, 30);
    assert(p != NULL && strcmp(p->name, "Zed") == 0);
    assert(find_by_id(people, 4, 25) == NULL);
    assert(find_by_id(people, 0, 10) == NULL);

    const char *words[] = {"spi", "can", "uart", "i2c"};
    sort_strings(words, 4);
    assert(strcmp(words[0], "can") == 0 && strcmp(words[1], "i2c") == 0);
    assert(strcmp(words[2], "spi") == 0 && strcmp(words[3], "uart") == 0);

    puts("OK");
    return 0;
}
