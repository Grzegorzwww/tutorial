#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

int sum_of_squares(const std::vector<int>& v) {
    return std::accumulate(v.begin(), v.end(), 0, [](int acc, int x) { return acc + x * x; });
}

void sort_by_abs_desc(std::vector<int>& v) {
    std::stable_sort(v.begin(), v.end(), [](int a, int b) { return std::abs(a) > std::abs(b); });
}

void remove_negatives(std::vector<int>& v) {
    std::erase_if(v, [](int x) { return x < 0; });
}

std::size_t partition_evens_first(std::vector<int>& v) {
    auto mid = std::stable_partition(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
    return static_cast<std::size_t>(mid - v.begin());
}

auto make_counter(int start) {
    return [value = start]() mutable { return value++; };
}

auto make_adder(int n) {
    return [n](int x) { return x + n; };
}

auto compose = [](auto f, auto g) {
    return [=](auto x) { return f(g(x)); };
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
