#include <catch2/catch_test_macros.hpp>

// This file only exercises anything meaningful when compiled under a
// standard/toolchain where std::span is actually available -- gated
// on the same Feature Test deck/cpp17/span.hpp uses to select
// passthrough, so this never tries to #include <span> on a toolchain
// that doesn't have it. See tests/deck/CMakeLists.txt: it's only added
// to the C++20-forced target, matching how
// bridge_deck_expected_cpp23_tests exercises expected's own
// passthrough path.
#include <rivets/features.hpp>

#if BRIDGE_RIVETS_FEATURES_LIB_SPAN >= 202002L

#    include <array>
#    include <span>
#    include <type_traits>

#    include <deck/cpp17/span.hpp>

namespace {

// bridge::deck::span (and bridge::span) is a plain alias to std::span
// under this Feature Test value -- not another distinct type,
// confirming the passthrough path is actually selected here.
static_assert(std::is_same_v<bridge::deck::span<int>, std::span<int>>);
static_assert(std::is_same_v<bridge::span<int, 4>, std::span<int, 4>>);
static_assert(bridge::dynamic_extent == std::dynamic_extent);

// Truss never passes through (docs/adr/0010's rule, applied to span
// per docs/adr/0015), so bridge::truss::span and std::span genuinely
// coexist as distinct types in this one C++20 translation unit,
// letting these static_asserts compare the polyfill directly against
// the real type, rather than testing each path in isolation and
// hoping they agree.
template <class T, std::size_t Extent>
constexpr bool trait_parity_v = std::is_trivially_copyable_v<bridge::truss::span<T, Extent>> ==
                                     std::is_trivially_copyable_v<std::span<T, Extent>> &&
                                 std::is_default_constructible_v<bridge::truss::span<T, Extent>> ==
                                     std::is_default_constructible_v<std::span<T, Extent>> &&
                                 sizeof(bridge::truss::span<T, Extent>) == sizeof(std::span<T, Extent>);

static_assert(trait_parity_v<int, std::dynamic_extent>);
static_assert(trait_parity_v<int, 4>);
static_assert(trait_parity_v<int, 0>);

} // namespace

TEST_CASE("bridge::span is a passthrough alias to std::span under C++20", "[deck][span][differential]") {
    int arr[3] = {1, 2, 3};
    bridge::span<int> s(arr);
    std::span<int> native(arr);
    static_assert(std::is_same_v<decltype(s), decltype(native)>);
    REQUIRE(s.size() == native.size());
    REQUIRE(s[0] == native[0]);
}

TEST_CASE("bridge::truss::span and real std::span agree on behavior directly", "[deck][span][differential]") {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    bridge::truss::span<int, 5> truss_fixed(arr);
    std::span<int, 5> std_fixed(arr);
    REQUIRE(truss_fixed.size() == std_fixed.size());

    auto truss_first = truss_fixed.first(2);
    auto std_first = std_fixed.first(2);
    REQUIRE(truss_first.size() == std_first.size());
    REQUIRE(truss_first[0] == std_first[0]);
    REQUIRE(truss_first[1] == std_first[1]);

    auto truss_sub = truss_fixed.subspan(1, 2);
    auto std_sub = std_fixed.subspan(1, 2);
    REQUIRE(truss_sub[0] == std_sub[0]);
    REQUIRE(truss_sub[1] == std_sub[1]);

    auto truss_bytes = bridge::truss::as_bytes(truss_fixed);
    auto std_bytes = std::as_bytes(std_fixed);
    REQUIRE(truss_bytes.size() == std_bytes.size());
}

#else

TEST_CASE("span differential trait checks skipped: __cpp_lib_span unavailable here",
          "[deck][span][differential]") {
    SUCCEED();
}

#endif
