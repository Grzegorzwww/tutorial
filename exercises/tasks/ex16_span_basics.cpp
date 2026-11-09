// Rozdzial 3, zadanie 1: jedna funkcja dla wszystkich buforow
//
//   - xor_checksum: XOR wszystkich bajtow (uint8_t)
//   - zero_out:     zeruje bufor (span bez const - modyfikuje oryginal)
//   - count_byte:   ile razy wystepuje bajt
// Kazda funkcja ma jedna sygnature, a dziala na tablicy C, std::array, vector i (ptr, size).
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

std::uint8_t xor_checksum(std::span<const std::uint8_t> data) {
    // TODO
    return 0;
}

void zero_out(std::span<std::uint8_t> data) {
    // TODO
}

std::size_t count_byte(std::span<const std::uint8_t> data, std::uint8_t value) {
    // TODO
    return 0;
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
