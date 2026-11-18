# Chapter 5: Synchronizacja

[← Spis treści](README.md)

**Cel rozdziału:** wybierać właściwy mechanizm synchronizacji (mutex,
semafor, zmienna warunkowa, spinlock, operacje atomowe) i wiedzieć, **co wolno
wywołać z przerwania**.

> Koncepcyjnie różnica *mutex vs semafor* jest taka sama jak w FreeRTOS. Jeśli
> potrzebujesz odświeżenia (priority inversion, dziedziczenie priorytetu),
> zobacz [Chapter 5 kursu FreeRTOS](../tutorial_freertos/chapter5_semaphore_vs_mutex.md).
> Tu skupiamy się na **API Zephyra** i jego regułach dla ISR.

---

## 1. Mutex – wzajemne wykluczanie

```c
K_MUTEX_DEFINE(counter_lock);           // statyczna definicja i inicjalizacja

k_mutex_lock(&counter_lock, K_FOREVER);
shared_counter++;                        // sekcja krytyczna
k_mutex_unlock(&counter_lock);
```

Cechy `k_mutex`:
- **ma właściciela**: tylko wątek, który zablokował mutex, może go odblokować,
- **rekurencyjny**: ten sam wątek może zablokować go wielokrotnie (trzeba
  tyle samo `unlock`),
- **dziedziczenie priorytetu**: gdy wątek o wysokim priorytecie czeka na
  mutex trzymany przez wątek o niskim, ten drugi tymczasowo dziedziczy wysoki
  priorytet (ogranicza *priority inversion*),
- **nie wolno** używać w ISR.

Zwraca `0` przy sukcesie, `-EBUSY` (`K_NO_WAIT` i zajęty) albo `-EAGAIN`
(timeout). **Zawsze sprawdzaj wynik**, gdy używasz timeoutu.

## 2. Semafor – sygnalizacja i zliczanie

```c
K_SEM_DEFINE(data_ready, 0, 1);   // wartość startowa 0, limit 1 = semafor binarny
//                       ^  ^
//                       |  maksymalna wartość licznika
//                       początkowa wartość licznika

k_sem_give(&data_ready);                       // zwiększ licznik (budzi czekającego)
int ret = k_sem_take(&data_ready, K_MSEC(50)); // zmniejsz licznik lub czekaj
```

`k_sem_take` zwraca `0`, `-EBUSY` (`K_NO_WAIT`, brak zasobu) lub `-EAGAIN`
(timeout). Limit większy niż 1 daje semafor zliczający (np. liczba wolnych
buforów).

Semafor **nie ma właściciela** i **nie dziedziczy priorytetu**. Nadaje się do
sygnalizacji zdarzeń (np. przerwanie → wątek), a nie do ochrony danych.

## 3. Przykład: mutex + semafor oddawany z przerwania

```c
#include <zephyr/kernel.h>

#define STACK_SIZE 1024
#define LOOPS      50000

K_MUTEX_DEFINE(counter_lock);
K_SEM_DEFINE(data_ready, 0, 1);		/* semafor binarny: wartość startowa 0, limit 1 */

static int shared_counter;

static void incrementer(void *p1, void *p2, void *p3)
{
	for (int i = 0; i < LOOPS; i++) {
		k_mutex_lock(&counter_lock, K_FOREVER);
		shared_counter++;
		k_mutex_unlock(&counter_lock);
	}
}

K_THREAD_DEFINE(inc1, STACK_SIZE, incrementer, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(inc2, STACK_SIZE, incrementer, NULL, NULL, NULL, 5, 0, 0);

/* Wywoływane w kontekście przerwania zegara - k_sem_give jest tu dozwolone. */
static void timer_expiry(struct k_timer *timer)
{
	k_sem_give(&data_ready);
}
K_TIMER_DEFINE(event_timer, timer_expiry, NULL);

int main(void)
{
	k_thread_join(inc1, K_FOREVER);
	k_thread_join(inc2, K_FOREVER);
	printk("counter = %d (oczekiwane %d)\n", shared_counter, 2 * LOOPS);

	k_timer_start(&event_timer, K_MSEC(200), K_NO_WAIT);

	/* 1) czekamy krócej niż trwa timer -> timeout */
	int ret = k_sem_take(&data_ready, K_MSEC(50));
	printk("take z timeoutem 50 ms: %d (%s)\n", ret, ret == -EAGAIN ? "-EAGAIN" : "ok");

	/* 2) czekamy bez limitu -> semafor oddany z przerwania timera */
	ret = k_sem_take(&data_ready, K_FOREVER);
	printk("take po timerze: %d\n", ret);

	return 0;
}
```

Wynik (QEMU):

```
counter = 100000 (oczekiwane 100000)
take z timeoutem 50 ms: -11 (-EAGAIN)
take po timerze: 0
```

Dwa wątki o tym samym priorytecie (między którymi może przełączać *time slicing*)
poprawnie zwiększają wspólny licznik dzięki mutexowi. `-11` to `-EAGAIN`.
Semafor został oddany w **przerwaniu timera** (`timer_expiry`), a odebrany w
wątku `main`: klasyczny wzorzec ISR → wątek.

> Wynik `counter == 100000` z mutexem jest gwarantowany. Bez mutexa
> `shared_counter++` to odczyt-modyfikacja-zapis w kilku instrukcjach i
> wywłaszczenie w środku może zgubić inkrementację. Wynik zależy od
> momentu przełączenia, więc błąd bywa **niewidoczny przy testach**, a
> pojawia się w produkcji. Dlatego rozumuj o poprawności, a nie „u mnie działa”.

## 4. Zmienna warunkowa, spinlock, operacje atomowe

```c
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/spinlock.h>

static struct k_spinlock lock;
static int shared;
static atomic_t flags = ATOMIC_INIT(0);

K_MUTEX_DEFINE(cv_mutex);
K_CONDVAR_DEFINE(cv);
static bool ready;

static void waiter(void *a, void *b, void *c)
{
	k_mutex_lock(&cv_mutex, K_FOREVER);
	while (!ready) {			/* zawsze w pętli: możliwe wybudzenia pozorne */
		k_condvar_wait(&cv, &cv_mutex, K_FOREVER);
	}
	k_mutex_unlock(&cv_mutex);
	printk("waiter: warunek spelniony\n");
}
K_THREAD_DEFINE(waiter_tid, 1024, waiter, NULL, NULL, NULL, 5, 0, 0);

int main(void)
{
	/* Spinlock: wyłącza przerwania na czas sekcji - tylko na bardzo krótkie fragmenty. */
	K_SPINLOCK(&lock) {
		shared++;
	}

	/* Operacje atomowe bez blokady. */
	atomic_or(&flags, BIT(3));
	printk("shared=%d, bit3=%d, bit2=%d\n", shared,
	       (int)atomic_test_bit(&flags, 3), (int)atomic_test_bit(&flags, 2));

	k_msleep(50);
	k_mutex_lock(&cv_mutex, K_FOREVER);
	ready = true;
	k_condvar_signal(&cv);
	k_mutex_unlock(&cv_mutex);

	k_thread_join(waiter_tid, K_SECONDS(1));
	return 0;
}
```

Wynik:

```
shared=1, bit3=1, bit2=0
waiter: warunek spelniony
```

| Mechanizm | Do czego | Uwagi |
|---|---|---|
| **`k_condvar`** | Czekanie na **warunek** złożony z danych współdzielonych | Używaj razem z mutexem i warunkiem w pętli `while` |
| **`k_spinlock`** | Bardzo krótka ochrona danych współdzielonych **także z ISR** | Na jednym rdzeniu wyłącza przerwania, więc sekcja musi trwać ułamki µs. Nie wolno w niej blokować |
| **`atomic_t`** | Pojedyncze flagi i liczniki (`atomic_or`, `atomic_inc`, `atomic_test_bit`) | Bez blokad, wolno w ISR |

## 5. Co wolno w przerwaniu (ISR)

Zephyr **nie ma osobnych wersji `...FromISR`**. Ta sama funkcja działa w
wątku i w przerwaniu, o ile **nie blokuje**. Reguła brzmi:

> W ISR nigdy nie wolno **czekać**. Możesz sygnalizować, ale nie możesz blokować.

| Funkcja | W ISR? |
|---|---|
| `k_sem_give` | ✅ |
| `k_sem_take(..., K_NO_WAIT)` | ✅ |
| `k_sem_take(..., timeout != K_NO_WAIT)` | ❌ |
| `k_msgq_put(..., K_NO_WAIT)` | ✅ |
| `k_event_post` | ✅ |
| `k_work_submit` | ✅ |
| `k_timer_start`, `k_timer_stop` | ✅ |
| `atomic_*` | ✅ |
| `k_mutex_lock` / `k_mutex_unlock` | ❌ (mutex ma właściciela, ISR nie jest wątkiem) |
| `k_msleep`, `k_sleep`, `k_condvar_wait` | ❌ |
| `printk` | ✅ (ale wolne, nie nadużywaj) |

Do sprawdzenia, w jakim kontekście działa kod: `k_is_in_isr()` (patrz Chapter 7).

## 6. Kiedy co

| Potrzeba | Mechanizm |
|---|---|
| Ochrona danych współdzielonych między wątkami | `k_mutex` |
| Sygnał „zdarzenie się zdarzyło” (ISR → wątek) | `k_sem` (binarny) lub `k_event` |
| Liczba wolnych zasobów | `k_sem` zliczający |
| Czekanie na złożony warunek | `k_condvar` + `k_mutex` |
| Ochrona danych współdzielonych **z ISR** | `k_spinlock` (krótko) lub `atomic_t` |
| Pojedyncza flaga lub licznik | `atomic_t` |
| Przekazanie danych | `k_msgq`/`k_fifo` (Chapter 6) |

## 🔧 Różnica względem FreeRTOS

| FreeRTOS | Zephyr |
|---|---|
| `xSemaphoreCreateMutex()` | `K_MUTEX_DEFINE` (statycznie) |
| `xSemaphoreTake/Give` na mutexie | `k_mutex_lock/unlock` |
| `xSemaphoreCreateBinary()` (startuje od 0) | `K_SEM_DEFINE(s, 0, 1)` |
| `xSemaphoreGiveFromISR(&woken)` + `portYIELD_FROM_ISR` | `k_sem_give` (kernel sam przełącza kontekst po wyjściu z ISR) |
| Osobne API `FromISR` | Jedno API, liczy się czy blokujesz |
| Mutex rekurencyjny = osobny typ | `k_mutex` jest rekurencyjny |
| `taskENTER_CRITICAL()` | `K_SPINLOCK` / `irq_lock()` |

---

## ✅ Zadania

1. Uruchom przykład z mutexem. Usuń `k_mutex_lock/unlock` i zwiększ `LOOPS`.
   Czy zawsze widzisz błędny wynik? Wyjaśnij, dlaczego nie zawsze.
2. Zamień mutex na semafor binarny z początkową wartością 1 i porównaj
   zachowanie. Co się stanie, gdy wątek, który nie zablokował zasobu, go odda?
3. Zmień timeout w `k_sem_take(&data_ready, K_MSEC(50))` na `K_MSEC(500)`.
   Jaki jest kod zwrotny i dlaczego?
4. Napisz wątek producenta, który co 100 ms oddaje semafor, i konsumenta
   zliczającego, ile razy został obudzony. Ustaw limit semafora na 5 i opóźnij
   konsumenta. Co się dzieje z licznikiem?
5. Dla każdej funkcji z tabeli sekcji 5 napisz, czy wolno ją wywołać z
   `timer_expiry` i uzasadnij.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 6 – Komunikacja między wątkami](chapter6_message_passing.md)
