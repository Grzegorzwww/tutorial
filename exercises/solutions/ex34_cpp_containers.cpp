#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

std::map<std::string, int> word_frequency(const std::string& text) {
    std::map<std::string, int> freq;
    std::istringstream in(text);
    std::string word;
    while (in >> word) {
        std::transform(word.begin(), word.end(), word.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        ++freq[word];
    }
    return freq;
}

std::vector<std::pair<std::string, int>> top_n(const std::map<std::string, int>& freq, std::size_t n) {
    std::vector<std::pair<std::string, int>> items(freq.begin(), freq.end());
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    if (items.size() > n) items.resize(n);
    return items;
}

std::vector<int> common_elements(const std::vector<int>& a, const std::vector<int>& b) {
    std::set<int> sa(a.begin(), a.end());
    std::set<int> sb(b.begin(), b.end());
    std::vector<int> out;
    std::set_intersection(sa.begin(), sa.end(), sb.begin(), sb.end(), std::back_inserter(out));
    return out;
}

std::optional<int> first_duplicate(const std::vector<int>& v) {
    std::unordered_set<int> seen;
    for (int x : v) {
        if (!seen.insert(x).second) return x;
    }
    return std::nullopt;
}

std::map<std::size_t, std::vector<std::string>> group_by_length(const std::vector<std::string>& words) {
    std::map<std::size_t, std::vector<std::string>> groups;
    for (const auto& w : words) groups[w.size()].push_back(w);
    return groups;
}

int main() {
    auto freq = word_frequency("the cat the dog The  end");
    assert(freq.size() == 4);
    assert(freq["the"] == 3 && freq["cat"] == 1 && freq["dog"] == 1 && freq["end"] == 1);
    assert(word_frequency("").empty());

    using Pairs = std::vector<std::pair<std::string, int>>;
    assert((top_n(freq, 2) == Pairs{{"the", 3}, {"cat", 1}}));
    assert(top_n(freq, 10).size() == 4);
    assert(top_n(freq, 0).empty());

    assert((common_elements({1, 2, 2, 3, 5}, {2, 3, 3, 4}) == std::vector<int>{2, 3}));
    assert(common_elements({1, 2}, {3, 4}).empty());
    assert(common_elements({}, {1}).empty());

    assert(first_duplicate({1, 2, 3, 2, 1}) == 2);
    assert(!first_duplicate({1, 2, 3}).has_value());
    assert(!first_duplicate({}).has_value());

    auto groups = group_by_length({"a", "bb", "c", "dd", "eee"});
    assert(groups.size() == 3);
    assert((groups[1] == std::vector<std::string>{"a", "c"}));
    assert((groups[2] == std::vector<std::string>{"bb", "dd"}));
    assert((groups[3] == std::vector<std::string>{"eee"}));

    std::cout << "OK\n";
}
