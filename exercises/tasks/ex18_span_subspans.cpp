// Rozdzial 3, zadanie 3: first / last / subspan
//
//   - split_chunks:    dzieli dane na kawalki po `n` elementow (ostatni moze byc krotszy),
//                      zwraca widoki na oryginalne dane
//   - without_ends:    bez pierwszego i ostatniego elementu (pusty span gdy size < 3)
//   - find_subspan:    widok od pierwszego wystapienia `value` do konca; pusty gdy brak
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <vector>

std::vector<std::span<const int>> split_chunks(std::span<const int> data, std::size_t n) {
    // TODO: n > 0; uzyj subspan(offset, std::min(n, data.size() - offset))
    return {};
}

std::span<const int> without_ends(std::span<const int> data) {
    // TODO
    return data;
}

std::span<const int> find_subspan(std::span<const int> data, int value) {
    // TODO
    return data;
}

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};

    auto chunks = split_chunks(v, 3);
    assert(chunks.size() == 3);
    assert(chunks[0].size() == 3 && chunks[0][0] == 1);
    assert(chunks[1].size() == 3 && chunks[1][0] == 4);
    assert(chunks[2].size() == 1 && chunks[2][0] == 7);
    assert(chunks[1].data() == v.data() + 3);
    assert(split_chunks({}, 4).empty());

    auto mid = without_ends(v);
    assert(mid.size() == 5 && mid.front() == 2 && mid.back() == 6);
    assert(without_ends(std::span<const int>(v).first(2)).empty());

    auto from4 = find_subspan(v, 4);
    assert(from4.size() == 4 && from4[0] == 4);
    assert(find_subspan(v, 99).empty());

    std::cout << "OK\n";
}
