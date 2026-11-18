# Zephyr RTOS dla NUCLEO-H743ZI – Tutorial

Kurs praktyczny: od zbudowania pierwszego projektu (miganie diodą), przez
trzy filary Zephyra (**CMake + Kconfig + Devicetree**), wątki,
synchronizację i komunikację, aż po logowanie, shell, sterowniki płytki,
debugowanie i C++. Zakładam, że znasz już programowanie mikrokontrolerów
STM32, język C oraz podstawy RTOS-a (np. z
[kursu FreeRTOS](../tutorial_freertos/README.md)) – kurs skupia się na tym,
co jest **inne w Zephyrze**: system budowania, opis sprzętu w drzewie
urządzeń i konfiguracja przez Kconfig zamiast `FreeRTOSConfig.h` i CubeMX.

## Środowisko ćwiczeń

Kurs powstał na realnym środowisku i **wszystkie przykłady były zbudowane**
(a te niezależne od sprzętu – także uruchomione w QEMU):

| Element | Wersja / wartość |
|---|---|
| Płytka | **NUCLEO-H743ZI** (`nucleo_h743zi`, STM32H743ZI, Cortex-M7 480 MHz) |
| Zephyr | 4.5.99 – gałąź `main` (build `v4.5.0-2813-gf8da8ea37050`) |
| Toolchain | Zephyr SDK 1.0.1 (GCC 14.3.0, libc: picolibc) |
| Narzędzia | `west` 1.5.0 w `.venv`, CMake, Ninja, `dtc` |
| System | Windows (PowerShell) |

> ⚠️ **Gałąź `main` to wersja rozwojowa.** API potrafi się zmienić między
> wydaniami. Przykład z życia: w tej wersji komenda shella to
> `kernel thread list`, a nie znane ze starszych poradników `kernel threads`.
> Gdy coś się nie zgadza, sprawdź dokumentację **dokładnie tej wersji**,
> którą masz w katalogu `zephyr/`.

### Dwa sposoby ćwiczeń

| Opcja | Zalety | Uwagi |
|---|---|---|
| **NUCLEO-H743ZI** | Prawdziwy sprzęt: diody, przycisk, PWM, czujnik temperatury | Rozdziały 7 (GPIO) i 9 wymagają płytki |
| **QEMU** (`qemu_cortex_m3`) | Bez sprzętu, ten sam kernel Zephyra, szybka pętla | Działa dla rozdziałów 3–8, 10, 11 (bez GPIO/PWM/ADC). Czasy w QEMU są orientacyjne |

Uruchomienie w QEMU (z katalogu workspace, patrz Chapter 0):

```powershell
west build -b qemu_cortex_m3 <aplikacja> -d build_qemu -p always
west build -d build_qemu -t run        # wyjście z QEMU: Ctrl+a, potem x
```

## Spis treści

0. [Środowisko i workflow](chapter0_setup_and_workflow.md) – `west`,
   build/flash/debug na NUCLEO-H743ZI, konsola, typowe pułapki środowiska.
1. [Architektura i system budowania](chapter1_architecture_and_build.md) –
   trzy filary, co powstaje w katalogu `build/`, blinky linia po linii.
2. [Devicetree](chapter2_devicetree.md) – opis sprzętu, aliasy, overlay,
   piny i zegary w DT zamiast CubeMX.
3. [Kconfig i `prj.conf`](chapter3_kconfig.md) – konfiguracja kernela i
   funkcji, własne symbole, typowe błędy.
4. [Wątki i scheduling](chapter4_threads.md) – priorytety (odwrotnie niż w
   FreeRTOS!), preempcja, time slicing, okresowe zadania.
5. [Synchronizacja](chapter5_synchronization.md) – mutex, semafor, condvar,
   spinlock, operacje atomowe, co wolno w ISR.
6. [Komunikacja między wątkami](chapter6_message_passing.md) – `k_msgq`,
   `k_fifo`, `k_event`, kiedy co wybrać.
7. [Timery, work queue i przerwania](chapter7_timers_workqueue_isr.md) –
   deferred processing, przycisk z debouncingiem.
8. [Logging i Shell](chapter8_logging_shell.md) – `LOG_*`, własne komendy,
   wbudowane komendy kernela.
9. [Sterowniki i peryferia płytki](chapter9_drivers_peripherals.md) –
   device model, PWM, czujnik temperatury, UART.
10. [Debugowanie i typowe błędy](chapter10_debugging_pitfalls.md) – stos,
    analizator wątków, fault, raporty pamięci, pytania rekrutacyjne.
11. [C++ w Zephyrze](chapter11_cpp.md) – C++20 na płytce, RAII dla
    muteksu, `std::span`/`std::optional`.

## Ściągawka: FreeRTOS → Zephyr

| FreeRTOS | Zephyr | Uwagi |
|---|---|---|
| `xTaskCreate` | `K_THREAD_DEFINE` / `k_thread_create` | Zephyr: stos i struktura wątku **statyczne**, stos w **bajtach** |
| `vTaskDelay(pdMS_TO_TICKS(n))` | `k_msleep(n)` | |
| `vTaskDelayUntil` | `k_sleep(K_TIMEOUT_ABS_MS(t))` | Okres bez dryfu |
| priorytet: **większy = ważniejszy** | priorytet: **mniejszy = ważniejszy** | Ujemne = kooperacyjne |
| `xQueueCreate` / `xQueueSend` | `K_MSGQ_DEFINE` / `k_msgq_put` | Kopiowanie elementów |
| kolejka wskaźników | `k_fifo` / `k_lifo` | Bez kopiowania, zarezerwowane pierwsze słowo |
| `xSemaphoreCreateBinary/Counting` | `K_SEM_DEFINE` / `k_sem_*` | |
| `xSemaphoreCreateMutex` | `K_MUTEX_DEFINE` / `k_mutex_*` | Dziedziczenie priorytetu |
| Event Groups | `k_event` (`CONFIG_EVENTS=y`) | |
| `xTimerCreate` | `k_timer` / `k_work_delayable` | Callback `k_timer` działa w ISR |
| `...FromISR` | ta sama funkcja | Zephyr nie ma osobnych wersji `FromISR` – liczy się, czy wołasz **nieblokująco** |
| `FreeRTOSConfig.h` | `prj.conf` (Kconfig) | Typowane symbole z zależnościami |
| CubeMX (piny, zegary) | Devicetree | Piny i zegary opisane w `.dts`/overlay |
| `configASSERT` | `CONFIG_ASSERT=y` | |
| `vTaskStartScheduler()` | – | Scheduler startuje sam, `main()` jest już wątkiem |

## Jak korzystać

- Każdy rozdział ma zadania (✅) – rób je na płytce albo w QEMU.
- Bloki „Przykładowy wynik” pochodzą z **prawdziwych uruchomień** (QEMU,
  Zephyr 4.5.99). Czasy w QEMU są orientacyjne, więc na płytce wartości będą
  nieco inne (np. tick na tej płytce to 10 000 Hz, a w QEMU 100 Hz).
- Sekcje „🔧 Różnica względem FreeRTOS” to najkrótsza droga, jeśli znasz już
  FreeRTOS.
- Rozdziały 1–3 (build, Devicetree, Kconfig) to to, co **najbardziej
  odróżnia Zephyra** od reszty świata – warto je przerobić najstaranniej.
