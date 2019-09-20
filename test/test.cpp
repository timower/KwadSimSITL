#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "simulator.h"

#include <kissnet.hpp>

namespace kn = kissnet;

namespace {
std::pair<kn::udp_socket, kn::udp_socket> createSockets() {
    kn::udp_socket send_socket(kn::endpoint("localhost", 7777));
    kn::udp_socket recv_socket(kn::endpoint("localhost", 6666));
    recv_socket.bind();
    return {std::move(send_socket), std::move(recv_socket)};
}
}  // namespace

using namespace vmath;

TEST_CASE("Simulator init", "[simulator]") {
    auto [send_socket, recv_socket] = createSockets();

    auto& simulator = Simulator::getInstance();
    REQUIRE(simulator.micros_passed == 0);

    InitPacket init_packet;
    init_packet.motor_kv = 1000;
    init_packet.motor_R = 1;
    init_packet.motor_I0 = 0.1f;

    init_packet.prop_max_rpm = 10000;
    init_packet.prop_a_factor = 0.1f;
    init_packet.prop_torque_factor = 1.0f;
    init_packet.prop_inertia = 0.000001f;
    init_packet.prop_thrust_factors.value[0] = 1.0f;
    init_packet.prop_thrust_factors.value[1] = 1.0f;
    init_packet.prop_thrust_factors.value[2] = 1.0f;

    init_packet.frame_drag_area = vec3{1, 1, 1};
    init_packet.frame_drag_constant = 1.0f;

    init_packet.quad_mass = 0.5f;
    init_packet.quad_inv_inertia.value = vec3{1, 1, 1};
    init_packet.quad_vbat = 12.0f;
    init_packet.quad_motor_pos.value[0] = vec3{1, 0, 1};
    init_packet.quad_motor_pos.value[1] = vec3{1, 0, -1};
    init_packet.quad_motor_pos.value[2] = vec3{-1, 0, 1};
    init_packet.quad_motor_pos.value[3] = vec3{-1, 0, -1};

    send(send_socket, init_packet);
    simulator.connect();

    REQUIRE(receive<BoolT>(recv_socket));

    StatePacket state;
    state.delta = 1.0f;
    state.position = vec3{0, 0, 0};
    state.rotation.value = identity;
    state.angularVelocity = vec3{0, 0, 0};
    state.linearVelocity = vec3{0, 0, 0};

    state.crashed = false;

    send(send_socket, state);
    REQUIRE(simulator.step());

    REQUIRE(simulator.micros_passed == 1000000);
    auto update = receive<StateOsdUpdatePacket>(recv_socket);
    REQUIRE(update.angularVelocity.value == vec3{0, 0, 0});
    REQUIRE(update.linearVelocity.value[0] == 0);
    REQUIRE(update.linearVelocity.value[1] < 0);
    REQUIRE(update.linearVelocity.value[2] == 0);

    send_socket.send(reinterpret_cast<const std::byte*>("STOP"), 4);
    REQUIRE_FALSE(simulator.step());
}