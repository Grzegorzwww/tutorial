#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct MinMax {
    int min;
    int max;
};

MinMax min_max(const std::vector<int>& v) {
    auto [lo, hi] = std::minmax_element(v.begin(), v.end());
    return {*lo, *hi};
}

int sum_values(const std::map<std::string, int>& m) {
    int sum = 0;
    for (const auto& [key, value] : m) sum += value;
    return sum;
}

std::string key_of_max(const std::map<std::string, int>& m) {
    std::string best;
    int best_value = 0;
    bool first = true;
    for (const auto& [key, value] : m) {
        if (first || value > best_value) {
            best = key;
            best_value = value;
            first = false;
        }
    }
    return best;
}

int main() {
    std::vector<int> v{4, -2, 9, 0};
    auto [lo, hi] = min_max(v);
    assert(lo == -2);
    assert(hi == 9);

    std::map<std::string, int> m{{"a", 1}, {"b", 5}, {"c", 3}};
    assert(sum_values(m) == 9);
    assert(key_of_max(m) == "b");

    std::map<std::string, int> empty;
    assert(sum_values(empty) == 0);
    assert(key_of_max(empty).empty());

    std::cout << "OK\n";
}
