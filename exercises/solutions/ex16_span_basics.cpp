#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

std::uint8_t xor_checksum(std::span<const std::uint8_t> data) {
    std::uint8_t crc = 0;
    for (std::uint8_t b : data) crc ^= b;
    return crc;
}

void zero_out(std::span<std::uint8_t> data) {
    for (std::uint8_t& b : data) b = 0;
}

std::size_t count_byte(std::span<const std::uint8_t> data, std::uint8_t value) {
    return static_cast<std::size_t>(std::count(data.begin(), data.end(), value));
}

int main() {
    std::uint8_t c_arr[] = {1, 2, 3, 4, 5};
    std::array<std::uint8_t, 5> std_arr{1, 2, 3, 4, 3};
    std::vector<std::uint8_t> vec{1, 2, 3, 4, 2};
    std::uint8_t* ptr = c_arr;

    assert(xor_checksum(c_arr) == 1);
    assert(xor_checksum(std_arr) == 7);
    assert(xor_checksum(vec) == 6);
    assert(xor_checksum(std::span<const std::uint8_t>(ptr, 2)) == 3);
    assert(xor_checksum({}) == 0);

    assert(count_byte(std_arr, 3) == 2);
    assert(count_byte(vec, 9) == 0);

    zero_out(vec);
    assert((vec == std::vector<std::uint8_t>{0, 0, 0, 0, 0}));
    zero_out(std::span<std::uint8_t>(ptr, 2));  // tylko dwa pierwsze
    assert(c_arr[0] == 0 && c_arr[1] == 0 && c_arr[2] == 3);

    std::cout << "OK\n";
}
