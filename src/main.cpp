#include <chrono>
#include <experimental/type_traits>
#include <iostream>
#include <tuple>

#include "fmt/format.h"

#include <kissnet.hpp>
namespace kn = kissnet;

extern "C" {
#include "dyad.h"
}

namespace betaflight {
extern "C" {
#include "common/maths.h"

#include "fc/init.h"
#include "fc/runtime_config.h"
#include "fc/tasks.h"

#include "flight/imu.h"

#include "scheduler/scheduler.h"
#include "sensors/sensors.h"

#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/pwm_output.h"
#include "drivers/pwm_output_fake.h"

#include "rx/msp.h"

#include "io/displayport_fake.h"
#include "io/gps.h"

#include "src/target.h"
}
}  // namespace betaflight

static int64_t sleep_timer = 0;

static uint64_t micros_passed = 0;

// 20kHz scheduler, is enough to run PID at 8khz
const auto FREQUENCY = 20e3;
const auto DELTA = 1e6 / FREQUENCY;

auto totalDelta = 0;

using hr_clock = std::chrono::high_resolution_clock;

template <typename R, typename P>
auto to_us(std::chrono::duration<R, P> t) {
    return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
}

template <typename R, typename P>
auto to_ms(std::chrono::duration<R, P> t) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
}

void clearline() {
    fmt::print(
        "\r                                                                    "
        "                          \r");
}

std::tuple<kn::udp_socket, kn::udp_socket> make_sockets() {
    kn::udp_socket send_socket(kn::endpoint("127.0.0.1", 6666));
    kn::udp_socket recv_socket(kn::endpoint("127.0.0.1", 7777));
    recv_socket.bind();
    return {std::move(send_socket), std::move(recv_socket)};
}

template <uint32_t TypeID>
struct GodotT {
    uint32_t _typeId = TypeID;

    bool check() const {
        return _typeId == TypeID;
    }
};

struct BoolT : public GodotT<1> {
    int value;

    BoolT(bool v) : value(v) {
    }

    operator bool() const {
        return value;
    }
};

struct FloatT : GodotT<3> {
    float value;
};

static_assert(sizeof(FloatT) == 4 + 4);

struct Vec3T : GodotT<7> {
    float x;
    float y;
    float z;
};

static_assert(sizeof(Vec3T) == 16);

template <typename T, uint32_t Size>
struct ArrayT : GodotT<19> {
    uint32_t _len;
    std::array<T, Size> elems;

    bool check() const {
        if (!GodotT<19>::check() || Size != size()) return false;
        for (auto& elem : elems) {
            if (!elem.check()) return false;
        }
    }

    uint32_t size() {
        return _len & 0x7FFFFFFF;
    }
};

struct InitPacket : GodotT<19> {
    FloatT Kv, R, I0;
    FloatT Rpm, a, torqueF, inertia;

    ArrayT<FloatT, 3> thrustVels;

    Vec3T dragArea;
    FloatT dragC;
    FloatT Vbat;

    bool check() const {
        return GodotT<19>::check() && kv.check() && R.check() && I0.check;
    }
};

struct StatePacket : GodotT<19> {
    FloatT delta;
    Vec3T position;
    std::array<Vec3T, 3> rotation;

    Vec3T linear_velocity;
    Vec3T angular_velocity;

    std::array<FloatT, 8> rcData;
    BoolT crashed;
};

struct StateUpdatePacket {
    Vec3T linear_velocity;
    Vec3T angular_velocity;

    static StateUpdatePacket fromState(const StatePacket& state) {
        return {state.linear_velocity, state.angular_velocity};
    }
};

static const auto INIT_PACKET_SIZE = 109;

template <typename T>
T get(std::byte*& cur) {
    T ret = *reinterpret_cast<T*>(cur);

    ret.check();

    std::advance(cur, sizeof(T));
    return ret;
}

const InitPacket getInitPacket(kn::udp_socket& recv_socket) {
    std::array<std::byte, 1024> buf;
    auto [len, error] = recv_socket.recv(buf);
    assert(!error);
    assert(len == INIT_PACKET_SIZE);
    InitPacket packet;
    auto cur = buf.begin();

    auto typeId = get<uint32_t>(cur);
    assert(typeId == 19);  // array
    packet.delta = get<FloatT>(cur);

    return packet;
}
const StatePacket getStatePacket(kn::udp_socket& recv_socket);
void sendStateUpdatePacket(kn::udp_socket& send_socket,
                           const StateUpdatePacket& update);

int main() {
    auto last = hr_clock::now();
    const auto start = last;

    auto [send_socket, recv_socket] = make_sockets();

    fmt::print("waiting for init packet\n");

    const auto init_packet = getInitPacket(recv_socket);

    fmt::print("Initializing dyad\n");
    dyad_init();
    dyad_setUpdateTimeout(0.001);

    fmt::print("Initializing betaflight\n");
    betaflight::init();
    fmt::print("Done, running loop\n\n");

    for (auto i = 0u;; i++) {
        const auto state = getStatePacket(recv_socket);
        auto update = StateUpdatePacket::fromState(state);

        const auto deltaMicros = int(state.delta * 1e6);
        totalDelta += deltaMicros;

        const auto last = hr_clock::now();

        dyad_update();

        auto dyad_time = hr_clock::now() - last;
        long long dyad_time_i = to_us(dyad_time);

        int k = 0;

        for (auto k = 0u; totalDelta - DELTA > 0; k++) {
            totalDelta -= DELTA;
            micros_passed += DELTA;

            betaflight::scheduler();
        }

        sendStateUpdatePacket(send_socket, update);

        const auto elapsed_ms = hr_clock::now() - start;
        long long ms_i = to_us(elapsed_ms);
        long long delta = micros_passed - ms_i;
        if (i % 100 == 0) {
            clearline();
            fmt::print(
                "elapsed ms: {}, fake ms: {}, delta: {}, dyad time: {}, loops: "
                "{}",
                ms_i, micros_passed, delta, dyad_time_i, k);
        }
    }

    dyad_shutdown();
    return 0;
}

/*********************
 * Betaflight Stuff: *
 *********************/
extern "C" {
void systemInit(void) {
    int ret;

    printf("[system]Init...\n");

    betaflight::SystemCoreClock = 500 * 1000000;  // fake 500MHz
    betaflight::FLASH_Unlock();

    // serial can't been slow down
    // rescheduleTask(TASK_SERIAL, 1);
}

void systemReset(void) {
    printf("[system]Reset!\n");

    micros_passed = 0;
    exit(0);
}

void systemResetToBootloader(void) {
    printf("[system]ResetToBootloader!\n");

    micros_passed = 0;
    exit(1);
}

uint32_t micros(void) {
    return micros_passed & 0xFFFFFFFF;
}

uint32_t millis(void) {
    // static uint32_t last_mil = 0;
    uint32_t mil = (micros_passed / 1000) & 0xFFFFFFFF;
    // fix for gps double stuff:
    // if (mil == last_mil) mil += 1;
    // last_mil = mil;
    return mil;
}

void microsleep(uint32_t usec) {
    sleep_timer = usec;
}

void delayMicroseconds(uint32_t usec) {
    microsleep(usec);
}

void delay(uint32_t ms) {
    microsleep(ms * 1000);
}
}