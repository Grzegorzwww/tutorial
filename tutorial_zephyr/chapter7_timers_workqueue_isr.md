# Chapter 7: Timery, work queue i przerwania

[← Spis treści](README.md)

**Cel rozdziału:** używać `k_timer`, `k_work` i `k_work_delayable`, stosować
wzorzec *deferred interrupt processing* (ISR tylko zleca pracę) i obsłużyć
przycisk z debouncingiem na NUCLEO-H743ZI.

---

## 1. Złota zasada (jak w FreeRTOS)

> ISR ma robić **minimum**: potwierdzić sprzęt, zlecić pracę i wyjść.
> Resztę (obliczenia, logowanie, blokujące wywołania) wykonaj w wątku.

W Zephyrze narzędziem do tego jest **work queue**: ISR „zgłasza” element
pracy, a dedykowany wątek wykonuje go w normalnym kontekście, z dostępem do
całego API.

## 2. `k_timer` – timer, którego callback działa w ISR

```c
static void tick_expiry(struct k_timer *timer);
K_TIMER_DEFINE(tick_timer, tick_expiry, NULL);   // nazwa, callback po wygaśnięciu, callback po stop

k_timer_start(&tick_timer, K_MSEC(200), K_MSEC(200));   // pierwszy po 200 ms, potem co 200 ms
k_timer_stop(&tick_timer);
```

> ⚠️ Callback `k_timer` działa w **kontekście przerwania zegara systemowego**,
> a nie w wątku. Obowiązują reguły ISR z Chapter 5: bez blokowania, krótko.

Funkcje: `k_timer_start(timer, duration, period)` (`period = K_NO_WAIT` = jednorazowy),
`k_timer_stop`, `k_timer_status_get` (liczba wygaśnięć od ostatniego odczytu).

## 3. `k_work` – praca wykonywana w wątku

```c
static void tick_handler(struct k_work *work)
{
	printk("work: w ISR? %d, wątek: %s\n", k_is_in_isr(), k_thread_name_get(k_current_get()));
}
K_WORK_DEFINE(tick_work, tick_handler);

k_work_submit(&tick_work);        // wolno także z ISR
```

Domyślnie praca trafia do **systemowej kolejki pracy** (wątek `sysworkq`).

## 4. Przykład: timer → work

```c
#include <zephyr/kernel.h>

/* Wywoływane w KONTEKŚCIE PRZERWANIA zegara. */
static void tick_expiry(struct k_timer *timer);
K_TIMER_DEFINE(tick_timer, tick_expiry, NULL);

/* Wywoływane w WĄTKU kolejki systemowej (sysworkq). */
static void tick_handler(struct k_work *work)
{
	printk("work: w ISR? %d, wątek: %s\n", k_is_in_isr(), k_thread_name_get(k_current_get()));
}
K_WORK_DEFINE(tick_work, tick_handler);

static void tick_expiry(struct k_timer *timer)
{
	printk("timer expiry: w ISR? %d\n", k_is_in_isr());
	k_work_submit(&tick_work);		/* deferred processing: ISR tylko zleca pracę */
}

static void delayed_handler(struct k_work *work)
{
	printk("delayed work po ~350 ms\n");
}
K_WORK_DELAYABLE_DEFINE(delayed_work, delayed_handler);

int main(void)
{
	k_timer_start(&tick_timer, K_MSEC(200), K_MSEC(200));	/* pierwszy po 200 ms, potem co 200 */
	k_work_schedule(&delayed_work, K_MSEC(350));

	k_msleep(700);
	k_timer_stop(&tick_timer);
	printk("koniec, wywołań timera: %u\n", k_timer_status_get(&tick_timer));
	return 0;
}
```

Wymaga `CONFIG_THREAD_NAME=y` (żeby wypisać nazwę wątku). Wynik (QEMU):

```
timer expiry: w ISR? 1
work: w ISR? 0, wątek: sysworkq
delayed work po ~350 ms
timer expiry: w ISR? 1
work: w ISR? 0, wątek: sysworkq
timer expiry: w ISR? 1
work: w ISR? 0, wątek: sysworkq
koniec, wywołań timera: 3
```

To dokładnie obrazuje wzorzec: **callback timera = ISR** (`k_is_in_isr()` zwraca 1),
a praca **zlecona** przez `k_work_submit` wykonuje się w **wątku** `sysworkq`
(`k_is_in_isr()` zwraca 0), gdzie wolno już np. blokować i logować.

## 5. `k_work_delayable` i debouncing

`K_WORK_DELAYABLE_DEFINE` tworzy pracę z **opóźnieniem**:

| Funkcja | Zachowanie |
|---|---|
| `k_work_schedule(&w, delay)` | Zaplanuje pracę za `delay`. Jeśli już zaplanowana, **nic nie zmienia** |
| `k_work_reschedule(&w, delay)` | Zaplanuje pracę, a jeśli już zaplanowana, **przesuwa termin** |
| `k_work_cancel_delayable(&w)` | Anuluje oczekującą pracę |

`k_work_reschedule` jest idealne do **debouncingu**: każde kolejne zbocze
przesuwa termin, więc praca uruchomi się dopiero, gdy sygnał ustabilizował się
na czas `delay`. Sprawdzony test (5 zboczy co 20 ms, termin 50 ms):

```c
static int deb_runs;
static void deb_handler(struct k_work *work)
{
	deb_runs++;
	printk("deb_handler: uruchomiony (%d)\n", deb_runs);
}
K_WORK_DELAYABLE_DEFINE(deb, deb_handler);

for (int i = 0; i < 5; i++) {
	k_work_reschedule(&deb, K_MSEC(50));
	k_msleep(20);
}
```

Wynik: `deb uruchomiony 1 raz(y) dla 5 zboczy`. Pięć zdarzeń dało **jedno** wykonanie.

## 6. Kolejka systemowa jest kooperacyjna. Uwaga na długie handlery

Wątek `sysworkq` ma priorytet **-1** (kooperacyjny, Chapter 4). To znaczy:
- działa **przed** wszystkimi wątkami wywłaszczalnymi (w tym `main`),
- **nie jest wywłaszczany** przez wątki wywłaszczalne,
- jedna długa funkcja pracy **blokuje inne prace** w tej samej kolejce i
  opóźnia wątki użytkownika.

Dlatego handler w `sysworkq` ma być **krótki**. Wolne prace przenieś do własnej
kolejki z własnym priorytetem:

```c
K_THREAD_STACK_DEFINE(my_wq_stack, 1024);
static struct k_work_q my_wq;

static void slow_handler(struct k_work *work)
{
	printk("slow work w watku: %s\n", k_thread_name_get(k_current_get()));
	k_msleep(100);				/* wolna praca - nie w sysworkq */
}
K_WORK_DEFINE(slow_work, slow_handler);

int main(void)
{
	const struct k_work_queue_config cfg = { .name = "my_wq" };

	k_work_queue_start(&my_wq, my_wq_stack, K_THREAD_STACK_SIZEOF(my_wq_stack), 5, &cfg);
	k_work_submit_to_queue(&my_wq, &slow_work);	/* wykona się w wątku "my_wq" o priorytecie 5 */
	/* ... */
}
```

Wynik (QEMU): `slow work w watku: my_wq`.

## 7. Przerwanie od przycisku z debouncingiem (NUCLEO-H743ZI)

Przycisk użytkownika to alias `sw0` (**PC13**, aktywny stan wysoki wg DTS).
Wzorzec: ISR tylko przesuwa termin pracy, a po ustabilizowaniu sygnału wątek
odczytuje stan i przełącza diodę.

```c
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static struct gpio_callback button_cb_data;
static struct k_work_delayable debounce_work;

/* Wątek (sysworkq): można spokojnie czytać GPIO, logować, blokować. */
static void debounce_handler(struct k_work *work)
{
	if (gpio_pin_get_dt(&button) > 0) {	/* nadal wciśnięty po 30 ms */
		gpio_pin_toggle_dt(&led);
	}
}

/* ISR: tylko zleca pracę. Każde kolejne zbocze przesuwa termin (debouncing). */
static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_work_reschedule(&debounce_work, K_MSEC(30));
}

int main(void)
{
	if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
		return 0;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&button, GPIO_INPUT);

	k_work_init_delayable(&debounce_work, debounce_handler);
	gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

	k_sleep(K_FOREVER);			/* cała praca dzieje się w przerwaniu i work queue */
	return 0;
}
```

> Ten przykład został **zbudowany** dla `nucleo_h743zi` (Flash ~22 KB), ale nie
> uruchomiony w środowisku bez płytki. Zachowanie (przełączanie LD1 po
> wciśnięciu przycisku) sprawdź na sprzęcie.

Kolejność wywołań konfiguracyjnych jest ważna:
1. skonfiguruj piny (`gpio_pin_configure_dt`),
2. zainicjuj callback (`gpio_init_callback`) i dodaj go (`gpio_add_callback`),
3. dopiero **na końcu** włącz przerwanie (`gpio_pin_interrupt_configure_dt`),
   żeby nie dostać zbocza, zanim wszystko jest gotowe.

`GPIO_INT_EDGE_TO_ACTIVE` reaguje na zbocze do stanu *aktywnego* (z uwzględnieniem
polaryzacji z DTS), czyli tu na wciśnięcie. Dzięki temu zwolnienie przycisku nie
generuje kolejnego zgłoszenia pracy.

## 8. Przerwania niskiego poziomu

Własne przerwanie możesz podłączyć makrami `IRQ_CONNECT` (zwykły ISR przez
mechanizm Zephyra) lub `ISR_DIRECT_DECLARE` (bezpośredni ISR o minimalnym
opóźnieniu, bez narzutu jądra, ale z ograniczeniami). W typowej aplikacji
**nie potrzebujesz tego**: używasz przerwań przez sterowniki (jak `gpio_callback`
powyżej), a Zephyr obsługuje wektory i priorytety.

## 🔧 Różnica względem FreeRTOS

| FreeRTOS | Zephyr |
|---|---|
| Software Timer: callback w wątku *timer service* | `k_timer`: callback w **ISR** (uwaga!). Do pracy w wątku: `k_work_delayable` |
| `xTimerPendFunctionCallFromISR` | `k_work_submit` z ISR |
| Deferred interrupt processing: ISR → kolejka/semafor → zadanie | ISR → `k_work_submit` → wątek work queue |
| Task Notification z ISR do zadania | `k_sem_give` / `k_event_post` / `k_work_submit` |
| Zadanie obsługi przerwania o wysokim priorytecie | Dedykowana kolejka pracy (`k_work_queue_start`) |
| `portYIELD_FROM_ISR(&woken)` | Niepotrzebne. Kernel przełącza kontekst po wyjściu z ISR |

---

## ✅ Zadania

1. Uruchom przykład timer → work. Zmień `k_work_submit` na bezpośrednie
   wywołanie `tick_handler(NULL)` z `tick_expiry`. Co wypisze `k_is_in_isr()`
   i dlaczego to niebezpieczne dla zwykłej pracy?
2. Zmień okres timera na 5 ms i dodaj do `tick_handler` `k_msleep(20)`.
   Co się dzieje z pracą? (Podpowiedź: sprawdź liczbę wygaśnięć i
   priorytet `sysworkq`).
3. Przenieś pracę do własnej kolejki (priorytet 5) i porównaj zachowanie
   z zadaniem 2.
4. Zbuduj i wgraj przykład z przyciskiem. Zmień czas debouncingu na `K_MSEC(1)`
   i `K_MSEC(200)`. Co zauważasz?
5. Zamień `GPIO_INT_EDGE_TO_ACTIVE` na `GPIO_INT_EDGE_BOTH`. Ile razy
   zadziała przełączenie przy jednym wciśnięciu i zwolnieniu? Wyjaśnij, jak
   zmienić `debounce_handler`, żeby reagował tylko na wciśnięcie.
6. Dodaj do przykładu licznik naciśnięć w `atomic_t` zwiększany w
   `button_isr` i wypisuj go z wątku. Porównaj z licznikiem zwiększanym w
   `debounce_handler`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 8 – Logging i Shell](chapter8_logging_shell.md)
