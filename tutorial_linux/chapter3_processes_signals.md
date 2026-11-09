# Chapter 3: Procesy i sygnały

[← Spis treści](README.md)

**Cel rozdziału:** Zrozumieć czym jest proces w Linuksie, jakie ma
stany życia, oraz jak procesy komunikują się (a raczej – jak jądro i
inne procesy je "przerywają") przy pomocy sygnałów. To codzienny
warsztat każdego Embedded Linux Engineera przy debugowaniu aplikacji
działających na docelowym sprzęcie.

---

## 1. Czym jest proces

Proces to uruchomiona instancja programu – ma własny identyfikator
(**PID**), własną przestrzeń pamięci, oraz wskazanie kto go uruchomił
(**PPID** – parent PID). Nowe procesy w Linuksie powstają wyłącznie
przez zduplikowanie istniejącego procesu:

- **`fork()`** – tworzy kopię bieżącego procesu (nowy PID, ta sama
  pamięć skopiowana "kopiuj przy zapisie" – copy-on-write).
- **`execve()`** – podmienia obraz pamięci procesu na inny program,
  zachowując ten sam PID.

Typowy schemat: bash robi `fork()` (powstaje kopia basha), a potem ta
kopia robi `execve("/bin/ls")` – i tak "shell uruchamia ls".

```bash
ps aux
```
Pełna lista procesów w systemie. Najważniejsze kolumny:
- `USER` – właściciel procesu
- `PID` – identyfikator procesu
- `%CPU`, `%MEM` – bieżące zużycie zasobów
- `STAT` – stan procesu (patrz tabela niżej)
- `COMMAND` – polecenie/ścieżka, którym uruchomiono proces

```bash
top       # albo htop jeśli zainstalowany
```
Widok "na żywo", odświeżany co sekundę – najczęściej pierwsze narzędzie
gdy urządzenie embedded "muli" i szukasz co zjada CPU/RAM.

```bash
pstree
```
Pokazuje procesy w formie drzewa rodzic→dziecko – bardzo pomocne żeby
zrozumieć "kto kogo odpalił" (np. `systemd → sshd → bash → ps`).

### Stany procesu

| Kod (`STAT`) | Nazwa | Znaczenie |
|---|---|---|
| `R` | Running | Aktualnie wykonywany lub gotowy do wykonania w kolejce schedulera |
| `S` | Sleeping | Czeka na zdarzenie (np. dane z sieci) – można przerwać sygnałem |
| `D` | Uninterruptible sleep | Czeka na I/O (np. dysk) – **nie** reaguje na sygnały, dopóki I/O się nie zakończy |
| `Z` | Zombie | Proces zakończony, ale rodzic jeszcze nie odebrał jego kodu wyjścia (`wait()`) |
| `T` | Stopped | Zatrzymany (np. przez `SIGSTOP` albo `Ctrl+Z`) |

> 💡 Zombie to **nie** proces-widmo pożerający zasoby – zajmuje tylko
> wpis w tabeli procesów, czeka aż rodzic odczyta jego status. Jeśli
> rodzic nigdy tego nie zrobi (błąd w kodzie), zombie zostaje aż do
> zakończenia rodzica.

---

## 2. Kontrola zadań w shellu (job control)

```bash
sleep 100 &
```
`&` na końcu odpala polecenie **w tle** (background) – shell od razu
zwraca kontrolę i wypisuje numer zadania oraz PID, np. `[1] 12345`.

```bash
jobs
```
Lista zadań w tle uruchomionych z **bieżącego** shella (numeracja `[1]`,
`[2]`...; różni się od PID, który jest globalny w systemie).

```bash
ps aux | grep sleep
```
Szukamy w pełnej liście procesów tego konkretnego `sleep`, żeby poznać
jego PID (przydatne gdy proces nie pochodzi z tego shella, więc `jobs`
go nie pokaże).

```bash
fg %1        # przywraca zadanie [1] na pierwszy plan
Ctrl+Z       # zawiesza działający na pierwszym planie proces (wysyła SIGTSTP)
bg %1        # wznawia zawieszone zadanie [1], ale w tle
```

---

## 3. Sygnały – asynchroniczne "przerwania" dla procesów

Sygnał to komunikat wysyłany do procesu przez jądro albo inny proces,
informujący o zdarzeniu ("przerwij", "zatrzymaj się", "skończ czas",
"nastąpił błąd pamięci"...). To najbliższy odpowiednik przerwań (IRQ)
ze świata mikrokontrolerów, tylko na poziomie procesów, nie sprzętu.

| Sygnał | Domyślna akcja | Można złapać/zignorować? | Typowe użycie |
|---|---|---|---|
| `SIGINT` (2) | Zakończ proces | Tak | Wysyłany przez `Ctrl+C` w terminalu |
| `SIGTERM` (15) | Zakończ proces | Tak | "Grzeczna" prośba o zamknięcie – domyślny sygnał `kill` |
| `SIGKILL` (9) | Zakończ proces | **Nie – nigdy** | Twarde zabicie, jądro usuwa proces natychmiast |
| `SIGSTOP` (19) | Zatrzymaj proces | **Nie – nigdy** | Wstrzymuje proces do `SIGCONT` |
| `SIGCONT` (18) | Wznów proces | Tak | Wznawia proces zatrzymany przez `SIGSTOP`/`SIGTSTP` |
| `SIGHUP` (1) | Zakończ proces | Tak | Historycznie: zerwanie terminala; dziś często używany jako "przeładuj konfigurację" |
| `SIGCHLD` (17) | Ignoruj | Tak | Wysyłany do rodzica, gdy dziecko się zakończy |
| `SIGSEGV` (11) | Zakończ + core dump | Tak (ale zwykle nie warto) | Naruszenie ochrony pamięci ("Segmentation fault") |

**Kluczowa różnica:** `SIGTERM` to prośba, którą proces może obsłużyć
(np. domknąć pliki, zapisać stan) albo zignorować. `SIGKILL` i
`SIGSTOP` są realizowane **bezpośrednio przez jądro**, proces nie ma
nad nimi żadnej kontroli – dlatego są "ostatnią deską ratunku" gdy
proces się zawiesił i nie reaguje na nic innego.

### Eksperyment praktyczny

```bash
sleep 100 &
ps aux | grep sleep       # znajdź PID (kolumna 2)
kill -SIGTERM <PID>       # "grzeczna" prośba - sleep i tak nie ma handlera, więc się kończy
```

```bash
sleep 100 &
kill -SIGSTOP <PID>       # zatrzymaj proces (STAT zmieni się na T)
ps aux | grep sleep       # zobacz stan T
kill -SIGCONT <PID>       # wznów - STAT wraca do S
kill -SIGKILL <PID>       # zabij bezwarunkowo
```

```bash
kill -l                   # pełna lista dostępnych sygnałów z numerami
kill -0 <PID>             # nie wysyła żadnego sygnału - tylko sprawdza czy PID istnieje
                           # (typowe w skryptach: "if kill -0 $PID 2>/dev/null; then ...")
```

### Łapanie sygnałów w skrypcie bash – `trap`

Bardzo praktyczne przy pisaniu skryptów uruchamianych na urządzeniu
embedded (np. usługa systemd, która musi posprzątać przed zamknięciem):

```bash
#!/bin/bash
cleanup() {
    echo "Otrzymano sygnał zamknięcia, sprzątam..."
    exit 0
}
trap cleanup SIGINT SIGTERM

echo "PID: $$, pracuję... (Ctrl+C albo kill -TERM $$ żeby przetestować)"
while true; do
    sleep 1
done
```
`trap` rejestruje funkcję `cleanup` jako handler dla `SIGINT` i
`SIGTERM` – zamiast domyślnego natychmiastowego zabicia, skrypt
wykona najpierw swój kod sprzątający.

### Inne przydatne narzędzia

```bash
killall sleep       # zabij WSZYSTKIE procesy o nazwie "sleep" (po nazwie, nie PID)
pkill -f "sleep 100" # zabij procesy pasujące do wzorca w pełnej linii poleceń

nice -n 10 ./moj_program     # uruchom z niższym priorytetem (grzeczniejszy dla innych)
renice -n 5 -p <PID>          # zmień priorytet już działającego procesu
```

---

## ✅ Zadania

1. Odtwórz eksperyment z `SIGTERM` i `SIGSTOP`/`SIGCONT` powyżej –
   zanotuj co pokazywało `STAT` w `ps aux` na każdym etapie.
2. Napisz krótki skrypt z `trap` (jak wyżej), uruchom go, i w drugim
   terminalu wyślij mu `SIGTERM` (`kill -TERM <PID>`). Czy zobaczyłeś
   komunikat sprzątający?
3. Wyjaśnij własnymi słowami: dlaczego `SIGKILL` i `SIGSTOP` nie mogą
   być przechwycone przez proces, a `SIGTERM` może?
4. Uruchom proces w tle, sprawdź jego PID, a potem `kill -0 <PID>` –
   jaki kod wyjścia (`echo $?`) dostajesz gdy proces istnieje, a jaki
   gdy go już nie ma?

**Notatki / odpowiedzi:**
```
(tu wpisz swoje wyniki)
```

---

## Podsumowanie

Po tym rozdziale powinieneś:
- [ ] Rozumieć czym jest proces, PID/PPID, oraz model `fork()`+`execve()`
- [ ] Znać podstawowe stany procesu (R, S, D, Z, T) i co oznaczają
- [ ] Umieć obsługiwać zadania w tle (`&`, `jobs`, `fg`, `bg`, `Ctrl+Z`)
- [ ] Rozumieć czym są sygnały i znać różnicę między `SIGTERM` a `SIGKILL`/`SIGSTOP`
- [ ] Umieć obsłużyć sygnał w skrypcie bash przy pomocy `trap`

**Następny rozdział:** [Chapter 4 – Użytkownicy, uprawnienia i udev](chapter4_permissions_udev.md)
