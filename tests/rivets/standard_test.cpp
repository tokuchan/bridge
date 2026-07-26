#include <catch2/catch_test_macros.hpp>

#include <rivets/standard.hpp>

static_assert(BRIDGE_CPLUSPLUS >= BRIDGE_CPP17, "bridge requires C++17");

TEST_CASE("BRIDGE_CPLUSPLUS reports at least C++17", "[rivets][standard]") {
    REQUIRE(BRIDGE_CPLUSPLUS >= BRIDGE_CPP17);
}

TEST_CASE("bridge::rivets::standard::year_code_of maps known ordinals", "[rivets][standard]") {
    REQUIRE(bridge::rivets::standard::year_code_of(17) == BRIDGE_CPP17);
    REQUIRE(bridge::rivets::standard::year_code_of(20) == BRIDGE_CPP20);
    REQUIRE(bridge::rivets::standard::year_code_of(23) == BRIDGE_CPP23);
    REQUIRE(bridge::rivets::standard::year_code_of(99) == -1);
}

TEST_CASE("bridge::rivets::standard::ge(17) is always true (bridge's floor)", "[rivets][standard]") {
    REQUIRE(bridge::rivets::standard::ge(17));
}

TEST_CASE("bridge::rivets::standard comparators agree with BRIDGE_CPLUSPLUS", "[rivets][standard]") {
    REQUIRE(bridge::rivets::standard::gt(17) == (BRIDGE_CPLUSPLUS > BRIDGE_CPP17));
    REQUIRE(bridge::rivets::standard::ge(20) == (BRIDGE_CPLUSPLUS >= BRIDGE_CPP20));
    REQUIRE(bridge::rivets::standard::le(23) == (BRIDGE_CPLUSPLUS <= BRIDGE_CPP23));
    REQUIRE(bridge::rivets::standard::eq(17) == (BRIDGE_CPLUSPLUS == BRIDGE_CPP17));
}
