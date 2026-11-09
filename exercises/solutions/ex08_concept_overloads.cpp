#include <array>
#include <concepts>
#include <cstddef>
#include <forward_list>
#include <iostream>
#include <string>
#include <vector>

template<typename T>
concept Container = requires(const T& t) {
    t.begin();
    t.end();
    { t.size() } -> std::convertible_to<std::size_t>;
};

std::string describe(const auto&) {
    return "scalar";
}

std::string describe(const Container auto& x) {
    return "container:" + std::to_string(x.size());
}

struct OnlySize {
    std::size_t size() const { return 0; }
};

static_assert(Container<std::vector<int>>);
static_assert(Container<std::array<int, 2>>);
static_assert(Container<std::string>);
static_assert(!Container<int>);
static_assert(!Container<OnlySize>);
static_assert(!Container<std::forward_list<int>>);

int main() {
    std::vector<int> v{1, 2, 3};
    std::array<int, 4> arr{};
    if (describe(v) != "container:3" || describe(arr) != "container:4" ||
        describe(std::string("ab")) != "container:2" || describe(42) != "scalar" ||
        describe(OnlySize{}) != "scalar") {
        std::cout << "FAIL\n";
        return 1;
    }
    std::cout << "OK\n";
}
