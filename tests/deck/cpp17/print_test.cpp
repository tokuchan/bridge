#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <sstream>
#include <string>

#include <deck/cpp17/print.hpp>

namespace {

// Same fmemopen technique as truss/cpp17/print_test.cpp: gives a real
// FILE* backed by an in-memory buffer so the FILE*-targeting overloads
// can be exercised and asserted on directly.
std::string captured_from_file_print(void (*fn)(std::FILE*)) {
    char buf[256] = {};
    std::FILE* f = fmemopen(buf, sizeof(buf), "w");
    REQUIRE(f != nullptr);
    fn(f);
    std::fclose(f);
    return std::string(buf);
}

} // namespace

TEST_CASE("bridge::print writes formatted output to a FILE*", "[deck][print]") {
    auto out = captured_from_file_print([](std::FILE* f) { bridge::print(f, "hello {}", "world"); });
    REQUIRE(out == "hello world");
}

TEST_CASE("bridge::println writes formatted output plus a newline to a FILE*", "[deck][print]") {
    auto out = captured_from_file_print([](std::FILE* f) { bridge::println(f, "value: {}", 42); });
    REQUIRE(out == "value: 42\n");
}

TEST_CASE("bridge::print writes formatted output to an ostream", "[deck][print]") {
    std::ostringstream oss;
    bridge::print(oss, "to stream: {}", 3);
    REQUIRE(oss.str() == "to stream: 3");
}

TEST_CASE("bridge::println writes formatted output plus a newline to an ostream", "[deck][print]") {
    std::ostringstream oss;
    bridge::println(oss, "value: {}", 4);
    REQUIRE(oss.str() == "value: 4\n");
}

TEST_CASE("bridge::print/println without an explicit stream compile against stdout", "[deck][print]") {
    // Same rationale as truss/cpp17/print_test.cpp's equivalent case:
    // correctness of overload resolution to stdout is covered by
    // compiling and linking cleanly; stdout content isn't captured
    // here to avoid interfering with the test runner's own output.
    bridge::print("");
    bridge::println("");
    SUCCEED();
}
