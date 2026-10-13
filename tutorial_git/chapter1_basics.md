# Chapter 1: Podstawy – repozytorium, commit, historia

[← Spis treści](README.md)

---

## 1. Czym jest repozytorium git

Repozytorium to katalog śledzony przez git – każda zmiana w plikach
może zostać zapisana jako **commit** (migawka stanu projektu w danym
momencie). Cała historia (wszystkie commity) jest przechowywana
lokalnie, w ukrytym katalogu `.git/`.

```bash
git init
```
Tworzy nowe, puste repozytorium w bieżącym katalogu (katalog `.git/`).

```bash
git clone https://github.com/user/repo.git
```
Kopiuje istniejące repozytorium (razem z całą historią) z serwera
zdalnego na dysk lokalny.

```bash
git status
```
Najczęściej używane polecenie – pokazuje: na jakim jesteś branchu,
jakie pliki są zmodyfikowane, jakie są w "staging area" (gotowe do
commitu), a jakie nieznane gitowi (untracked).

---

## 2. Staging area – "poczekalnia" przed commitem

Git ma pośredni etap między "zmieniłem plik na dysku" a "zapisałem to
w historii" – tzw. **staging area** (indeks). Dzięki temu możesz
wybrać dokładnie, które zmiany trafią do commita, nawet jeśli
zmodyfikowałeś więcej plików naraz.

```bash
git add plik.cpp          # dodaj konkretny plik do staging area
git add .                  # dodaj wszystkie zmienione/nowe pliki w bieżącym katalogu
git add -p                 # interaktywnie wybierz fragmenty (hunki) zmian do dodania - bardzo przydatne
```

```bash
git commit -m "Krótki opis zmiany"
```
Zapisuje wszystko co jest w staging area jako nowy commit w historii,
z podanym opisem.

```bash
git commit -am "Opis"
```
Skrót łączący `git add -u` (tylko już śledzone, zmodyfikowane pliki –
**nie** nowe pliki) z `git commit` w jednym poleceniu.

> 💡 **Dobra praktyka:** commituj małe, logicznie spójne zmiany z
> jasnym opisem ("Fix off-by-one error in ring buffer push()"), zamiast
> jednego wielkiego commita na koniec dnia z opisem "zmiany".

---

## 3. Przeglądanie historii

```bash
git log
```
Pełna historia commitów (hash, autor, data, opis) od najnowszego do
najstarszego.

```bash
git log --oneline              # skrócony widok: jedna linia na commit
git log --oneline --graph --all # wizualizacja gałęzi w formie ASCII-art
git log -p plik.cpp             # historia zmian konkretnego pliku, ze szczegółowym diff
git log --author="Jan"          # tylko commity danego autora
git log --since="2 weeks ago"   # tylko commity z ostatnich 2 tygodni
```

```bash
git show <hash>
```
Pokazuje pełne szczegóły (diff) konkretnego commita.

```bash
git diff
```
Różnica między plikami na dysku a ostatnim commitem (co jest
niezacommitowane).

```bash
git diff --staged
```
Różnica między staging area a ostatnim commitem (co zostanie
zacommitowane, jeśli teraz zrobisz `git commit`).

```bash
git diff branch1 branch2
```
Różnica między dwoma branchami/commitami.

---

## 4. Ignorowanie plików – `.gitignore`

```
# .gitignore
build/
*.o
*.log
.vscode/
```
Plik `.gitignore` w katalogu głównym repozytorium mówi gitowi, których
plików/katalogów **nigdy** nie proponować do śledzenia (nie pojawią
się w `git status` jako "untracked"). Kluczowe w embedded: pliki
budowania (`build/`, `*.o`, `*.elf`, `*.bin`), pliki IDE
(`.vscode/`, `.idea/`), lokalne pliki konfiguracyjne.

> ⚠️ `.gitignore` działa tylko na pliki **jeszcze nieśledzone**. Jeśli
> plik już trafił kiedyś do repozytorium, dodanie go do `.gitignore`
> nie usunie go z historii – trzeba dodatkowo `git rm --cached plik`.

---

## 5. Nazywanie i przeglądanie konkretnych commitów

```bash
git log --oneline -5      # ostatnie 5 commitów
```
Każdy commit ma unikalny hash (np. `a3f5c9d`) – można się do niego
odwoływać skróconą formą (pierwsze 7-8 znaków wystarcza w praktyce).

```bash
HEAD          # aktualny commit, na którym "stoisz"
HEAD~1        # jeden commit przed HEAD (rodzic)
HEAD~3        # trzy commity przed HEAD
```

---

## ✅ Zadania

1. Stwórz testowe repozytorium (`git init test_repo`), dodaj plik
   `README.md` z dowolną treścią, zrób pierwszy commit.
2. Zmodyfikuj plik, sprawdź `git status` i `git diff` przed dodaniem
   do staging area, potem po `git add`.
3. Zrób 3-4 kolejne commity z drobnymi zmianami, obejrzyj historię
   przez `git log --oneline --graph`.
4. Dodaj `.gitignore` ignorujący katalog `build/`, sprawdź że
   utworzenie `build/plik.o` nie pojawia się w `git status`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 2 – Branże i scalanie](chapter2_branching_merging.md)
