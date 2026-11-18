#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

size_t str_append(char *dst, size_t size, const char *src) {
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    if (dst_len + 1 < size) {
        size_t room = size - dst_len - 1;
        size_t n = src_len < room ? src_len : room;
        memcpy(dst + dst_len, src, n);
        dst[dst_len + n] = '\0';
    }
    return dst_len + src_len;
}

bool to_hex(const uint8_t *data, size_t n, char *out, size_t out_size) {
    if (out_size == 0) return false;
    out[0] = '\0';
    size_t used = 0;
    for (size_t i = 0; i < n; ++i) {
        int w = snprintf(out + used, out_size - used, i ? " %02X" : "%02X", data[i]);
        if (w < 0 || (size_t)w >= out_size - used) {
            out[0] = '\0';
            return false;
        }
        used += (size_t)w;
    }
    return true;
}

char *trim(char *s) {
    while (isspace((unsigned char)*s)) ++s;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) --len;
    s[len] = '\0';
    return s;
}

size_t split_in_place(char *s, char sep, char **fields, size_t max) {
    if (max == 0) return 0;
    size_t n = 0;
    fields[n++] = s;
    for (; *s && n < max; ++s) {
        if (*s == sep) {
            *s = '\0';
            fields[n++] = s + 1;
        }
    }
    return n;
}

bool parse_kv(char *line, char **key, char **value) {
    char *eq = strchr(line, '=');
    if (!eq) return false;
    *eq = '\0';
    char *k = trim(line);
    if (*k == '\0') return false;
    *key = k;
    *value = trim(eq + 1);
    return true;
}

int main(void) {
    char buf[8] = "ab";
    assert(str_append(buf, sizeof buf, "cd") == 4 && strcmp(buf, "abcd") == 0);
    assert(str_append(buf, sizeof buf, "efghij") == 10);   /* obciete */
    assert(strcmp(buf, "abcdefg") == 0);
    assert(str_append(buf, sizeof buf, "") == 7);

    const uint8_t bytes[] = {0x0A, 0xFF, 0x00};
    char hex[16];
    assert(to_hex(bytes, 3, hex, sizeof hex) && strcmp(hex, "0A FF 00") == 0);
    assert(to_hex(bytes, 0, hex, sizeof hex) && strcmp(hex, "") == 0);
    char small[8];
    assert(!to_hex(bytes, 3, small, sizeof small) && strcmp(small, "") == 0);
    char exact[9];
    assert(to_hex(bytes, 3, exact, sizeof exact));

    char t1[] = "  \thello world \n";
    assert(strcmp(trim(t1), "hello world") == 0);
    char t2[] = "   ";
    assert(strcmp(trim(t2), "") == 0);
    char t3[] = "x";
    assert(trim(t3) == t3);

    char csv[] = "GPS,52.1,,21.0";
    char *f[8];
    assert(split_in_place(csv, ',', f, 8) == 4);
    assert(strcmp(f[0], "GPS") == 0 && strcmp(f[1], "52.1") == 0);
    assert(strcmp(f[2], "") == 0 && strcmp(f[3], "21.0") == 0);
    assert(f[1] == csv + 4);                                 /* bez kopiowania */
    char csv2[] = "a,b,c";
    assert(split_in_place(csv2, ',', f, 2) == 2);
    assert(strcmp(f[0], "a") == 0 && strcmp(f[1], "b,c") == 0);
    char empty[] = "";
    assert(split_in_place(empty, ',', f, 8) == 1 && strcmp(f[0], "") == 0);

    char kv[] = "  baud = 115200 ";
    char *k, *v;
    assert(parse_kv(kv, &k, &v));
    assert(strcmp(k, "baud") == 0 && strcmp(v, "115200") == 0);
    char kv2[] = "novalue";
    assert(!parse_kv(kv2, &k, &v));
    char kv3[] = " = x";
    assert(!parse_kv(kv3, &k, &v));
    char kv4[] = "mode=";
    assert(parse_kv(kv4, &k, &v) && strcmp(k, "mode") == 0 && strcmp(v, "") == 0);

    puts("OK");
    return 0;
}
