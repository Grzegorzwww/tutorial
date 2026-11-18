/* Zadanie 59 (C, rozdzial 9): debouncing przycisku jako automat stanow
 *
 * Przycisk probkujemy co 1 ms. Stan "stabilny" zmienia sie dopiero po `threshold`
 * KOLEJNYCH probkach rozniacych sie od niego; kazda probka rowna stanowi stabilnemu
 * zeruje licznik.
 *   debounce_init:    stan poczatkowy (stable = initial, licznik 0)
 *   debounce_update:  przetwarza jedna probke; zwraca BTN_PRESSED (stable: 0 -> 1),
 *                     BTN_RELEASED (1 -> 0) albo BTN_NONE
 *   count_presses:    liczba zdarzen BTN_PRESSED w ciagu probek (start z puszczonym)
 *   longest_press:    najdluzszy czas (w probkach) miedzy PRESSED a RELEASED;
 *                     niezakonczone nacisniecie na koncu ciagu liczy sie do konca ciagu
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum { BTN_NONE, BTN_PRESSED, BTN_RELEASED } ButtonEvent;

typedef struct {
    bool stable;
    uint8_t counter;
    uint8_t threshold;
} Debounce;

void debounce_init(Debounce *d, bool initial, uint8_t threshold) {
    /* TODO */
}

ButtonEvent debounce_update(Debounce *d, bool sample) {
    /* TODO */
    return BTN_NONE;
}

size_t count_presses(const bool *samples, size_t n, uint8_t threshold) {
    /* TODO */
    return 99;
}

size_t longest_press(const bool *samples, size_t n, uint8_t threshold) {
    /* TODO */
    return 99;
}

int main(void) {
    Debounce d;
    debounce_init(&d, false, 3);
    assert(!d.stable);

    const bool seq[] = {1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0};
    const ButtonEvent expected[] = {
        BTN_NONE, BTN_NONE, BTN_NONE,              /* drganie: licznik zerowany */
        BTN_NONE, BTN_NONE, BTN_PRESSED,           /* 3 kolejne jedynki */
        BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE,    /* 0 0 1 - za krotko */
        BTN_NONE, BTN_NONE, BTN_RELEASED,          /* 3 kolejne zera */
    };
    for (size_t i = 0; i < sizeof seq / sizeof seq[0]; ++i) {
        assert(debounce_update(&d, seq[i]) == expected[i]);
    }
    assert(!d.stable);

    Debounce one;
    debounce_init(&one, true, 1);
    assert(debounce_update(&one, false) == BTN_RELEASED);
    assert(debounce_update(&one, true) == BTN_PRESSED);

    const bool presses[] = {0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1};
    assert(count_presses(presses, 13, 2) == 3);
    assert(count_presses(presses, 13, 3) == 1);
    assert(count_presses(presses, 0, 2) == 0);

    /* threshold 2: PRESSED przy i=2, RELEASED przy i=4 (2), PRESSED i=6, RELEASED i=10 (4),
       PRESSED i=12 do konca ciagu (1) */
    assert(longest_press(presses, 13, 2) == 4);
    const bool held[] = {1, 1, 1, 1, 1};
    assert(longest_press(held, 5, 2) == 4);

    puts("OK");
    return 0;
}
