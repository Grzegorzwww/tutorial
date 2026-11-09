#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

std::vector<int> leading_run(const std::vector<int>& v) {
    std::vector<int> out;
    for (int x : v | std::views::take_while([](int n) { return n > 0; })) out.push_back(x);
    return out;
}

std::vector<int> skip_leading_zeros(const std::vector<int>& v) {
    std::vector<int> out;
    for (int x : v | std::views::drop_while([](int n) { return n == 0; })) out.push_back(x);
    return out;
}

std::vector<int> flatten(const std::vector<std::vector<int>>& vv) {
    std::vector<int> out;
    for (int x : vv | std::views::join) out.push_back(x);
    return out;
}

struct LazyReport {
    int after_build;
    int after_first;
};

LazyReport evaluated_calls() {
    std::vector<int> v{1, 2, 3, 4, 5};
    int calls = 0;
    auto view = v | std::views::transform([&calls](int x) {
                    ++calls;
                    return x * 2;
                });
    int after_build = calls;
    [[maybe_unused]] int first = *view.begin();
    return {after_build, calls};
}

int main() {
    assert((leading_run({3, 2, 1, -1, 5}) == std::vector<int>{3, 2, 1}));
    assert(leading_run({-1, 2}).empty());

    assert((skip_leading_zeros({0, 0, 1, 0, 2}) == std::vector<int>{1, 0, 2}));
    assert(skip_leading_zeros({0, 0}).empty());

    assert((flatten({{1, 2}, {}, {3}, {4, 5}}) == std::vector<int>{1, 2, 3, 4, 5}));
    assert(flatten({}).empty());

    auto r = evaluated_calls();
    assert(r.after_build == 0);
    assert(r.after_first == 1);

    std::cout << "OK\n";
}
