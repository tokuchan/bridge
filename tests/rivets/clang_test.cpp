#include <catch2/catch_test_macros.hpp>

#include <rivets/clang.hpp>
#include <rivets/gcc.hpp>

TEST_CASE("bridge::rivets::clang detects Clang, and only Clang", "[rivets][clang]") {
    if constexpr (bridge::rivets::clang::version > 0) {
        REQUIRE(bridge::rivets::clang::ge(bridge::rivets::clang::version));
        REQUIRE_FALSE(bridge::rivets::clang::gt(bridge::rivets::clang::version));
        // Clang also defines __GNUC__ for compatibility; gcc.hpp must not
        // be fooled by that.
        REQUIRE(bridge::rivets::gcc::version == 0);
    } else {
        REQUIRE(bridge::rivets::clang::version == 0);
        REQUIRE_FALSE(bridge::rivets::clang::ge(1));
    }
}

TEST_CASE("BRIDGE_RIVETS_CLANG_GE/_GT are #if-usable for any n, under any compiler", "[rivets][clang]") {
#if BRIDGE_RIVETS_CLANG_GE(0)
    SUCCEED();
#else
    FAIL("BRIDGE_RIVETS_CLANG_VERSION >= 0 should always be true");
#endif

#if BRIDGE_RIVETS_CLANG_GT(999)
    FAIL("no real Clang major version is > 999");
#else
    SUCCEED();
#endif
}

TEST_CASE("Named Detector ge_18 matches its generic comparator", "[rivets][clang]") {
    REQUIRE(bridge::rivets::clang::ge_18() == bridge::rivets::clang::ge(18));
}
