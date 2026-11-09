// Rozdzial 0, zadanie 4: std::variant i std::visit
//
//   - describe: "int:42", "bool:true", "string:abc"  (uzyj std::visit)
//   - as_int:   wartosc, jesli variant trzyma int, inaczej nullopt (uzyj std::get_if)
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

using Value = std::variant<int, bool, std::string>;

std::string describe(const Value& v) {
    // TODO
    return std::visit([](const auto &val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
                return "int:" + std::to_string(val);
        }
        else if constexpr (std::is_same_v<T, bool>){
            std::cout << "bool:"<<(val ? "true": "false")<<std::endl;
            return std::string("bool:") + (val ? "true": "false");
        }
        else if constexpr (std::is_same_v<T, std::string>){
            std::cout << "string:"<<val<<std::endl;
            return "string:" + val;
            
        }
    }, v);

    return "";
}

std::optional<int> as_int(const Value& v) {
    const int* p  = std::get_if<int>(&v);
    if(p)
        return *p;
    else 
    return std::nullopt;
}

int main() {
    assert(describe(Value{42}) == "int:42");
    assert(describe(Value{true}) == "bool:true");
    assert(describe(Value{false}) == "bool:false");
    assert(describe(Value{std::string("abc")}) == "string:abc");

    assert(as_int(Value{7}).value_or(-1) == 7);
    assert(!as_int(Value{true}).has_value());
    assert(!as_int(Value{std::string("7")}).has_value());

    std::cout << "OK\n";
}
