/* Zadanie 52 (C, rozdzial 6): event bus z callbackami i kontekstem `void *ctx`
 *
 *   bus_init:         pusta szyna
 *   bus_subscribe:    dodaje (fn, ctx); false gdy fn == NULL, szyna pelna albo para (fn, ctx)
 *                     juz jest zapisana. Ta sama funkcja z INNYM ctx to osobny subskrybent.
 *   bus_unsubscribe:  usuwa pare (fn, ctx), zachowujac kolejnosc pozostalych; false gdy brak
 *   bus_publish:      wywoluje wszystkich subskrybentow w kolejnosci zapisu;
 *                     zwraca liczbe wywolanych
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define MAX_SUBSCRIBERS 4

typedef void (*EventHandler)(int event, void *ctx);

typedef struct {
    EventHandler fn;
    void *ctx;
} Subscriber;

typedef struct {
    Subscriber subs[MAX_SUBSCRIBERS];
    size_t count;
} EventBus;

void bus_init(EventBus *bus) {
    /* TODO */
}

bool bus_subscribe(EventBus *bus, EventHandler fn, void *ctx) {
    /* TODO */
    return false;
}

bool bus_unsubscribe(EventBus *bus, EventHandler fn, void *ctx) {
    /* TODO */
    return false;
}

size_t bus_publish(EventBus *bus, int event) {
    /* TODO */
    return 0;
}

/* ---- kod testowy ---- */

typedef struct {
    int calls;
    int last_event;
} Counter;

static void count_handler(int event, void *ctx) {
    Counter *c = ctx;
    c->calls++;
    c->last_event = event;
}

typedef struct {
    char log[16];
    size_t len;
} Trace;

static Trace s_trace;

static void trace_handler(int event, void *ctx) {
    const char *tag = ctx;
    if (s_trace.len + 1 < sizeof s_trace.log) {
        s_trace.log[s_trace.len++] = tag[0];
        s_trace.log[s_trace.len] = '\0';
    }
    (void)event;
}

int main(void) {
    EventBus bus;
    bus_init(&bus);
    assert(bus_publish(&bus, 1) == 0);

    Counter a = {0}, b = {0};
    assert(bus_subscribe(&bus, count_handler, &a));
    assert(bus_subscribe(&bus, count_handler, &b));     /* ta sama funkcja, inny kontekst */
    assert(!bus_subscribe(&bus, count_handler, &a));    /* duplikat */
    assert(!bus_subscribe(&bus, NULL, &a));

    assert(bus_publish(&bus, 7) == 2);
    assert(a.calls == 1 && a.last_event == 7);
    assert(b.calls == 1 && b.last_event == 7);

    assert(bus_unsubscribe(&bus, count_handler, &a));
    assert(!bus_unsubscribe(&bus, count_handler, &a));
    assert(bus_publish(&bus, 8) == 1);
    assert(a.calls == 1 && b.calls == 2 && b.last_event == 8);

    bus_init(&bus);
    assert(bus_subscribe(&bus, trace_handler, "A"));
    assert(bus_subscribe(&bus, trace_handler, "B"));
    assert(bus_subscribe(&bus, trace_handler, "C"));
    assert(bus_subscribe(&bus, trace_handler, "D"));
    assert(!bus_subscribe(&bus, trace_handler, "E"));   /* pelna */
    bus_publish(&bus, 0);
    assert(s_trace.len == 4 && s_trace.log[0] == 'A' && s_trace.log[3] == 'D');

    s_trace.len = 0;
    const char *b_tag = bus.subs[1].ctx;
    assert(bus_unsubscribe(&bus, trace_handler, (void *)b_tag));
    bus_publish(&bus, 0);
    assert(s_trace.len == 3 && s_trace.log[0] == 'A' && s_trace.log[1] == 'C' && s_trace.log[2] == 'D');

    puts("OK");
    return 0;
}
