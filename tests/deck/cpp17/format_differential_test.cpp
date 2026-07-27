#include <catch2/catch_test_macros.hpp>

#include <string>

#include <deck/cpp17/format.hpp>

namespace {

// A user type formattable via *both* engines -- the pattern
// docs/adr/0012's "formatter<T> is not transparently unifiable"
// section discloses: since bridge::formatter<T> is an alias template
// on both branches (C++ can't specialize an alias template), a type
// meant to stay formattable across the passthrough boundary needs two
// separate specializations, one per engine. The bridge::truss::formatter
// specialization below is unconditional (not gated on
// BRIDGE_RIVETS_FEATURES_LIB_FORMAT) so it's compiled and exercised
// under every standard this suite runs at, including the default
// C++17 build -- not just the C++20-forced passthrough target further
// down this file.
struct point {
    int x;
    int y;
};

} // namespace

// The polyfill-path specialization: picked up by bridge::truss::format
// always (Truss never passes through), and by bridge::format too on any
// standard below the passthrough threshold.
template <>
struct bridge::truss::formatter<point> {
    bridge::truss::format_parse_context::iterator parse(bridge::truss::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <class FormatContext>
    auto format(const point& p, FormatContext& ctx) const -> typename FormatContext::iterator {
        auto out = ctx.out();
        std::string s = "(" + std::to_string(p.x) + "," + std::to_string(p.y) + ")";
        for (char c : s) *out++ = c;
        return out;
    }
};

TEST_CASE("a user type with only the polyfill's formatter specialization formats via bridge::truss::format",
          "[deck][format][differential]") {
    point p{3, 4};
    REQUIRE(bridge::truss::format("{}", p) == "(3,4)");
}

// Everything below only exercises anything meaningful when compiled
// under a standard/toolchain where std::format is actually available
// -- gated on the same Feature Test deck/cpp17/format.hpp uses to
// select passthrough, so this never tries to #include <format> on a
// toolchain that doesn't have it. See tests/deck/CMakeLists.txt: it's
// only added to the C++20-forced target, matching how
// bridge_deck_expected_cpp23_tests exercises expected's own
// passthrough path.
#include <rivets/features.hpp>

#if BRIDGE_RIVETS_FEATURES_LIB_FORMAT >= 201907L

#    include <format>
#    include <type_traits>

// The passthrough-path specialization: picked up by std::format (and
// so, in this translation unit, by bridge::format itself, since
// BRIDGE_RIVETS_FEATURES_LIB_FORMAT selects passthrough here). Real
// std::formatter<T> requires this exact shape -- a constexpr parse
// returning an iterator, a const format templated on the context type.
template <>
struct std::formatter<point> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <class FormatContext>
    auto format(const point& p, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "({},{})", p.x, p.y);
    }
};

namespace {

// bridge::formatter (and bridge::format_context, bridge::format_error,
// etc.) are plain aliases to whichever path is active under
// passthrough -- not another distinct type -- confirming the
// passthrough path is actually selected in this translation unit, the
// same style of assertion expected_differential_test.cpp makes for
// bridge::deck::expected.
static_assert(std::is_same_v<bridge::deck::format_error, std::format_error>);
static_assert(std::is_same_v<bridge::deck::format_parse_context, std::format_parse_context>);

} // namespace

TEST_CASE("a type with both formatter specializations formats correctly through both engines",
          "[deck][format][differential]") {
    point p{3, 4};

    // bridge::format is std::format here (passthrough) -- looks up
    // std::formatter<point>.
    REQUIRE(bridge::format("{}", p) == "(3,4)");

    // bridge::truss::format never passes through -- always looks up
    // bridge::truss::formatter<point>, regardless of which path
    // bridge::format itself selected.
    REQUIRE(bridge::truss::format("{}", p) == "(3,4)");

    // Both engines agree on this type, demonstrating the disclosed
    // dual-specialization pattern actually works end to end, not just
    // compiles.
    REQUIRE(bridge::format("{}", p) == bridge::truss::format("{}", p));
}

TEST_CASE("bridge::format matches real std::format for built-in types", "[deck][format][differential]") {
    REQUIRE(bridge::format("{:>6.2f}", 3.14159) == std::format("{:>6.2f}", 3.14159));
    REQUIRE(bridge::format("{:#x}", 255) == std::format("{:#x}", 255));
    REQUIRE(bridge::format("{:^10}", "hi") == std::format("{:^10}", "hi"));
}

#else

TEST_CASE("format differential trait checks skipped: __cpp_lib_format unavailable here",
          "[deck][format][differential]") {
    SUCCEED();
}

#endif
