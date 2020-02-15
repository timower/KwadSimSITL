#include "catch.hpp"

#include <kissnet.hpp>
#include "packets.h"

namespace kn = kissnet;

namespace {
std::pair<kn::udp_socket, kn::udp_socket> createSockets() {
    kn::udp_socket send_socket(kn::endpoint("localhost", 17777));
    kn::udp_socket recv_socket(kn::endpoint("localhost", 17777));
    recv_socket.bind();
    return {std::move(send_socket), std::move(recv_socket)};
}
}  // namespace

TEST_CASE("advance", "[packets]") {
    std::array<std::byte, 5> test;
    std::size_t len = 5;
    auto* p = &test[0];

    REQUIRE(advance(p, len, 2));
    REQUIRE(len == 3);
    REQUIRE(advance(p, len, 3));
    REQUIRE_FALSE(advance(p, len, 1));
    len = 5;
    p = &test[0];
    REQUIRE_FALSE(advance(p, len, 6));
}

TEST_CASE("packet send & receive", "[packets]") {
    auto [send_socket, recv_socket] = createSockets();

    BoolT b;
    b.value = true;
    send(send_socket, b);
    REQUIRE(receive<BoolT>(recv_socket).value == b.value);

    FloatT testF;
    testF.value = 12.0f;
    send(send_socket, testF);
    REQUIRE(receive<FloatT>(recv_socket).value == 12.0f);

    struct __attribute__((packed)) float64 {
        uint32_t id = 3 | (1 << 16);
        double val = 12.0f;
    } packet;
    send(send_socket, packet);
    REQUIRE(receive<FloatT>(recv_socket) == 12.0f);

    Vec3T testV;
    testV.value = {12.0f, 0.0f, -12.2e5f};
    send(send_socket, testV);
    auto recvV = receive<Vec3T>(recv_socket).value;
    REQUIRE(recvV[0] == 12.0f);
    REQUIRE(recvV[1] == 0.0f);
    REQUIRE(recvV[2] == -12.2e5f);

    BasisT basis;
    basis.value[0][2] = 44.2f;
    send(send_socket, basis);
    auto recvB =  receive<BasisT>(recv_socket).value;
    REQUIRE(recvB[0][2] == 44.2f);


    InitPacket initpacket;
    initpacket.motor_I0.value = 2.3f;
    send(send_socket, initpacket);
    auto recvInitP = receive<InitPacket>(recv_socket);
    REQUIRE(recvInitP.motor_I0.value == initpacket.motor_I0.value);

    StatePacket state;
    state.delta.value = 0.16f;
    send(send_socket, state);
    auto recv_state = receive<StatePacket>(recv_socket);
    REQUIRE(state.delta.value == recv_state.delta.value);

    StateUpdatePacket update;
    update.linearVelocity.value[0] = -1234.0123f;
    update.linearVelocity.value[1] = 1234.0123f;
    update.linearVelocity.value[2] = 34.0123f;
    send(send_socket, update);
    auto recv_update = receive<StateUpdatePacket>(recv_socket);
    REQUIRE(recv_update.linearVelocity.value == update.linearVelocity.value);

    StateOsdUpdatePacket updateOsd;
    update.angularVelocity.value[0] = -1234.0123f;
    update.angularVelocity.value[1] = 1234.0123f;
    update.angularVelocity.value[2] = 34.0123f;
    send(send_socket, updateOsd);
    auto recv_updateOsd = receive<StateOsdUpdatePacket>(recv_socket);
    REQUIRE(recv_updateOsd.angularVelocity.value ==
            updateOsd.angularVelocity.value);

    send_socket.send(reinterpret_cast<const std::byte*>("STOP"), 4);
    REQUIRE(receive<Vec3T, true>(recv_socket) == std::nullopt);

    send_socket.send(reinterpret_cast<const std::byte*>("AAAA"), 4);
    REQUIRE(receive<Vec3T, true, true>(recv_socket) == std::nullopt);

    send_socket.send(reinterpret_cast<const std::byte*>("BBBB"), 4);
    REQUIRE(receive<Vec3T, false, true>(recv_socket) == std::nullopt);
}
