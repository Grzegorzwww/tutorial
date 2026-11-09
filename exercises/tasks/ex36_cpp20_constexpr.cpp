// Zadanie 36 (C++20): constexpr, consteval, constinit
//
// Wszystko ponizej ma byc policzone w czasie kompilacji (static_assert).
//   factorial(n):        iteracyjnie; factorial(0) == 1
//   is_prime(n):         dla n < 2 -> false
//   kib(n):              consteval - n * 1024 (nie da sie wywolac z wartoscia z runtime)
//   make_squares():      tablica {0, 1, 4, 9, ... , 81}
//   make_crc8_table():   tablica 256 wpisow CRC-8 (wielomian 0x07, bez odbic):
//                        dla i: c = i; 8 razy: c = (c & 0x80) ? (c << 1) ^ 0x07 : c << 1  (obciete do 8 bitow)
//   crc8(data):          crc = 0; dla kazdego bajtu: crc = table[crc ^ bajt]
//   boot_counter:        zmienna globalna z constinit, wartosc poczatkowa 42
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>

constexpr int factorial(int n) {
    // TODO
    return 0;
}

constexpr bool is_prime(unsigned n) {
    // TODO
    return false;
}

consteval std::size_t kib(std::size_t n) {
    // TODO
    return 0;
}

constexpr std::array<int, 10> make_squares() {
    std::array<int, 10> a{};
    // TODO
    return a;
}

constexpr std::array<std::uint8_t, 256> make_crc8_table() {
    std::array<std::uint8_t, 256> table{};
    // TODO
    return table;
}

inline constexpr auto crc8_table = make_crc8_table();

constexpr std::uint8_t crc8(std::span<const std::uint8_t> data) {
    // TODO
    return 0;
}

int boot_counter = 0;  // TODO: constinit + wartosc 42

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
