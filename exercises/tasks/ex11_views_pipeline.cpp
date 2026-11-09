// Rozdzial 2, zadanie 1: filter + transform + take
//
// Z wektora 1..20 wybierz liczby podzielne przez 3, podnies je do kwadratu
// i zwroc tylko pierwsze `count` wynikow - jednym potokiem views, bez
// tymczasowych kontenerow (do vector kopiujesz dopiero na koncu).
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

std::vector<int> first_squares_of_multiples_of_3(const std::vector<int>& v, int count) {
    std::vector<int> out;

    auto pipelline = v | std::views::filter([](int e ) {return (e % 3 == 0);})
     | std::views::transform([](int e) {return e * e;})
     | std::views::take(count);


    if(!pipelline.empty()){
        for (int i : pipelline){
        std::cout <<i <<" ";
        out.push_back(i);
    }    
    }
    

    return out;
}

int main() {
    std::vector<int> v;
    for (int i : std::views::iota(1, 21)) v.push_back(i);

    assert((first_squares_of_multiples_of_3(v, 3) == std::vector<int>{9, 36, 81}));
    assert((first_squares_of_multiples_of_3(v, 100) == std::vector<int>{9, 36, 81, 144, 225, 324}));
    assert(first_squares_of_multiples_of_3(v, 0).empty());
    assert(first_squares_of_multiples_of_3({1, 2, 4}, 3).empty());

    std::cout << "OK\n";
}
