#include <algorithm>
#include <cassert>
#include <charconv>
#include <compare>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

struct Version {
    int major;
    int minor;
    int patch;
    auto operator<=>(const Version&) const = default;
};

std::optional<Version> parse_version(std::string_view s) {
    int parts[3];
    std::size_t count = 0;
    while (true) {
        auto pos = s.find('.');
        auto token = s.substr(0, pos);
        if (count == 3 || token.empty()) return std::nullopt;

        int value = 0;
        auto [end, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
        if (ec != std::errc{} || end != token.data() + token.size()) return std::nullopt;
        parts[count++] = value;

        if (pos == std::string_view::npos) break;
        s.remove_prefix(pos + 1);
    }
    if (count != 3) return std::nullopt;
    return Version{parts[0], parts[1], parts[2]};
}

std::string to_string(const Version& v) {
    return std::to_string(v.major) + "." + std::to_string(v.minor) + "." + std::to_string(v.patch);
}

const Version* newest(const std::vector<Version>& versions) {
    auto it = std::ranges::max_element(versions);
    return it == versions.end() ? nullptr : &*it;
}

struct Person {
    std::string name;
    int age;

    std::strong_ordering operator<=>(const Person& other) const {
        if (auto cmp = age <=> other.age; cmp != 0) return cmp;
        return name <=> other.name;
    }
    bool operator==(const Person&) const = default;
};

static_assert(std::three_way_comparable<Version>);
static_assert(std::three_way_comparable<Person>);

int main() {
    Version a{1, 2, 3}, b{1, 10, 0};
    assert(a < b && b > a && a <= a && a >= a);
    assert(a == (Version{1, 2, 3}) && a != b);

    Version d{.major = 2, .minor = 0, .patch = 0};
    assert(d > b);

    assert(parse_version("1.2.3") == (Version{1, 2, 3}));
    assert(parse_version("10.0.255") == (Version{10, 0, 255}));
    assert(!parse_version("1.2").has_value());
    assert(!parse_version("1.2.3.4").has_value());
    assert(!parse_version("a.b.c").has_value());
    assert(!parse_version("1..3").has_value());
    assert(!parse_version("").has_value());
    assert(!parse_version("1.2.").has_value());

    assert(to_string(Version{1, 2, 3}) == "1.2.3");

    std::vector<Version> vs{{1, 2, 3}, {1, 10, 0}, {0, 9, 9}};
    assert(newest(vs) == &vs[1]);
    assert(newest({}) == nullptr);
    std::ranges::sort(vs);
    assert(vs.front() == (Version{0, 9, 9}) && vs.back() == (Version{1, 10, 0}));

    Person p1{"Zed", 30}, p2{"Amy", 30}, p3{"Bob", 25};
    assert(p3 < p2);
    assert(p2 < p1);
    assert(p1 != p2);
    assert(p1 == (Person{"Zed", 30}));
    assert(!(p1 < p1));

    std::cout << "OK\n";
}
