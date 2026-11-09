#include <cassert>
#include <iostream>
#include <string_view>
#include <utility>

std::pair<std::string_view, std::string_view> split_first(std::string_view s, char delim) {
    auto pos = s.find(delim);
    if (pos == std::string_view::npos) return {s, {}};
    return {s.substr(0, pos), s.substr(pos + 1)};
}

std::string_view trim_left(std::string_view s) {
    auto pos = s.find_first_not_of(" \t");
    if (pos == std::string_view::npos) return {};
    return s.substr(pos);
}

int main() {
    std::string_view s = "key=value";
    auto [k, v] = split_first(s, '=');
    assert(k == "key");
    assert(v == "value");
    assert(v.data() == s.data() + 4);

    auto [a, b] = split_first("novalue", '=');
    assert(a == "novalue");
    assert(b.empty());

    auto [c, d] = split_first("=x", '=');
    assert(c.empty());
    assert(d == "x");

    assert(trim_left("  \tabc ") == "abc ");
    assert(trim_left("   ").empty());
    assert(trim_left("abc") == "abc");

    std::cout << "OK\n";
}
