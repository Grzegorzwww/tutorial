#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

template<typename T>
std::string to_string_debug(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else {
        return std::string(value);
    }
}

int main() {
    assert(to_string_debug(42) == "42");
    assert(to_string_debug(-7L) == "-7");
    assert(to_string_debug(true) == "true");
    assert(to_string_debug(false) == "false");
    assert(to_string_debug(std::string("abc")) == "abc");
    assert(to_string_debug("literal") == "literal");
    std::cout << "OK\n";
}
