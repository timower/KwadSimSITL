#include "catch.hpp"

#include <kissnet.hpp>
#include "packets.h"

namespace kn = kissnet;

std::pair<kn::udp_socket, kn::udp_socket> createSockets() {
    kn::udp_socket send_socket(kn::endpoint("127.0.0.1", 7777));
    kn::udp_socket recv_socket(kn::endpoint("127.0.0.1", 7777));
    recv_socket.bind();
    return {std::move(send_socket), std::move(recv_socket)};
}

TEST_CASE("packet stufff", "[packets]") {
    auto [send_socket, recv_socket] = createSockets();

    BoolT b;
    b.value = true;
    send(send_socket, b);
    REQUIRE(receive<BoolT>(recv_socket).value == b.value);

    FloatT testF;
    testF.value = 12.0f;
    send(send_socket, testF);
    REQUIRE(receive<FloatT>(recv_socket).value == 12.0f);

    Vec3T testV;
    testV.value = {12.0f, 0.0f, -12.2e5f};
    send(send_socket, testV);
    auto recvV = receive<Vec3T>(recv_socket).value;
    REQUIRE(recvV[0] == 12.0f);
    REQUIRE(recvV[1] == 0.0f);
    REQUIRE(recvV[2] == -12.2e5f);

    InitPacket initpacket;
    send(send_socket, initpacket);
    auto recvInitP = receive<InitPacket>(recv_socket);
    REQUIRE(recvInitP.motor_I0.value == initpacket.motor_I0.value);

    StatePacket state;
    send(send_socket, state);
    auto recv_state = receive<StatePacket>(recv_socket);
    REQUIRE(state.delta.value == recv_state.delta.value);

    StateUpdatePacket update;
    send(send_socket, update);
    auto recv_update = receive<StateUpdatePacket>(recv_socket);
    REQUIRE(recv_update.linearVelocity.value == update.linearVelocity.value);

    StateOsdUpdatePacket updateOsd;
    send(send_socket, updateOsd);
    auto recv_updateOsd = receive<StateOsdUpdatePacket>(recv_socket);
    REQUIRE(recv_updateOsd.linearVelocity.value ==
            updateOsd.linearVelocity.value);

    send_socket.send(reinterpret_cast<const std::byte*>("STOP"), 4);
    REQUIRE(receive<Vec3T, true>(recv_socket) == std::nullopt);
}