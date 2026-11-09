#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

long sum_range(int a, int b) {
    long sum = 0;
    for (int i : std::views::iota(a, b)) sum += i;
    return sum;
}

std::vector<int> countdown(int n) {
    std::vector<int> out;
    for (int i : std::views::iota(1, n + 1) | std::views::reverse) out.push_back(i);
    return out;
}

std::vector<int> middle(const std::vector<int>& v) {
    std::vector<int> out;
    auto view = v
              | std::views::drop(1)
              | std::views::reverse
              | std::views::drop(1)
              | std::views::reverse;
    for (int x : view) out.push_back(x);
    return out;
}

int main() {
    assert(sum_range(1, 6) == 15);
    assert(sum_range(5, 5) == 0);
    assert(sum_range(-2, 3) == 0);

    assert((countdown(4) == std::vector<int>{4, 3, 2, 1}));
    assert(countdown(0).empty());

    assert((middle({1, 2, 3, 4, 5}) == std::vector<int>{2, 3, 4}));
    assert((middle({1, 2, 3}) == std::vector<int>{2}));
    assert(middle({1, 2}).empty());
    assert(middle({1}).empty());
    assert(middle({}).empty());

    std::cout << "OK\n";
}
