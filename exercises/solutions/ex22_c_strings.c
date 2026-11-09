#include <assert.h>
#include <stddef.h>
#include <stdio.h>

size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) ++p;
    return (size_t)(p - s);
}

char *my_strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++) != '\0') {
    }
    return dst;
}

int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

void reverse_string(char *s) {
    size_t n = my_strlen(s);
    if (n < 2) return;
    char *left = s;
    char *right = s + n - 1;
    while (left < right) {
        char tmp = *left;
        *left++ = *right;
        *right-- = tmp;
    }
}

size_t count_char(const char *s, char c) {
    size_t count = 0;
    for (; *s; ++s) {
        if (*s == c) ++count;
    }
    return count;
}

size_t copy_truncated(char *dst, size_t dst_size, const char *src) {
    if (dst_size == 0) return 0;
    size_t i = 0;
    while (i + 1 < dst_size && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
    return i;
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
