#include <catch2/catch_test_macros.hpp>

#include <optional>

#include <rivets/features.hpp>

TEST_CASE("bridge::rivets::features::lib_optional matches __cpp_lib_optional", "[rivets][features]") {
#ifdef __cpp_lib_optional
    REQUIRE(bridge::rivets::features::lib_optional == __cpp_lib_optional);
#else
    REQUIRE(bridge::rivets::features::lib_optional == 0);
#endif
}

TEST_CASE("bridge::rivets::features::lib_optional is at least the C++17 baseline", "[rivets][features]") {
    // std::optional itself is a C++17 feature; whichever standard mode
    // this compiles under, __cpp_lib_optional must be defined and at
    // least the original C++17 value (201606L).
    REQUIRE(bridge::rivets::features::lib_optional >= 201606L);
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 201606L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL should be at least the C++17 baseline");
#endif
}
