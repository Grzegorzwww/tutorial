# Przygotowanie do rekrutacji – Embedded C++ / Embedded Linux Engineer

Zbiór potencjalnych pytań rekrutacyjnych i zadań praktycznych, dobrany
pod Twoje CV (C/C++, STM32/AVR/ESP32, Qt, Linux, MQTT/MODBUS, git,
CMake, Jenkins) oraz kierunek, w który zmierzasz (Embedded Linux
Engineer). Każde pytanie ma zwięzłą odpowiedź/kluczowe punkty – nie po
to, żeby się ich nauczyć na pamięć, ale żeby sprawdzić czy potrafisz to
wyjaśnić **swoimi słowami** i na własnym przykładzie z pracy.

## Spis treści

1. [Fundamenty C/C++](chapter1_c_cpp_fundamentals.md) – wskaźniki,
   pamięć, `const`/`volatile`, RAII, wirtualność, UB, szablony.
2. [Embedded-specific](chapter2_embedded_specific.md) – przerwania,
   dostęp do rejestrów, watchdog, bootloader, linker script, RTOS,
   stack/heap na mikrokontrolerze.
3. [Linux i systemy operacyjne](chapter3_linux_os.md) – procesy vs
   wątki, IPC, sygnały, syscalle, boot, kernel vs user space,
   sterowniki.
4. [Protokoły komunikacyjne i narzędzia](chapter4_protocols_and_tools.md) –
   UART/SPI/I2C, MODBUS, MQTT, USB, git, CMake, CI/CD, debugowanie
   (gdb, JTAG, logic analyzer).
5. [Zadania praktyczne](chapter5_practical_tasks.md) – konkretne
   zadania kodowe, jakie może dać rekruter (z rozwiązaniami/hintami),
   od live-codingu po code review.
6. [Pytania sytuacyjne/behawioralne i jak mówić o CV](chapter6_behavioral_and_cv.md) –
   jak opowiedzieć o projektach z Somfy/Hymon/ZMT, pytania o pracę
   zespołową, oczekiwania, STAR method.

## Jak z tego korzystać

- Przeczytaj pytania **bez** patrzenia na odpowiedź, spróbuj
  odpowiedzieć na głos (albo napisać) – dopiero potem sprawdź.
- Przy pytaniach embedded/Linux staraj się wiązać odpowiedź z
  konkretnym przykładem z Twojej pracy (Somfy, Hymon, ZMT) – rekruterzy
  to bardzo lubią, pokazuje praktyczne doświadczenie, nie tylko teorię.
- Rozdział 5 (zadania praktyczne) rób **z klawiaturą w ręku**, nie w
  głowie – live coding testuje też płynność pisania, nie tylko wiedzę.
