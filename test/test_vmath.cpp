#include "catch.hpp"

#include "vector_math.h"

using namespace vmath;

TEST_CASE("general vector math", "[vmath]") {
    vec3 a{1, 0, 0};
    vec3 b{0, 1, 0};
    REQUIRE(dot(a, b) == 0.0f);
}