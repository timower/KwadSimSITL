#include "packets.h"

namespace kn = kissnet;

namespace {
std::string string_to_hex(const std::string& input) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        if (i % 4 == 0) output.push_back(' ');
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}
}  // namespace

const InitPacket getInitPacket(kn::udp_socket& recv_socket) {
    std::array<std::byte, 1024> buf;
    auto [len, no_error] = recv_socket.recv(buf);

    // std::string str(reinterpret_cast<const char*>(&buf[0]), len);
    // fmt::print("Got: {}\n", string_to_hex(str));

    assert(no_error && "Error recv init packet");

    return get<InitPacket>(&buf[0], len);
}

std::optional<StatePacket> getStatePacket(kn::udp_socket& recv_socket) {
    std::array<std::byte, 1024> buf;
    auto [len, no_error] = recv_socket.recv(buf);
    assert(no_error && "Error recv state packet");

    std::string str(reinterpret_cast<const char*>(&buf[0]), len);
    if (str == "STOP") {
        return std::nullopt;
    }
    // fmt::print("Got: {}\n", string_to_hex(str));

    return get<StatePacket>(&buf[0], len);
}