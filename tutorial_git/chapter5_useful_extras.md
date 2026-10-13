# Chapter 5: Przydatne narzędzia dodatkowe – tagi, bisect, blame, .gitignore

[← Spis treści](README.md)

---

## 1. Tagi – oznaczanie wersji

```bash
git tag v1.0.0                        # lekki tag na bieżącym commicie
git tag -a v1.0.0 -m "Release 1.0.0"  # tag z opisem i metadanymi (zalecane dla release'ów)
git tag                               # lista wszystkich tagów
git push origin v1.0.0                # tagi NIE są wypychane automatycznie z 'git push'!
git push origin --tags                # wypchnij wszystkie tagi naraz
```
Tagi to nazwane, **nieprzesuwające się** wskaźniki na konkretny commit
(w przeciwieństwie do branchy) – idealne do oznaczania wydanych wersji
firmware/oprogramowania (`v1.2.0`, `release-2024-03`).

```bash
git checkout v1.0.0    # "przenieś się" do stanu z danego tagu (tzw. detached HEAD)
```

---

## 2. `git bisect` – automatyczne szukanie commita, który wprowadził buga

Sytuacja: wiesz, że kod działał 50 commitów temu, a teraz jest zepsuty,
ale nie wiesz **który** z 50 commitów to spowodował. Ręczne sprawdzanie
każdego byłoby żmudne – `bisect` robi wyszukiwanie binarne
(logarytmiczne, nie liniowe) automatycznie.

```bash
git bisect start
git bisect bad                  # bieżący commit jest zepsuty
git bisect good v1.0.0           # ten stary tag/commit na pewno działał

# git automatycznie przełącza na commit "w połowie" zakresu:
# przetestuj kod ręcznie, potem powiedz gitowi wynik:
git bisect good    # jeśli TEN commit jeszcze działał
# albo:
git bisect bad     # jeśli TEN commit już był zepsuty

# powtarzaj, aż git wskaże DOKŁADNIE ten jeden commit winny:
# "a3f5c9d is the first bad commit"

git bisect reset   # zakończ, wróć do brancha sprzed bisect
```

**Automatyzacja z testem/skryptem** (jeszcze szybsze – zero ręcznego
testowania):
```bash
git bisect start HEAD v1.0.0
git bisect run ./run_tests.sh    # git sam uruchamia skrypt na każdym kroku
                                   # (skrypt musi zwrócić 0 = dobry, niezerowy = zły)
```
To bardzo praktyczne narzędzie przy regresji w firmware – np. gdy
test HW-in-the-loop (jak w Twoim projekcie CI z Somfy) zaczyna nagle
nie przechodzić, a nie wiadomo dokładnie po którym commicie.

---

## 3. `git blame` – kto i kiedy zmienił konkretną linię

```bash
git blame plik.cpp
```
Dla każdej linii pliku pokazuje: hash commita, autora i datę ostatniej
zmiany tej linii. Nieocenione przy pytaniu "dlaczego ten kod tak
wygląda?" – zamiast pytać kolegę, sprawdzasz historię i opis commita,
który to wprowadził.

```bash
git blame -L 10,20 plik.cpp    # tylko linie 10-20
git log -p -1 <hash>            # zobacz PEŁNY kontekst commita znalezionego przez blame
```

> 💡 Jeśli `git blame` wskazuje na commit typu "reformat całego pliku"
> (a nie faktyczną zmianę logiki), dodaj `--ignore-rev <hash>` albo
> użyj `git blame -w` (ignoruj zmiany białych znaków), żeby przeskoczyć
> do prawdziwego autora zmiany logicznej.

---

## 4. Szukanie w historii – `git log` z filtrami i `git grep`

```bash
git log -S"nazwa_funkcji"          # commity, które DODAŁY lub USUNĘŁY to dokładne wystąpienie tekstu
git log --grep="fix.*uart" -i      # commity, których OPIS pasuje do wzorca (regex, -i = ignoruj wielkość liter)
git grep "TODO"                     # przeszukaj zawartość plików w BIEŻĄCYM stanie repo (szybsze niż zwykły grep w dużym repo)
git grep "TODO" $(git rev-list --all)   # przeszukaj TOTALNIE całą historię wszystkich commitów
```

---

## 5. `.gitattributes` – kontrola nad obsługą plików (krótko)

```
# .gitattributes
*.sh text eol=lf          # wymuś końcówki linii LF (Unix) dla skryptów, nawet na Windows
*.png binary               # jawnie oznacz jako binarny (git nie będzie próbował robić diff tekstowego)
*.bin binary
```
Przydatne w zespołach mieszających Windows/Linux (typowe w embedded –
część zespołu na Windows z STM32CubeIDE, część na Linuksie) – zapobiega
problemom z końcówkami linii (CRLF vs LF) psującymi diffy i buildy.

---

## 6. Submoduły (krótka wzmianka)

```bash
git submodule add https://github.com/user/lib.git external/lib
git submodule update --init --recursive   # po sklonowaniu repo z submodułami - pobierz ich zawartość
```
Submoduł to osobne repozytorium "wpięte" wewnątrz głównego repo pod
konkretnym commitem (np. zewnętrzna biblioteka jak HAL producenta
mikrokontrolera). Częsty w embedded, ale też źródło częstych problemów
dla początkujących (zapominanie o `--recursive` przy klonowaniu,
niezaktualizowany wskaźnik submodułu) – warto znać podstawową
obsługę, nawet bez głębokiego wchodzenia w temat.

---

## ✅ Zadania

1. Utwórz tag `v0.1.0` na jednym z wcześniejszych commitów w swoim
   testowym repo, sprawdź `git tag` i `git checkout v0.1.0`.
2. Celowo wprowadź "buga" (np. zmień jedną linię logiki) w jednym z
   kilkunastu kolejnych commitów w testowym repo, a potem użyj
   `git bisect` (ręcznie, `good`/`bad`) żeby go zlokalizować.
3. Użyj `git blame` na dowolnym pliku z historią kilku commitów –
   sprawdź kto/kiedy zmienił konkretną linię.
4. Użyj `git log -S"jakiś_tekst"` żeby znaleźć commit, w którym dodałeś
   konkretny fragment kodu w swoim testowym repo.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie kursu

Po ukończeniu wszystkich rozdziałów powinieneś:
- [ ] Rozumieć commit, staging area, i umieć czytać historię (`log`, `diff`, `show`)
- [ ] Swobodnie tworzyć branche i rozumieć różnicę merge vs rebase
- [ ] Umieć pracować z repozytorium zdalnym (push/pull/fetch) i przejść przez workflow Pull Requesta
- [ ] Bezpiecznie cofać zmiany (`restore`, `reset`, `revert`, `stash`) i wiedzieć o `reflog` jako siatce bezpieczeństwa
- [ ] Znać tagi, `bisect`, `blame` jako narzędzia do pracy z historią projektu

**Gratulacje – to koniec kursu Git.**
