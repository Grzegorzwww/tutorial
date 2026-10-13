# Git – Tutorial

Krótszy kurs (5 rozdziałów) skupiony na praktycznym, codziennym
użyciu gita w pracy zespołowej – od podstaw po rzeczy, które realnie
przydają się w projektach embedded/firmware (praca na branchach,
cofanie błędów, współpraca przez pull requesty).

Środowisko: dowolny terminal z zainstalowanym `git` (Ubuntu ma go
zwykle domyślnie – sprawdź `git --version`).

## Spis treści

1. [Podstawy – repozytorium, commit, historia](chapter1_basics.md)
2. [Branże (branching) i scalanie (merge/rebase)](chapter2_branching_merging.md)
3. [Praca z repozytorium zdalnym (remote, push/pull, PR)](chapter3_remote_collaboration.md)
4. [Cofanie zmian – reset, revert, stash, reflog](chapter4_undoing_changes.md)
5. [Przydatne narzędzia dodatkowe – tagi, bisect, blame, .gitignore](chapter5_useful_extras.md)

## Jak korzystać

- Rób ćwiczenia w **testowym** repozytorium (np. `mkdir git_test && cd git_test && git init`),
  żeby bezpiecznie eksperymentować z poleceniami, które modyfikują historię.
- Każdy rozdział ma sekcję zadań (✅) – wykonaj je w praktyce, nie tylko przeczytaj.
