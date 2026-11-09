#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

using Value = std::variant<int, bool, std::string>;

std::string describe(const Value& v) {
    return std::visit(
        [](const auto& x) -> std::string {
            using T = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<T, int>) {
                return "int:" + std::to_string(x);
            } else if constexpr (std::is_same_v<T, bool>) {
                return std::string("bool:") + (x ? "true" : "false");
            } else {
                return "string:" + x;
            }
        },
        v);
}

std::optional<int> as_int(const Value& v) {
    if (const int* p = std::get_if<int>(&v)) return *p;
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
