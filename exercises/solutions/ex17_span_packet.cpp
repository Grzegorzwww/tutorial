#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <type_traits>

struct ParsedPacket {
    std::span<const std::uint8_t, 2> header;
    std::span<const std::uint8_t, 7> payload;
    std::span<const std::uint8_t, 1> crc;
};

ParsedPacket parse_packet(std::span<const std::uint8_t, 10> packet) {
    return {packet.first<2>(), packet.subspan<2, 7>(), packet.last<1>()};
}

bool verify_crc(std::span<const std::uint8_t, 10> packet) {
    std::uint8_t x = 0;
    for (std::uint8_t b : packet.first<9>()) x ^= b;
    return x == packet.last<1>()[0];
}

// Rozmiar sprawdzany w czasie kompilacji:
static_assert(std::is_convertible_v<std::array<std::uint8_t, 10>&, std::span<const std::uint8_t, 10>>);
static_assert(!std::is_convertible_v<std::array<std::uint8_t, 8>&, std::span<const std::uint8_t, 10>>);

int main() {
    std::array<std::uint8_t, 10> packet{0xAA, 0x01, 1, 2, 3, 4, 5, 6, 7, 0};
    std::uint8_t x = 0;
    for (int i = 0; i < 9; ++i) x ^= packet[i];
    packet[9] = x;

    auto p = parse_packet(packet);
    assert(p.header[0] == 0xAA && p.header[1] == 0x01);
    assert(p.payload.size() == 7 && p.payload[0] == 1 && p.payload[6] == 7);
    assert(p.crc[0] == x);
    assert(p.payload.data() == packet.data() + 2);  // to ten sam bufor, zero kopii
    assert(verify_crc(packet));

    packet[4] ^= 0xFF;
    assert(!verify_crc(packet));

    std::cout << "OK\n";
}
