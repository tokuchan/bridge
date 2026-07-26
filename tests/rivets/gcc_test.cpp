#include <catch2/catch_test_macros.hpp>

#include <rivets/clang.hpp>
#include <rivets/gcc.hpp>

TEST_CASE("bridge::rivets::gcc detects real GCC, and only real GCC", "[rivets][gcc]") {
    if constexpr (bridge::rivets::gcc::version > 0) {
        // Compiling with real GCC (excludes Clang, which also defines __GNUC__).
        REQUIRE(bridge::rivets::gcc::ge(bridge::rivets::gcc::version));
        REQUIRE_FALSE(bridge::rivets::gcc::gt(bridge::rivets::gcc::version));
        REQUIRE(bridge::rivets::clang::version == 0);
    } else {
        REQUIRE(bridge::rivets::gcc::version == 0);
        REQUIRE_FALSE(bridge::rivets::gcc::ge(1));
    }
}

TEST_CASE("BRIDGE_RIVETS_GCC_GE/_GT are #if-usable for any n, under any compiler", "[rivets][gcc]") {
    // version is always >= 0 (either a real positive major version, or the
    // not-GCC sentinel) and never > 999 (no real GCC major version is that
    // high) -- both hold regardless of which compiler runs this test, which
    // is what makes them useful as a compiler-agnostic proof that the
    // #if-usable macro form itself evaluates correctly.
#if BRIDGE_RIVETS_GCC_GE(0)
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_GCC_VERSION >= 0 should always be true");
#endif

#if BRIDGE_RIVETS_GCC_GT(999)
    FAIL("no real GCC major version is > 999");
#else
    SUCCEED();
#endif
}

TEST_CASE("Named Detector ge_13 matches its generic comparator", "[rivets][gcc]") {
    REQUIRE(bridge::rivets::gcc::ge_13() == bridge::rivets::gcc::ge(13));
}
