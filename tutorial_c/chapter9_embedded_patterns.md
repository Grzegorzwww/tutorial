# Chapter 9: Wzorce embedded

[← Spis treści](README.md)

**Cel rozdziału:** zebrać wzorce, które w kodzie firmware'u w C
powtarzają się codziennie: dostęp do rejestrów przez struktury,
moduły z ukrytą implementacją (opaque pointer) i automaty stanów.

---

## 1. Rejestry peryferiów jako struktura

Producenci (CMSIS, ST, NXP) opisują blok rejestrów peryferium jako
strukturę, którą "nakłada się" na adres bazowy:

```c
#include <stddef.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER;    /* offset 0x00: tryb pinów, 2 bity na pin */
    volatile uint32_t OTYPER;   /* 0x04 */
    volatile uint32_t OSPEEDR;  /* 0x08 */
    volatile uint32_t PUPDR;    /* 0x0C */
    volatile uint32_t IDR;      /* 0x10: stan wejść */
    volatile uint32_t ODR;      /* 0x14: stan wyjść */
    volatile uint32_t BSRR;     /* 0x18: atomowe ustawianie/kasowanie */
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0x48000000u)

_Static_assert(offsetof(GPIO_TypeDef, ODR) == 0x14, "register map");
```

**Read-modify-write pola wielobitowego:**
```c
void gpio_set_mode(GPIO_TypeDef *port, unsigned pin, uint32_t mode) {
    uint32_t v = port->MODER;                 /* jeden odczyt */
    v &= ~(3u << (pin * 2));                  /* wyczyść 2 bity pola */
    v |= (mode & 3u) << (pin * 2);            /* wpisz nową wartość */
    port->MODER = v;                          /* jeden zapis */
}
```
**Rejestry "write-1-to-set" (BSRR):** zapis `1 << pin` ustawia pin, a
`1 << (pin + 16)` go kasuje – **bez** read-modify-write, więc atomowo
względem przerwań.

**Testowanie na PC:** funkcje przyjmują `GPIO_TypeDef *` zamiast używać
`GPIOA` na sztywno – w testach przekazujesz adres zwykłej zmiennej:
```c
GPIO_TypeDef fake = {0};
gpio_set_mode(&fake, 5, 1);
assert(fake.MODER == (1u << 10));
```

## 2. Opaque pointer – enkapsulacja w C

Nagłówek deklaruje typ, ale **nie** ujawnia jego zawartości:

```c
/* ring_logger.h */
typedef struct Logger Logger;              /* incomplete type */

Logger *logger_create(void);
void    logger_destroy(Logger *l);
int     logger_write(Logger *l, const char *msg);
size_t  logger_count(const Logger *l);
```
```c
/* ring_logger.c */
struct Logger {                            /* definicja tylko tutaj */
    char   lines[16][32];
    size_t count;
};
```
Użytkownik może trzymać tylko `Logger *` – nie może zajrzeć do pól, nie
zrobi `sizeof(Logger)` ani nie utworzy go na stosie. Zmiana wnętrza nie
wymaga rekompilacji kodu użytkownika. Tak zrobione jest `FILE *` w
bibliotece standardowej.

**Bez `malloc`:** w embedded `*_create` często zwraca obiekt ze
statycznej puli:
```c
#define MAX_LOGGERS 4
static struct Logger pool[MAX_LOGGERS];
static bool used[MAX_LOGGERS];

Logger *logger_create(void) {
    for (int i = 0; i < MAX_LOGGERS; ++i) {
        if (!used[i]) { used[i] = true; memset(&pool[i], 0, sizeof pool[i]); return &pool[i]; }
    }
    return NULL;
}
```

## 3. Automaty stanów

Prawie każdy firmware to zbiór automatów: protokół komunikacyjny,
obsługa przycisku, sekwencja startu urządzenia, ładowanie baterii.

**Wariant `switch`** – czytelny dla małych automatów:
```c
typedef enum { ST_IDLE, ST_WAIT_ACK, ST_DONE } State;

State step(State s, Event e) {
    switch (s) {
        case ST_IDLE:     if (e == EV_SEND) { send(); return ST_WAIT_ACK; } break;
        case ST_WAIT_ACK: if (e == EV_ACK) return ST_DONE;
                          if (e == EV_TIMEOUT) { send(); } break;
        case ST_DONE:     break;
    }
    return s;
}
```

**Wariant z tablicą przejść** – dane zamiast kodu, łatwo audytować:
```c
static const State transitions[ST_COUNT][EV_COUNT] = {
    [ST_IDLE]     = { [EV_SEND] = ST_WAIT_ACK, ... },
};
```

**Wariant z funkcją na stan** – każdy stan to funkcja zwracająca
następny stan; dobre, gdy stany mają dużo logiki.

### Przykład: debouncing przycisku

Mechaniczny styk "drga" przez kilka ms. Zamiast `delay` – próbkowanie co
1 ms i automat: stan zmienia się dopiero po N kolejnych identycznych
próbkach.
```c
typedef struct {
    bool     stable;     /* ostatni zatwierdzony stan */
    uint8_t  counter;    /* ile kolejnych próbek różni się od stable */
} Debounce;

/* zwraca true, gdy zatwierdzony stan się zmienił */
bool debounce_update(Debounce *d, bool sample, uint8_t threshold) {
    if (sample == d->stable) { d->counter = 0; return false; }
    if (++d->counter >= threshold) {
        d->stable = sample;
        d->counter = 0;
        return true;
    }
    return false;
}
```
Brak blokowania, deterministyczny czas, łatwe testowanie na PC (podajesz
sekwencję próbek).

---

## ⚠️ Pułapki

- Read-modify-write rejestru współdzielonego z przerwaniem – potrzebna
  sekcja krytyczna lub rejestr typu set/clear (BSRR).
- Brak `volatile` w strukturze rejestrów – kompilator zoptymalizuje odczyty statusu.
- Opaque type z publicznym `sizeof` (np. przez makro) – łamie enkapsulację.
- Automat bez obsługi "nieoczekiwanego zdarzenia" w każdym stanie –
  ignoruj je jawnie, a nie przypadkiem.

---

## ✅ Zadania

1. Znajdź w CMSIS/HAL swojego MCU definicję `GPIO_TypeDef` i porównaj
   offsety z dokumentacją (Reference Manual).
2. Narysuj diagram automatu debouncingu dla progu 3 i sekwencji próbek
   `0 1 0 1 1 1 0 0 0`.
3. Ćwiczenia praktyczne: **ex57** (rejestry GPIO), **ex58** (moduł z
   opaque pointerem), **ex59** (debouncing przycisku).

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 10 – Obsługa błędów](chapter10_error_handling.md)
