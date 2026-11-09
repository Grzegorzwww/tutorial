// Rozdzial 3, zadanie 5: span nie jest wlascicielem (dangling)
//
// 1) Odpowiedz w komentarzu na dole pliku (ANSWER): co sie stanie ze spanem po
//    v.push_back(...) wymuszajacym realokacje i dlaczego const w span<const T> tego nie chroni?
// 2) Zaimplementuj:
//      - views_same_storage(s, v): czy span wskazuje dokladnie na dane wektora
//                                  (ten sam data() i ten sam size())
//      - rebuilt(v):               swiezy span na caly wektor (tak sie "naprawia" span po realokacji)
//      - append_and_get_view(v, x): dopisuje x do v i zwraca ZAWSZE aktualny span (nie stary!)
#include <cassert>
#include <iostream>
#include <span>
#include <vector>

bool views_same_storage(std::span<const int> s, const std::vector<int>& v) {
    // TODO
    return false;
}

std::span<const int> rebuilt(const std::vector<int>& v) {
    // TODO
    return {};
}

std::span<const int> append_and_get_view(std::vector<int>& v, int x) {
    // TODO
    return {};
}

int main() {
    std::vector<int> v{1, 2, 3};
    std::span<const int> s(v);
    assert(views_same_storage(s, v));

    const int* old_data = v.data();
    v.reserve(1000);  // pojemnosc 3 -> 1000: dane zostaja przeniesione
    assert(old_data != v.data());
    assert(!views_same_storage(s, v));  // s nadal "pamieta" stary adres - NIE UZYWAJ go!

    s = rebuilt(v);
    assert(views_same_storage(s, v));
    assert(s.size() == 3 && s[2] == 3);

    auto fresh = append_and_get_view(v, 4);
    assert(views_same_storage(fresh, v));
    assert(fresh.size() == 4 && fresh.back() == 4);

    std::cout << "OK\n";
}

/*
ANSWER:
(tu wpisz wlasnymi slowami)
*/
