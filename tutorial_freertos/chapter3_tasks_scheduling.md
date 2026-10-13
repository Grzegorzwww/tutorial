# Chapter 3: Zadania i scheduling

[← Spis treści](README.md)

**Cel rozdziału:** poprawnie tworzyć zadania, dobierać priorytety i stos,
używać opóźnień okresowych oraz pisać zadania w C++.

---

## 1. Tworzenie zadania

```c
BaseType_t xTaskCreate(
    TaskFunction_t       pxTaskCode,     // funkcja: void fn(void *pvParameters)
    const char * const   pcName,         // nazwa (debugger, vTaskList)
    configSTACK_DEPTH_TYPE usStackDepth, // rozmiar stosu w SŁOWACH (na ESP-IDF: w bajtach)
    void *               pvParameters,   // argument przekazany do funkcji zadania
    UBaseType_t          uxPriority,     // 0..configMAX_PRIORITIES-1, większy = ważniejszy
    TaskHandle_t *       pxCreatedTask   // opcjonalny uchwyt (może być NULL)
);
```
Zwraca `pdPASS` albo `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`, jeśli
zabrakło sterty – **zawsze sprawdzaj wynik** dla zadań tworzonych
dynamicznie.

```c
TaskHandle_t xSensorHandle = NULL;
BaseType_t rc = xTaskCreate(vSensorTask, "sensor", 256, NULL, 3, &xSensorHandle);
configASSERT(rc == pdPASS);
```

Zadanie kończy się wyłącznie przez `vTaskDelete(NULL)` (usuń samego
siebie) lub `vTaskDelete(handle)`. **Nigdy nie kończ funkcji zadania
przez `return`** – to undefined behavior (powrót w nieznane miejsce).

```c
void vOneShotTask(void *arg)
{
    do_initialization();
    vTaskDelete(NULL);    // sprzątanie zrobi Idle task
}
```

## 2. Opóźnienia – `vTaskDelay` vs `vTaskDelayUntil`

```c
vTaskDelay(pdMS_TO_TICKS(100));
```
Blokuje zadanie na **co najmniej** 100 ms **licząc od momentu wywołania**.
Jeśli reszta pętli trwa 20 ms, cały cykl trwa 120 ms – okres "pływa".

```c
TickType_t xLastWake = xTaskGetTickCount();
for (;;) {
    sample_adc();                                   // trwa ~ 20 ms (i zmiennie)
    vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(100)); // stały okres 100 ms
}
```
`vTaskDelayUntil` (w nowszych wersjach `xTaskDelayUntil`) odmierza czas
od **poprzedniego momentu wybudzenia**, więc okres jest stały niezależnie
od czasu wykonania ciała pętli (o ile ciało jest krótsze niż okres).
Używaj go dla zadań cyklicznych: próbkowanie, regulatory PID, ramki
protokołu.

`pdMS_TO_TICKS(ms)` konwertuje milisekundy na ticki zależnie od
`configTICK_RATE_HZ` – **nie wpisuj "gołych" liczb ticków**, bo kod
przestanie działać po zmianie częstotliwości ticka.

## 3. Priorytety – jak je dobierać

Reguła kciuka (zbliżona do Rate Monotonic Scheduling): **im krótszy
wymagany czas reakcji / okres, tym wyższy priorytet.**

| Przykład zadania | Sugerowany priorytet |
|---|---|
| Obsługa krytycznego zdarzenia (awaria, zabezpieczenie) | Wysoki |
| Pętla regulacji 1 kHz | Wysoki |
| Odbiór/parsowanie protokołu | Średni |
| Logika aplikacji, UI | Niski-średni |
| Logowanie, diagnostyka, mruganie diodą | Niski |

Zasady:
- Mało poziomów jest lepsze niż wiele (łatwiej analizować).
- Zadania wysokopriorytetowe powinny **robić mało i blokować się
  szybko** – przekazują ciężką pracę niżej przez kolejki.
- Dwa zadania o tym samym priorytecie dzielą czas (time slicing) –
  dobre dla niezależnych, mało krytycznych prac.

## 4. Stos zadania – ile dać

Stos musi pomieścić: zmienne lokalne, ramki wywołań, zapisany kontekst
(rzędu 17-50 słów zależnie od FPU), oraz wnętrze wywoływanych
bibliotek (`printf` z `%f` potrafi zjeść kilkaset bajtów do kilku KB).

Metoda empiryczna:
1. Daj hojny stos (np. 512 słów).
2. Uruchom aplikację w typowych i skrajnych scenariuszach.
3. Sprawdź "high water mark" – najmniejszą ilość wolnego stosu, jaka kiedykolwiek była:

```c
UBaseType_t uxFreeWords = uxTaskGetStackHighWaterMark(NULL);  // NULL = bieżące zadanie
```
4. Zmniejsz stos tak, by zostawić zapas ok. 20-30%.

Unikaj dużych tablic lokalnych (`char buf[1024]` w zadaniu o stosie 256
słów). Duże bufory deklaruj jako `static` lub alokuj z puli.

## 5. Przekazywanie parametrów do zadania

```c
typedef struct {
    uint8_t  led_pin;
    uint32_t period_ms;
} BlinkConfig_t;

static const BlinkConfig_t cfgA = { .led_pin = 1, .period_ms = 250 };
static const BlinkConfig_t cfgB = { .led_pin = 2, .period_ms = 700 };

void vBlinkTask(void *pvParameters)
{
    const BlinkConfig_t *cfg = (const BlinkConfig_t *)pvParameters;
    for (;;) {
        gpio_toggle(cfg->led_pin);
        vTaskDelay(pdMS_TO_TICKS(cfg->period_ms));
    }
}

xTaskCreate(vBlinkTask, "blinkA", 128, (void *)&cfgA, 1, NULL);
xTaskCreate(vBlinkTask, "blinkB", 128, (void *)&cfgB, 1, NULL);
```
Jedna funkcja, dwie instancje zadania. **Wskazywane dane muszą żyć dłużej
niż zadanie** – nie przekazuj adresu zmiennej lokalnej funkcji, która
zaraz się skończy (klasyczny dangling pointer).

## 6. Zadania w C++

Funkcja zadania musi mieć sygnaturę `void(void*)`, więc **nie może być
zwykłą metodą niestatyczną**. Standardowy wzorzec: statyczny "trampolina"
+ `this` jako parametr.

```cpp
class SensorTask {
public:
    bool start()
    {
        return xTaskCreate(&SensorTask::trampoline, "sensor", 256, this, 3, &handle_) == pdPASS;
    }

private:
    static void trampoline(void *self) { static_cast<SensorTask *>(self)->run(); }

    void run()
    {
        TickType_t last = xTaskGetTickCount();
        for (;;) {
            sample();
            vTaskDelayUntil(&last, pdMS_TO_TICKS(100));
        }
    }

    void sample();
    TaskHandle_t handle_ = nullptr;
};
```
Uważaj, by obiekt `SensorTask` żył co najmniej tak długo jak zadanie
(typowo: obiekt statyczny/globalny albo alokowany raz na starcie). Unikaj
wyjątków i RTTI w zadaniach (narzut na stosie, `-fno-exceptions` w
embedded).

## 7. Inne przydatne funkcje

```c
taskYIELD();                       // oddaj CPU zadaniom o tym samym priorytecie
vTaskPrioritySet(h, 4);            // zmiana priorytetu w locie
vTaskSuspend(h); vTaskResume(h);   // wstrzymanie / wznowienie
UBaseType_t n = uxTaskGetNumberOfTasks();
TaskHandle_t me = xTaskGetCurrentTaskHandle();
```

```c
char buf[512];
vTaskList(buf);    // tekstowa tabela zadań: nazwa, stan, priorytet, wolny stos
```
(`vTaskList` wymaga `configUSE_TRACE_FACILITY` i `configUSE_STATS_FORMATTING_FUNCTIONS`;
tylko do debugowania – dużo kosztuje i wyłącza przerwania na czas działania.)

---

## ✅ Zadania

1. Napisz zadanie próbkujące wejście co dokładnie 50 ms, z
   `vTaskDelayUntil`. Dodaj sztuczne `vTaskDelay(pdMS_TO_TICKS(random 0..20))`
   w środku pętli i zmierz (GPIO + analizator logiczny lub timestampy)
   okres. Powtórz z `vTaskDelay(50)` i porównaj jitter.
2. Zbuduj jedną funkcję `vBlinkTask` uruchomioną w dwóch instancjach z
   różnymi parametrami (struktura konfiguracji).
3. Zmierz `uxTaskGetStackHighWaterMark` dla zadania wywołującego
   `printf("%f", x)` i porównaj z wersją bez formatowania zmiennoprzecinkowego.
4. Zaimplementuj klasę C++ `Led` z metodą `start()` tworzącą własne
   zadanie (trampolina), mrugającą z zadanym okresem.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 4 – Kolejki i komunikacja z ISR](chapter4_queues_and_isr.md)
