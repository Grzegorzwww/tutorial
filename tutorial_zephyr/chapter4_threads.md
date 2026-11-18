# Chapter 4: Wątki i scheduling

[← Spis treści](README.md)

**Cel rozdziału:** tworzyć wątki (statycznie i dynamicznie), rozumieć
priorytety (**odwrotnie niż w FreeRTOS**), preempcję, time slicing, okresowe
zadania bez dryfu oraz życie wątku.

---

## 1. Wątek w Zephyrze

Każdy wątek to: **funkcja wejściowa**, **stos** i **struktura `k_thread`**.
W Zephyrze oba bloki pamięci są domyślnie **alokowane statycznie** (w czasie
kompilacji), a stos podajesz w **bajtach**.

Funkcja wejściowa zawsze ma trzy parametry:

```c
void entry(void *p1, void *p2, void *p3);
```

### Sposób 1: `K_THREAD_DEFINE` – statyczny, startuje automatycznie

```c
#define STACK_SIZE 1024

K_THREAD_DEFINE(fast_tid, STACK_SIZE, worker, "fast", (void *)(uintptr_t)100, NULL, 5, 0, 0);
//              nazwa     stos        funkcja p1      p2                      p3   prio opcje opóźnienie [ms]
```

Makro samo definiuje stos i `k_thread`, a `fast_tid` to uchwyt (`k_tid_t`).
Ostatni argument to opóźnienie startu w ms (`0` = od razu, `K_FOREVER` =
nie startuj, dopóki nie wywołasz `k_thread_start()`).

### Sposób 2: `k_thread_create` – dynamicznie, w czasie działania

```c
K_THREAD_STACK_DEFINE(dyn_stack, STACK_SIZE);     // stos (z wymaganym wyrównaniem)
static struct k_thread dyn_thread;                // struktura wątku

k_tid_t tid = k_thread_create(&dyn_thread, dyn_stack, K_THREAD_STACK_SIZEOF(dyn_stack),
                              worker, "dyn", (void *)(uintptr_t)200, NULL,
                              7, 0, K_NO_WAIT);
k_thread_name_set(tid, "dyn");
```

Nazwa i stos „dynamicznego” wątku nadal są statyczne. „Dynamiczne” oznacza
tylko, że **moment utworzenia** wybierasz w czasie działania. Używaj
`K_THREAD_STACK_DEFINE`, a nie zwykłej tablicy: makro zadba o wyrównanie
wymagane przez ochronę stosu (MPU).

## 2. Pełny przykład

```c
#include <zephyr/kernel.h>

#define STACK_SIZE 1024

static void worker(void *p1, void *p2, void *p3)
{
	const char *name = p1;
	uint32_t period_ms = (uint32_t)(uintptr_t)p2;

	for (int i = 0; i < 3; i++) {
		printk("[%4lld ms] %s tick %d\n", k_uptime_get(), name, i);
		k_msleep(period_ms);
	}
	printk("%s done\n", name);
}

/* Wątek startuje automatycznie przy starcie systemu (delay = 0). */
K_THREAD_DEFINE(fast_tid, STACK_SIZE, worker, "fast", (void *)(uintptr_t)100, NULL, 5, 0, 0);

/* Wątek z opóźnionym startem (500 ms). */
K_THREAD_DEFINE(slow_tid, STACK_SIZE, worker, "slow", (void *)(uintptr_t)300, NULL, 6, 0, 500);

/* Wątek tworzony dynamicznie w czasie działania programu. */
K_THREAD_STACK_DEFINE(dyn_stack, STACK_SIZE);
static struct k_thread dyn_thread;

int main(void)
{
	printk("main: start, priorytet %d\n", k_thread_priority_get(k_current_get()));

	k_tid_t tid = k_thread_create(&dyn_thread, dyn_stack, K_THREAD_STACK_SIZEOF(dyn_stack),
				      worker, "dyn", (void *)(uintptr_t)200, NULL,
				      7, 0, K_NO_WAIT);
	k_thread_name_set(tid, "dyn");

	k_thread_join(tid, K_FOREVER);   /* czekaj, aż wątek dyn zakończy pracę */
	printk("main: dyn zakonczony\n");

	return 0;
}
```

Przykładowy wynik (QEMU; czasy orientacyjne, symulator nie jest dokładny):

```
main: start, priorytet 0
[   0 ms] fast tick 0
[   0 ms] dyn tick 0
[ 110 ms] fast tick 1
[ 210 ms] dyn tick 1
[ 220 ms] fast tick 2
fast done
[ 420 ms] dyn tick 2
[ 510 ms] slow tick 0
dyn done
main: dyn zakonczony
[ 820 ms] slow tick 1
[1130 ms] slow tick 2
slow done
```

Co z tego widać:
- `main` ma priorytet **0** (to wątek jak każdy inny),
- `slow` wystartował po ~500 ms (opóźnienie z `K_THREAD_DEFINE`),
- wątki odmierzają czas **równolegle**, a `k_thread_join` w `main` czeka na koniec `dyn`.

Aby w kodzie dostępne były nazwy wątków (`k_thread_name_set`, logi, shell),
włącz `CONFIG_THREAD_NAME=y`.

## 3. Priorytety: mniejsza liczba = ważniejszy wątek

> ⚠️ **Najczęstsza pułapka dla osób z FreeRTOS.** W FreeRTOS wyższa liczba to
> wyższy priorytet. W Zephyrze jest **odwrotnie**.

```
   -16  ......  -1  |  0  ......  14  |  15
   kooperacyjne      |  wywłaszczalne  |  idle
   (najważniejsze)   |                 |  (najmniej ważny)
```

Dla Twojej płytki (`NUM_COOP_PRIORITIES=16`, `NUM_PREEMPT_PRIORITIES=15`):

| Priorytet | Rodzaj | Zachowanie |
|---|---|---|
| **ujemny** (-16..-1) | kooperacyjny | Wątek nie jest wywłaszczany przez inne wątki. Oddaje CPU tylko gdy sam się zablokuje lub zawoła `k_yield()` |
| **nieujemny** (0..14) | wywłaszczalny | Wątek o wyższym priorytecie, który stał się gotowy, **natychmiast** przejmuje CPU |
| 15 | *idle* | Wątek bezczynności, najniższy |

Makra pomocnicze: `K_PRIO_COOP(x)` i `K_PRIO_PREEMPT(x)`. Zauważ, że
`K_PRIO_COOP(0)` to **-16**, najwyższy możliwy priorytet, a `K_PRIO_COOP(15)` to -1.

**Domyślne wątki systemowe (Twoja płytka):**

| Wątek | Priorytet | Uwagi |
|---|---|---|
| `main` | 0 | Wywłaszczalny |
| kolejka pracy `sysworkq` | **-1** | Kooperacyjna. Patrz Chapter 7 |
| *idle* | 15 | |

Wniosek praktyczny: wątek `main` z priorytetem 0 jest **wyższy** niż wątki
z priorytetem 5 z przykładu. Jeśli `main` w pętli nic nie blokuje, wątki
`fast`/`slow` **nigdy** nie dostaną CPU. Zawsze blokuj się (`k_msleep`,
semafor, kolejka) w pętlach.

## 4. Preempcja – dowód

```c
#include <zephyr/kernel.h>

K_SEM_DEFINE(go, 0, 1);

/* Wysoki priorytet (1) czeka na semafor. */
static void high(void *a, void *b, void *c)
{
	k_sem_take(&go, K_FOREVER);
	printk("HIGH: obudzony, przejmuje CPU natychmiast\n");
}
K_THREAD_DEFINE(high_tid, 1024, high, NULL, NULL, NULL, 1, 0, 0);

/* Niski priorytet (10) startuje po 100 ms i oddaje semafor. */
static void low(void *a, void *b, void *c)
{
	printk("LOW: przed k_sem_give\n");
	k_sem_give(&go);
	printk("LOW: po k_sem_give\n");
}
K_THREAD_DEFINE(low_tid, 1024, low, NULL, NULL, NULL, 10, 0, 100);
```

Wynik:

```
LOW: przed k_sem_give
HIGH: obudzony, przejmuje CPU natychmiast
LOW: po k_sem_give
```

Komunikat `HIGH` pojawia się **między** dwiema liniami `LOW`. Gdy `LOW` wywołał
`k_sem_give`, odblokował wątek o wyższym priorytecie, a scheduler od razu go
uruchomił, nie czekając na koniec funkcji `low`.

## 5. Time slicing

Wątki o **tym samym** priorytecie mogą dzielić CPU w trybie round-robin.
Na Twojej płytce jest to domyślnie włączone: `CONFIG_TIMESLICING=y`,
`CONFIG_TIMESLICE_SIZE=20` (ms). Wątki kooperacyjne nie są wywłaszczane
przez time slicing. Aby wątek oddał CPU dobrowolnie: `k_yield()`.

## 6. Czas i timeouty

Tick na Twojej płytce to **10 000 Hz** (100 µs), a nie 1 kHz jak w typowym
FreeRTOS. Nie musisz jednak liczyć ticków. API przyjmuje **timeouty**:

| Zapis | Znaczenie |
|---|---|
| `K_NO_WAIT` | Nie czekaj |
| `K_FOREVER` | Czekaj bez limitu |
| `K_MSEC(100)`, `K_SECONDS(2)`, `K_USEC(50)` | Czas względny |
| `K_TIMEOUT_ABS_MS(t)` | Czas **bezwzględny** (do okresowych zadań) |

Funkcje: `k_msleep(ms)`, `k_sleep(timeout)`, `k_uptime_get()` (ms),
`k_busy_wait(us)` (aktywne oczekiwanie, **nie oddaje CPU**: tylko na ułamki
ms, np. czasy sprzętowe).

### Okres bez dryfu (odpowiednik `vTaskDelayUntil`)

```c
int64_t start = k_uptime_get();
int64_t next = start;

for (int i = 0; i < 3; i++) {
	next += 100;
	k_sleep(K_TIMEOUT_ABS_MS(next));       /* czekaj do terminu, nie "100 ms od teraz" */
	printk("okres %d: +%lld ms\n", i, k_uptime_get() - start);
}
```

Wynik (kolejne terminy co 100 ms):

```
okres 0: +100 ms
okres 1: +200 ms
okres 2: +300 ms
```

Zwykłe `k_msleep(100)` w pętli dryfuje, bo do każdego okresu dochodzi czas
wykonania ciała pętli. Termin bezwzględny usuwa ten błąd.

## 7. Cykl życia wątku

| Operacja | Funkcja |
|---|---|
| Czekaj na zakończenie | `k_thread_join(&thread, timeout)` |
| Zatrzymaj / wznów | `k_thread_suspend(tid)` / `k_thread_resume(tid)` |
| Zakończ wątek | `k_thread_abort(tid)` |
| Zmień priorytet | `k_thread_priority_set(tid, prio)` |
| Oddaj CPU | `k_yield()` |
| Uchwyt bieżącego wątku | `k_current_get()` |

Wątek kończy się, gdy jego funkcja **zwróci** (nie ma pętli nieskończonej
`for(;;)` jak w FreeRTOS). Zakończony wątek można dołączyć przez
`k_thread_join`. Funkcja `main()` też może się zakończyć.

## 🔧 Różnica względem FreeRTOS

| FreeRTOS | Zephyr |
|---|---|
| `xTaskCreate(fn, "n", stack_words, arg, prio, &h)` | `K_THREAD_DEFINE(...)` lub `k_thread_create(...)` |
| Stos w **słowach** (`StackType_t`) | Stos w **bajtach** |
| Priorytet: większa liczba = ważniejszy | Priorytet: **mniejsza liczba = ważniejszy** |
| Wszystkie zadania wywłaszczalne (domyślnie) | Priorytety kooperacyjne (ujemne) i wywłaszczalne |
| `vTaskStartScheduler()` | Scheduler startuje sam, `main()` jest wątkiem |
| Zadanie nie może wrócić (pętla `for(;;)`) | Wątek może wrócić i skończyć się |
| `vTaskDelay`, `vTaskDelayUntil` | `k_msleep`, `k_sleep(K_TIMEOUT_ABS_MS(...))` |
| Pamięć z `heap_x` lub statyczna (`xTaskCreateStatic`) | Domyślnie statyczna (makra), sterta jądra wyłączona |

---

## ✅ Zadania

1. Uruchom przykład z sekcji 2. Dodaj do `prj.conf` `CONFIG_THREAD_NAME=y`
   i wypisz nazwę bieżącego wątku (`k_thread_name_get(k_current_get())`).
2. Zmień priorytet `fast` z 5 na **-1** (kooperacyjny). Co się zmienia w
   kolejności wypisywania? Dlaczego?
3. Wstaw do `fast` pętlę aktywną `while (1) {}` (bez `k_msleep`). Które wątki
   przestają działać przy priorytecie 5, a które przy -1? Pamiętaj, że `main` ma 0.
4. Zbuduj dwa wątki o **tym samym** priorytecie, każdy w pętli aktywnej
   drukujący swoją nazwę co kilkaset iteracji. Zaobserwuj time slicing.
   Wyłącz `CONFIG_TIMESLICING` i porównaj.
5. Napisz wątek okresowy co 250 ms z `K_TIMEOUT_ABS_MS` i porównaj z wersją
   `k_msleep(250)` po dodaniu sztucznego opóźnienia (`k_busy_wait(20000)`)
   w ciele pętli.
6. Przeczytaj `kernel.h`: czym różni się `k_thread_abort` od zwykłego
   powrotu z funkcji wątku?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 5 – Synchronizacja](chapter5_synchronization.md)
