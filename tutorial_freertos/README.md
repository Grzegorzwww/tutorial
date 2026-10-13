# FreeRTOS dla Embedded – Tutorial

Kurs praktyczny: od zrozumienia, czym jest RTOS, przez konfigurację
(`FreeRTOSConfig.h`), tworzenie zadań, komunikację między nimi, aż po
**różnicę mutex vs semafor** z praktycznymi przykładami i typowe błędy.
Zakładam, że znasz już programowanie mikrokontrolerów (STM32/ESP32,
przerwania, bare-metal "superloop") – kurs skupia się na tym, co
dochodzi, gdy między Twoim kodem a sprzętem pojawia się scheduler.

## Środowisko ćwiczeń (wybierz jedno)

| Opcja | Zalety | Uwagi |
|---|---|---|
| **ESP32 + ESP-IDF** (masz już zainstalowane) | FreeRTOS wbudowany, zero konfiguracji kernela | Różnice w API opisane w Chapter 2 (stos w bajtach, dwa rdzenie, `app_main`) |
| **STM32 (CubeMX / CubeIDE)** | Klasyczny Cortex-M, dokładnie ten scenariusz z pracy | CubeMX domyślnie generuje wrapper CMSIS-RTOS v2 – w kursie używamy natywnego API FreeRTOS |
| **Symulator POSIX na Ubuntu** | Bez sprzętu, można debugować w `gdb` | Port `GCC/Posix` z repozytorium FreeRTOS-Kernel; ISR trzeba symulować sygnałami |

Przykłady w kursie używają **natywnego API FreeRTOS** (nagłówki
`FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`). Pod ESP-IDF zmieniają się
tylko ścieżki include (`freertos/FreeRTOS.h`) i kilka detali opisanych
w Chapter 2.

## Spis treści

1. [Podstawy RTOS i FreeRTOS](chapter1_rtos_basics.md) – po co RTOS,
   scheduler, stany zadań, tick, przełączanie kontekstu.
2. [Konfiguracja](chapter2_configuration.md) – `FreeRTOSConfig.h`,
   schematy sterty (`heap_1..5`), priorytety przerwań na Cortex-M,
   różnice ESP-IDF.
3. [Zadania i scheduling](chapter3_tasks_scheduling.md) – `xTaskCreate`,
   opóźnienia, priorytety, stos, zadania w C++.
4. [Kolejki i komunikacja z ISR](chapter4_queues_and_isr.md) – kolejki,
   `...FromISR`, deferred interrupt processing, gatekeeper task.
5. [Semafor vs Mutex](chapter5_semaphore_vs_mutex.md) – **najważniejszy
   rozdział**: sygnalizacja vs wzajemne wykluczanie, priority
   inversion, priority inheritance, praktyczne przykłady.
6. [Task notifications, Event Groups, Software Timers](chapter6_notifications_events_timers.md) –
   lżejsze i bardziej wyspecjalizowane mechanizmy.
7. [Typowe błędy, debugowanie i pytania rekrutacyjne](chapter7_pitfalls_debugging.md)

## Jak korzystać

- Każdy rozdział kończy się zadaniami (✅) – rób je na prawdziwym
  sprzęcie lub symulatorze, RTOS-a nie da się zrozumieć samym czytaniem
  (zwłaszcza efektów typu priority inversion).
- Przy każdym przykładzie zwracaj uwagę na **kontekst wywołania**
  (task vs ISR) – to źródło większości błędów początkujących.
- Rozdział 5 jest wprost pod pytanie rekrutacyjne "czym różni się mutex
  od semafora" – warto go przerobić najstaranniej.
