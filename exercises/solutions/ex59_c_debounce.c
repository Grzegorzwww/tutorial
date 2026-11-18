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
    d->stable = initial;
    d->counter = 0;
    d->threshold = threshold;
}

ButtonEvent debounce_update(Debounce *d, bool sample) {
    if (sample == d->stable) {
        d->counter = 0;
        return BTN_NONE;
    }
    if (++d->counter < d->threshold) return BTN_NONE;

    d->stable = sample;
    d->counter = 0;
    return sample ? BTN_PRESSED : BTN_RELEASED;
}

size_t count_presses(const bool *samples, size_t n, uint8_t threshold) {
    Debounce d;
    debounce_init(&d, false, threshold);
    size_t presses = 0;
    for (size_t i = 0; i < n; ++i) {
        if (debounce_update(&d, samples[i]) == BTN_PRESSED) ++presses;
    }
    return presses;
}

size_t longest_press(const bool *samples, size_t n, uint8_t threshold) {
    Debounce d;
    debounce_init(&d, false, threshold);
    size_t longest = 0, pressed_at = 0;
    for (size_t i = 0; i < n; ++i) {
        ButtonEvent e = debounce_update(&d, samples[i]);
        if (e == BTN_PRESSED) {
            pressed_at = i;
        } else if (e == BTN_RELEASED && i - pressed_at > longest) {
            longest = i - pressed_at;
        }
    }
    if (d.stable && n - pressed_at > longest) longest = n - pressed_at;
    return longest;
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
