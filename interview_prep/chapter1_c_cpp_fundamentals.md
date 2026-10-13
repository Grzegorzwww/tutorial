# Chapter 1: Fundamenty C/C++

[← Spis treści](README.md)

---

## Pamięć i wskaźniki

**P: Jaka jest różnica między stosem (stack) a stertą (heap)?**
- Stos: pamięć na zmienne lokalne i wywołania funkcji, zarządzana
  automatycznie (alokacja/zwolnienie przy wejściu/wyjściu z zakresu),
  szybka, ograniczona rozmiarem (typowo kilka KB–kilka MB, na
  mikrokontrolerze często ustawiana w linker script).
- Sterta: pamięć alokowana ręcznie (`malloc`/`new`) lub przez
  kontenery (`std::vector`), żyje aż do jawnego zwolnienia
  (`free`/`delete`) lub końca życia obiektu (smart pointer, RAII),
  wolniejsza (fragmentacja, alokator), w embedded często unikana lub
  ograniczana (deterministyczny czas wykonania, ryzyko fragmentacji
  przy długim czasie działania urządzenia).

**P: Co to jest wyciek pamięci (memory leak) i jak mu zapobiegać w C++?**
- Pamięć zaalokowana (`new`/`malloc`), do której program traci
  wszystkie wskaźniki, nigdy niezwolniona.
- Zapobieganie: RAII – opakowanie zasobu w obiekt, którego destruktor
  zwalnia zasób automatycznie (`std::unique_ptr`, `std::shared_ptr`,
  `std::vector` zamiast surowych tablic). "Rule of Zero" – jeśli
  używasz odpowiednich typów RAII, nie musisz pisać własnych
  destruktorów/konstruktorów kopiujących wcale.

**P: Różnica między `std::unique_ptr` a `std::shared_ptr`?**
- `unique_ptr` – jeden właściciel, nie da się skopiować (tylko
  `std::move`), zero narzutu w runtime (tak szybki jak surowy
  wskaźnik).
- `shared_ptr` – wielu właścicieli, licznik referencji (atomowy –
  koszt w runtime), zasób zwalniany gdy licznik spadnie do zera.
  Uwaga na cykle referencji (`A` trzyma `shared_ptr` do `B`, `B` do
  `A`) – rozwiązanie: `std::weak_ptr` po jednej ze stron.

**P: Co się dzieje przy dereferencji `nullptr`? A przy wyjściu poza
zakres tablicy?**
- `nullptr` dereference: Undefined Behavior – na systemach z MMU
  (Linux) zwykle skutkuje `SIGSEGV` (segfault), na mikrokontrolerze
  bez MMU może po prostu odczytać/zapisać losowy adres blisko zera
  (często pada w Hard Fault na Cortex-M dzięki MPU/regionowi 0).
- Wyjście poza tablicę: też UB – **nie ma gwarancji** błędu w runtime;
  program może "po cichu" nadpisać sąsiednią pamięć (klasyczny,
  podstępny bug – działa "czasami", zależnie od layoutu pamięci).

**P: `const` vs `constexpr` vs `volatile` – różnice i typowe zastosowanie w embedded?**
- `const` – wartość niemodyfikowalna po inicjalizacji, ale może być
  ustalona w runtime (`const int x = read_sensor();`).
- `constexpr` – wartość **musi** być obliczalna w compile-time
  (patrz kurs C++20, Chapter 4, dla `consteval`/`constinit`).
- `volatile` – mówi kompilatorowi "nie optymalizuj dostępu do tej
  zmiennej, może się zmienić spoza normalnego przepływu programu"
  (rejestr sprzętowy, zmienna modyfikowana w ISR). **Nie** daje
  atomowości ani synchronizacji między wątkami – to częsty błąd w
  odpowiedziach kandydatów. Do współdzielenia danych między wątkami
  służy `std::atomic`, nie `volatile`.

**P: Napisz różnicę między przekazaniem przez wartość, referencję i wskaźnik.**
```cpp
void by_value(int x);       // kopia - zmiany nie wpływają na oryginał
void by_ref(int& x);        // referencja - zmiany WPŁYWAJĄ na oryginał, nie może być "null"
void by_ptr(int* x);        // wskaźnik - zmiany wpływają, ALE może być nullptr (trzeba sprawdzać)
void by_const_ref(const int& x); // referencja tylko do odczytu - unika kopii dla dużych obiektów
```

---

## `const` correctness i modyfikatory

**P: Co robi `const` w różnych miejscach sygnatury metody?**
```cpp
class Sensor {
    int value;
public:
    int get_value() const;              // metoda nie modyfikuje stanu obiektu (this jest const)
    void set(const int& v);             // parametr przekazany przez referencję do stałej
    const int* get_ptr() const;         // wskaźnik do stałego int, sam wskaźnik można zmienić
    int* const get_ptr2();              // stały wskaźnik do int (niemodyfikowalnego adresu), wartość spod niego można zmienić
};
```

**P: Czym różni się `#define` od `constexpr`/`const` przy definiowaniu stałych?**
- `#define` – działa na poziomie preprocesora, tekstowa podmiana, brak
  typu, brak sprawdzania zakresu/typu przez kompilator, nie widoczne w
  debuggerze (nie istnieje jako symbol).
- `constexpr`/`const` – ma typ, widoczne w debuggerze, sprawdzane przez
  kompilator, respektuje zasady zakresu (namespace/klasa).

---

## RAII i zarządzanie zasobami

**P: Co to jest RAII i dlaczego jest tak ważne w C++?**
- *Resource Acquisition Is Initialization* – zasób (pamięć, uchwyt
  pliku, mutex, peryferium sprzętowe) jest przejmowany w konstruktorze
  obiektu i zwalniany w destruktorze. Dzięki temu zasób jest zawsze
  zwolniony, nawet gdy funkcja zakończy się przez wyjątek albo
  wczesny `return` – nie trzeba pamiętać o ręcznym `free()`/`close()`
  w każdej możliwej ścieżce wyjścia z funkcji.

```cpp
class GpioPin {
    int pin;
public:
    GpioPin(int p) : pin(p) { gpio_claim(pin); }
    ~GpioPin() { gpio_release(pin); }   // zawsze wywołane, nawet przy wyjątku
};
```

**P: Co to jest "Rule of Three/Five/Zero"?**
- Rule of Three (C++98/03): jeśli piszesz destruktor, konstruktor
  kopiujący lub `operator=`, prawdopodobnie potrzebujesz wszystkich
  trzech.
- Rule of Five (C++11): dochodzą konstruktor przenoszący i
  `operator=` przenoszący (move semantics).
- Rule of Zero (nowoczesny C++): najlepiej **nie pisać żadnego** z
  nich – użyj typów RAII (`unique_ptr`, `vector`, `string`) jako pól
  klasy, kompilator sam wygeneruje poprawne wersje.

---

## Wirtualność i polimorfizm

**P: Jak działa `virtual` "pod maską" (vtable)?**
- Klasa z choć jedną metodą `virtual` dostaje ukryty wskaźnik
  (vptr) do tablicy wskaźników na funkcje (vtable) – jedna vtable per
  klasa, jeden vptr per obiekt. Wywołanie metody wirtualnej to
  dodatkowy poziom pośredniości (odczyt z vtable, potem skok) –
  niewielki koszt w runtime i w rozmiarze obiektu (+1 wskaźnik).

**P: Kiedy destruktor **musi** być `virtual`?**
- Gdy usuwasz obiekt klasy pochodnej przez wskaźnik do klasy bazowej
  (`Base* b = new Derived(); delete b;`) – bez `virtual` destruktor
  klasy pochodnej **nie zostanie wywołany** (UB/wyciek zasobów klasy
  pochodnej).

**P: Co to jest "slicing" (cięcie obiektu)?**
```cpp
void print(Base b) {   // przez WARTOŚĆ, nie referencję/wskaźnik!
    b.describe();
}
Derived d;
print(d);   // 'd' zostaje "obcięty" do Base - część danych Derived jest tracona
```
Rozwiązanie: przekazywać przez referencję/wskaźnik, nie przez wartość,
gdy oczekujesz polimorficznego zachowania.

---

## Undefined Behavior i pułapki

**P: Podaj 3 przykłady Undefined Behavior w C++.**
- Dereferencja `nullptr` lub dzikiego wskaźnika.
- Wyjście poza granice tablicy (`arr[size]` lub dalej).
- Odczyt niezainicjalizowanej zmiennej.
- Przepełnienie liczby ze znakiem (`INT_MAX + 1`) – w przeciwieństwie
  do `unsigned`, gdzie zachowanie jest **zdefiniowane** (zawijanie
  modulo).
- Wielokrotna modyfikacja tej samej zmiennej bez punktu sekwencji
  (`i = i++ + ++i;`).
- Wyścig danych (data race) – dwa wątki, jeden pisze, jeden czyta tę
  samą zmienną bez synchronizacji.

**P: Dlaczego przepełnienie `unsigned int` jest zdefiniowane, a `int` nie?**
- Standard C++ definiuje arytmetykę `unsigned` jako modulo $2^n$ (tzw.
  "zawijanie"), natomiast przepełnienie `signed int` jest Undefined
  Behavior – kompilator ma prawo założyć, że nigdy nie występuje, i
  zoptymalizować kod na tej podstawie (co bywa źródłem zaskakujących
  bugów przy `-O2`).

---

## Szablony i nowoczesny C++ (skrót – szerzej w kursie C++20)

**P: Po co są szablony (templates) i czym różnią się od makr?**
- Szablony generują kod w compile-time, ale są **typowane** –
  kompilator sprawdza poprawność typów przy instancjacji, daje błędy
  kompilacji zamiast dziwnych runtime bugów. Makra to ślepa, tekstowa
  podmiana bez wiedzy o typach/składni.

**P: Co to jest "move semantics" i po co jest `std::move`?**
- Move semantics pozwala **przenieść** zasoby (np. bufor wewnętrzny
  `std::vector`) z jednego obiektu do drugiego bez kopiowania danych –
  tylko "podmiana właściciela" wskaźnika wewnętrznego.
  `std::move(x)` **nie przenosi nic samo w sobie** – to tylko rzutowanie
  na referencję r-value, które **umożliwia** kompilatorowi wybór
  konstruktora/operatora przenoszącego zamiast kopiującego.

---

## ✅ Szybki self-test (odpowiedz bez podglądania)

1. Dlaczego `volatile` nie zastępuje `std::atomic` przy współdzieleniu
   danych między wątkami?
2. Napisz klasę z RAII zarządzającą uchwytem pliku (otwórz w
   konstruktorze, zamknij w destruktorze).
3. Co się stanie, jeśli klasa bazowa ma nie-wirtualny destruktor, a Ty
   usuwasz obiekt pochodny przez wskaźnik bazowy?
4. Wyjaśnij różnicę: `const int* p`, `int* const p`, `const int* const p`.

**Notatki / odpowiedzi:**
```
(tu wpisz swoje odpowiedzi)
```

---

**Następny rozdział:** [Chapter 2 – Embedded-specific](chapter2_embedded_specific.md)
