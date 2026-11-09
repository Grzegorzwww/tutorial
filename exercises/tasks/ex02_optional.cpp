// Rozdzial 0, zadanie 2: std::optional
//
// 1) find_first_negative: indeks pierwszego elementu < 0 albo std::nullopt.
// 2) parse_digit: znak '0'..'9' -> wartosc 0..9, w przeciwnym razie std::nullopt.
// W C++11 zwrocilbys pewnie -1 jako "brak"; tu kazdy indeks (takze 0) jest poprawny.
#include <cassert>
#include <cstddef>
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

std::optional<std::size_t> find_first_negative(const std::vector<int>& v) {
    // TODO
    auto  it =  std::find_if(v.begin(), v.end(), []( int e) {return (e < 0);});

    if (it == v.end()){
        return std::nullopt;
    }else{
        size_t index = std::distance(v.begin(), it);
        return std::optional<std::size_t>(index);
    }

    
}

std::optional<int> parse_digit(char c) {
    std::optional<int> ans = std::nullopt;

    int value  = int(c) - int('0');
    if (value >= 0 && value<= 9) {
       ans = value;
    }
    std::cout <<"value = "<<value<<std::endl;
    return ans;
}

int main() {
    std::vector<int> mixed{1, 2, -3, 4, -5};
    auto a = find_first_negative(mixed);
    assert(a.has_value() && *a == 2);

    std::vector<int> first_negative{-1, 5};
    auto b = find_first_negative(first_negative);
    assert(b.has_value() && *b == 0);

    std::vector<int> none{0, 1, 2};
    assert(!find_first_negative(none).has_value());

    std::vector<int> empty;
    assert(!find_first_negative(empty).has_value());

    assert(parse_digit('7').value_or(-1) == 7);
    assert(parse_digit('0').value_or(-1) == 0);
    assert(!parse_digit('x').has_value());
    assert(!parse_digit('/').has_value());

    std::cout << "OK\n";
}
