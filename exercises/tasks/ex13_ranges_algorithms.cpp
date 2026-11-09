// Rozdzial 2, zadanie 3: algorytmy std::ranges::
//
// Uzyj wersji z std::ranges:: (bez begin()/end()) i projekcji:
//   - sort_by_length:     sortuje stringi rosnaco po dlugosci (ranges::stable_sort + projekcja)
//   - index_of:           indeks pierwszego wystapienia wartosci albo -1 (ranges::find)
//   - all_positive:       czy wszystkie elementy > 0 (ranges::all_of)
//   - longest:            najdluzszy napis; przy remisie pierwszy; "" dla pustego
//                         (ranges::max_element z projekcja - zwraca pierwszy najwiekszy)
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

void sort_by_length(std::vector<std::string>& v) {
    // TODO
    std::ranges::stable_sort(v, [](std::string a, std::string b) { return a.length() < b.length(); } );
    // for (auto i : v){
    //     std::cout << i <<std::endl;
    // }

}   

int index_of(const std::vector<int>& v, int value) {
    // TODO
    auto it = std::ranges::find(v, value);
    if(it != v.end()){
        return std::distance(v.begin(), it);

    }
    return -1;
}

bool all_positive(const std::vector<int>& v) {
    // TODO

    return std::ranges::all_of(v.begin(), v.end(), [](int e) {return (e >0 );});
}

std::string longest(const std::vector<std::string>& v) {
    // TODO
    if(v.empty()){
        return "";
    }
    
    auto ans  = std::ranges::max_element(v, [](std::string e1, std::string e2 ) {return (e1.length() < e2.length());} );

    return *ans;
}

int main() {
    std::vector<std::string> words{"ccc", "a", "bb", "dd", "e"};
    sort_by_length(words);
    assert((words == std::vector<std::string>{"a", "e", "bb", "dd", "ccc"}));  // stabilnie

    assert(index_of({5, 3, 1, 3}, 3) == 1);
    assert(index_of({5, 3, 1}, 9) == -1);
    assert(index_of({}, 1) == -1);

    assert(all_positive({1, 2, 3}));
    assert(!all_positive({1, 0, 3}));
    assert(all_positive({}));

    assert(longest({"aa", "bbb", "ccc", "d"}) == "bbb");
    assert(longest({}).empty());

    std::cout << "OK\n";
}
