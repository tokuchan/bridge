#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

#include <truss/cpp17/format.hpp>

using bridge::truss::format_context;
using bridge::truss::format_error;
using bridge::truss::format_parse_context;
using bridge::truss::formatter;
namespace fd = bridge::detail::truss::cpp17::format;

namespace {

// Every case below was cross-checked against real std::format directly
// (both individually and via a ~200-combination batch differential
// probe) before being written here, not derived from the standard
// text alone.
template <class T>
std::string do_format(std::string_view spec_str, const T& value) {
    std::string out;
    fd::dynamic_arg_source src([](std::size_t) -> long long { throw format_error("no dynamic args in this test"); });
    format_context<std::back_insert_iterator<std::string>> ctx(std::back_inserter(out), src);
    format_parse_context pctx(spec_str);
    formatter<T> f;
    f.parse(pctx);
    f.format(value, ctx);
    return out;
}

} // namespace

TEST_CASE("integer formatter: basic and signed", "[truss][format][builtin]") {
    REQUIRE(do_format("", 42) == "42");
    REQUIRE(do_format("", -42) == "-42");
    REQUIRE(do_format("+", 42) == "+42");
    REQUIRE(do_format(" ", 42) == " 42");
}

TEST_CASE("integer formatter: alternate bases with alternate form", "[truss][format][builtin]") {
    REQUIRE(do_format("#x", 255) == "0xff");
    REQUIRE(do_format("#X", 255) == "0XFF");
    REQUIRE(do_format("#b", 5) == "0b101");
    REQUIRE(do_format("#o", 8) == "010");
}

TEST_CASE("integer formatter: zero-pad interacts correctly with sign and prefix",
          "[truss][format][builtin]") {
    REQUIRE(do_format("08", 255) == "00000255");
    REQUIRE(do_format("+08", 5) == "+0000005");
    REQUIRE(do_format("#08x", 255) == "0x0000ff");
}

TEST_CASE("integer formatter: width, alignment, and fill", "[truss][format][builtin]") {
    REQUIRE(do_format("10", 42) == "        42");
    REQUIRE(do_format("<10", 42) == "42        ");
    REQUIRE(do_format("^10", 42) == "    42    ");
    REQUIRE(do_format("*^12", 42) == "*****42*****");
}

TEST_CASE("integer formatter: rejects precision", "[truss][format][builtin]") {
    REQUIRE_THROWS_AS(do_format(".2", 42), format_error);
}

TEST_CASE("floating-point formatter: default is shortest round-trip", "[truss][format][builtin]") {
    REQUIRE(do_format("", 3.14159) == "3.14159");
    REQUIRE(do_format("", 0.0001234) == "0.0001234");
}

TEST_CASE("floating-point formatter: explicit presentation types with precision",
          "[truss][format][builtin]") {
    REQUIRE(do_format(".2f", 3.14159) == "3.14");
    REQUIRE(do_format(".3e", 12345.6789) == "1.235e+04");
    REQUIRE(do_format("E", 12345.6789) == "1.234568E+04");
    REQUIRE(do_format("a", 255.5) == "1.ffp+7");
}

TEST_CASE("floating-point formatter: f/e/g default to precision 6 without an explicit one",
          "[truss][format][builtin]") {
    REQUIRE(do_format("f", 3.14159265358979) == "3.141593");
    REQUIRE(do_format("g", 3.14159265358979) == "3.14159");
    REQUIRE(do_format("e", 3.14159265358979) == "3.141593e+00");
}

TEST_CASE("floating-point formatter: sign and zero-pad", "[truss][format][builtin]") {
    REQUIRE(do_format("+", 3.14) == "+3.14");
    REQUIRE(do_format("", -3.14) == "-3.14");
    REQUIRE(do_format("010", 3.14) == "0000003.14");
}

TEST_CASE("bool formatter: default is true/false, string-like left alignment",
          "[truss][format][builtin]") {
    REQUIRE(do_format("", true) == "true");
    REQUIRE(do_format("", false) == "false");
    REQUIRE(do_format("6", true) == "true  ");
    REQUIRE(do_format(">6", true) == "  true");
}

TEST_CASE("bool formatter: numeric presentation gives 0/1", "[truss][format][builtin]") {
    REQUIRE(do_format("d", true) == "1");
    REQUIRE(do_format("d", false) == "0");
}

TEST_CASE("bool formatter: rejects precision", "[truss][format][builtin]") {
    REQUIRE_THROWS_AS(do_format(".2", true), format_error);
}

TEST_CASE("char formatter: default writes the character, string-like left alignment",
          "[truss][format][builtin]") {
    REQUIRE(do_format("", 'x') == "x");
    REQUIRE(do_format("c", 'x') == "x");
    REQUIRE(do_format("5", 'x') == "x    ");
    REQUIRE(do_format(">5", 'x') == "    x");
}

TEST_CASE("char formatter: numeric presentation gives the underlying value",
          "[truss][format][builtin]") {
    REQUIRE(do_format("d", 'x') == "120");
    REQUIRE(do_format("#x", 'x') == "0x78");
}

TEST_CASE("char formatter: debug format wraps in single quotes with escaping",
          "[truss][format][builtin]") {
    REQUIRE(do_format("?", 'x') == "'x'");
    REQUIRE(do_format("?", '\n') == "'\\n'");
}

TEST_CASE("pointer formatter: 0x-prefixed hex, right-aligned by default", "[truss][format][builtin]") {
    REQUIRE(do_format("", nullptr) == "0x0");
    REQUIRE(do_format("20", nullptr) == "                 0x0");

    int x = 0;
    void* p = &x;
    REQUIRE(do_format("", p).substr(0, 2) == "0x");
}

TEST_CASE("pointer formatter: rejects precision", "[truss][format][builtin]") {
    REQUIRE_THROWS_AS(do_format(".2", nullptr), format_error);
}

TEST_CASE("string formatter: default writes content as-is, left-aligned by default",
          "[truss][format][builtin]") {
    REQUIRE(do_format("", std::string("hello")) == "hello");
    REQUIRE(do_format("10", std::string("hi")) == "hi        ");
}

TEST_CASE("string formatter: precision truncates by byte length", "[truss][format][builtin]") {
    REQUIRE(do_format(".3", std::string("hello")) == "hel");
    REQUIRE(do_format("10.3", std::string("hello")) == "hel       ");
}

TEST_CASE("string formatter: debug format quotes and escapes", "[truss][format][builtin]") {
    REQUIRE(do_format("?", std::string("hi\"there\n")) == "\"hi\\\"there\\n\"");
}

TEST_CASE("string_view and const char* formatters work the same way", "[truss][format][builtin]") {
    std::string_view sv = "hello";
    REQUIRE(do_format("", sv) == "hello");
    const char* cs = "hello";
    REQUIRE(do_format("", cs) == "hello");
}

TEST_CASE("formatter<T>::parse throws on an unknown presentation type", "[truss][format][builtin]") {
    REQUIRE_THROWS_AS(do_format("z", 42), format_error);
    REQUIRE_THROWS_AS(do_format("z", 3.14), format_error);
}
