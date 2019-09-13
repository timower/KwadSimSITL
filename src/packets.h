#pragma once

#include "vector_math.h"

#include <array>
#include <cstdint>
#include <optional>
#include <tuple>

#include <fmt/format.h>
#include <kissnet.hpp>

inline bool advance(std::byte*& data, std::size_t& len, std::size_t n) {
    if (len - n >= 0) {
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
        advance(data, len, sizeof(uint32_t));

        if (len < 0) {
            return false;
        }
        return true;
    }
};

struct BoolT : public GodotT<1> {
    uint32_t value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;
        value = *reinterpret_cast<uint32_t*&>(data);

        return advance(data, len, sizeof(uint32_t));
    }
};

struct FloatT : GodotT<3> {
    float value;

    bool _parse(std::byte*& data, std::size_t& len) {
        const bool is64 = ((*reinterpret_cast<uint32_t*&>(data)) & 0xFFFF0000);
        if (!GodotT::_parse(data, len)) return false;

        if (!is64) {
            value = *reinterpret_cast<float*&>(data);
            advance(data, len, sizeof(float));
        } else {
            value = *reinterpret_cast<double*&>(data);
            advance(data, len, sizeof(double));
        }

        return len >= 0;
    }
};

static_assert(sizeof(FloatT) == 4 + 4);

struct Vec3T : public GodotT<7> {
    vmath::vec3 value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        for (auto i = 0u; i < 3; i++) {
            value[i] = *reinterpret_cast<float*&>(data);
            advance(data, len, sizeof(float));
        }

        return len >= 0;
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
                advance(data, len, sizeof(float));
            }
        }

        return len >= 0;
    }
};

static_assert(sizeof(BasisT) == 40);

template <typename T, uint32_t Size>
struct ArrayT : public GodotT<19> {
    uint32_t _len;
    std::array<T, Size> value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        _len = *reinterpret_cast<uint32_t*&>(data);
        if (_len != Size) return false;
        advance(data, len, sizeof(uint32_t));

        for (auto i = 0u; i < Size; i++) {
            T v;
            if (!v._parse(data, len)) return false;
            value[i] = v;
        }

        return len >= 0;
    }
};

static_assert(sizeof(ArrayT<BoolT, 2>) == 24);

template <typename... Args>
struct TupleT : public GodotT<19> {
    uint32_t _len = sizeof...(Args);
    std::tuple<Args...> values;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        _len = *reinterpret_cast<uint32_t*&>(data);
        if (_len != sizeof...(Args)) return false;
        advance(data, len, sizeof(uint32_t));

        const auto subResult = std::apply(
            [&data, &len](auto&... vs) {
                return (vs._parse(data, len) && ...);
            },
            values);
        if (!subResult) return false;

        return len >= 0;
    }
};

static_assert(sizeof(TupleT<BoolT, FloatT>) == 24);

#define PARAM_REF(idx, name)                \
    auto& name() {                          \
        return std::get<idx>(values).value; \
    }                                       \
    const auto& name() const {              \
        return std::get<idx>(values).value; \
    }

struct InitPacket
    : public TupleT<FloatT, FloatT, FloatT, FloatT, FloatT, FloatT, FloatT,
                    ArrayT<FloatT, 3>, Vec3T, FloatT, FloatT, Vec3T, FloatT,
                    ArrayT<Vec3T, 4>> {
    PARAM_REF(0, motor_kv)
    PARAM_REF(1, motor_R)
    PARAM_REF(2, motor_I0)

    PARAM_REF(3, prop_max_rpm)
    PARAM_REF(4, prop_a_factor)
    PARAM_REF(5, prop_torque_factor)
    PARAM_REF(6, prop_inertia)
    PARAM_REF(7, prop_thrust_factors)

    PARAM_REF(8, frame_drag_area)
    PARAM_REF(9, frame_drag_constant)

    PARAM_REF(10, quad_mass)
    PARAM_REF(11, quad_inv_inertia)
    PARAM_REF(12, quad_vbat)
    PARAM_REF(13, quad_motor_pos)
};

constexpr auto InitPacketSize = sizeof(InitPacket);

struct StatePacket : public TupleT<FloatT, Vec3T, BasisT, Vec3T, Vec3T,
                                   ArrayT<FloatT, 8>, BoolT> {
    PARAM_REF(0, delta)
    PARAM_REF(1, position)
    PARAM_REF(2, rotation)

    PARAM_REF(3, angularVelocity)
    PARAM_REF(4, linearVelocity)
    PARAM_REF(5, rcData)
    PARAM_REF(6, crashed)
};

constexpr auto StatePacketSize = sizeof(StatePacket);

template <uint32_t Size>
struct PoolByteArrayT : GodotT<20> {
    uint32_t _len = Size;
    std::array<uint8_t, Size> value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        _len = *reinterpret_cast<uint32_t*&>(data);
        if (_len != Size) return false;
        advance(data, len, sizeof(uint32_t));

        std::memcpy(&value[0], data, sizeof(uint8_t) * Size);
        advance(data, len, sizeof(uint8_t) * Size);

        return len >= 0;
    }
};

struct StateUpdatePacket {
    uint32_t _typeId = 19;
    uint32_t _len = 2;

    Vec3T _angularVelocity;
    Vec3T _linearVelocity;

    auto& angularVelocity() {
        return _angularVelocity.value;
    }

    const auto& angularVelocity() const {
        return _angularVelocity.value;
    }

    auto& linearVelocity() {
        return _linearVelocity.value;
    }

    const auto& linearVelocity() const {
        return _linearVelocity.value;
    }

    static StateUpdatePacket fromState(const StatePacket& state) {
        StateUpdatePacket ret;
        ret.linearVelocity() = state.linearVelocity();
        ret.angularVelocity() = state.angularVelocity();
        return ret;
    }
};

#define CHARS_PER_LINE 30
#define VIDEO_LINES 16

struct StateOsdUpdatePacket {
    uint32_t _typeId = 19;
    uint32_t _len = 3;

    Vec3T _angularVelocity;
    Vec3T _linearVelocity;
    PoolByteArrayT<16 * 30> osd;

    auto& angularVelocity() {
        return _angularVelocity.value;
    }

    const auto& angularVelocity() const {
        return _angularVelocity.value;
    }

    auto& linearVelocity() {
        return _linearVelocity.value;
    }

    const auto& linearVelocity() const {
        return _linearVelocity.value;
    }

    void copyScreenData(uint8_t osdScreen[VIDEO_LINES][CHARS_PER_LINE]) {
        for (int y = 0; y < VIDEO_LINES; y++) {
            for (int x = 0; x < CHARS_PER_LINE; x++) {
                osd.value[y * CHARS_PER_LINE + x] = osdScreen[y][x];
            }
        }
    }

    static StateOsdUpdatePacket fromState(const StatePacket& state) {
        StateOsdUpdatePacket ret;
        ret.linearVelocity() = state.linearVelocity();
        ret.angularVelocity() = state.angularVelocity();
        return ret;
    }
};

template <typename T>
T get(std::byte* cur, std::size_t len) {
    auto result = parse<T>(cur, len);
    assert(result);
    assert(len == 0);

    return *result;
}

template <typename T>
void send(kissnet::udp_socket& send_socket, const T& packet) {
    auto [len, no_error] = send_socket.send(
        reinterpret_cast<const std::byte*>(&packet), sizeof(T));
    assert(no_error && "Error send");
    assert(len == sizeof(T) && "Error size send");
}

const InitPacket getInitPacket(kissnet::udp_socket& recv_socket);

std::optional<StatePacket> getStatePacket(kissnet::udp_socket& recv_socket);