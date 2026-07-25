#include <catch2/catch_test_macros.hpp>

#include <truss/cpp17/version.hpp>

TEST_CASE("bridge::truss::version_info reports the declared version", "[truss][version]") {
    constexpr bridge::truss::version_info v{};
    REQUIRE(v.major == BRIDGE_TRUSS_VERSION_MAJOR);
    REQUIRE(v.minor == BRIDGE_TRUSS_VERSION_MINOR);
    REQUIRE(v.patch == BRIDGE_TRUSS_VERSION_PATCH);
}
