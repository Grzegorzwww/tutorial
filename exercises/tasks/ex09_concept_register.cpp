// Rozdzial 1, zadanie 4: embedded - rejestry
//
// RegisterType: typ calkowity bez znaku o rozmiarze 1, 2 lub 4 bajty.
// Ogranicz nim write_register, set_bits i clear_bits.
//
// Uwaga: w C++20 `*reg |= mask` na zmiennej volatile jest deprecated,
// wiec zapisz jawnie: *reg = static_cast<T>(*reg | mask).
#include <concepts>
#include <cstdint>
#include <iostream>
#include <string>

template<typename T>
concept RegisterType = (sizeof(T) == 1 ||  sizeof(T) == 2 ||  sizeof(T) == 4) && std::unsigned_integral<T>; // TODO

template<RegisterType T>  // TODO: ogranicz koncepcja
void write_register(volatile T  * reg, T  value) {
    *reg = value;
}

template<RegisterType T>  // TODO: ogranicz koncepcja
void set_bits(volatile T* reg, T mask) {
    // TODO
    *reg = *reg | mask;

}

template<RegisterType T>  // TODO: ogranicz koncepcja
void clear_bits(volatile T* reg, T mask) {
    *reg = *reg &  ~mask;
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
