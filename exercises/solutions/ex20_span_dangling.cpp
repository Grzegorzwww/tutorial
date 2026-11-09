#include <cassert>
#include <iostream>
#include <span>
#include <vector>

bool views_same_storage(std::span<const int> s, const std::vector<int>& v) {
    return s.data() == v.data() && s.size() == v.size();
}

std::span<const int> rebuilt(const std::vector<int>& v) {
    return std::span<const int>(v);
}

std::span<const int> append_and_get_view(std::vector<int>& v, int x) {
    v.push_back(x);
    return rebuilt(v);
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
span to tylko wskaznik + rozmiar, nie jest wlascicielem danych. Gdy vector realokuje pamiec
(push_back, reserve, resize) lub zostanie zniszczony, span dalej trzyma stary adres - to
wiszacy wskaznik, a uzycie go to UB. `const` w span<const T> tylko zabrania zmiany elementow
przez span, ale nie przedluza zycia ani nie zamraza adresu danych.
*/
