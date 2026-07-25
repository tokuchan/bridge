#include <catch2/catch_test_macros.hpp>

#include <deck/cpp17/version.hpp>
#include <truss/cpp17/version.hpp>

TEST_CASE("bridge::deck::version_info reports the declared version", "[deck][version]") {
    constexpr bridge::deck::version_info v{};
    REQUIRE(v.major == BRIDGE_DECK_VERSION_MAJOR);
    REQUIRE(v.minor == BRIDGE_DECK_VERSION_MINOR);
    REQUIRE(v.patch == BRIDGE_DECK_VERSION_PATCH);
}

TEST_CASE("bridge::deck::version_info embeds the Truss version it links against", "[deck][version]") {
    constexpr bridge::deck::version_info v{};
    REQUIRE(v.truss.major == BRIDGE_TRUSS_VERSION_MAJOR);
    REQUIRE(v.truss.minor == BRIDGE_TRUSS_VERSION_MINOR);
    REQUIRE(v.truss.patch == BRIDGE_TRUSS_VERSION_PATCH);
}
