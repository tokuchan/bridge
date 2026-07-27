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

TEST_CASE("bridge::rivets::features::lib_format matches __cpp_lib_format", "[rivets][features]") {
#ifdef __cpp_lib_format
    REQUIRE(bridge::rivets::features::lib_format == __cpp_lib_format);
#else
    // std::format doesn't exist at all before C++20 -- __cpp_lib_format
    // is legitimately undefined under -std=c++17, confirmed by direct
    // compiler probe before this Feature Test was written.
    REQUIRE(bridge::rivets::features::lib_format == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_FORMAT is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_FORMAT == 0
    SUCCEED(); // pre-C++20 toolchains legitimately report 0 here
#elif BRIDGE_RIVETS_FEATURES_LIB_FORMAT >= 201907L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_FORMAT should be 0 or at least the C++20 baseline");
#endif
}

TEST_CASE("bridge::rivets::features::lib_print matches __cpp_lib_print", "[rivets][features]") {
#ifdef __cpp_lib_print
    REQUIRE(bridge::rivets::features::lib_print == __cpp_lib_print);
#else
    // std::print doesn't exist at all before C++23 (confirmed empirically:
    // the <print> header is includable under -std=c++20 on GCC, but
    // std::print/std::println themselves are hard-rejected with "only
    // available from C++23 onwards") -- __cpp_lib_print is legitimately
    // undefined under -std=c++17/-std=c++20.
    REQUIRE(bridge::rivets::features::lib_print == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_PRINT is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_PRINT == 0
    SUCCEED(); // pre-C++23 toolchains legitimately report 0 here
#elif BRIDGE_RIVETS_FEATURES_LIB_PRINT >= 202211L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_PRINT should be 0 or at least the C++23 baseline");
#endif
}

TEST_CASE("bridge::rivets::features::lib_span matches __cpp_lib_span", "[rivets][features]") {
#ifdef __cpp_lib_span
    REQUIRE(bridge::rivets::features::lib_span == __cpp_lib_span);
#else
    // std::span doesn't exist at all before C++20 -- __cpp_lib_span is
    // legitimately undefined under -std=c++17, confirmed by direct
    // compiler probe before this Feature Test was written (both GCC
    // 15.3 and Clang 20 report exactly 202002L from -std=c++20 onward,
    // including under -std=c++23 -- this libstdc++ hasn't bumped it to
    // the C++23 tuple-like-interface value 202311L yet).
    REQUIRE(bridge::rivets::features::lib_span == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_SPAN is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_SPAN == 0
    SUCCEED(); // pre-C++20 toolchains legitimately report 0 here
#elif BRIDGE_RIVETS_FEATURES_LIB_SPAN >= 202002L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_SPAN should be 0 or at least the C++20 baseline");
#endif
}

TEST_CASE("bridge::rivets::features::lib_jthread matches __cpp_lib_jthread", "[rivets][features]") {
#ifdef __cpp_lib_jthread
    REQUIRE(bridge::rivets::features::lib_jthread == __cpp_lib_jthread);
#else
    // std::jthread doesn't exist at all before C++20 -- __cpp_lib_jthread is
    // legitimately undefined under -std=c++17, confirmed by direct compiler
    // probe before this Feature Test was written (both GCC 13-15 and Clang
    // 20 report exactly 201911L from -std=c++20 onward).
    REQUIRE(bridge::rivets::features::lib_jthread == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_JTHREAD is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_JTHREAD == 0
    SUCCEED(); // pre-C++20 toolchains legitimately report 0 here
#elif BRIDGE_RIVETS_FEATURES_LIB_JTHREAD >= 201911L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_JTHREAD should be 0 or at least the C++20 baseline");
#endif
}

TEST_CASE("bridge::rivets::features::lib_stop_token matches __cpp_lib_stop_token", "[rivets][features]") {
    // Confirmed by direct compiler probe (not assumed): on every GCC (13-15)
    // and Clang (20) this project's matrix covers, __cpp_lib_stop_token is
    // never defined, even under -std=c++20/-std=c++23 -- a libstdc++-wide
    // gap in publishing the macro, not a real absence of the feature (see
    // bridge::rivets::libstdcxx and deck/cpp17/jthread.hpp's override). This
    // test follows the same ifdef-mirrors-the-macro shape every other
    // Feature Test test uses, so it stays correct if some future libstdc++
    // (or a different stdlib entirely) finally defines the macro.
#ifdef __cpp_lib_stop_token
    REQUIRE(bridge::rivets::features::lib_stop_token == __cpp_lib_stop_token);
#else
    REQUIRE(bridge::rivets::features::lib_stop_token == 0);
#endif
}

TEST_CASE("BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN is #if-usable", "[rivets][features]") {
#if BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN == 0
    SUCCEED(); // absent everywhere in this project's matrix today
#elif BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN >= 201907L
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN should be 0 or at least the C++20 baseline");
#endif
}
