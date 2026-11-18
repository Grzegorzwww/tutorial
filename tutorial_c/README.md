# Czysty C dla programisty Embedded – Tutorial

Kurs zakłada, że znasz składnię C (pętle, funkcje, `if`, podstawowe
wskaźniki) i skupia się na tym, co **naprawdę** decyduje o jakości kodu
w C: modelu kompilacji, typach i ich pułapkach, pamięci, niezdefiniowanym
zachowaniu i wzorcach typowych dla systemów wbudowanych.

**Standard:** C11 (z uwagami, co zmienia C17/C23).

**Środowisko ćwiczeń:** GCC (Linux) albo MinGW-w64 (Windows). Kompilacja:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic plik.c -o program
# z wykrywaniem błędów pamięci i UB (Linux, GCC/Clang):
gcc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined plik.c -o program
```
> Sanitizery (`-fsanitize=...`) są najlepszym nauczycielem C – pokazują
> błędy, które "normalnie" przechodzą bez śladu. Na MinGW nie są
> dostępne; używaj ich na Linuksie/WSL.

Każdy rozdział to osobny plik z: opisem koncepcyjnym, kodem
przykładowym (kompilowalnym), sekcją "⚠️ Pułapki" i zadaniami.

## Spis treści

0. [Model kompilacji](chapter0_compilation_model.md) – preprocesor,
   kompilacja, linkowanie, nagłówki, `static` i `extern`, czas życia zmiennych.
1. [Typy całkowite i ich pułapki](chapter1_integer_types.md) –
   `<stdint.h>`, promocje, `signed`/`unsigned`, przepełnienia.
1a. [Reprezentacja liczb i konwersje w bitach](chapter1a_integer_representation.md) –
   kod uzupełnień do dwóch, zawężanie (zostają młodsze bity), rozszerzanie
   znakiem, reguły konwersji, przesunięcia.
2. [Wskaźniki i tablice](chapter2_pointers_arrays.md) – rozpad tablicy,
   `const` przy wskaźnikach, tablice wielowymiarowe, parametry wyjściowe.
3. [Pamięć](chapter3_memory.md) – stos, sterta, pamięć statyczna,
   `malloc`/`realloc`/`free`, pule pamięci, typowe błędy.
4. [Napisy](chapter4_strings.md) – `'\0'`, bezpieczne kopiowanie,
   `snprintf`, parsowanie liczb przez `strtol`.
5. [`struct`, `union`, `enum`](chapter5_structs_unions_enums.md) –
   padding, alignment, serializacja, unie z tagiem, flagi bitowe.
6. [Wskaźniki na funkcje i callbacki](chapter6_function_pointers.md) –
   `void *ctx`, tablice dispatch, `qsort`/`bsearch`.
7. [Preprocesor](chapter7_preprocessor.md) – makra i ich pułapki,
   `do { } while (0)`, X-macros, `_Static_assert`.
8. [Niezdefiniowane zachowanie, `volatile`, `restrict`](chapter8_ub_volatile.md) –
   czego kompilator "nie musi" zrobić i jak pisać przenośnie.
9. [Wzorce embedded](chapter9_embedded_patterns.md) – rejestry przez
   struktury, opaque pointer, automaty stanów.
10. [Obsługa błędów](chapter10_error_handling.md) – kody powrotu,
    `errno`, wzorzec `goto cleanup`.

## Ćwiczenia

Zadania praktyczne do kursu są w [../exercises](../exercises) (pliki
`ex41`–`ex60`, rozszerzenie `.c`). Każde ma szkielet z `TODO` i testy
`assert` w `main`; gotowe rozwiązania są w `solutions/`.

```powershell
cd tutorial/exercises
make run N=41                 # zadanie
make run N=41 DIR=solutions   # rozwiązanie wzorcowe
```

Starsze zadania w C (`ex21`–`ex25`: wskaźniki, napisy, bity, wskaźniki
na funkcje, bufor cykliczny) też pasują do tego kursu.

| Rozdział | Zadania |
|---|---|
| 0 | ex41 |
| 1 | ex42 |
| 1a | ex42, ex56 |
| 2 | ex43 |
| 3 | ex44, ex45, ex46 |
| 4 | ex47, ex48 |
| 5 | ex49, ex50, ex51 |
| 6 | ex52, ex53 |
| 7 | ex54, ex55 |
| 8 | ex56 |
| 9 | ex57, ex58, ex59 |
| 10 | ex60 |

## Jak korzystać

- Kompiluj i uruchamiaj **każdy** przykład sam, najlepiej z sanitizerami.
- Sekcja "⚠️ Pułapki" w każdym rozdziale to najważniejszy fragment –
  tam są błędy, które realnie trafiają do produkcji.
- Zadania (✅) rozwiązuj pisząc kod, nie tylko w głowie.
