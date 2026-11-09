#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

void sort_by_length(std::vector<std::string>& v) {
    std::ranges::stable_sort(v, {}, &std::string::size);
}

int index_of(const std::vector<int>& v, int value) {
    auto it = std::ranges::find(v, value);
    if (it == v.end()) return -1;
    return static_cast<int>(std::distance(v.begin(), it));
}

bool all_positive(const std::vector<int>& v) {
    return std::ranges::all_of(v, [](int x) { return x > 0; });
}

std::string longest(const std::vector<std::string>& v) {
    auto it = std::ranges::max_element(v, {}, &std::string::size);
    return it == v.end() ? std::string{} : *it;
}

int main() {
    std::vector<std::string> words{"ccc", "a", "bb", "dd", "e"};
    sort_by_length(words);
    assert((words == std::vector<std::string>{"a", "e", "bb", "dd", "ccc"}));  // stabilnie

    assert(index_of({5, 3, 1, 3}, 3) == 1);
    assert(index_of({5, 3, 1}, 9) == -1);
    assert(index_of({}, 1) == -1);

    assert(all_positive({1, 2, 3}));
    assert(!all_positive({1, 0, 3}));
    assert(all_positive({}));

    assert(longest({"aa", "bbb", "ccc", "d"}) == "bbb");
    assert(longest({}).empty());

    std::cout << "OK\n";
}
