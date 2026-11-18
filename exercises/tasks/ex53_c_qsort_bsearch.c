/* Zadanie 53 (C, rozdzial 6): qsort i bsearch z wlasnymi komparatorami
 *
 *   cmp_int_asc / cmp_int_desc:  bez odejmowania (a - b przepelnia sie dla INT_MIN!)
 *   cmp_person_age_then_name:    rosnaco po wieku, przy remisie alfabetycznie po imieniu
 *   cmp_person_id:               rosnaco po id
 *   find_by_id:                  bsearch po tablicy POSORTOWANEJ po id; NULL gdy brak
 *   cmp_cstr / sort_strings:     sortuje tablice `const char *` alfabetycznie.
 *                                Uwaga: qsort przekazuje do komparatora WSKAZNIKI NA ELEMENTY,
 *                                czyli tu `const char *const *`, nie `const char *`!
 */
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

int cmp_int_asc(const void *a, const void *b) {
    /* TODO */
    return 0;
}

int cmp_int_desc(const void *a, const void *b) {
    /* TODO */
    return 0;
}

int cmp_person_age_then_name(const void *a, const void *b) {
    /* TODO */
    return 0;
}

int cmp_person_id(const void *a, const void *b) {
    /* TODO */
    return 0;
}

const Person *find_by_id(const Person *people, size_t n, int id) {
    /* TODO */
    return NULL;
}

int cmp_cstr(const void *a, const void *b) {
    /* TODO */
    return 0;
}

void sort_strings(const char **arr, size_t n) {
    /* TODO */
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
