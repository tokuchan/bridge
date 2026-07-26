#include <catch2/catch_test_macros.hpp>

// This file only exercises anything meaningful when compiled under a
// standard/toolchain where std::expected is actually available --
// gated on the same Feature Test deck/cpp17/expected.hpp uses to
// select passthrough, so this never tries to #include <expected> on a
// toolchain that doesn't have it. See tests/deck/CMakeLists.txt: it's
// only added to the C++23-forced target, matching how
// bridge_deck_optional_cpp23_tests exercises optional's own
// passthrough path.
#include <rivets/features.hpp>

#if BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L

#    include <expected>
#    include <string>
#    include <type_traits>

#    include <deck/cpp17/expected.hpp>

namespace {

struct throwing_move {
    throwing_move() = default;
    throwing_move(const throwing_move&) = default;
    throwing_move(throwing_move&&) noexcept(false) {}
    throwing_move& operator=(const throwing_move&) = default;
    throwing_move& operator=(throwing_move&&) noexcept(false) { return *this; }
};

// docs/adr/0010-expected-truss-owns-the-class.md's decision #6: Truss
// never passes through, so bridge::truss::expected<T,E> and
// std::expected<T,E> genuinely coexist as distinct types in this one
// C++23 translation unit -- letting these static_asserts compare the
// polyfill directly against the real type, rather than testing each
// path in isolation and hoping they agree.
template <class T, class E>
constexpr bool trait_parity_v =
    std::is_copy_constructible_v<bridge::truss::expected<T, E>> == std::is_copy_constructible_v<std::expected<T, E>> &&
    std::is_move_constructible_v<bridge::truss::expected<T, E>> == std::is_move_constructible_v<std::expected<T, E>> &&
    std::is_copy_assignable_v<bridge::truss::expected<T, E>> == std::is_copy_assignable_v<std::expected<T, E>> &&
    std::is_move_assignable_v<bridge::truss::expected<T, E>> == std::is_move_assignable_v<std::expected<T, E>>;

// A nothrow-move type on both sides.
static_assert(trait_parity_v<int, std::string>);
// A throwing-move type on one side -- exercises decision #5's
// nothrow-move-OR condition on assignment (both should be assignable,
// since std::string is nothrow-move-constructible).
static_assert(trait_parity_v<throwing_move, std::string>);
static_assert(trait_parity_v<std::string, throwing_move>);
// A throwing-move type on *both* sides -- both should have assignment
// deleted entirely.
static_assert(trait_parity_v<throwing_move, throwing_move>);
static_assert(!std::is_copy_assignable_v<bridge::truss::expected<throwing_move, throwing_move>>);
static_assert(!std::is_copy_assignable_v<std::expected<throwing_move, throwing_move>>);

// bridge::deck::expected (and bridge::expected) is a plain alias to
// std::expected under this Feature Test value -- not another distinct
// type, confirming the passthrough path is actually selected here.
static_assert(std::is_same_v<bridge::deck::expected<int, std::string>, std::expected<int, std::string>>);
static_assert(std::is_same_v<bridge::expected<int, std::string>, std::expected<int, std::string>>);

} // namespace

TEST_CASE("bridge::expected is a passthrough alias to std::expected under C++23", "[deck][expected][differential]") {
    bridge::expected<int, std::string> e{42};
    std::expected<int, std::string> native{42};
    static_assert(std::is_same_v<decltype(e), decltype(native)>);
    REQUIRE(*e == *native);
}

#else

TEST_CASE("expected differential trait checks skipped: __cpp_lib_expected unavailable here",
          "[deck][expected][differential]") {
    SUCCEED();
}

#endif
