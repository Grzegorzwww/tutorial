#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <vector>

std::vector<std::span<const int>> split_chunks(std::span<const int> data, std::size_t n) {
    std::vector<std::span<const int>> chunks;
    for (std::size_t offset = 0; offset < data.size(); offset += n) {
        chunks.push_back(data.subspan(offset, std::min(n, data.size() - offset)));
    }
    return chunks;
}

std::span<const int> without_ends(std::span<const int> data) {
    if (data.size() < 3) return {};
    return data.subspan(1, data.size() - 2);
}

std::span<const int> find_subspan(std::span<const int> data, int value) {
    auto it = std::find(data.begin(), data.end(), value);
    if (it == data.end()) return {};
    return data.subspan(static_cast<std::size_t>(it - data.begin()));
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
