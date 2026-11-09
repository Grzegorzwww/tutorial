# Embedded Linux Engineer – Tutorial

Spis treści kursu przygotowanego pod przejście z Embedded C/C++ na
Embedded Linux Engineer. Środowisko ćwiczeń:
- **Ubuntu** (stacja robocza, x86)
- **BeagleBone Black** (Debian Bullseye, ARM, `ssh debian@192.168.7.2`)

Każdy rozdział to osobny plik. Pracuj po kolei, rozwiązuj zadania i
zapisuj notatki bezpośrednio w plikach rozdziałów.

## Część 1: Linux jako system

1. [Filesystem, /proc i /sys](chapter1_filesystem_proc_sys.md) –
   struktura systemu plików, wirtualne filesystemy `/proc` i `/sys`.
2. [Czym jest jądro Linux i jak współgra z otoczeniem](chapter2_kernel_basics.md) –
   kernel space vs user space, syscalle, moduły, proces bootowania.
3. [Procesy i sygnały](chapter3_processes_signals.md) – cykl życia
   procesu, stany, sygnały, kontrola zadań w shellu.
4. [Użytkownicy, uprawnienia i udev](chapter4_permissions_udev.md) –
   model uprawnień, pliki urządzeń, dynamiczne zarządzanie sprzętem.
5. [Kompilacja jądra](chapter5_kernel_compilation.md) – struktura
   źródeł, konfiguracja, budowanie, wprowadzenie do cross-kompilacji.

## Część 2 (następna): Systemy budowania obrazów

Buildroot, Yocto Project i cross-kompilacja pod ARM (BeagleBone) –
zostanie dodane po ukończeniu Części 1.

## Jak korzystać z tego kursu

- Rób jeden rozdział na raz, w tempie ok. 1-2h dziennie.
- Każdy rozdział kończy się zadaniami (✅) i miejscem na notatki –
  wypełniaj je swoimi wynikami.
- Gdy skończysz rozdział, wróć do rozmowy z podsumowaniem odpowiedzi –
  sprawdzimy je razem i przejdziemy dalej.
