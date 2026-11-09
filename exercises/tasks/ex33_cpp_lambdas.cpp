// Zadanie 33 (C++): lambdy i algorytmy STL
//
//   sum_of_squares:           std::accumulate z lambda
//   sort_by_abs_desc:         po module malejaco, stabilnie ({3,-3} zostaje {3,-3})
//   remove_negatives:         usun ujemne (std::erase_if albo erase-remove)
//   partition_evens_first:    parzyste na poczatek, zachowaj kolejnosc (stable_partition),
//                             zwroc liczbe parzystych
//   make_counter(start):      zwraca lambde; kazde wywolanie zwraca biezaca wartosc i zwieksza ja
//   make_adder(n):            zwraca lambde x -> x + n
//   compose:                  generyczna lambda: compose(f, g)(x) == f(g(x))
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

int sum_of_squares(const std::vector<int>& v) {
    // TODO
    return -1;
}

void sort_by_abs_desc(std::vector<int>& v) {
    // TODO
}

void remove_negatives(std::vector<int>& v) {
    // TODO
}

std::size_t partition_evens_first(std::vector<int>& v) {
    // TODO
    return 0;
}

auto make_counter(int start) {
    // TODO: [value = start]() mutable { ... }
    return []() { return 0; };
}

auto make_adder(int n) {
    // TODO
    return [](int x) { return x; };
}

auto compose = [](auto f, auto g) {
    // TODO
    return [](auto x) { return x; };
};

int main() {
    assert(sum_of_squares({1, 2, 3}) == 14);
    assert(sum_of_squares({}) == 0);

    std::vector<int> v{3, -3, 1, -5, 2};
    sort_by_abs_desc(v);
    assert((v == std::vector<int>{-5, 3, -3, 2, 1}));

    std::vector<int> w{1, -2, 3, -4, 5};
    remove_negatives(w);
    assert((w == std::vector<int>{1, 3, 5}));

    std::vector<int> p{1, 2, 3, 4, 5, 6};
    assert(partition_evens_first(p) == 3);
    assert((p == std::vector<int>{2, 4, 6, 1, 3, 5}));

    auto c1 = make_counter(5);
    auto c2 = make_counter(100);
    assert(c1() == 5 && c1() == 6 && c1() == 7);
    assert(c2() == 100);  // liczniki sa niezalezne
    assert(c1() == 8);

    assert(make_adder(3)(4) == 7);
    assert(compose(make_adder(1), [](int x) { return x * 2; })(5) == 11);
    assert(compose([](double x) { return x / 2; }, [](double x) { return x + 1; })(3.0) == 2.0);

    std::cout << "OK\n";
}
