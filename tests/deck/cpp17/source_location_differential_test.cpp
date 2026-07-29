#include <catch2/catch_test_macros.hpp>

// This file only exercises anything meaningful when compiled under a
// standard/toolchain where std::source_location is actually available
// -- gated on the same condition deck/cpp17/source_location.hpp uses
// to select passthrough (BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH, a
// bare Feature Test check -- see docs/adr/0019), so this never tries
// to #include <source_location> on a toolchain that doesn't have it.
// See tests/deck/CMakeLists.txt: only added to the C++20-forced
// target, matching how bridge_deck_span_cpp20_tests exercises span's
// own passthrough path.
#include <deck/cpp17/source_location.hpp>

#if BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH

#    include <cstring>
#    include <source_location>
#    include <string>
#    include <type_traits>

#    include <rivets/clang.hpp>
#    include <truss/cpp17/source_location.hpp>

namespace {

// bridge::deck::source_location (and bridge::source_location) are
// plain aliases to the real type under this passthrough condition --
// not another distinct type, confirming the passthrough path is
// actually selected here.
static_assert(std::is_same_v<bridge::deck::source_location, std::source_location>);
static_assert(std::is_same_v<bridge::source_location, std::source_location>);

// Both current() calls default-evaluate here, at the same textual
// call site, so their captured line/function/file agree by
// construction -- this is what lets the differential tests below
// compare them meaningfully.
void capture_both(bridge::truss::source_location& truss_out, std::source_location& std_out,
                   bridge::truss::source_location truss_loc = bridge::truss::source_location::current(),
                   std::source_location std_loc = std::source_location::current()) {
    truss_out = truss_loc;
    std_out = std_loc;
}

} // namespace

TEST_CASE("bridge::source_location is a passthrough alias to std::source_location under C++20",
          "[deck][source_location][differential]") {
    auto loc = bridge::source_location::current();
    static_assert(std::is_same_v<decltype(loc), std::source_location>);
    REQUIRE(loc.line() > 0);
}

TEST_CASE("bridge::truss::source_location and real std::source_location agree on line/file",
          "[deck][source_location][differential]") {
    bridge::truss::source_location truss_loc;
    std::source_location std_loc;
    capture_both(truss_loc, std_loc);

    REQUIRE(truss_loc.line() == std_loc.line());
    REQUIRE(std::strcmp(truss_loc.file_name(), std_loc.file_name()) == 0);
}

TEST_CASE("bridge::truss::source_location's function_name() divergence from real std::source_location is disclosed, not silent",
          "[deck][source_location][differential]") {
    // docs/adr/0019: confirmed by direct probe against this project's
    // own toolchains (GCC, and Clang paired with libstdc++, the only
    // pairing this project's matrix covers): real
    // std::source_location::function_name() reports a full function
    // signature on libstdc++ (e.g. "int main()"), while Truss's
    // polyfill, using the portable __builtin_FUNCTION(), can only
    // report the bare name (e.g. "main"). This is a standard-library
    // fact, not a Clang-vs-GCC split -- unlike the column() gap below,
    // this test does not branch on BRIDGE_RIVETS_CLANG_GE. This test
    // documents the divergence directly: the bare name is still a
    // substring of the real signature, rather than asserting an
    // equality libstdc++ never satisfies.
    bridge::truss::source_location truss_loc;
    std::source_location std_loc;
    capture_both(truss_loc, std_loc);

    std::string real_signature(std_loc.function_name());
    REQUIRE(real_signature.find(truss_loc.function_name()) != std::string::npos);
}

TEST_CASE("bridge::truss::source_location's column() divergence from real std::source_location is disclosed, not silent",
          "[deck][source_location][differential]") {
    // docs/adr/0019: Truss's polyfill can't portably read a column
    // number on a toolchain with no public column-number builtin --
    // chiefly GCC, which has never implemented one. Real
    // std::source_location, once available (this test only compiles
    // when it is), always reports the true column, even on that same
    // toolchain. This test documents the divergence directly, rather
    // than asserting an equality that GCC could never satisfy.
    bridge::truss::source_location truss_loc;
    std::source_location std_loc;
    capture_both(truss_loc, std_loc);

#    if BRIDGE_RIVETS_CLANG_GE(9)
    REQUIRE(truss_loc.column() == std_loc.column());
#    else
    REQUIRE(truss_loc.column() == 0);
    REQUIRE(std_loc.column() > 0);
#    endif
}

#else

TEST_CASE("source_location differential trait checks skipped: passthrough condition unmet here",
          "[deck][source_location][differential]") {
    SUCCEED();
}

#endif
