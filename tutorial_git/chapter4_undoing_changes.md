# Chapter 4: Cofanie zmian – reset, revert, stash, reflog

[← Spis treści](README.md)

To rozdział "ratunkowy" – najczęściej wracasz do niego, gdy coś
poszło nie tak. Dobra wiadomość: **git prawie nigdy nic naprawdę nie
usuwa od razu** – nawet pozornie "utracone" commity zwykle da się
odzyskać (patrz sekcja `reflog`).

---

## 1. Cofanie niezacommitowanych zmian

```bash
git restore plik.cpp
```
Przywraca plik do stanu z ostatniego commita, **odrzucając** lokalne,
niezacommitowane zmiany w tym pliku. (Starszy odpowiednik:
`git checkout -- plik.cpp`.)

```bash
git restore --staged plik.cpp
```
Usuwa plik ze staging area (cofa `git add`), ale **zachowuje** zmiany
w samym pliku na dysku – przydatne gdy przez pomyłkę dodałeś coś do
staging area i chcesz to jeszcze przemyśleć/rozdzielić na osobne
commity.

```bash
git clean -n     # pokaż co ZOSTAŁOBY usunięte (dry-run, bezpieczne)
git clean -fd    # faktycznie usuń nieśledzone pliki i katalogi (NIEODWRACALNE!)
```
`git clean` usuwa pliki, które git w ogóle nie śledzi (nie są nawet w
`.gitignore` – po prostu nowe, niedodane pliki). W przeciwieństwie do
`restore`, to jest operacja **nieodwracalna** (pliki nie trafiły nigdy
do historii, więc nie ma skąd ich odzyskać).

---

## 2. `git reset` – przesuwanie wskaźnika HEAD/brancha

```bash
git reset --soft HEAD~1
```
Cofa ostatni commit, ale **zachowuje** wszystkie zmiany w staging area
– jakbyś jeszcze nie zrobił `git commit`, ale `git add` już tak.
Przydatne do "połączenia" ostatniego commita z nowymi zmianami.

```bash
git reset --mixed HEAD~1     # (--mixed to domyślna opcja, można pominąć: git reset HEAD~1)
```
Cofa ostatni commit **i** wyjmuje zmiany ze staging area – zmiany
nadal są na dysku w plikach, ale trzeba je od nowa dodać (`git add`).

```bash
git reset --hard HEAD~1
```
Cofa ostatni commit **i całkowicie odrzuca** wszystkie związane z nim
zmiany – dysk wraca dokładnie do stanu sprzed tego commita.
**Nieodwracalne w sensie: tracisz niezacommitowane zmiany** (choć sam
commit często da się odzyskać przez `reflog`, patrz niżej).

> ⚠️ **`--hard` to najbardziej niebezpieczna opcja `reset`** – zawsze
> upewnij się dwa razy, że nie masz ważnych, niezacommitowanych zmian
> na dysku przed jego użyciem.

**Porównanie trzech trybów `reset` (po cofnięciu 1 commita):**

| | Commit cofnięty? | Staging area | Pliki na dysku |
|---|---|---|---|
| `--soft` | Tak | Zachowane (jakby po `git add`) | Zachowane |
| `--mixed` (domyślny) | Tak | Wyczyszczone | Zachowane |
| `--hard` | Tak | Wyczyszczone | **Odrzucone** |

---

## 3. `git revert` – bezpieczne cofanie na współdzielonej historii

```bash
git revert <hash>
```
W przeciwieństwie do `reset`, `revert` **nie usuwa** historii – tworzy
**nowy** commit, który odwraca zmiany z podanego commita. Bezpieczne
do użycia nawet na już wypchniętych, współdzielonych branchach
(`main`), bo nie zmienia istniejących hashy commitów.

```bash
git revert HEAD           # odwróć ostatni commit
git revert HEAD~2         # odwróć konkretny, starszy commit (nie musi być ostatni)
git revert --no-commit <hash1> <hash2>   # odwróć kilka commitów naraz, jeden wspólny commit na końcu
```

> 💡 **Zasada praktyczna:** na własnym, lokalnym, niewypchniętym
> branchu możesz swobodnie używać `reset` (szybciej "sprząta"
> historię). Na współdzielonej, już wypchniętej gałęzi (`main`,
> `develop`) zawsze używaj `revert` – nigdy `reset --hard` + `push --force`
> na branchu, z którego korzystają inni.

---

## 4. `git stash` – tymczasowe odłożenie zmian na bok

Sytuacja: pracujesz nad czymś, zmiany nie są gotowe do commita, ale
musisz pilnie przełączyć się na inny branch (np. naprawić krytyczny
bug). `git stash` "chowa" bieżące, niezacommitowane zmiany, czyszcząc
working directory, żeby móc się bezpiecznie przełączyć.

```bash
git stash                    # schowaj bieżące zmiany
git stash list                # lista schowanych zestawów zmian
git stash pop                 # przywróć NAJNOWSZY schowany zestaw i usuń go ze stosu
git stash apply                # przywróć najnowszy zestaw, ale ZACHOWAJ go na stosie (można zaaplikować gdzie indziej)
git stash drop                 # usuń zestaw ze stosu bez aplikowania
git stash push -m "opis"       # schowaj z czytelnym opisem (przydatne gdy stashy jest kilka)
```

```bash
# typowy scenariusz:
git stash                       # odłóż bieżącą pracę
git switch main
git switch -c hotfix/urgent-bug
# ... napraw bug, commit, push ...
git switch feature/original-work
git stash pop                    # wróć do przerwanej pracy
```

---

## 5. `git reflog` – "siatka bezpieczeństwa" gita

```bash
git reflog
```
Pokazuje **pełną historię** wszystkiego, co robiłeś z `HEAD` (każdy
commit, checkout, reset, rebase, merge) – nawet commity, które
"zniknęły" po `reset --hard` albo zostały "przepisane" przez `rebase`,
zwykle nadal tu widnieją (dopóki nie zadziała garbage collector gita,
domyślnie po ok. 30-90 dniach).

```bash
git reflog
# a3f5c9d (HEAD -> main) HEAD@{0}: reset: moving to HEAD~1
# f8e2d1a HEAD@{1}: commit: Dodaj obsługę I2C
# ...

git reset --hard f8e2d1a    # "odzyskaj" przypadkowo cofnięty commit
```

> 💡 **Zapamiętaj to jedno polecenie na wszelki wypadek:** jeśli
> przypadkowo zrobiłeś `reset --hard` za daleko, albo rebase poszedł
> źle – **zanim zaczniesz panikować, sprawdź `git reflog`**. Prawie
> zawsze da się wrócić do poprzedniego stanu.

---

## ✅ Zadania

1. Zrób commit, potem `git reset --soft HEAD~1` – sprawdź `git status`,
   czy zmiany są nadal w staging area.
2. Zrób commit z celowym błędem, wypchnij (do lokalnego/testowego
   remote'a albo po prostu symulacyjnie), potem cofnij go przez
   `git revert` zamiast `reset`.
3. Zacznij zmianę w pliku, zrób `git stash`, przełącz się na inny
   branch, wróć, zrób `git stash pop`.
4. Zrób `git reset --hard HEAD~2` (na testowym repo!), a potem odzyskaj
   "utracone" commity przez `git reflog` + `git reset --hard <hash>`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 5 – Przydatne narzędzia dodatkowe](chapter5_useful_extras.md)
