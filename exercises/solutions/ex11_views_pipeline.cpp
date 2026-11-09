#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

std::vector<int> first_squares_of_multiples_of_3(const std::vector<int>& v, int count) {
    std::vector<int> out;
    auto pipeline = v
                  | std::views::filter([](int n) { return n % 3 == 0; })
                  | std::views::transform([](int n) { return n * n; })
                  | std::views::take(count);
    for (int n : pipeline) out.push_back(n);
    return out;
}

int main() {
    std::vector<int> v;
    for (int i : std::views::iota(1, 21)) v.push_back(i);

    assert((first_squares_of_multiples_of_3(v, 3) == std::vector<int>{9, 36, 81}));
    assert((first_squares_of_multiples_of_3(v, 100) == std::vector<int>{9, 36, 81, 144, 225, 324}));
    assert(first_squares_of_multiples_of_3(v, 0).empty());
    assert(first_squares_of_multiples_of_3({1, 2, 4}, 3).empty());

    std::cout << "OK\n";
}
