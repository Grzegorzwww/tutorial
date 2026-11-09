#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

std::optional<std::size_t> find_first_negative(const std::vector<int>& v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] < 0) return i;
    }
    return std::nullopt;
}

std::optional<int> parse_digit(char c) {
    if (c < '0' || c > '9') return std::nullopt;
    return c - '0';
}

int main() {
    std::vector<int> mixed{1, 2, -3, 4, -5};
    auto a = find_first_negative(mixed);
    assert(a.has_value() && *a == 2);

    std::vector<int> first_negative{-1, 5};
    auto b = find_first_negative(first_negative);
    assert(b.has_value() && *b == 0);

    std::vector<int> none{0, 1, 2};
    assert(!find_first_negative(none).has_value());

    std::vector<int> empty;
    assert(!find_first_negative(empty).has_value());

    assert(parse_digit('7').value_or(-1) == 7);
    assert(parse_digit('0').value_or(-1) == 0);
    assert(!parse_digit('x').has_value());
    assert(!parse_digit('/').has_value());

    std::cout << "OK\n";
}
