// Zadanie 26 (C++): referencje i const-poprawnosc
//
//   swap_values:     zamienia wartosci (przez referencje)
//   max_ref:         zwraca referencje do WIEKSZEJ z dwoch zmiennych (do `a`, gdy rowne) -
//                    zapis do wyniku zmienia oryginal
//   longer:          zwraca referencje do dluzszego napisu (do `a`, gdy rowne dlugosci)
//   append_all:      dopisuje wszystkie elementy src na koniec dst
//   count_positive:  liczba elementow > 0
//   Counter:         value() const, increment(), add(n) zwracajace *this (lancuchowanie)
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

void swap_values(int& a, int& b) {
    // TODO
}

int& max_ref(int& a, int& b) {
    // TODO
    return a;
}

const std::string& longer(const std::string& a, const std::string& b) {
    // TODO
    return a;
}

void append_all(std::vector<int>& dst, const std::vector<int>& src) {
    // TODO
}

std::size_t count_positive(const std::vector<int>& v) {
    // TODO
    return 0;
}

class Counter {
public:
    int value() const {
        // TODO
        return -1;
    }
    void increment() {
        // TODO
    }
    Counter& add(int n) {
        // TODO
        return *this;
    }

private:
    int value_ = 0;
};

int main() {
    int a = 1, b = 2;
    swap_values(a, b);
    assert(a == 2 && b == 1);

    int x = 5, y = 9;
    max_ref(x, y) = 0;
    assert(x == 5 && y == 0);
    int p = 4, q = 4;
    assert(&max_ref(p, q) == &p);

    std::string s1 = "ab", s2 = "abc", s3 = "cd";
    assert(&longer(s1, s2) == &s2);
    assert(&longer(s1, s3) == &s1);

    std::vector<int> dst{1, 2};
    append_all(dst, {3, 4, 5});
    assert((dst == std::vector<int>{1, 2, 3, 4, 5}));
    append_all(dst, {});
    assert(dst.size() == 5);

    assert(count_positive({-1, 0, 1, 2}) == 2);
    assert(count_positive({}) == 0);

    Counter c;
    assert(c.value() == 0);
    c.increment();
    c.add(5).add(2);
    assert(c.value() == 8);
    const Counter& cr = c;
    assert(cr.value() == 8);

    std::cout << "OK\n";
}
