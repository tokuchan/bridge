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

TEST_CASE("bridge::rivets::features::lib_expected matches __cpp_lib_expected", "[rivets][features]") {
#ifdef __cpp_lib_expected
    REQUIRE(bridge::rivets::features::lib_expected == __cpp_lib_expected);
#else
    // Unlike __cpp_lib_optional (a C++17 baseline feature), std::expected
    // doesn't exist at all before C++23 -- __cpp_lib_expected is
    // legitimately undefined under -std=c++17/-std=c++20, confirmed by
    // direct compiler probe before this Feature Test was written.
    REQUIRE(bridge::rivets::features::lib_expected == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_EXPECTED is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_EXPECTED == 0
    SUCCEED(); // pre-C++23 toolchains legitimately report 0 here
#elif BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_EXPECTED should be 0 or at least the C++23 baseline");
#endif
}
