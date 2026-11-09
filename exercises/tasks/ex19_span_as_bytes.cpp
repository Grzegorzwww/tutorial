// Rozdzial 3, zadanie 4: as_bytes
//
//   - frame_bytes: bajtowy widok na SensorFrame (std::as_bytes na span jednego elementu)
//   - to_hex:      "0A FF 00" - bajty jako hex wielkimi literami, rozdzielone spacja
//   - read_u16_le: odczyt uint16_t z dwoch pierwszych bajtow w kolejnosci little-endian,
//                  niezaleznie od endianowosci platformy (nie rzutuj wskaznika!)
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>

struct SensorFrame {
    std::uint32_t timestamp;
    float temperature;
    float humidity;
};

std::span<const std::byte> frame_bytes(const SensorFrame& frame) {
    // TODO
    return {};
}

std::string to_hex(std::span<const std::byte> bytes) {
    // TODO
    return "?";
}

std::uint16_t read_u16_le(std::span<const std::byte> bytes) {
    // TODO
    return 0;
}

int main() {
    SensorFrame f{0x01020304u, 1.5f, 2.5f};
    auto bytes = frame_bytes(f);
    assert(bytes.size() == sizeof(SensorFrame));
    assert(static_cast<const void*>(bytes.data()) == static_cast<const void*>(&f));

    if constexpr (std::endian::native == std::endian::little) {
        assert(bytes[0] == std::byte{0x04} && bytes[3] == std::byte{0x01});
    } else {
        assert(bytes[0] == std::byte{0x01} && bytes[3] == std::byte{0x04});
    }

    const std::byte raw[] = {std::byte{0x0A}, std::byte{0xFF}, std::byte{0x00}};
    assert(to_hex(raw) == "0A FF 00");
    assert(to_hex({}).empty());

    assert(read_u16_le(raw) == 0xFF0A);

    std::cout << "OK\n";
}
