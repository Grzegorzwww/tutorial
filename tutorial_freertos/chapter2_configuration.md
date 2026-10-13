# Chapter 2: Konfiguracja

[← Spis treści](README.md)

**Cel rozdziału:** zrozumieć najważniejsze opcje `FreeRTOSConfig.h`,
wybrać odpowiedni schemat zarządzania pamięcią i poprawnie ustawić
priorytety przerwań na Cortex-M (źródło większości "tajemniczych"
crashy u początkujących).

---

## 1. Struktura projektu z FreeRTOS

Do projektu dodajesz:

```
FreeRTOS-Kernel/
├── tasks.c, queue.c, list.c, timers.c, event_groups.c, stream_buffer.c
├── include/                          # FreeRTOS.h, task.h, queue.h, semphr.h ...
└── portable/
    ├── GCC/ARM_CM4F/                 # port.c + portmacro.h dla Twojego rdzenia
    └── MemMang/heap_4.c              # JEDEN wybrany schemat zarządzania pamięcią
FreeRTOSConfig.h                      # TWÓJ plik konfiguracyjny (w ścieżce include projektu)
```

Kluczowe: wybierasz **dokładnie jeden** port (zależny od rdzenia i
kompilatora, np. `ARM_CM4F` = Cortex-M4 z FPU) i **dokładnie jeden**
plik `heap_x.c`.

## 2. `FreeRTOSConfig.h` – najważniejsze opcje

```c
/* --- Scheduler --- */
#define configUSE_PREEMPTION                1        /* 1 = wywłaszczanie (prawie zawsze) */
#define configUSE_TIME_SLICING              1        /* round-robin dla równych priorytetów */
#define configCPU_CLOCK_HZ                  (SystemCoreClock)
#define configTICK_RATE_HZ                  ((TickType_t)1000)   /* 1 tick = 1 ms */
#define configMAX_PRIORITIES                7
#define configMINIMAL_STACK_SIZE            ((uint16_t)128)      /* w SŁOWACH (4 B na Cortex-M) */
#define configMAX_TASK_NAME_LEN             16
#define configUSE_16_BIT_TICKS              0        /* 0 = 32-bitowy licznik ticków */

/* --- Pamięć --- */
#define configSUPPORT_DYNAMIC_ALLOCATION    1
#define configSUPPORT_STATIC_ALLOCATION     0
#define configTOTAL_HEAP_SIZE               ((size_t)(20 * 1024))

/* --- Prymitywy synchronizacji --- */
#define configUSE_MUTEXES                   1
#define configUSE_RECURSIVE_MUTEXES         1
#define configUSE_COUNTING_SEMAPHORES       1
#define configQUEUE_REGISTRY_SIZE           8        /* nazwy kolejek widoczne w debuggerze */

/* --- Timery programowe --- */
#define configUSE_TIMERS                    1
#define configTIMER_TASK_PRIORITY           (configMAX_PRIORITIES - 2)
#define configTIMER_QUEUE_LENGTH            10
#define configTIMER_TASK_STACK_DEPTH        (configMINIMAL_STACK_SIZE * 2)

/* --- Diagnostyka (włącz na czas developmentu) --- */
#define configCHECK_FOR_STACK_OVERFLOW      2
#define configUSE_MALLOC_FAILED_HOOK        1
#define configUSE_TRACE_FACILITY            1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configASSERT(x)  if ((x) == 0) { taskDISABLE_INTERRUPTS(); for (;;); }

/* --- Które funkcje API są kompilowane --- */
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1        /* wymagane dla portMAX_DELAY = czekaj w nieskończoność */
#define INCLUDE_uxTaskGetStackHighWaterMark 1
```

| Opcja | Co robi / na co uważać |
|---|---|
| `configTICK_RATE_HZ` | Rozdzielczość czasu kernela. 1000 Hz = timeouty co 1 ms, ale częstsze przerwania (więcej narzutu, mniej snu CPU). Dla aplikacji nisko-poborowych bywa 100 Hz. |
| `configMAX_PRIORITIES` | Im więcej, tym większe zużycie RAM/CPU w schedulerze. Zwykle 5-8 wystarcza. |
| `configMINIMAL_STACK_SIZE` | Rozmiar stosu Idle task i punkt odniesienia dla innych. **Jednostka: słowa (words), nie bajty.** |
| `configUSE_16_BIT_TICKS` | Przy 1 kHz licznik 16-bit przepełnia się po ~65 s; 32-bit po ~49 dni. Na 32-bitowych MCU zawsze 0. |
| `configCHECK_FOR_STACK_OVERFLOW` | 1 = sprawdza wskaźnik stosu przy przełączeniu; 2 = dodatkowo wzorzec bajtów na końcu stosu (pewniejsze). Wymaga implementacji `vApplicationStackOverflowHook`. |
| `configASSERT` | Zawsze definiuj w buildzie developerskim. Wiele błędów użycia API (zły priorytet ISR, zwolnienie mutexa przez inne zadanie) kończy się właśnie tu. |
| `INCLUDE_vTaskSuspend` | Bez tego `portMAX_DELAY` oznacza tylko "bardzo długo" (~49 dni), a nie "bez limitu". |

## 3. Zarządzanie pamięcią – schematy sterty

FreeRTOS alokuje na własnej, statycznej tablicy `configTOTAL_HEAP_SIZE`
pamięć na stosy zadań, TCB, kolejki, semafory. Wybierasz jeden z plików:

| Plik | Zwalnianie | Łączenie wolnych bloków | Kiedy |
|---|---|---|---|
| `heap_1.c` | Nie | – | Wszystko tworzone raz na starcie, nigdy nie usuwane. Najprostsze i w pełni deterministyczne. |
| `heap_2.c` | Tak | Nie (fragmentacja!) | Praktycznie przestarzałe, zastąpione przez heap_4. |
| `heap_3.c` | Tak | Zależy od libc | Opakowuje `malloc`/`free` (wymaga thread-safe libc; wynik niedeterministyczny). |
| **`heap_4.c`** | Tak | **Tak** | **Domyślny wybór** w większości projektów. |
| `heap_5.c` | Tak | Tak | Jak heap_4, ale potrafi używać kilku rozłącznych regionów RAM (np. SRAM + CCM). |

Przydatne funkcje diagnostyczne (heap_4/5):
```c
size_t free_now  = xPortGetFreeHeapSize();               // ile wolne teraz
size_t free_min  = xPortGetMinimumEverFreeHeapSize();    // najgorszy zapas od startu
```
`MinimumEver` mówi, czy 20 KB sterty to rozsądny rozmiar, czy ocierasz
się o granicę.

### Alokacja statyczna – bez sterty w ogóle

W systemach o wysokich wymaganiach niezawodności (brak fragmentacji,
brak `malloc failed`) tworzysz obiekty z własnych buforów:

```c
#define configSUPPORT_STATIC_ALLOCATION 1

static StaticTask_t xBlinkTcb;
static StackType_t  xBlinkStack[128];

TaskHandle_t h = xTaskCreateStatic(vBlinkTask, "blink", 128, NULL, 1,
                                   xBlinkStack, &xBlinkTcb);
```
Przy włączonej alokacji statycznej musisz też dostarczyć
`vApplicationGetIdleTaskMemory()` (i `vApplicationGetTimerTaskMemory()`,
jeśli używasz timerów).

## 4. Priorytety przerwań na Cortex-M (bardzo ważne!)

Cortex-M ma sprzętowe priorytety przerwań NVIC, **niezależne** od
priorytetów zadań FreeRTOS. Kernel potrafi maskować część przerwań
w sekcjach krytycznych (rejestr `BASEPRI`), ale nie wszystkie.

```c
/* STM32F4: 4 bity priorytetu (16 poziomów, 0 = NAJWYŻSZY priorytet) */
#define configPRIO_BITS                          4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY  15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY \
        (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
        (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
```

Reguła:

| Priorytet NVIC ISR | Co wolno |
|---|---|
| **0-4** (numerycznie niższe niż `MAX_SYSCALL`) | Logicznie *wyższe* niż kernel. **Nie wolno** wywoływać żadnego API FreeRTOS (nawet `FromISR`). Nigdy nie są maskowane – dla bardzo krytycznych czasowo przerwań. |
| **5-15** (>= `MAX_SYSCALL`) | **Wolno** wywoływać funkcje `...FromISR`. Kernel może je maskować w sekcjach krytycznych. |

Pułapki:
- Numeryczne wartości są "odwrócone": **mniejsza liczba = wyższy priorytet**
  w NVIC, ale **większa liczba = wyższy priorytet** w zadaniach FreeRTOS.
- Przerwanie wołające `xQueueSendFromISR` z priorytetem np. 2 kończy się
  zawieszeniem lub `configASSERT` (w porcie jest `vPortValidateInterruptPriority`).
- Ustaw grupowanie priorytetów tak, żeby wszystkie bity były preemption:
  `HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4)` (i nie zmieniaj tego
  później). STM32CubeMX robi to zwykle sam.

### Mapowanie handlerów

```c
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
/* SysTick_Handler: przy HAL wołasz najpierw HAL_IncTick(), potem xPortSysTickHandler() */
```

**STM32 + HAL:** HAL domyślnie używa SysTick jako bazy czasu (`HAL_GetTick`),
a FreeRTOS też. CubeMX proponuje wtedy przełączenie HAL timebase na
inny timer (np. TIM6/TIM7), żeby uniknąć konfliktu priorytetów i
dziwnych opóźnień w `HAL_Delay` wołanym z zadania.

## 5. Różnice w ESP-IDF (ESP32)

| Aspekt | Vanilla FreeRTOS (Cortex-M) | ESP-IDF |
|---|---|---|
| Konfiguracja | `FreeRTOSConfig.h` ręcznie | `idf.py menuconfig` → *Component config → FreeRTOS* (plik generowany) |
| Stos w `xTaskCreate` | **Słowa** | **Bajty** (!) |
| Start | `main()` + `vTaskStartScheduler()` | Scheduler już działa, Twój kod to zadanie `app_main()` |
| Rdzenie | Zwykle 1 | 2 rdzenie (ESP32/S3): `xTaskCreatePinnedToCore(...)`, wersja SMP |
| Tick domyślny | 1000 Hz (zwykle) | `CONFIG_FREERTOS_HZ` domyślnie **100 Hz** |
| Sekcje krytyczne | `taskENTER_CRITICAL()` | `portENTER_CRITICAL(&spinlock)` z `portMUX_TYPE` (muszą chronić przed drugim rdzeniem) |
| Nagłówki | `FreeRTOS.h`, `task.h` | `freertos/FreeRTOS.h`, `freertos/task.h` |

> Pamiętaj z poprzednich doświadczeń: ESP-IDF jest globalny dla
> użytkownika (`~/esp/esp-idf`), więc zmiany w kernelu nie należą do
> projektu – konfiguruj przez `menuconfig`/`sdkconfig`, nie edytując
> źródeł IDF.

---

## ✅ Zadania

1. Policz, ile RAM zużyje zadanie utworzone z `usStackDepth = 256` na
   Cortex-M4 (word = 4 B) i na ESP32 przy tym samym wywołaniu
   (stack w bajtach). Wyjaśnij różnicę.
2. Dla STM32F4 z `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`:
   ISR UART ma priorytet NVIC 3, ISR timera ma 7. Który z nich może
   wołać `xQueueSendFromISR` i dlaczego?
3. Wybierz schemat sterty dla: (a) urządzenia, które po starcie nie
   tworzy ani nie usuwa obiektów; (b) urządzenia tworzącego dynamicznie
   zadania sesji TCP. Uzasadnij.
4. Włącz `configCHECK_FOR_STACK_OVERFLOW = 2`, zaimplementuj
   `vApplicationStackOverflowHook` (nieskończona pętla z diodą) i
   sprowokuj przepełnienie, dając zadaniu za mały stos i duży bufor lokalny.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 3 – Zadania i scheduling](chapter3_tasks_scheduling.md)
