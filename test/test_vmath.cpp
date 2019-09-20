#include "catch.hpp"

#include "vector_math.h"

using namespace vmath;

TEST_CASE("general vector math", "[vmath]") {
    vec3 a{1, 0, 0};
    vec3 b{0, 1, 0};
    REQUIRE(dot(a, b) == 0.0f);
    REQUIRE(dot(a, a) == 1.0f);

    REQUIRE(length2(b) == 1.0f);
    REQUIRE(length(a) == 1.0f);

    REQUIRE(cross(a, b) == vec3{0, 0, 1});

    REQUIRE((a / 2) == vec3{0.5, 0, 0});
    REQUIRE((a * 2) == vec3{2, 0, 0});

    REQUIRE(a + b == vec3{1, 1, 0});
    REQUIRE(a - b == vec3{1, -1, 0});
    REQUIRE(a * b == vec3{0, 0, 0});

    REQUIRE(normalize(a) == a);

    REQUIRE(abs(a - b) == vec3{1, 1, 0});

    REQUIRE(clamp(0.5, 0, 1) == 0.5);
    REQUIRE(clamp(-0.5, 0, 1) == 0);
    REQUIRE(clamp(1.5, 0, 1) == 1);

    mat3 identity = {vec3{1, 0, 0}, vec3{0, 1, 0}, vec3{0, 0, 1}};
    mat3 rotate = {vec3{0, -1, 0}, vec3{1, 0, 0}, vec3{0, 0, 1}};

    REQUIRE(xform(identity, a) == a);
    REQUIRE(xform_inv(identity, a) == a);
    REQUIRE(transpose(identity) == identity);
    REQUIRE(identity * rotate == rotate);
}