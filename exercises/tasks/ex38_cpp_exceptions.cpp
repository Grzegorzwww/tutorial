// Zadanie 38 (C++): wyjatki
//
//   ParseError:       wyjatek dziedziczacy po std::runtime_error, przechowuje pozycje bledu
//                     (position()); komunikat what() ma byc niepusty
//   parse_int:        string_view -> int, opcjonalny znak '-'.
//                     - brak cyfr lub niecyfra: ParseError z pozycja pierwszego blednego miejsca
//                       ("" -> 0, "-" -> 1, "12a4" -> 2)
//                     - wartosc poza zakresem int: std::out_of_range
//                     (INT_MIN "-2147483648" jest poprawne)
//   safe_div:         rzuca std::domain_error przy dzieleniu przez zero
//   try_parse_int:    wersja bez wyjatkow: optional<int> (lapie ParseError i out_of_range)
//   add_noexcept:     funkcja oznaczona noexcept (juz jest - zobacz static_assert)
#include <cassert>
#include <climits>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg, std::size_t position) : std::runtime_error(msg) {
        // TODO: zapamietaj position
    }
    std::size_t position() const {
        // TODO
        return 999;
    }
};

int parse_int(std::string_view s) {
    // TODO
    return 0;
}

int safe_div(int a, int b) {
    // TODO
    return 0;
}

std::optional<int> try_parse_int(std::string_view s) {
    // TODO
    return std::nullopt;
}

int add_noexcept(int a, int b) noexcept { return a + b; }
static_assert(noexcept(add_noexcept(1, 2)));
static_assert(!noexcept(parse_int("1")));

int main() {
    assert(parse_int("123") == 123);
    assert(parse_int("-45") == -45);
    assert(parse_int("0") == 0);
    assert(parse_int("2147483647") == INT_MAX);
    assert(parse_int("-2147483648") == INT_MIN);

    auto expect_parse_error = [](std::string_view s, std::size_t pos) {
        try {
            parse_int(s);
        } catch (const ParseError& e) {
            return e.position() == pos && std::string(e.what()).size() > 0;
        } catch (...) {
        }
        return false;
    };
    assert(expect_parse_error("", 0));
    assert(expect_parse_error("-", 1));
    assert(expect_parse_error("12a4", 2));
    assert(expect_parse_error("x", 0));

    bool out_of_range = false;
    try { parse_int("99999999999"); } catch (const std::out_of_range&) { out_of_range = true; }
    assert(out_of_range);
    out_of_range = false;
    try { parse_int("2147483648"); } catch (const std::out_of_range&) { out_of_range = true; }
    assert(out_of_range);

    assert(safe_div(7, 2) == 3);
    bool div_threw = false;
    try { safe_div(1, 0); } catch (const std::domain_error&) { div_threw = true; }
    assert(div_threw);

    assert(try_parse_int("42") == 42);
    assert(!try_parse_int("4x2").has_value());
    assert(!try_parse_int("99999999999").has_value());

    std::cout << "OK\n";
}
