#pragma once

#ifdef _WIN32
#include "winsock2.h"
#endif

#include "vector_math.h"

#include <array>
#include <cstdint>
#include <optional>
#include <tuple>

#include <fmt/format.h>
#include <kissnet.hpp>

namespace detail {
inline std::string string_to_hex(const std::string& input) {
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
}  // namespace detail

[[nodiscard]] inline bool advance(std::byte*& data,
                                  std::size_t& len,
                                  std::size_t n) {
    if (len >= n) {
        data += n;
        len -= n;
        return true;
    }
    return false;
}

template <typename T>
std::optional<T> parse(std::byte*& data, std::size_t& len) {
    T child;

    if (!child._parse(data, len)) {
        return std::nullopt;
    }

    return child;
}

template <uint32_t TypeID>
struct GodotT {
    uint32_t _typeId = TypeID;

    bool _parse(std::byte*& data, std::size_t& len) {
        const auto tid = reinterpret_cast<uint32_t*&>(data);
        if (((*tid) & 0xFFFF) != TypeID) {
            return false;
        }

        return advance(data, len, sizeof(uint32_t));
    }
};

struct BoolT : public GodotT<1> {
    uint32_t value = 0;

    BoolT() = default;
    BoolT(bool val) : value(val) {
    }

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;
        value = *reinterpret_cast<uint32_t*&>(data);

        return advance(data, len, sizeof(uint32_t));
    }

    operator bool() const {
        return value;
    }
};

struct FloatT : GodotT<3> {
    float value = 0.0f;

    FloatT() = default;
    FloatT(float val) : value(val) {
    }

    bool _parse(std::byte*& data, std::size_t& len) {
        const bool is64 = ((*reinterpret_cast<uint32_t*&>(data)) & 0xFFFF0000);
        if (!GodotT::_parse(data, len)) return false;

        if (!is64) {
            value = *reinterpret_cast<float*&>(data);
            return advance(data, len, sizeof(float));
        }

        value = *reinterpret_cast<double*&>(data);
        return advance(data, len, sizeof(double));
    }

    operator float() const {
        return value;
    }
};

static_assert(sizeof(FloatT) == 4 + 4);

struct Vec3T : public GodotT<7> {
    vmath::vec3 value;

    Vec3T() = default;
    Vec3T(const vmath::vec3& val) : value(val) {
    }

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        for (auto i = 0u; i < 3; i++) {
            value[i] = *reinterpret_cast<float*&>(data);
            if (!advance(data, len, sizeof(float))) return false;
        }

        return true;
    }
};

static_assert(sizeof(Vec3T) == 16);

struct BasisT : public GodotT<12> {
    vmath::mat3 value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        for (auto i = 0u; i < 3; i++) {
            for (auto j = 0u; j < 3; j++) {
                value[i][j] = *reinterpret_cast<float*&>(data);
                if (!advance(data, len, sizeof(float))) return false;
            }
        }

        return true;
    }
};

static_assert(sizeof(BasisT) == 40);

template <typename T, uint32_t Size>
struct ArrayT : public GodotT<19> {
    uint32_t _len = Size;
    std::array<T, Size> value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        _len = *reinterpret_cast<uint32_t*&>(data);
        if (_len != Size) return false;
        if (!advance(data, len, sizeof(uint32_t))) return false;

        for (auto i = 0u; i < Size; i++) {
            T v;
            if (!v._parse(data, len)) return false;
            value[i] = v;
        }

        return true;
    }
};

static_assert(sizeof(ArrayT<BoolT, 2>) == 24);

template <uint32_t Size>
struct PoolByteArrayT : GodotT<20> {
    uint32_t _len = Size;
    std::array<uint8_t, Size> value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        _len = *reinterpret_cast<uint32_t*&>(data);
        if (_len != Size) return false;
        if (!advance(data, len, sizeof(uint32_t))) return false;

        std::memcpy(&value[0], data, sizeof(uint8_t) * Size);
        return advance(data, len, sizeof(uint8_t) * Size);
    }
};

#define S(...) __VA_ARGS__

#define PACKET(name, size)     \
    struct name : GodotT<19> { \
        uint32_t _len = size;

#define FIELD(type, name) type name;

#define END_PACKET()                                 \
    bool _parse(std::byte*& data, std::size_t& len); \
    }                                                \
    ;

#include "packets.def"

#undef FIELD
#undef END_PACKET
#undef PACKET

#define PACKET(name, size)                                         \
    inline bool name::_parse(std::byte*& data, std::size_t& len) { \
        if (!GodotT::_parse(data, len)) return false;              \
        _len = *reinterpret_cast<uint32_t*&>(data);                \
        if (_len != size) return false;                            \
        if (!advance(data, len, sizeof(uint32_t))) return false;

#define FIELD(type, name) \
    if (!this->name._parse(data, len)) return false;

#define END_PACKET() \
    return true;     \
    }

#include "packets.def"

#undef FIELD
#undef END_PACKET
#undef PACKET
#undef S

constexpr auto InitPacketSize = sizeof(InitPacket);

constexpr auto StatePacketSize = sizeof(StatePacket);

constexpr auto StateUpdatePacketSize = sizeof(StateUpdatePacket);
constexpr auto StateOsdUpdatePacketSize = sizeof(StateOsdUpdatePacket);

template <typename T>
std::optional<T> get(std::byte* cur, std::size_t len) {
    auto result = parse<T>(cur, len);
    if (len != 0) {
        return std::nullopt;
    }

    return result;
}

template <typename T>
void send(kissnet::udp_socket& send_socket, const T& packet) {
    auto [len, no_error] =
      send_socket.send(reinterpret_cast<const std::byte*>(&packet), sizeof(T));
    assert(no_error && "Error send");
    assert(len == sizeof(T) && "Error size send");
}

template <typename T, bool AllowStop = false, bool AllowError = false>
auto receive(kissnet::udp_socket& recv_socket)
  -> std::conditional_t<AllowStop or AllowError, std::optional<T>, T> {
    std::array<std::byte, 2 * sizeof(T)> buf;
    auto [len, no_error] = recv_socket.recv(buf);
    assert(no_error && "Error recv state packet");

    if constexpr (AllowStop) {
        std::string str(reinterpret_cast<const char*>(&buf[0]), len);
        if (str == "STOP") {
            return std::nullopt;
        }
    }

    // std::string str(reinterpret_cast<const char *>(&buf[0]), len);
    // fmt::print("Got: {}\n", string_to_hex(str));

    const auto result = get<T>(&buf[0], len);
    if constexpr (AllowError) {
        return result;
    } else {
        assert(result && "failed to parse received packet");
        return *result;
    }
}
