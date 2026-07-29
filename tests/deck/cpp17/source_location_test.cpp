#include <catch2/catch_test_macros.hpp>

#include <cstring>

#include <deck/cpp17/source_location.hpp>

TEST_CASE("bridge::source_location default-constructs with an empty, zeroed state", "[deck][source_location]") {
    bridge::source_location loc;
    REQUIRE(std::strlen(loc.file_name()) == 0);
    REQUIRE(std::strlen(loc.function_name()) == 0);
    REQUIRE(loc.line() == 0);
    REQUIRE(loc.column() == 0);
}

TEST_CASE("bridge::source_location::current captures a real call site", "[deck][source_location]") {
    auto first_line = __LINE__ + 1;
    auto loc = bridge::source_location::current();
    REQUIRE(std::strlen(loc.file_name()) > 0);
    REQUIRE(std::strlen(loc.function_name()) > 0);
    REQUIRE(loc.line() == static_cast<std::uint_least32_t>(first_line));
}

namespace {
bridge::source_location where_am_i(bridge::source_location loc = bridge::source_location::current()) { return loc; }
} // namespace

TEST_CASE("bridge::source_location::current, as a default argument, captures its caller's call site",
          "[deck][source_location]") {
    auto call_line = __LINE__ + 1;
    auto loc = where_am_i();
    REQUIRE(loc.line() == static_cast<std::uint_least32_t>(call_line));
}
