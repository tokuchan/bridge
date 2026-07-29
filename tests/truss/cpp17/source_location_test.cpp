#include <catch2/catch_test_macros.hpp>

#include <cstring>

#include <rivets/clang.hpp>
#include <truss/cpp17/source_location.hpp>

TEST_CASE("bridge::truss::source_location default-constructs with an empty, zeroed state", "[truss][source_location]") {
    bridge::truss::source_location loc;
    REQUIRE(std::strlen(loc.file_name()) == 0);
    REQUIRE(std::strlen(loc.function_name()) == 0);
    REQUIRE(loc.line() == 0);
    REQUIRE(loc.column() == 0);
}

namespace {
bridge::truss::source_location where_am_i(bridge::truss::source_location loc = bridge::truss::source_location::current()) {
    return loc;
}
} // namespace

TEST_CASE("bridge::truss::source_location::current captures a real call site", "[truss][source_location]") {
    auto loc = bridge::truss::source_location::current();
    REQUIRE(std::strlen(loc.file_name()) > 0);
    REQUIRE(std::strlen(loc.function_name()) > 0);
    REQUIRE(loc.line() > 0);
}

TEST_CASE("bridge::truss::source_location::current, as a default argument, captures its caller's call site",
          "[truss][source_location]") {
    auto call_line = __LINE__ + 1;
    auto loc = where_am_i();
    REQUIRE(loc.line() == static_cast<std::uint_least32_t>(call_line));
}

TEST_CASE("bridge::truss::source_location::current reports the exact line it is called on", "[truss][source_location]") {
    auto first_line = __LINE__ + 1;
    auto loc = bridge::truss::source_location::current();
    REQUIRE(loc.line() == static_cast<std::uint_least32_t>(first_line));
}

TEST_CASE("bridge::truss::source_location::column matches this project's disclosed compiler split", "[truss][source_location]") {
    // Confirmed by direct compiler probe (not assumed) before this
    // polyfill was written: GCC has never implemented a public
    // __builtin_COLUMN(), on any version, so this polyfill's column()
    // always reports 0 on GCC. Clang has had __builtin_COLUMN() since
    // Clang 9, well below this project's matrix floor (Clang 18), so
    // this polyfill's column() reports the real column there. See
    // docs/adr/0019-source-location-truss-owns-the-class.md.
    auto loc = bridge::truss::source_location::current();
#if BRIDGE_RIVETS_CLANG_GE(9)
    REQUIRE(loc.column() > 0);
#else
    REQUIRE(loc.column() == 0);
#endif
}
