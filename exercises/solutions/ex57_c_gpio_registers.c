#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
} GPIO_TypeDef;

_Static_assert(offsetof(GPIO_TypeDef, IDR) == 0x10, "register map");
_Static_assert(offsetof(GPIO_TypeDef, BSRR) == 0x18, "register map");

enum { GPIO_MODE_INPUT = 0, GPIO_MODE_OUTPUT = 1, GPIO_MODE_ALT = 2, GPIO_MODE_ANALOG = 3 };
enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 };

static uint32_t write_field2(uint32_t reg, unsigned pin, uint32_t value) {
    unsigned shift = pin * 2u;
    reg &= ~(3u << shift);
    reg |= (value & 3u) << shift;
    return reg;
}

void gpio_set_mode(GPIO_TypeDef *port, unsigned pin, uint32_t mode) {
    port->MODER = write_field2(port->MODER, pin, mode);
}

uint32_t gpio_get_mode(const GPIO_TypeDef *port, unsigned pin) {
    return (port->MODER >> (pin * 2u)) & 3u;
}

void gpio_set_pull(GPIO_TypeDef *port, unsigned pin, uint32_t pull) {
    port->PUPDR = write_field2(port->PUPDR, pin, pull);
}

void gpio_write(GPIO_TypeDef *port, unsigned pin, bool level) {
    port->BSRR = level ? (1u << pin) : (1u << (pin + 16u));
}

void gpio_toggle(GPIO_TypeDef *port, unsigned pin) {
    port->ODR = port->ODR ^ (1u << pin);
}

bool gpio_read(const GPIO_TypeDef *port, unsigned pin) {
    return (port->IDR >> pin) & 1u;
}

int main(void) {
    GPIO_TypeDef port = {0};

    gpio_set_mode(&port, 5, GPIO_MODE_OUTPUT);
    assert(port.MODER == (1u << 10));
    gpio_set_mode(&port, 0, GPIO_MODE_ANALOG);
    assert(port.MODER == ((1u << 10) | 3u));
    gpio_set_mode(&port, 5, GPIO_MODE_ALT);              /* nadpisanie pola, nie OR */
    assert(port.MODER == ((2u << 10) | 3u));
    gpio_set_mode(&port, 15, GPIO_MODE_ANALOG);
    assert((port.MODER >> 30) == 3u);
    assert(gpio_get_mode(&port, 5) == GPIO_MODE_ALT);
    assert(gpio_get_mode(&port, 1) == GPIO_MODE_INPUT);
    assert(gpio_get_mode(&port, 15) == GPIO_MODE_ANALOG);

    port.PUPDR = 0xFFFFFFFFu;
    gpio_set_pull(&port, 3, GPIO_PULL_NONE);
    assert(port.PUPDR == ~(3u << 6));
    gpio_set_pull(&port, 3, GPIO_PULL_DOWN);
    assert(port.PUPDR == (~(3u << 6) | (2u << 6)));

    port.BSRR = 0xDEADu;
    gpio_write(&port, 4, true);
    assert(port.BSRR == (1u << 4));
    gpio_write(&port, 4, false);
    assert(port.BSRR == (1u << 20));

    port.ODR = 0x0001u;
    gpio_toggle(&port, 0);
    gpio_toggle(&port, 7);
    assert(port.ODR == (1u << 7));

    port.IDR = (1u << 2) | (1u << 9);
    assert(gpio_read(&port, 2) && gpio_read(&port, 9));
    assert(!gpio_read(&port, 3));

    puts("OK");
    return 0;
}
