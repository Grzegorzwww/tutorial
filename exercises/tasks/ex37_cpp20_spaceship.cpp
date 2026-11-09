// Zadanie 37 (C++20): operator <=> i inicjalizacja wskazana (designated initializers)
//
// 1) Version {major, minor, patch}: dodaj `operator<=>` (wystarczy `= default`) i `==`,
//    zeby dzialaly <, >, <=, >=, ==, != oraz std::ranges::sort.
// 2) parse_version("1.2.3"): dokladnie trzy liczby rozdzielone kropkami, inaczej nullopt.
// 3) to_string(Version): "1.2.3".
// 4) newest: wskaznik na najwyzsza wersje albo nullptr.
// 5) Person {name, age}: kolejnosc NAJPIERW po wieku, potem po imieniu (czyli inna niz
//    kolejnosc skladowych - domyslny <=> nie wystarczy; napisz wlasny zwracajacy
//    std::strong_ordering). operator== moze byc `= default`.
#include <algorithm>
#include <cassert>
#include <charconv>
#include <compare>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Version {
    int major;
    int minor;
    int patch;
    // TODO: operator<=> i operator==
};

std::optional<Version> parse_version(std::string_view s) {
    // TODO: std::from_chars
    return std::nullopt;
}

std::string to_string(const Version& v) {
    // TODO
    return "";
}

const Version* newest(const std::vector<Version>& versions) {
    // TODO
    return nullptr;
}

struct Person {
    std::string name;
    int age;
    // TODO: operator<=> (wiek, potem imie) i operator==
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
