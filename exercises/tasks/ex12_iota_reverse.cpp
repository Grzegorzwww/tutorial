// Rozdzial 2, zadanie 2: iota, reverse, drop, take
//
//   - sum_range(a, b):   suma liczb z polotwartego zakresu [a, b) przez std::views::iota
//   - countdown(n):      wektor n, n-1, ..., 1 (iota + reverse)
//   - middle(v):         elementy bez pierwszego i ostatniego (drop + reverse + drop + reverse,
//                        bo w C++20 nie ma drop_last)
// Wskazowka: `v | std::views::drop(1)` zwraca widok, nie kopie.
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

long sum_range(int a, int b) {
    // TODO
    int sum {0};
    for(int i: std::views::iota(a, b))
        sum += i;

    return sum;
}

std::vector<int> countdown(int n) {
    std::vector<int> ans{};
    if(n == 0){
        return ans;
    }
    auto reversed = std::views::iota(1, n+1) | std::views::reverse;
    for(int i: reversed){
        ans.push_back(i);
        // std::cout << i<<" ";
    }

    return ans;
}

std::vector<int> middle(const std::vector<int>& v) {
    std::vector<int> ans{};
    auto temp = v | std::views::drop(1)  | std::views::reverse  | std::views::drop(1) | std::views::reverse;
    for(auto i: temp){
        ans.push_back(i);
        std::cout << i<< ", ";
    }

    return ans;
}

int main() {
    assert(sum_range(1, 6) == 15);
    assert(sum_range(5, 5) == 0);
    assert(sum_range(-2, 3) == 0);

    assert((countdown(4) == std::vector<int>{4, 3, 2, 1}));
    assert(countdown(0).empty());

    assert((middle({1, 2, 3, 4, 5}) == std::vector<int>{2, 3, 4}));
    assert((middle({1, 2, 3}) == std::vector<int>{2}));
    assert(middle({1, 2}).empty());
    assert(middle({1}).empty());
    assert(middle({}).empty());

    std::cout << "OK\n";
}
