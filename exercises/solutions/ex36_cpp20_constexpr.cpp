#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>

constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

constexpr bool is_prime(unsigned n) {
    if (n < 2) return false;
    for (unsigned d = 2; d * d <= n; ++d) {
        if (n % d == 0) return false;
    }
    return true;
}

consteval std::size_t kib(std::size_t n) {
    return n * 1024;
}

constexpr std::array<int, 10> make_squares() {
    std::array<int, 10> a{};
    for (int i = 0; i < 10; ++i) a[static_cast<std::size_t>(i)] = i * i;
    return a;
}

constexpr std::array<std::uint8_t, 256> make_crc8_table() {
    std::array<std::uint8_t, 256> table{};
    for (unsigned i = 0; i < 256; ++i) {
        auto c = static_cast<std::uint8_t>(i);
        for (int bit = 0; bit < 8; ++bit) {
            c = (c & 0x80) ? static_cast<std::uint8_t>((c << 1) ^ 0x07) : static_cast<std::uint8_t>(c << 1);
        }
        table[i] = c;
    }
    return table;
}

inline constexpr auto crc8_table = make_crc8_table();

constexpr std::uint8_t crc8(std::span<const std::uint8_t> data) {
    std::uint8_t crc = 0;
    for (std::uint8_t b : data) crc = crc8_table[crc ^ b];
    return crc;
}

constinit int boot_counter = 42;

static_assert(factorial(0) == 1);
static_assert(factorial(5) == 120);
static_assert(factorial(10) == 3628800);

static_assert(is_prime(2) && is_prime(97) && is_prime(7919));
static_assert(!is_prime(0) && !is_prime(1) && !is_prime(91));

static_assert(kib(4) == 4096);
inline constexpr std::size_t buffer_size = kib(2);
static_assert(buffer_size == 2048);

inline constexpr auto squares = make_squares();
static_assert(squares[0] == 0 && squares[3] == 9 && squares[9] == 81);

static_assert(crc8_table[0] == 0x00 && crc8_table[1] == 0x07 && crc8_table[255] == 0xF3);
inline constexpr std::array<std::uint8_t, 9> check_msg{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
static_assert(crc8(check_msg) == 0xF4);
static_assert(crc8({}) == 0);

int main() {
    volatile int n = 6;  // te same funkcje dzialaja tez w runtime
    assert(factorial(n) == 720);
    assert(is_prime(13));

    assert(boot_counter == 42);
    ++boot_counter;
    assert(boot_counter == 43);

    std::cout << "OK\n";
}
