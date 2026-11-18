# Chapter 6: Komunikacja między wątkami

[← Spis treści](README.md)

**Cel rozdziału:** przekazywać dane i zdarzenia między wątkami (oraz z
przerwań) przez `k_msgq`, `k_fifo` i `k_event` oraz wybierać odpowiedni
mechanizm.

---

## 1. Przegląd

| Mechanizm | Co przekazuje | Kopiowanie | Typowe użycie |
|---|---|---|---|
| **`k_msgq`** | Komunikaty **stałego rozmiaru** | Tak (przez wartość) | Próbki z czujnika, komendy |
| **`k_fifo` / `k_lifo`** | **Wskaźniki** do elementów | Nie (przekazanie własności) | Duże bufory, pule |
| **`k_event`** | 32 flagi (bity) | – | „Coś się stało”, łączenie warunków |
| `k_sem` | Samo zliczanie | – | Sygnał bez danych (Chapter 5) |
| `k_pipe` | Strumień bajtów | Tak | Dane bez struktury (opis na końcu) |

## 2. `k_msgq` – kolejka komunikatów

```c
struct sample {
	uint32_t ts_ms;
	int16_t  temp_x10;
};

K_MSGQ_DEFINE(sample_q, sizeof(struct sample), 4, 4);   /* rozmiar, liczba, wyrównanie */
//             nazwa    rozmiar elementu        max_msgs  wyrównanie bufora
```

- `k_msgq_put(&q, &item, timeout)` **kopiuje** element do kolejki,
- `k_msgq_get(&q, &item, timeout)` kopiuje element z kolejki,
- zarówno `put`, jak i `get` zwracają `0` przy sukcesie, `-ENOMSG` gdy wracają
  **bez czekania** (`K_NO_WAIT`: pełna kolejka przy `put`, pusta przy `get`) lub
  po `k_msgq_purge`, oraz `-EAGAIN` przy timeoucie. **Zawsze sprawdzaj wynik**,
- `put` z przerwania działa **tylko z `K_NO_WAIT`**.

Producent z polityką „nie blokuj czujnika, gdy kolejka pełna” i wolniejszy
konsument:

```c
static uint32_t dropped;

static void producer(void *p1, void *p2, void *p3)
{
	for (int i = 0; i < 12; i++) {
		struct sample s = { .ts_ms = k_uptime_get_32(), .temp_x10 = 230 + i };

		if (k_msgq_put(&sample_q, &s, K_NO_WAIT) != 0) {
			dropped++;		/* kolejka pełna - nie blokuj producenta */
		}
		k_msleep(50);
	}
}

static void consumer(void *p1, void *p2, void *p3)
{
	struct sample s;

	while (k_msgq_get(&sample_q, &s, K_MSEC(500)) == 0) {
		printk("consumer: temp=%d.%d\n", s.temp_x10 / 10, s.temp_x10 % 10);
		k_msleep(200);			/* konsument wolniejszy od producenta */
	}
	printk("consumer: koniec, dropped=%u\n", dropped);
}

K_THREAD_DEFINE(prod_tid, STACK_SIZE, producer, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(cons_tid, STACK_SIZE, consumer, NULL, NULL, NULL, 5, 0, 0);
```

Wynik (fragment):

```
consumer: temp=23.0
consumer: temp=23.1
consumer: temp=23.2
...
consumer: temp=24.1
consumer: koniec, dropped=4
```

Producent wysłał 12 próbek co 50 ms, konsument przetwarza jedną na 200 ms, a
kolejka mieści 4. Odebrano 8, **4 zgubiono** (zauważ skok `23.5 → 23.7 → 24.1`).
Dobór długości kolejki to ta sama analiza co w FreeRTOS: musi pochłonąć
**najgorszy chwilowy rozjazd** szybkości producenta i konsumenta.

Dodatkowe funkcje: `k_msgq_peek` (odczyt bez usuwania), `k_msgq_purge`
(wyczyszczenie), `k_msgq_num_used_get` / `k_msgq_num_free_get`.

## 3. `k_fifo` – przekazywanie wskaźników

Zamiast kopiować duży element, przekazujesz **wskaźnik** i **własność**: po
`put` nadawca nie dotyka elementu, a odbiorca decyduje, co z nim zrobić.

```c
struct item {
	void    *fifo_reserved;		/* WYMAGANE: pierwsze słowo zarezerwowane dla jądra */
	uint32_t value;
};

K_FIFO_DEFINE(item_fifo);
static struct item items[3] = { { .value = 10 }, { .value = 20 }, { .value = 30 } };

for (int i = 0; i < 3; i++) {
	k_fifo_put(&item_fifo, &items[i]);	/* własność przechodzi na odbiorcę */
}

struct item *it;

while ((it = k_fifo_get(&item_fifo, K_NO_WAIT)) != NULL) {
	printk("fifo: %u\n", it->value);
}
```

Wynik:

```
fifo: 10
fifo: 20
fifo: 30
```

Zasady:
- **pierwsze pole** struktury musi być zarezerwowane dla jądra
  (`void *fifo_reserved`), bo jądro używa go do wiązania elementów w listę,
  bez dodatkowej alokacji,
- element musi **żyć**, dopóki jest w kolejce (statyczny, z puli), nigdy
  zmienna lokalna nadawcy,
- `k_fifo_alloc_put` kopiuje do bloku ze sterty jądra, a Twoja płytka ma domyślnie
  `CONFIG_HEAP_MEM_POOL_SIZE=0`, więc w embedded stosuj wzorzec ze statycznymi
  elementami (pula),
- `k_lifo` działa tak samo, ale w kolejności LIFO (stos).

To odpowiednik wzorca „kolejka niesie wskaźnik do bufora z puli” z kursu FreeRTOS.

## 4. `k_event` – flagi zdarzeń

```c
#define EV_BUTTON  BIT(0)
#define EV_TIMEOUT BIT(1)

K_EVENT_DEFINE(app_events);

static void event_waiter(void *p1, void *p2, void *p3)
{
	for (int i = 0; i < 2; i++) {
		uint32_t ev = k_event_wait_safe(&app_events, EV_BUTTON | EV_TIMEOUT, false, K_FOREVER);

		printk("event_waiter: zdarzenia 0x%x\n", ev);
	}
}
K_THREAD_DEFINE(ev_tid, STACK_SIZE, event_waiter, NULL, NULL, NULL, 4, 0, 0);

/* ... w main(): */
k_event_post(&app_events, EV_BUTTON);
k_msleep(100);
k_event_post(&app_events, EV_TIMEOUT);
```

Wynik:

```
event_waiter: zdarzenia 0x1
event_waiter: zdarzenia 0x2
```

> ⚠️ **Wymaga `CONFIG_EVENTS=y`.** Bez tego kod się skompiluje, ale linker
> zgłosi `undefined reference to z_impl_k_event_post` (Chapter 3).

Funkcje:

| Funkcja | Znaczenie |
|---|---|
| `k_event_post(&e, bity)` | **Dodaje** bity do zdarzeń (wolno w ISR) |
| `k_event_set(&e, bity)` | **Zastępuje** zestaw zdarzeń |
| `k_event_wait_safe(&e, maska, false, t)` | Czeka na **dowolny** bit z maski i **atomowo czyści** odebrane |
| `k_event_wait_all_safe(&e, maska, false, t)` | Czeka na **wszystkie** bity z maski |
| `k_event_wait` / `k_event_wait_all` | Wersje bez czyszczenia (czyścisz sam przez `k_event_clear`) |

> ⚠️ **Pułapka `reset`.** W `k_event_wait(..., reset, ...)` wartość `true` czyści
> zdarzenia **przed** czekaniem, więc zdarzenie wysłane chwilę wcześniej przepadnie.
> Nagłówek wprost ostrzega, że czyszczenie przez `reset` lub `k_event_clear`
> „may result in lost event information”. Dlatego używaj wariantów `_safe`
> z `reset = false`: odbierają zdarzenie i czyszczą je w jednej operacji atomowej.

`k_event` to odpowiednik **Event Groups** z FreeRTOS: wygodny, gdy wątek czeka
na „przycisk **lub** timeout” albo na „czujnik gotowy **i** komunikacja gotowa”.

## 5. Inne mechanizmy (krótko)

- **`k_poll`** – czekanie na kilka obiektów naraz (semafor, kolejka, sygnał).
- **`k_pipe`** – strumień bajtów w buforze: `k_pipe_init(pipe, buffer, size)`,
  `k_pipe_write(pipe, data, len, timeout)`, `k_pipe_read(...)`. Uwaga: API
  `k_pipe` zmieniło się w nowszych wersjach, więc sprawdź nagłówek swojej wersji.
- **`zbus`** – magistrala publish/subscribe (`CONFIG_ZBUS`) do luźnego
  łączenia modułów.

## 6. Kiedy co

| Sytuacja | Wybór |
|---|---|
| Małe, stałe komunikaty (próbka, komenda) | `k_msgq` |
| Duże ramki z puli | `k_fifo` ze wskaźnikami |
| Sygnał bez danych | `k_sem` |
| Łączenie wielu warunków | `k_event` |
| Wiele źródeł naraz | `k_poll` |
| Wymiana między luźno powiązanymi modułami | `zbus` |

## 🔧 Różnica względem FreeRTOS

| FreeRTOS | Zephyr |
|---|---|
| `xQueueCreate` + `xQueueSend` (kopia) | `K_MSGQ_DEFINE` + `k_msgq_put` (kopia) |
| Kolejka wskaźników na bufory z puli | `k_fifo` (z zarezerwowanym pierwszym słowem) |
| `xQueueSendFromISR` + `portYIELD_FROM_ISR` | `k_msgq_put(..., K_NO_WAIT)` w ISR |
| Event Groups | `k_event` (`CONFIG_EVENTS=y`) |
| `xQueueOverwrite` (kolejka długości 1) | Brak odpowiednika 1:1. „Ostatnią wartość” trzymaj w zmiennej chronionej `k_spinlock` lub `atomic_t` |
| Task Notifications | `k_sem` / `k_event` |
| Rozmiar kolejki w elementach, zawsze alokacja | Statyczna alokacja (makro `K_..._DEFINE`) |

---

## ✅ Zadania

1. Uruchom producenta i konsumenta z sekcji 2. Dobierz długość kolejki tak,
   aby przy tych okresach nie tracić próbek. Wynik uzasadnij rachunkiem.
2. Zamień `K_NO_WAIT` w `k_msgq_put` na `K_FOREVER`. Co się dzieje z producentem?
   Dlaczego to złe rozwiązanie dla czujnika czasu rzeczywistego?
3. Napisz wątek, który odbiera ramki przez `k_fifo` z puli 4 statycznych
   buforów 64 B i zwraca je do puli (drugi `k_fifo`).
4. Zbuduj przykład z `k_event` **bez** `CONFIG_EVENTS=y`, obejrzyj błąd,
   napraw.
5. Zmień `k_event_wait_safe` na `k_event_wait(..., true, ...)` i wyślij dwa zdarzenia
   jedno po drugim, zanim wątek wróci do czekania. Co się dzieje z drugim?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 7 – Timery, work queue i przerwania](chapter7_timers_workqueue_isr.md)
