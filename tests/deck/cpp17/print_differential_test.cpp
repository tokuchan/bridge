#include <catch2/catch_test_macros.hpp>

// Same shape as format_differential_test.cpp: only exercises anything
// meaningful when compiled under a standard/toolchain where
// std::print/std::println are actually available -- gated on the same
// Feature Test deck/cpp17/print.hpp uses to select passthrough. See
// tests/deck/CMakeLists.txt: only added to the C++23-forced print
// target.
#include <rivets/features.hpp>

#if BRIDGE_RIVETS_FEATURES_LIB_PRINT >= 202211L

#    include <cstdio>
#    include <print>
#    include <sstream>
#    include <string>

#    include <deck/cpp17/print.hpp>

namespace {

std::string captured_from_file(void (*fn)(std::FILE*)) {
    char buf[256] = {};
    std::FILE* f = fmemopen(buf, sizeof(buf), "w");
    REQUIRE(f != nullptr);
    fn(f);
    std::fclose(f);
    return std::string(buf);
}

} // namespace

TEST_CASE("bridge::print matches real std::print for FILE* output", "[deck][print][differential]") {
    auto bridge_out = captured_from_file([](std::FILE* f) { bridge::print(f, "{} + {} = {}", 1, 2, 3); });
    auto std_out = captured_from_file([](std::FILE* f) { std::print(f, "{} + {} = {}", 1, 2, 3); });
    REQUIRE(bridge_out == std_out);
}

TEST_CASE("bridge::println matches real std::println for FILE* output", "[deck][print][differential]") {
    auto bridge_out = captured_from_file([](std::FILE* f) { bridge::println(f, "value: {}", 42); });
    auto std_out = captured_from_file([](std::FILE* f) { std::println(f, "value: {}", 42); });
    REQUIRE(bridge_out == std_out);
}

TEST_CASE("bridge::print matches real std::print for ostream output", "[deck][print][differential]") {
    std::ostringstream bridge_oss;
    bridge::print(bridge_oss, "{} + {} = {}", 1, 2, 3);
    std::ostringstream std_oss;
    std::print(std_oss, "{} + {} = {}", 1, 2, 3);
    REQUIRE(bridge_oss.str() == std_oss.str());
}

// Truss never passes through (docs/adr/0010's rule, applied to
// format/print per docs/adr/0012), so bridge::truss::print and
// std::print genuinely coexist as distinct things to compare directly
// in this one C++23 translation unit, rather than testing each path
// in isolation and hoping they agree.
TEST_CASE("bridge::truss::print matches real std::print directly, unaffected by passthrough",
          "[deck][print][differential]") {
    auto truss_out = captured_from_file([](std::FILE* f) { bridge::truss::print(f, "{:.2f}", 3.14159); });
    auto std_out = captured_from_file([](std::FILE* f) { std::print(f, "{:.2f}", 3.14159); });
    REQUIRE(truss_out == std_out);
}

#else

TEST_CASE("print differential trait checks skipped: __cpp_lib_print unavailable here",
          "[deck][print][differential]") {
    SUCCEED();
}

#endif
