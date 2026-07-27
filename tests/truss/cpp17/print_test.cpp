#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <sstream>
#include <string>

#include <truss/cpp17/print.hpp>

namespace {

// fmemopen gives a real FILE* backed by an in-memory buffer, so
// print/println's FILE*-targeting overloads can be exercised and
// asserted on directly, the same way the ostream-targeting overloads
// are tested against an ostringstream.
std::string captured_from_file_print(void (*fn)(std::FILE*)) {
    char buf[256] = {};
    std::FILE* f = fmemopen(buf, sizeof(buf), "w");
    REQUIRE(f != nullptr);
    fn(f);
    std::fclose(f);
    return std::string(buf);
}

} // namespace

TEST_CASE("print writes formatted output to a FILE*", "[truss][print]") {
    auto out = captured_from_file_print([](std::FILE* f) { bridge::truss::print(f, "hello {}", "world"); });
    REQUIRE(out == "hello world");
}

TEST_CASE("println writes formatted output plus a newline to a FILE*", "[truss][print]") {
    auto out = captured_from_file_print([](std::FILE* f) { bridge::truss::println(f, "value: {}", 42); });
    REQUIRE(out == "value: 42\n");
}

TEST_CASE("print writes formatted output to an ostream", "[truss][print]") {
    std::ostringstream oss;
    bridge::truss::print(oss, "to stream: {}", 3);
    REQUIRE(oss.str() == "to stream: 3");
}

TEST_CASE("println writes formatted output plus a newline to an ostream", "[truss][print]") {
    std::ostringstream oss;
    bridge::truss::println(oss, "value: {}", 4);
    REQUIRE(oss.str() == "value: 4\n");
}

TEST_CASE("print/println without an explicit stream compile against stdout", "[truss][print]") {
    // Correctness of the no-stream overloads resolving to stdout is
    // covered by the fact that this compiles and links at all (the
    // overload set is unambiguous); actual stdout content isn't
    // captured here to avoid interfering with the test runner's own
    // output.
    bridge::truss::print("");
    bridge::truss::println("");
    SUCCEED();
}

TEST_CASE("print/println propagate format_error for a malformed format string", "[truss][print]") {
    std::ostringstream oss;
    REQUIRE_THROWS_AS(bridge::truss::print(oss, "{"), bridge::truss::format_error);
    REQUIRE_THROWS_AS(bridge::truss::println(oss, "{5}", 1), bridge::truss::format_error);
}

TEST_CASE("print/println work with no arguments", "[truss][print]") {
    std::ostringstream oss;
    bridge::truss::print(oss, "no args here");
    bridge::truss::println(oss, "");
    REQUIRE(oss.str() == "no args here\n");
}
