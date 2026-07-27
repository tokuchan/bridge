#include <catch2/catch_test_macros.hpp>

#include <rivets/libstdcxx.hpp>

TEST_CASE("bridge::rivets::libstdcxx detects the active libstdc++, if any", "[rivets][libstdcxx]") {
    if constexpr (bridge::rivets::libstdcxx::version > 0) {
        REQUIRE(bridge::rivets::libstdcxx::ge(bridge::rivets::libstdcxx::version));
        REQUIRE_FALSE(bridge::rivets::libstdcxx::gt(bridge::rivets::libstdcxx::version));
    } else {
        REQUIRE(bridge::rivets::libstdcxx::version == 0);
        REQUIRE_FALSE(bridge::rivets::libstdcxx::ge(1));
    }
}

TEST_CASE("BRIDGE_RIVETS_LIBSTDCXX_GE/_GT are #if-usable for any n, under any standard library", "[rivets][libstdcxx]") {
    // version is always >= 0 (either a real positive release, or the
    // not-libstdc++ sentinel) and never > 999 (no real libstdc++ release is
    // that high) -- both hold regardless of which standard library runs this
    // test, which is what makes them useful as a library-agnostic proof that
    // the #if-usable macro form itself evaluates correctly.
#if BRIDGE_RIVETS_LIBSTDCXX_GE(0)
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_LIBSTDCXX_VERSION >= 0 should always be true");
#endif

#if BRIDGE_RIVETS_LIBSTDCXX_GT(999)
    FAIL("no real libstdc++ release is > 999");
#else
    SUCCEED();
#endif
}

TEST_CASE("Named Detector ge_13 matches its generic comparator", "[rivets][libstdcxx]") {
    REQUIRE(bridge::rivets::libstdcxx::ge_13() == bridge::rivets::libstdcxx::ge(13));
}
