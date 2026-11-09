// Rozdzial 3, zadanie 2: span o stalym rozmiarze i podzial pakietu
//
// Pakiet ma dokladnie 10 bajtow: header (2) | payload (7) | crc (1).
// parse_packet zwraca trzy widoki na ten sam bufor (bez kopiowania).
// verify_crc: crc == XOR bajtow header + payload.
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
    // TODO: zastap first<7>() i first<1>() poprawnymi: subspan<2, 7>() i last<1>()
    return {packet.first<2>(), packet.first<7>(), packet.first<1>()};
}

bool verify_crc(std::span<const std::uint8_t, 10> packet) {
    // TODO
    return false;
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
