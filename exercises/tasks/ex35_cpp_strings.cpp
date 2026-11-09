// Zadanie 35 (C++): przetwarzanie napisow
//
//   split:         dzieli po znaku, ZACHOWUJE puste tokeny: "a,,b" -> {"a","","b"}, "" -> {""}
//   join:          laczy z separatorem
//   to_upper:      wielkie litery (ASCII); pamietaj o rzutowaniu na unsigned char dla std::toupper
//   is_palindrome: ignoruje znaki nie-alfanumeryczne i wielkosc liter; "" jest palindromem
//   caesar:        szyfr Cezara tylko dla liter (zachowuje wielkosc), shift moze byc ujemny/duzy
//   replace_all:   zamienia wszystkie wystapienia `from` na `to`; pusty `from` -> bez zmian.
//                  Uwaga na nieskonczona petle, gdy `to` zawiera `from`.
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> split(std::string_view s, char delim) {
    // TODO
    return {};
}

std::string join(const std::vector<std::string>& parts, std::string_view sep) {
    // TODO
    return "?";
}

std::string to_upper(std::string s) {
    // TODO
    return s;
}

bool is_palindrome(std::string_view s) {
    // TODO
    return false;
}

std::string caesar(std::string_view s, int shift) {
    // TODO
    return std::string(s);
}

std::string replace_all(std::string s, std::string_view from, std::string_view to) {
    // TODO
    return s;
}

int main() {
    using V = std::vector<std::string>;
    assert((split("a,b,c", ',') == V{"a", "b", "c"}));
    assert((split("a,,b", ',') == V{"a", "", "b"}));
    assert((split("", ',') == V{""}));
    assert((split(",", ',') == V{"", ""}));

    assert(join({"a", "b", "c"}, ", ") == "a, b, c");
    assert(join({"x"}, ",") == "x");
    assert(join({}, ",").empty());

    assert(to_upper("Hello, World 1") == "HELLO, WORLD 1");

    assert(is_palindrome("A man, a plan, a canal: Panama"));
    assert(is_palindrome(""));
    assert(is_palindrome("x"));
    assert(!is_palindrome("hello"));

    assert(caesar("Hello, World!", 3) == "Khoor, Zruog!");
    assert(caesar("abc", -1) == "zab");
    assert(caesar("xyz", 29) == "abc");
    assert(caesar("Khoor, Zruog!", -3) == "Hello, World!");

    assert(replace_all("aaa", "a", "bb") == "bbbbbb");
    assert(replace_all("abcabc", "bc", "") == "aa");
    assert(replace_all("abc", "", "x") == "abc");
    assert(replace_all("abc", "z", "y") == "abc");
    assert(replace_all("ab", "ab", "abab") == "abab");

    std::cout << "OK\n";
}
