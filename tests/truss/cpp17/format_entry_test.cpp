#include <catch2/catch_test_macros.hpp>

#include <string>

#include <truss/cpp17/format.hpp>

using namespace bridge::truss;

// Every case below was cross-checked against real std::vformat
// directly before being written here.

TEST_CASE("format handles basic substitution and multiple arguments", "[truss][format][entry]") {
    REQUIRE(format("{}", 42) == "42");
    REQUIRE(format("{} {}", 1, 2) == "1 2");
    REQUIRE(format("{}-{}-{}", 1, 2, 3) == "1-2-3");
    REQUIRE(format("no args here") == "no args here");
}

TEST_CASE("format handles explicit (manual) argument indices", "[truss][format][entry]") {
    REQUIRE(format("{1} {0}", "world", "hello") == "hello world");
    REQUIRE(format("{0}{0}{0}", "x") == "xxx");
}

TEST_CASE("format handles escaped braces", "[truss][format][entry]") {
    REQUIRE(format("{{literal}} {}", 5) == "{literal} 5");
}

TEST_CASE("format handles dynamic width and precision via {}", "[truss][format][entry]") {
    REQUIRE(format("{:{}}", 42, 6) == "    42");
    REQUIRE(format("{:.{}}", 3.14159, 2) == "3.1");
    REQUIRE(format("{:{}.{}}", 3.14159, 8, 2) == "     3.1");
}

TEST_CASE("format handles mixed argument types in one call", "[truss][format][entry]") {
    REQUIRE(format("{} {} {} {}", 1, 3.14, "hi", true) == "1 3.14 hi true");
}

TEST_CASE("format throws on an unmatched opening brace", "[truss][format][entry]") {
    REQUIRE_THROWS_AS(format("{"), format_error);
}

TEST_CASE("format throws on an unmatched closing brace", "[truss][format][entry]") {
    REQUIRE_THROWS_AS(format("}"), format_error);
}

TEST_CASE("format throws on an out-of-range explicit argument index", "[truss][format][entry]") {
    REQUIRE_THROWS_AS(format("{5}", 1), format_error);
}

TEST_CASE("format throws on mixing automatic and manual argument indexing",
          "[truss][format][entry]") {
    REQUIRE_THROWS_AS(format("{} {0}", 1, 2), format_error);
}

TEST_CASE("format_to writes through a generic output iterator", "[truss][format][entry]") {
    std::string out;
    auto it = format_to(std::back_inserter(out), "{}-{}", 1, 2);
    (void)it;
    REQUIRE(out == "1-2");
}

TEST_CASE("format_to_n truncates output but reports the untruncated size",
          "[truss][format][entry]") {
    std::string full = format("{} + {} = {}", 1, 2, 3);
    std::string buf(3, '\0');
    auto result = format_to_n(buf.begin(), 3, "{} + {} = {}", 1, 2, 3);
    std::string truncated(buf.begin(), result.out);
    REQUIRE(truncated == full.substr(0, 3));
    REQUIRE(static_cast<std::size_t>(result.size) == full.size());
}

TEST_CASE("formatted_size matches the length of the equivalent format call",
          "[truss][format][entry]") {
    std::string full = format("{} + {} = {}", 1, 2, 3);
    REQUIRE(formatted_size("{} + {} = {}", 1, 2, 3) == full.size());
}

TEST_CASE("vformat formats a type-erased argument pack built via make_format_args",
          "[truss][format][entry]") {
    int a = 1;
    int b = 2;
    REQUIRE(vformat("{} {}", make_format_args(a, b)) == "1 2");
}

TEST_CASE("format_string implicitly converts from a string literal", "[truss][format][entry]") {
    format_string<int> fs = "value: {}";
    REQUIRE(fs.get() == "value: {}");
}
