#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> split(std::string_view s, char delim) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true) {
        auto pos = s.find(delim, start);
        if (pos == std::string_view::npos) {
            parts.emplace_back(s.substr(start));
            return parts;
        }
        parts.emplace_back(s.substr(start, pos - start));
        start = pos + 1;
    }
}

std::string join(const std::vector<std::string>& parts, std::string_view sep) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) out += sep;
        out += parts[i];
    }
    return out;
}

std::string to_upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool is_palindrome(std::string_view s) {
    std::size_t left = 0, right = s.size();
    while (left < right) {
        if (!std::isalnum(static_cast<unsigned char>(s[left]))) { ++left; continue; }
        if (!std::isalnum(static_cast<unsigned char>(s[right - 1]))) { --right; continue; }
        if (std::tolower(static_cast<unsigned char>(s[left])) !=
            std::tolower(static_cast<unsigned char>(s[right - 1]))) {
            return false;
        }
        ++left;
        --right;
    }
    return true;
}

std::string caesar(std::string_view s, int shift) {
    shift = ((shift % 26) + 26) % 26;
    std::string out(s);
    for (char& c : out) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>('a' + (c - 'a' + shift) % 26);
        else if (c >= 'A' && c <= 'Z') c = static_cast<char>('A' + (c - 'A' + shift) % 26);
    }
    return out;
}

std::string replace_all(std::string s, std::string_view from, std::string_view to) {
    if (from.empty()) return s;
    std::size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();  // przeskocz wstawiony tekst - inaczej mozliwa nieskonczona petla
    }
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
