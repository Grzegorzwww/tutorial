/* Zadanie 41 (C, rozdzial 0): static i czas zycia zmiennych
 *
 *   next_id / reset_ids:  kolejne identyfikatory 1, 2, 3...; reset_ids zaczyna od nowa.
 *                         Uzyj zmiennej `static` na poziomie pliku (s_last_id),
 *                         bo dwie funkcje musza widziec ten sam stan.
 *   init_once:            true tylko przy PIERWSZYM wywolaniu (static lokalna flaga)
 *   running_sum:          dodaje x do sumy pamietanej miedzy wywolaniami i ja zwraca
 *   format_temp:          temperatura w dziesiatych stopnia -> "23.5C", "-0.5C", "0.0C".
 *                         Zwraca wskaznik na STATYCZNY bufor (jak np. strerror) -
 *                         kolejne wywolanie nadpisuje poprzedni wynik.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int s_last_id = 0;

int next_id(void) {
    /* TODO */
    
    return 0;
}

void reset_ids(void) {
    /* TODO */
}

bool init_once(void) {
    /* TODO */
    return false;
}

int running_sum(int x) {
    /* TODO */
    return 0;
}

const char *format_temp(int deci_celsius) {
    /* TODO: static char buf[16]; snprintf(...) */
    return "";
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
