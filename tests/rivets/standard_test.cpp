#include <catch2/catch_test_macros.hpp>

#include <rivets/standard.hpp>

static_assert(BRIDGE_CPLUSPLUS >= BRIDGE_CPP17, "bridge requires C++17");

TEST_CASE("BRIDGE_CPLUSPLUS reports at least C++17", "[rivets][standard]") {
    REQUIRE(BRIDGE_CPLUSPLUS >= BRIDGE_CPP17);
}
