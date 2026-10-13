# Chapter 6: Task notifications, Event Groups, Software Timers

[← Spis treści](README.md)

**Cel rozdziału:** poznać lżejsze i bardziej wyspecjalizowane mechanizmy
FreeRTOS, które często zastępują semafory i kolejki w prostszych
przypadkach.

---

## 1. Task Notifications – najlżejszy sposób na sygnał do zadania

Każde zadanie ma w TCB wbudowaną "skrzynkę" (32-bitowa wartość +
flaga oczekiwania). Nie trzeba tworzyć osobnego obiektu kernela
(semafora/kolejki), więc jest **mniej RAM** i **szybciej** niż binarny
semafor.

```c
static TaskHandle_t xHandlerTask;

void EXTI0_IRQHandler(void)
{
    BaseType_t w = pdFALSE;
    exti_clear_flag();
    vTaskNotifyGiveFromISR(xHandlerTask, &w);     // jak Give na semaforze zliczającym
    portYIELD_FROM_ISR(w);
}

void vHandlerTask(void *arg)
{
    for (;;) {
        uint32_t n = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // czeka; zwraca licznik i zeruje
        while (n--) {
            process_event();
        }
    }
}

xTaskCreate(vHandlerTask, "handler", 256, NULL, 4, &xHandlerTask);
```

- `ulTaskNotifyTake(pdTRUE, ...)` zachowuje się jak semafor **zliczający**
  ("ile zdarzeń przyszło"); z `pdFALSE` jak binarny (dekrement o 1).
- Bardziej ogólna wersja: `xTaskNotify(handle, value, eAction)` pozwala
  ustawiać bity, wpisywać wartość, inkrementować (`eSetBits`,
  `eSetValueWithOverwrite`, `eIncrement`...) – działa jak lekka
  event group lub skrzynka na 1 wartość.

**Ograniczenia (kiedy NIE używać):**
- Odbiorcą może być tylko **jedno, znane zadanie** (adresujesz po uchwycie).
- Odbiorca może blokować się na notyfikacji, ale **nadawca nie** może
  czekać, aż notyfikacja zostanie odebrana.
- Nie służy do ochrony zasobu (to nie mutex) i nie niesie wielu wartości w kolejce.

Reguła: sygnał ISR → konkretne zadanie? **Zacznij od task notification.**
Wielu odbiorców, bufor wielu wartości lub brak uchwytu? Semafor/kolejka.

## 2. Event Groups – czekanie na kombinację zdarzeń

Zestaw bitów (flag), na które zadania mogą czekać w warunku "dowolny"
(OR) lub "wszystkie" (AND). Przydatne do synchronizacji startu,
oczekiwania na wiele warunków.

```c
#define EV_WIFI_UP     (1u << 0)
#define EV_SENSOR_OK   (1u << 1)
#define EV_TIME_SYNCED (1u << 2)

static EventGroupHandle_t xEvents;

/* Różne zadania/ISR ustawiają bity, gdy coś się stanie */
xEventGroupSetBits(xEvents, EV_WIFI_UP);

/* Zadanie czeka aż WSZYSTKIE trzy warunki będą spełnione */
void vPublisherTask(void *arg)
{
    for (;;) {
        EventBits_t bits = xEventGroupWaitBits(
            xEvents,
            EV_WIFI_UP | EV_SENSOR_OK | EV_TIME_SYNCED,
            pdFALSE,        /* xClearOnExit: nie czyść bitów po wyjściu */
            pdTRUE,         /* xWaitForAllBits: AND (pdFALSE = OR) */
            portMAX_DELAY);
        publish_measurement();
    }
}

xEvents = xEventGroupCreate();
```

Zastosowania: "rendez-vous" kilku zadań na starcie, stan systemu
(połączono/skonfigurowano/gotowe), czekanie na **jedno z wielu**
zdarzeń (`xWaitForAllBits = pdFALSE`).

Uwaga: z ISR używaj `xEventGroupSetBitsFromISR` (deleguje pracę do
timer service task); liczba bitów jest ograniczona (24 przy 32-bitowym tiku).

## 3. Software Timers – opóźnione/okresowe wywołania bez własnego zadania

Timer programowy wywołuje funkcję-callback po zadanym czasie
(jednorazowo lub okresowo). Nie wymaga osobnego zadania – callbacki
wykonuje wspólne **Timer Service Task** (tworzone automatycznie przy
`configUSE_TIMERS = 1`).

```c
static TimerHandle_t xDebounceTimer;

static void vDebounceCallback(TimerHandle_t t)
{
    if (button_is_pressed()) {
        handle_button_press();            // przycisk stabilny przez 30 ms
    }
}

/* Utwórz timer jednorazowy (pdFALSE = bez auto-reload) 30 ms */
xDebounceTimer = xTimerCreate("deb", pdMS_TO_TICKS(30), pdFALSE, NULL, vDebounceCallback);

/* ISR: każde zbocze restartuje odliczanie - callback ruszy dopiero po 30 ms ciszy */
void EXTI0_IRQHandler(void)
{
    BaseType_t w = pdFALSE;
    exti_clear_flag();
    xTimerResetFromISR(xDebounceTimer, &w);
    portYIELD_FROM_ISR(w);
}
```
To eleganckie rozwiązanie debounce z zadania rekrutacyjnego, bez
własnych liczników.

**Reguły dla callbacków timerów (krytyczne):**
- Wykonują się w kontekście **Timer Service Task**, nie w ISR.
- **Nie wolno ich blokować** (`vTaskDelay`, czekanie na semafor/kolejkę
  bez timeoutu 0) – zablokowałoby to wszystkie timery systemu.
- Mają być krótkie; ciężką pracę przekaż do zadania (kolejka/notification).
- Priorytet Timer Service Task (`configTIMER_TASK_PRIORITY`) ustaw
  wyżej niż zadania, od których zależy dokładność timerów.

API sterujące (`xTimerStart`, `xTimerStop`, `xTimerReset`,
`xTimerChangePeriod`) wysyła komendy do kolejki Timer Service Task
(`configTIMER_QUEUE_LENGTH`), a nie wykonuje ich bezpośrednio – w ISR
używaj wersji `...FromISR`.

## 4. Stream Buffers i Message Buffers (krótko)

- **Stream buffer** – strumień bajtów od **jednego** nadawcy do
  **jednego** odbiorcy (np. UART → parser). Zoptymalizowany pod kątem
  prostoty i szybkości, bez kopiowania elementów po jednym.
- **Message buffer** – jak wyżej, ale zachowuje granice wiadomości o
  zmiennej długości.

Gdy masz ciągły strumień bajtów (UART, DMA), stream buffer bywa
prostszy i tańszy niż kolejka jednobajtowych elementów. Ograniczenie:
tylko jeden zapisujący i jeden czytający (single producer / single consumer).

## 5. Który mechanizm wybrać – ściąga

| Potrzeba | Narzędzie |
|---|---|
| ISR/zadanie → **konkretne zadanie**, sygnał/licznik zdarzeń | Task notification |
| Przekazać **dane** (struktury) | Kolejka |
| Ciągły **strumień bajtów** 1:1 | Stream buffer |
| Poczekać na **kombinację** warunków (AND/OR) | Event group |
| Chronić **zasób** | Mutex |
| Sygnał do **wielu** zadań / bez znajomości odbiorcy | Semafor lub event group |
| Wywołać funkcję **po czasie / okresowo** | Software timer |

---

## ✅ Zadania

1. Zamień przykład przycisk-ISR z semaforem binarnego (Chapter 5) na
   `vTaskNotifyGiveFromISR`/`ulTaskNotifyTake`. Zmierz (GPIO lub
   `DWT->CYCCNT` na Cortex-M) czas od ISR do uruchomienia zadania dla obu wersji.
2. Zrób event group z trzema bitami (Wi-Fi, czujnik, czas) i zadanie
   publikujące dopiero, gdy wszystkie są ustawione. Ustawiaj bity z
   różnych zadań w losowej kolejności.
3. Zaimplementuj debounce przycisku z timerem programowym i porównaj z
   wersją `vTaskDelay(30)` w zadaniu obsługi.
4. Wyjaśnij, dlaczego `vTaskDelay` wewnątrz callbacku timera jest błędem.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 7 – Typowe błędy, debugowanie i pytania rekrutacyjne](chapter7_pitfalls_debugging.md)
