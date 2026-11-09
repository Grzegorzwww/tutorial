/* Zadanie 22 (C): napisy w stylu C
 *
 * Bez <string.h>! Napisz wlasne wersje:
 *   my_strlen:        dlugosc napisu bez '\0'
 *   my_strcpy:        kopiuje wraz z '\0', zwraca dst
 *   my_strcmp:        <0, 0, >0 (porownuj jako unsigned char)
 *   reverse_string:   odwraca napis w miejscu
 *   count_char:       ile razy wystepuje znak
 *   copy_truncated:   kopiuje co najwyzej dst_size-1 znakow i ZAWSZE konczy '\0'
 *                     (gdy dst_size > 0); zwraca liczbe skopiowanych znakow;
 *                     dla dst_size == 0 niczego nie zapisuje i zwraca 0
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

size_t my_strlen(const char *s) {
    /* TODO */
    return 0;
}

char *my_strcpy(char *dst, const char *src) {
    /* TODO */
    return dst;
}

int my_strcmp(const char *a, const char *b) {
    /* TODO */
    return 0;
}

void reverse_string(char *s) {
    /* TODO */
}

size_t count_char(const char *s, char c) {
    /* TODO */
    return 0;
}

size_t copy_truncated(char *dst, size_t dst_size, const char *src) {
    /* TODO */
    return 0;
}

int main(void) {
    assert(my_strlen("") == 0);
    assert(my_strlen("hello") == 5);

    char buf[16];
    assert(my_strcpy(buf, "abc") == buf);
    assert(buf[0] == 'a' && buf[2] == 'c' && buf[3] == '\0');

    assert(my_strcmp("abc", "abc") == 0);
    assert(my_strcmp("abc", "abd") < 0);
    assert(my_strcmp("b", "abc") > 0);
    assert(my_strcmp("ab", "abc") < 0);
    assert(my_strcmp("abc", "ab") > 0);
    assert(my_strcmp("", "") == 0);

    char s1[] = "abcd";
    reverse_string(s1);
    assert(my_strcmp(s1, "dcba") == 0);
    char s2[] = "abc";
    reverse_string(s2);
    assert(my_strcmp(s2, "cba") == 0);
    char s3[] = "";
    reverse_string(s3);
    assert(s3[0] == '\0');

    assert(count_char("banana", 'a') == 3);
    assert(count_char("banana", 'z') == 0);

    char small[4];
    assert(copy_truncated(small, sizeof small, "abcdef") == 3);
    assert(my_strcmp(small, "abc") == 0);
    assert(copy_truncated(small, sizeof small, "xy") == 2);
    assert(my_strcmp(small, "xy") == 0);
    char guard[2] = {'x', 'x'};
    assert(copy_truncated(guard, 0, "abc") == 0);
    assert(guard[0] == 'x' && guard[1] == 'x');
    assert(copy_truncated(guard, 1, "abc") == 0);
    assert(guard[0] == '\0');

    puts("OK");
    return 0;
}
