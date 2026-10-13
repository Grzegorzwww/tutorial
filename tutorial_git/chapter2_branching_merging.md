# Chapter 2: Branże (branching) i scalanie (merge/rebase)

[← Spis treści](README.md)

---

## 1. Czym jest branch (gałąź)

Branch to po prostu **wskaźnik na konkretny commit**, który przesuwa
się do przodu wraz z nowymi commitami. Domyślny branch to zwykle
`main` (dawniej `master`). Branche pozwalają pracować nad nową
funkcjonalnością/poprawką **równolegle**, bez wpływu na stabilny kod.

```bash
git branch                     # lista lokalnych branchy, gwiazdka = na którym aktualnie jesteś
git branch nazwa-brancha       # utwórz nowy branch (ale NIE przełącz się na niego)
git checkout nazwa-brancha     # przełącz się na istniejący branch
git checkout -b nazwa-brancha  # utwórz branch I od razu się na niego przełącz (skrót)
git switch nazwa-brancha       # nowszy, czytelniejszy odpowiednik 'checkout' tylko do branchy
git switch -c nazwa-brancha    # nowszy odpowiednik 'checkout -b'
```

> 💡 `git switch` i `git restore` (rozdział 4) to nowsze polecenia
> (Git 2.23+) wprowadzone, żeby rozdzielić przeciążone dotychczas
> `git checkout` (które robiło zarówno przełączanie branchy, jak i
> przywracanie plików) na dwa czytelniejsze polecenia. Stare
> `checkout` nadal działa i jest szeroko spotykane w istniejących
> projektach/dokumentacji.

```bash
git branch -d nazwa-brancha    # usuń branch (bezpiecznie - tylko jeśli już zmergowany)
git branch -D nazwa-brancha    # usuń branch NA SIŁĘ (nawet jeśli niezmergowany - uważaj!)
```

---

## 2. Typowy workflow: feature branch

```bash
git switch -c feature/uart-driver    # nowy branch pod konkretną funkcjonalność
# ... praca, kilka commitów ...
git switch main
git merge feature/uart-driver         # scal zmiany z powrotem do main
```

Konwencja nazewnictwa branchy różni się między zespołami, ale typowe
prefiksy to: `feature/...`, `fix/...` lub `bugfix/...`, `hotfix/...`,
czasem z numerem zadania z trackera: `feature/JIRA-123-uart-driver`.

---

## 3. `git merge` – scalanie z zachowaniem pełnej historii

```bash
git switch main
git merge feature/uart-driver
```

Jeśli `main` nie miał żadnych nowych commitów od momentu utworzenia
brancha, git zrobi tzw. **fast-forward** – po prostu przesunie
wskaźnik `main` do przodu, bez tworzenia dodatkowego commita.

Jeśli oba branche miały niezależne commity, git utworzy **merge
commit** – nowy commit z dwoma rodzicami, łączący obie historie.

```
main:     A---B-------M
                     /
feature:       C---D
```

**Konflikty scalania:** jeśli te same linie w tym samym pliku zostały
zmienione na obu branchach inaczej, git zatrzyma się i poprosi o
ręczne rozwiązanie:
```bash
git merge feature/uart-driver
# CONFLICT (content): Merge conflict in uart.cpp
```
Otwórz plik – git wstawia znaczniki:
```cpp
<<<<<<< HEAD
uint32_t baudrate = 9600;
=======
uint32_t baudrate = 115200;
>>>>>>> feature/uart-driver
```
Ręcznie wybierz/połącz poprawną wersję, usuń znaczniki `<<<<<<<`,
`=======`, `>>>>>>>`, potem:
```bash
git add uart.cpp
git commit          # dokończ merge commit
```

---

## 4. `git rebase` – liniowa historia (alternatywa dla merge)

```bash
git switch feature/uart-driver
git rebase main
```
Zamiast tworzyć merge commit, `rebase` **przekłada** commity z
`feature/uart-driver` tak, jakby zostały napisane na aktualnym
wierzchołku `main` – wynik jest liniowy, bez rozgałęzień:
```
Przed rebase:
main:     A---B---E
                 \
feature:      C---D

Po rebase:
main:     A---B---E
                    \
feature:             C'---D'   (nowe hashe! C i D "przepisane" na C', D')
```

> ⚠️ **Złota zasada rebase:** **nigdy nie rebase'uj commitów, które już
> zostały wypchnięte (`push`) i są używane przez inne osoby.** Rebase
> zmienia hashe commitów – jeśli ktoś inny już bazuje na starych
> hashach, powstanie bałagan przy próbie synchronizacji. Rebase jest
> bezpieczny na **swoich, lokalnych, jeszcze niewypchniętych** branchach.

**Interactive rebase** – porządkowanie własnej historii przed wypchnięciem:
```bash
git rebase -i HEAD~3    # ostatnie 3 commity
```
Otwiera edytor z listą commitów, gdzie możesz: `pick` (zostaw),
`reword` (zmień opis), `squash`/`fixup` (połącz z poprzednim
commitem), `drop` (usuń commit). Bardzo przydatne, żeby przed
wysłaniem pull requesta "posprzątać" historię (np. połączyć 5 małych
commitów "fix typo", "wip", "działa" w jeden czytelny commit).

---

## 5. Merge vs rebase – kiedy które?

| | `merge` | `rebase` |
|---|---|---|
| Historia | Zachowuje pełną, prawdziwą historię (z rozgałęzieniami) | Historia liniowa, "czystsza" wizualnie |
| Bezpieczeństwo na współdzielonych branchach | Bezpieczne zawsze | Niebezpieczne na już wypchniętych branchach |
| Typowe użycie | Scalanie feature brancha do `main`/`develop` | Aktualizacja własnego brancha o najnowsze zmiany z `main` przed PR |

Wiele zespołów (i wiele narzędzi jak GitHub/GitLab PR) używa obu:
`rebase` na swoim branchu żeby być na bieżąco z `main`, a `merge`
(często przez "squash merge" w interfejsie PR) przy finalnym scaleniu
do głównej gałęzi.

---

## ✅ Zadania

1. Utwórz branch `feature/test`, zrób 2 commity, wróć do `main` i
   zmerguj branch – sprawdź w `git log --graph` czy był fast-forward
   czy powstał merge commit.
2. Celowo wywołaj konflikt: zmodyfikuj tę samą linię pliku na `main` i
   na branchu, spróbuj zmergować, rozwiąż konflikt ręcznie.
3. Utwórz nowy branch z 3 "brzydkimi" commitami (np. "wip", "fix",
   "działa teraz") i użyj `git rebase -i HEAD~3` żeby połączyć je w
   jeden, czytelny commit.
4. Wyjaśnij własnymi słowami, dlaczego rebase na już wypchniętym,
   współdzielonym branchu jest ryzykowne.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 3 – Praca z repozytorium zdalnym](chapter3_remote_collaboration.md)
