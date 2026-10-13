# Chapter 3: Praca z repozytorium zdalnym (remote, push/pull, PR)

[← Spis treści](README.md)

---

## 1. Co to jest "remote"

Remote to nazwana referencja do repozytorium hostowanego gdzie indziej
(GitHub, GitLab, Bitbucket, własny serwer). Domyślna nazwa remote'a po
`git clone` to `origin`.

```bash
git remote -v                          # lista remote'ów z ich adresami URL
git remote add origin git@github.com:user/repo.git   # dodaj remote do repo utworzonego lokalnie (git init)
```

---

## 2. Wysyłanie i pobieranie zmian

```bash
git push origin main
```
Wysyła lokalne commity z brancha `main` do remote'a `origin`.

```bash
git push -u origin feature/uart-driver
```
`-u` (`--set-upstream`) ustawia powiązanie brancha lokalnego z
branchem zdalnym – po tym wystarczy sam `git push`/`git pull` bez
podawania nazw za każdym razem.

```bash
git fetch origin
```
Pobiera nowe commity/branche z remote'a, **ale nie scala ich** z
Twoją bieżącą pracą – tylko aktualizuje lokalną "wiedzę" o tym, co
jest na serwerze (`origin/main`, `origin/feature/xyz`).

```bash
git pull origin main
```
To w praktyce `git fetch` + `git merge origin/main` w jednym poleceniu
– pobiera **i od razu** scala zmiany z Twoim bieżącym branchem.

```bash
git pull --rebase origin main
```
Wariant, gdzie zamiast `merge` używany jest `rebase` – Twoje lokalne,
jeszcze niewypchnięte commity zostają "przełożone" na wierzchołek
najnowszego `main`, dając liniową historię. Wielu zespołów ustawia to
jako domyślne zachowanie (`git config pull.rebase true`), żeby uniknąć
niepotrzebnych merge commitów przy każdym `pull`.

> 💡 **Różnica fetch vs pull w skrócie:** `fetch` = "zobacz co nowego
> jest na serwerze", `pull` = "zobacz co nowego I od razu wmieszaj to
> w moją pracę". Dobra praktyka: rób `fetch` + `git log origin/main`
> żeby **zobaczyć** zmiany zanim je scalisz, jeśli chcesz mieć większą
> kontrolę.

---

## 3. Klonowanie i konfiguracja tożsamości

```bash
git clone git@github.com:user/repo.git
git clone git@github.com:user/repo.git moj_folder   # sklonuj do konkretnej nazwy folderu
```

```bash
git config --global user.name "Jan Kowalski"
git config --global user.email "jan@example.com"
```
Ustawienia globalne (dla wszystkich repozytoriów na tym komputerze) –
bez tego commity będą podpisane domyślnymi/błędnymi danymi.

```bash
git config user.email "praca@firma.com"   # bez --global = tylko dla BIEŻĄCEGO repozytorium
```
Przydatne, gdy używasz innego adresu e-mail do projektów prywatnych i
firmowych.

---

## 4. Pull Request / Merge Request – workflow zespołowy

Sam git (jako narzędzie) nie ma pojęcia "pull request" – to
funkcjonalność platform hostingowych (GitHub nazywa to Pull Request,
GitLab/Bitbucket – Merge Request). Typowy przepływ:

```bash
git switch -c feature/new-sensor-driver
# ... praca, commity ...
git push -u origin feature/new-sensor-driver
```
Następnie na stronie GitHub/GitLab tworzysz PR z brancha
`feature/new-sensor-driver` do `main` – zespół robi code review
(komentarze do konkretnych linii), CI (np. Jenkins z Twojego CV)
automatycznie buduje i testuje zmiany, a po akceptacji PR zostaje
zmergowany (często przez przycisk "Squash and merge" w interfejsie
webowym, co łączy wszystkie commity z brancha w jeden czysty commit na
`main`).

**Aktualizacja swojego PR po komentarzach z code review:**
```bash
# popraw kod na podstawie komentarzy
git add .
git commit -m "Fix code review comments"
git push        # PR na GitHub/GitLab automatycznie się zaktualizuje
```

---

## 5. Repozytoria prywatne i uwierzytelnianie

```bash
git remote -v
# origin  git@github.com:user/repo.git (fetch)    <- SSH (klucz), zalecane
# origin  https://github.com/user/repo.git (fetch) <- HTTPS (login/token)
```
SSH (klucz publiczny/prywatny skonfigurowany raz w koncie GitHub/
GitLab) jest wygodniejszy do codziennej pracy niż HTTPS z tokenem
wpisywanym za każdym razem – warto skonfigurować to na starcie
(`ssh-keygen`, dodanie klucza publicznego do ustawień konta).

---

## ✅ Zadania

1. Utwórz konto/repozytorium testowe na GitHub (jeśli jeszcze nie
   masz), sklonuj je lokalnie, zrób commit i wypchnij (`push`).
2. Utwórz branch, wypchnij go (`push -u origin ...`), otwórz na stronie
   GitHub Pull Request (możesz go od razu zamknąć bez mergowania – to
   ćwiczenie samego przepływu).
3. Zasymuluj pracę dwóch osób: sklonuj to samo repo do dwóch różnych
   katalogów lokalnie, zrób commit i push z jednego, potem spróbuj
   push z drugiego (bez wcześniejszego pull) – zobacz błąd
   "rejected... fetch first", zrób `git pull` i spróbuj ponownie.
4. Wyjaśnij różnicę między `git fetch` a `git pull`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

**Następny rozdział:** [Chapter 4 – Cofanie zmian](chapter4_undoing_changes.md)
