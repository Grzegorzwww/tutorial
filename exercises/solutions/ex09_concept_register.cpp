#include <concepts>
#include <cstdint>
#include <iostream>
#include <string>

template<typename T>
concept RegisterType = std::unsigned_integral<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);

template<RegisterType T>
void write_register(volatile T* reg, T value) {
    *reg = value;
}

template<RegisterType T>
void set_bits(volatile T* reg, T mask) {
    *reg = static_cast<T>(*reg | mask);
}

template<RegisterType T>
void clear_bits(volatile T* reg, T mask) {
    *reg = static_cast<T>(*reg & ~mask);
}

static_assert(RegisterType<std::uint8_t>);
static_assert(RegisterType<std::uint16_t>);
static_assert(RegisterType<std::uint32_t>);
static_assert(!RegisterType<std::uint64_t>);
static_assert(!RegisterType<int>);
static_assert(!RegisterType<std::int16_t>);
static_assert(!RegisterType<float>);
static_assert(!RegisterType<std::string>);

template<typename T>
concept CanWrite = requires(volatile T* p, T v) { write_register(p, v); };
static_assert(CanWrite<std::uint32_t>);
static_assert(!CanWrite<std::uint64_t>);
static_assert(!CanWrite<int>);

int main() {
    volatile std::uint16_t reg = 0;
    write_register(&reg, std::uint16_t{0x1234});
    if (reg != 0x1234) { std::cout << "FAIL write\n"; return 1; }

    set_bits(&reg, std::uint16_t{0x000F});
    if (reg != 0x123F) { std::cout << "FAIL set\n"; return 1; }

    clear_bits(&reg, std::uint16_t{0x0030});
    if (reg != 0x120F) { std::cout << "FAIL clear\n"; return 1; }

    volatile std::uint8_t reg8 = 0xFF;
    clear_bits(&reg8, std::uint8_t{0x0F});
    if (reg8 != 0xF0) { std::cout << "FAIL clear8\n"; return 1; }

    std::cout << "OK\n";
}
