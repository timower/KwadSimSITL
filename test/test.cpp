#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "simulator.h"

TEST_CASE("Simulator init", "[simulator]") {
    auto& simulator = Simulator::getInstance();
    REQUIRE(simulator.micros_passed == 0);
}