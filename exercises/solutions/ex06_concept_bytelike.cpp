#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>

template<typename T>
concept ByteLike = (std::integral<T> && !std::same_as<T, bool> && sizeof(T) == 1) || std::same_as<T, std::byte>;

template<ByteLike T>
std::size_t count_zero(const T* data, std::size_t n) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (data[i] == T{}) ++count;
    }
    return count;
}

static_assert(ByteLike<std::uint8_t>);
static_assert(ByteLike<std::int8_t>);
static_assert(ByteLike<char>);
static_assert(ByteLike<std::byte>);
static_assert(!ByteLike<bool>);
static_assert(!ByteLike<int>);
static_assert(!ByteLike<std::uint16_t>);
static_assert(!ByteLike<float>);

template<typename T>
concept CanCount = requires(const T* p) { count_zero(p, std::size_t{1}); };
static_assert(CanCount<std::uint8_t>);
static_assert(!CanCount<int>);

int main() {
    std::uint8_t a[] = {0, 1, 0, 2};
    std::byte b[] = {std::byte{0}, std::byte{5}};
    if (count_zero(a, 4) != 2 || count_zero(b, 2) != 1) {
        std::cout << "FAIL\n";
        return 1;
    }
    std::cout << "OK\n";
}
