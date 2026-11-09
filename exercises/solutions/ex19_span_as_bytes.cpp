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
    return std::as_bytes(std::span<const SensorFrame, 1>(&frame, 1));
}

std::string to_hex(std::span<const std::byte> bytes) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string out;
    for (std::byte b : bytes) {
        if (!out.empty()) out += ' ';
        auto v = std::to_integer<unsigned>(b);
        out += digits[v >> 4];
        out += digits[v & 0xF];
    }
    return out;
}

std::uint16_t read_u16_le(std::span<const std::byte> bytes) {
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(bytes[0]) |
                                      (std::to_integer<unsigned>(bytes[1]) << 8));
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
