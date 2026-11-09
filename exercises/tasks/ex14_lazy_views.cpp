// Rozdzial 2, zadanie 4: take_while, drop_while, join, lenistwo
//
//   - leading_run:      poczatkowy ciag liczb dodatnich (take_while)
//   - skip_leading_zeros: wektor bez zer na poczatku (drop_while)
//   - flatten:          splaszcza vector<vector<int>> (std::views::join)
//   - evaluated_calls:  ile razy wywolano lambde transform, gdy potok jest tylko
//                       zbudowany (0) i gdy odczytano pierwszy element (1) - pokaz lenistwo.
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

std::vector<int> leading_run(const std::vector<int>& v) {
    // TODO
    std::vector<int> ans {};
    auto postive_numbers =  v | std::views::take_while([](int e){return (e > 0);});
    for(auto i : postive_numbers){
        ans.push_back(i);
    }
    return ans;
}

std::vector<int> skip_leading_zeros(const std::vector<int>& v) {
    // TODO
    std::vector<int> ans{};
    auto withoutzeros = v | std::views::drop_while([](int e){return (e==0);});
    for(auto i: withoutzeros){
        ans.push_back(i);
    }
    return ans;
}

std::vector<int> flatten(const std::vector<std::vector<int>>& vv) {
    std::vector<int> out;
    for (int x : vv | std::views::join) out.push_back(x);
    return out;
}

struct LazyReport {
    int after_build;
    int after_first;
};

LazyReport evaluated_calls() {
    std::vector<int> v{1, 2, 3, 4, 5};
    int calls = 0;
    // TODO: zbuduj potok `v | std::views::transform(...)`, w lambdzie zwiekszaj `calls`,
    //       zapisz calls po zbudowaniu, odczytaj *view.begin(), zapisz calls ponownie.
    return {-1, -1};
}

int main() {
    assert((leading_run({3, 2, 1, -1, 5}) == std::vector<int>{3, 2, 1}));
    assert(leading_run({-1, 2}).empty());

    assert((skip_leading_zeros({0, 0, 1, 0, 2}) == std::vector<int>{1, 0, 2}));
    assert(skip_leading_zeros({0, 0}).empty());

    assert((flatten({{1, 2}, {}, {3}, {4, 5}}) == std::vector<int>{1, 2, 3, 4, 5}));
    assert(flatten({}).empty());

    auto r = evaluated_calls();
    assert(r.after_build == 0);
    assert(r.after_first == 1);

    std::cout << "OK\n";
}
