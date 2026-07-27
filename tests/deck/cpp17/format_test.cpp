#include <catch2/catch_test_macros.hpp>

#include <string>

#include <deck/cpp17/format.hpp>

TEST_CASE("bridge::format formats basic arguments", "[deck][format]") {
    REQUIRE(bridge::format("{} + {} = {}", 1, 2, 3) == "1 + 2 = 3");
    REQUIRE(bridge::format("{:>5}", "hi") == "   hi");
}

TEST_CASE("bridge::format_to writes through an output iterator", "[deck][format]") {
    std::string out;
    bridge::format_to(std::back_inserter(out), "{}-{}", "a", "b");
    REQUIRE(out == "a-b");
}

TEST_CASE("bridge::format_to_n truncates and reports the untruncated size", "[deck][format]") {
    std::string out(3, '\0');
    auto result = bridge::format_to_n(out.begin(), 3, "{}", "hello");
    REQUIRE(std::string(out.begin(), result.out) == "hel");
    REQUIRE(result.size == 5);
}

TEST_CASE("bridge::formatted_size computes length without building a string", "[deck][format]") {
    REQUIRE(bridge::formatted_size("{}-{}", 12, 34) == 5);
}

TEST_CASE("bridge::vformat formats type-erased arguments", "[deck][format]") {
    int value = 42;
    REQUIRE(bridge::vformat("{}", bridge::make_format_args(value)) == "42");
}

TEST_CASE("bridge::vformat propagates format_error for a malformed format string", "[deck][format]") {
    // bridge::format takes its malformed-ness through vformat here, not
    // format() with a literal: real std::format_string validates
    // literal format strings at compile time (consteval), so a
    // malformed literal fails to *compile* under passthrough rather
    // than throwing at runtime -- exactly the disclosed
    // compile-time-vs-runtime-validation divergence in docs/adr/0012.
    // vformat's std::string_view argument is never consteval-checked
    // on either path, so it's the one entry point safe to test this
    // way uniformly.
    REQUIRE_THROWS_AS(bridge::vformat("{", bridge::make_format_args()), bridge::format_error);
}
