// Rozdzial 0, zadanie 1: if constexpr
//
// Zaimplementuj to_string_debug() jedna funkcja szablonowa, bez enable_if:
//   - bool                -> "true" / "false"
//   - typ arytmetyczny    -> std::to_string(value)
//   - cokolwiek innego    -> std::string(value)  (std::string, literal tekstowy)
// Uwaga: bool tez jest typem arytmetycznym - kolejnosc gałęzi ma znaczenie.
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

template<typename T>
std::string to_string_debug(const T& value) {

    if constexpr (std::is_integral_v<T>){
        if constexpr (std::is_same_v<T, bool>){
            if (value) {
                return "true";
            }else {
                 return "false";   
            }
        }
        else if constexpr (std::is_same_v<T, int>){
            return std::to_string(value);
        }
         else if constexpr (std::is_same_v<T, long int>){
            return std::to_string(value);
        }
    }else {
        if constexpr (std::is_same_v<T, std::string>){
            return value;
        }else{
            return std::string(value);
        }
    }

    // TODO

    return "";
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
