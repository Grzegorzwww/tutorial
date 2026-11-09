// Rozdzial 0, zadanie 3: structured bindings
//
// Uzyj structured bindings w obu funkcjach (w petli po mapie: `const auto& [key, value]`).
//   - min_max: najmniejszy i najwiekszy element (wektor jest niepusty)
//   - sum_values: suma wartosci w mapie
//   - key_of_max: klucz o najwiekszej wartosci ("" dla pustej mapy)
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct MinMax {
    int min;
    int max;
};

MinMax min_max(const std::vector<int>& v) {
    // TODO
    MinMax ans {};
    if (!v.empty()) {
        ans.min = ans.max = v[0];
        for (const auto& e : v) {
            if (e < ans.min) ans.min = e;
            if (e > ans.max) ans.max = e;
        }
    }
    std::cout <<  ans.min<<" "<<ans.max<<std::endl;
    return ans;
}

int sum_values(const std::map<std::string, int>& m) {
    // TODO
    int sum = 0;
    for(const auto & [key,value]: m){
        sum = sum + value;
    }
    return sum;
}

std::string key_of_max(const std::map<std::string, int>& m) {
    
    std::string ans_key {};
    int temp = INT_MIN;
    for(const auto & [key, value]: m){
        if(value > temp) {
            temp = value;
            ans_key = key;
        }
    }
    return ans_key;
}

int main() {
    std::vector<int> v{4, -2, 9, 0};
    auto [lo, hi] = min_max(v);
    assert(lo == -2);
    assert(hi == 9);

    std::map<std::string, int> m{{"a", 1}, {"b", 5}, {"c", 3}};
    assert(sum_values(m) == 9);
    assert(key_of_max(m) == "b");

    std::map<std::string, int> empty;
    assert(sum_values(empty) == 0);
    assert(key_of_max(empty).empty());

    std::cout << "OK\n";
}
