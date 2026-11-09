// Rozdzial 1, zadanie 5: z enable_if na concepts
//
// Stary kod C++11:
//   template<typename T, typename = typename std::enable_if<
//       std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
//   double average(const std::vector<T>& v);
//
// 1) Numeric = integral lub floating_point. average(): srednia (0.0 dla pustego wektora).
// 2) Addable: wyrazenie a + b jest poprawne i konwertuje sie do T (requires z `->`).
//    sum_all(): suma elementow zaczynajac od T{} (dziala tez dla std::string).
#include <concepts>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;  // TODO

template<typename T>
concept Addable = requires(T a, T b) {{a + b} -> std::convertible_to<T>;};

template<Numeric T>  // TODO: ogranicz Numeric
double average(const std::vector<T>& v) {
    T sum = T{};
    for(auto i: v){
        sum += i;
    }
    if(v.empty()){
        return 0.0;
    }
    return static_cast<double>(sum) / v.size();
}

template<Addable T>  // TODO: ogranicz Addable
T sum_all(const std::vector<T>& v) {
    if(v.empty()){
        return T{};
    }
    T sum {};
    
    for(auto i : v){
        sum = sum + i;
    }
    
    return static_cast<T>(sum);
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
