# Chapter 7: Typowe błędy, debugowanie i pytania rekrutacyjne

[← Spis treści](README.md)

---

## 1. Najczęstsze błędy początkujących

| Błąd | Objaw | Rozwiązanie |
|---|---|---|
| **Za mały stos zadania** | Losowe HardFault-y, uszkodzone zmienne, zawieszenia dziwnie zależne od ścieżki kodu | `configCHECK_FOR_STACK_OVERFLOW = 2`, `uxTaskGetStackHighWaterMark`; pamiętaj o jednostce (słowa vs bajty w ESP-IDF) |
| **Wołanie API bez `FromISR` w ISR** | HardFault, zawieszenie, `configASSERT` | Używaj wersji `...FromISR` + `portYIELD_FROM_ISR` |
| **Zły priorytet NVIC ISR wołającego API** | Zawieszenie/assert (`vPortValidateInterruptPriority`) | Priorytet ISR >= `configMAX_SYSCALL_INTERRUPT_PRIORITY` (Chapter 2) |
| **Zadanie z aktywnym czekaniem i wysokim priorytetem** | Niskie zadania i Idle nie działają, watchdog resetuje | Zawsze blokuj się: `vTaskDelay`, kolejka, semafor |
| **Binarny semafor jako zamek** | Inwersja priorytetów, opóźnienia nie do wyjaśnienia | Mutex (Chapter 5) |
| **Zapomniany `xSemaphoreGive` na jednej ścieżce wyjścia** | Deadlock (po jakimś czasie) | RAII w C++, jedno wyjście z funkcji, timeouty |
| **Zakleszczenie dwóch mutexów** | System "stoi", brak crasha | Stała kolejność brania, timeouty |
| **Blokowanie w callbacku timera** | Wszystkie timery przestają działać | Callback ma być krótki; ciężką pracę deleguj |
| **`return` z funkcji zadania** | HardFault / skok w nieznane | `vTaskDelete(NULL)` |
| **Dane zmienne lokalne przekazane do zadania/kolejki jako wskaźnik** | Śmieci w danych (dangling pointer) | Statyczne dane, kopiowanie przez kolejkę lub pula |
| **`volatile` zamiast synchronizacji** | Rzadkie, niepowtarzalne błędy | `volatile` nie daje atomowości; użyj kolejki/mutexa/atomic |
| **`printf`/`malloc` z wielu zadań bez ochrony** | Pomieszane wyjście, uszkodzona sterta | Gatekeeper do logów, `heap_4` (thread-safe), `configUSE_NEWLIB_REENTRANT` |
| **Fragmentacja sterty / brak pamięci** | `pdFAIL` z `xTaskCreate`, `malloc failed hook` | `heap_4`, alokacja statyczna, tworzenie obiektów raz na starcie |

### Sekcje krytyczne – kiedy i jak

Dla bardzo krótkich operacji na danych współdzielonych z ISR (kilka
instrukcji):

```c
taskENTER_CRITICAL();           // wyłącza przerwania do configMAX_SYSCALL_INTERRUPT_PRIORITY
shared_counter++;
taskEXIT_CRITICAL();

/* w ISR: */
UBaseType_t s = taskENTER_CRITICAL_FROM_ISR();
...
taskEXIT_CRITICAL_FROM_ISR(s);
```
Zasady: **kilka instrukcji, bez wywołań API, bez pętli** – inaczej
ucierpi opóźnienie przerwań i jitter całego systemu. Dla dłuższych
operacji użyj mutexa. Na ESP32 (dwa rdzenie) trzeba podać spinlock:
`portENTER_CRITICAL(&mux)`.

## 2. Narzędzia diagnostyczne

### Hooki kernela

```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask; (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) { /* tu: zapal diodę błędu / zapisz do backup RAM / break w debuggerze */ }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
```
Ustaw tu **breakpoint** w debuggerze – natychmiast widzisz, które
zadanie zawiodło (z call stackiem).

### Pamięć

```c
xPortGetFreeHeapSize();                   // wolna sterta teraz
xPortGetMinimumEverFreeHeapSize();        // najgorszy zapas od startu (kluczowe przy doborze rozmiaru sterty)
uxTaskGetStackHighWaterMark(NULL);        // minimalny zapas stosu zadania (w słowach)
```

### Podgląd zadań i obciążenia

```c
char buf[512];
vTaskList(buf);               // nazwa, stan (R/B/S/D), priorytet, wolny stos, numer
vTaskGetRunTimeStats(buf);    // procent czasu CPU na zadanie
```
Wymagają `configUSE_TRACE_FACILITY`, `configUSE_STATS_FORMATTING_FUNCTIONS`,
a statystyki czasu – także `configGENERATE_RUN_TIME_STATS` i
własnego, szybkiego licznika czasu (zwykle timer o częstotliwości
10-100x większej niż tick). Używaj do diagnostyki, nie w produkcji.

### Debugger i trace

- **GDB + OpenOCD** z obsługą FreeRTOS (`-rtos FreeRTOS`) lub
  rozszerzenie IDE (STM32CubeIDE "FreeRTOS Task List") – widzisz
  wszystkie zadania i ich stosy, nie tylko bieżący kontekst.
- **SEGGER SystemView** / **Percepio Tracealyzer** – graficzny zapis
  przełączeń kontekstu, ISR, blokad, użycia CPU; najlepsze do
  wychwycenia inwersji priorytetów i jitteru. Bezcenne, gdy "czasami
  coś się opóźnia".
- **Analizator logiczny / GPIO** – przełącz pin na wejściu/wyjściu
  zadania lub ISR; najprostszy sposób pomiaru opóźnień i jitteru.
- **`configASSERT`** – włączony w buildzie developerskim łapie ogromną
  część błędów użycia API zanim zamienią się w tajemnicze crashe.

### Metoda diagnozy "system stoi"

1. Zatrzymaj w debuggerze, zobacz listę zadań i ich stany.
2. Zadania w stanie **Blocked** na mutexie/semaforze/kolejce –
   wskazują, na co czekają (kolejka wskazuje właściciela przy
   `configQUEUE_REGISTRY_SIZE`, mutex ma holdera).
3. Szukaj cykli (A czeka na B, B czeka na A) → deadlock. Szukaj zadania
   o wysokim priorytecie w stanie Ready, które nigdy się nie blokuje →
   zagłodzenie.

---

## 3. Przykładowe pytania rekrutacyjne – FreeRTOS

**P: Czym różni się mutex od semafora?**
- Mutex chroni zasób przed równoczesnym dostępem; ten sam task bierze i
  oddaje; ma priority inheritance; nie używa się w ISR. Semafor
  sygnalizuje zdarzenie lub liczy zasoby; bierze i oddaje może różny
  byt (ISR → zadanie); nie ma priority inheritance. (Szczegóły: Chapter 5.)

**P: Co to jest priority inversion i jak ją rozwiązuje FreeRTOS?**
- Zadanie wysokopriorytetowe czeka na zasób trzymany przez
  niskopriorytetowe, które jest wywłaszczane przez średniopriorytetowe,
  więc wysokie czeka na średnie. FreeRTOS: mutexy z priority
  inheritance – holder tymczasowo dziedziczy priorytet najwyższego
  czekającego. Przykład historyczny: Mars Pathfinder.

**P: Czy można użyć mutexa w ISR?**
- Nie. Brak wersji `FromISR`; priority inheritance nie ma sensu dla
  ISR; ISR nie może się blokować. W ISR używaj semafora, kolejki lub
  task notification.

**P: Jak bezpiecznie przekazać dane z ISR do zadania?**
- `xQueueSendFromISR` (lub notification/semafor jako sygnał), z
  `xHigherPriorityTaskWoken` i `portYIELD_FROM_ISR`. ISR krótki,
  przetwarzanie w zadaniu (deferred interrupt processing).

**P: Czym różni się `vTaskDelay` od `vTaskDelayUntil`?**
- `vTaskDelay` czeka N ticków od momentu wywołania (okres pływa o czas
  wykonania ciała pętli); `vTaskDelayUntil` odmierza od poprzedniego
  wybudzenia, dając stały okres.

**P: Jak dobierasz rozmiar stosu zadania?**
- Zaczynam od hojnego, mierzę `uxTaskGetStackHighWaterMark` w skrajnych
  scenariuszach, zostawiam 20-30% zapasu, włączam
  `configCHECK_FOR_STACK_OVERFLOW = 2`. Uwaga na `printf` i duże
  bufory lokalne; jednostka to słowa (w ESP-IDF bajty).

**P: Dlaczego niektóre przerwania nie mogą wołać API FreeRTOS?**
- Cortex-M: ISR o priorytecie NVIC logicznie wyższym niż
  `configMAX_SYSCALL_INTERRUPT_PRIORITY` nie są maskowane przez kernel,
  więc mogłyby przerwać krytyczne sekcje kernela i uszkodzić jego
  struktury. Dzięki temu można mieć bardzo szybkie ISR niezależne od RTOS.

**P: Task notification vs semafor binarny – kiedy co?**
- Notification: szybsze i oszczędniejsze (brak obiektu kernela), ale
  tylko do jednego, znanego odbiorcy. Semafor, gdy odbiorcą może być
  dowolne zadanie albo potrzebujesz niezależnego obiektu.

**P: Jak uniknąć deadlocka?**
- Stała kolejność brania mutexów, minimalna liczba jednocześnie
  trzymanych, timeouty zamiast `portMAX_DELAY`, gatekeeper task zamiast
  wielu zamków.

**P: Co się stanie, gdy dwa zadania o równym priorytecie nigdy się nie blokują?**
- Przy `configUSE_TIME_SLICING = 1` dzielą CPU round-robin co tick; nic
  o niższym priorytecie (w tym Idle) nie dostaje czasu → zagłodzenie,
  brak sprzątania usuniętych zadań, brak trybu niskiego poboru energii.

---

## 4. Projekt końcowy (utrwalenie całego kursu)

**Specyfikacja:** mini-rejestrator danych na STM32/ESP32.

- `vSensorTask` (priorytet 3): próbuje czujnik I2C co 100 ms
  (`vTaskDelayUntil`), wysyła próbki do kolejki.
- `vProcessTask` (priorytet 2): odbiera próbki, liczy średnią
  kroczącą, zapisuje wynik do współdzielonej struktury chronionej
  **mutexem**.
- `vLogTask` (priorytet 1, **gatekeeper**): jedyny, który pisze do UART,
  odbiera komunikaty z **kolejki** logów.
- Przycisk w **ISR** (EXTI) → **task notification** do `vUiTask`
  (priorytet 2), który odczytuje strukturę (pod mutexem) i loguje
  aktualny wynik.
- Timer programowy 1 s wypisuje statystyki (`uxTaskGetStackHighWaterMark`,
  `xPortGetFreeHeapSize`) przez `log_print`.
- Włączone `configASSERT`, hooki przepełnienia stosu i błędu `malloc`.

Kryteria zaliczenia: brak gubienia danych przy 10 przyciskach/s,
stabilny okres próbkowania (zmierz GPIO), brak ostrzeżeń `configASSERT`,
`xPortGetMinimumEverFreeHeapSize()` ustabilizowane po 10 min działania.

---

## ✅ Zadania

1. Celowo przepełnij stos jednego zadania i wykryj to hookiem.
2. Zrób zadanie wysokopriorytetowe z pustą pętlą i zaobserwuj, co
   dzieje się z resztą systemu (i z watchdogiem, jeśli jest włączony).
3. Zbuduj projekt końcowy z sekcji 4.
4. Odpowiedz na głos (nagraj się) na 5 pytań rekrutacyjnych z sekcji 3,
   przy każdym podając przykład z własnego kodu.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie kursu

Po ukończeniu rozdziałów powinieneś:
- [ ] Rozumieć scheduler wywłaszczający i cykl życia zadania
- [ ] Skonfigurować FreeRTOS (`FreeRTOSConfig.h`, heap, priorytety NVIC)
- [ ] Tworzyć zadania, dobierać priorytety i stosy, używać `vTaskDelayUntil`
- [ ] Przekazywać dane kolejkami i bezpiecznie komunikować się z ISR
- [ ] **Jasno wyjaśnić różnicę mutex vs semafor** i znać priority inversion
- [ ] Używać task notifications, event groups i software timerów
- [ ] Diagnozować przepełnienia stosu, deadlocki i zagłodzenie

**Gratulacje – to koniec kursu FreeRTOS.** Naturalny następny krok:
przeniesienie wzorców (kolejki, gatekeeper, deferred processing) do
własnego projektu na ESP32 lub STM32 oraz porównanie z modelem wątków
POSIX w Linuksie (`pthread_mutex`, `sem_t`, `condition_variable`).
