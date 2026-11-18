#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int s_last_id = 0;

int next_id(void) {
    return ++s_last_id;
}

void reset_ids(void) {
    s_last_id = 0;
}

bool init_once(void) {
    static bool done = false;
    if (done) return false;
    done = true;
    return true;
}

int running_sum(int x) {
    static int sum = 0;
    sum += x;
    return sum;
}

const char *format_temp(int deci_celsius) {
    static char buf[16];
    int magnitude = deci_celsius < 0 ? -deci_celsius : deci_celsius;
    snprintf(buf, sizeof buf, "%s%d.%dC", deci_celsius < 0 ? "-" : "", magnitude / 10, magnitude % 10);
    return buf;
}

int main(void) {
    assert(next_id() == 1);
    assert(next_id() == 2);
    assert(next_id() == 3);
    reset_ids();
    assert(next_id() == 1);

    assert(init_once());
    assert(!init_once());
    assert(!init_once());

    assert(running_sum(5) == 5);
    assert(running_sum(10) == 15);
    assert(running_sum(-3) == 12);

    assert(strcmp(format_temp(235), "23.5C") == 0);
    assert(strcmp(format_temp(0), "0.0C") == 0);
    assert(strcmp(format_temp(-5), "-0.5C") == 0);
    assert(strcmp(format_temp(-123), "-12.3C") == 0);

    const char *a = format_temp(10);
    const char *b = format_temp(20);
    assert(a == b);                      /* ten sam statyczny bufor */
    assert(strcmp(a, "2.0C") == 0);      /* pierwszy wynik zostal nadpisany */

    puts("OK");
    return 0;
}
