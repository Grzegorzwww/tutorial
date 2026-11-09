#include <concepts>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<Numeric T>
double average(const std::vector<T>& v) {
    if (v.empty()) return 0.0;
    double sum = 0.0;
    for (T x : v) sum += static_cast<double>(x);
    return sum / static_cast<double>(v.size());
}

template<Addable T>
T sum_all(const std::vector<T>& v) {
    T total{};
    for (const T& x : v) total = total + x;
    return total;
}

struct NoPlus {};

static_assert(Numeric<int>);
static_assert(Numeric<double>);
static_assert(!Numeric<std::string>);

static_assert(Addable<int>);
static_assert(Addable<std::uint8_t>);
static_assert(Addable<std::string>);
static_assert(!Addable<NoPlus>);

template<typename T>
concept CanAverage = requires(const std::vector<T>& v) { average(v); };
static_assert(CanAverage<int>);
static_assert(!CanAverage<std::string>);

int main() {
    std::vector<int> ints{1, 2, 3, 4};
    std::vector<double> doubles{1.0, 2.0};
    std::vector<int> empty;
    std::vector<std::string> words{"a", "b", "c"};

    if (average(ints) != 2.5 || average(doubles) != 1.5 || average(empty) != 0.0 ||
        sum_all(ints) != 10 || sum_all(words) != "abc") {
        std::cout << "FAIL\n";
        return 1;
    }
    std::cout << "OK\n";
}
