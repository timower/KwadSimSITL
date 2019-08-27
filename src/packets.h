#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <tuple>

#include <fmt/format.h>

bool advance(std::byte*& data, std::size_t& len, std::size_t n) {
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
    std::array<float, 3> value;

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
    std::array<float, 9> value;

    bool _parse(std::byte*& data, std::size_t& len) {
        if (!GodotT::_parse(data, len)) return false;

        for (auto i = 0u; i < 9; i++) {
            value[i] = *reinterpret_cast<float*&>(data);
            advance(data, len, sizeof(float));
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
                    ArrayT<FloatT, 3>, Vec3T, FloatT, FloatT> {
    PARAM_REF(0, Kv)
    PARAM_REF(1, R)
    PARAM_REF(2, I0)

    PARAM_REF(3, Rpm)
    PARAM_REF(4, a)
    PARAM_REF(5, torqueF)
    PARAM_REF(6, inertia)

    PARAM_REF(7, thrustVel)
    PARAM_REF(8, dragArea)
    PARAM_REF(9, dragC)

    PARAM_REF(10, Vbat)
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

struct StateUpdatePacket : public TupleT<Vec3T, Vec3T> {
    PARAM_REF(0, linearVelocity);
    PARAM_REF(1, angularVelocity);

    static StateUpdatePacket fromState(const StatePacket& state) {
        StateUpdatePacket ret;
        ret.linearVelocity() = state.linearVelocity();
        ret.angularVelocity() = state.angularVelocity();
        return ret;
    }
};