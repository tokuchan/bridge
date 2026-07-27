#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

#include <truss/cpp17/format.hpp>

using bridge::truss::format_error;
using bridge::truss::format_parse_context;
namespace fmt_detail = bridge::detail::truss::cpp17::format;
using fmt_detail::align_t;
using fmt_detail::parse_std_spec;
using fmt_detail::width_or_precision;

TEST_CASE("parse_std_spec reads a bare type character", "[truss][format][parser]") {
    format_parse_context pctx("d");
    auto spec = parse_std_spec(pctx, "dbxX");
    REQUIRE(spec.type == 'd');
    REQUIRE(pctx.begin() == pctx.end());
}

TEST_CASE("parse_std_spec reads fill, align, and width", "[truss][format][parser]") {
    format_parse_context pctx("*>10d");
    auto spec = parse_std_spec(pctx, "d");
    REQUIRE(spec.fill == '*');
    REQUIRE(spec.align == align_t::right);
    REQUIRE(spec.width.k == width_or_precision::kind::literal);
    REQUIRE(spec.width.value == 10);
    REQUIRE(spec.type == 'd');
}

TEST_CASE("parse_std_spec reads align without an explicit fill", "[truss][format][parser]") {
    format_parse_context pctx("<5s");
    auto spec = parse_std_spec(pctx, "s");
    REQUIRE(spec.fill == ' ');
    REQUIRE(spec.align == align_t::left);
    REQUIRE(spec.width.value == 5);
}

TEST_CASE("parse_std_spec reads sign, alternate form, zero-pad, width, and precision", "[truss][format][parser]") {
    format_parse_context pctx("+#010.3f");
    auto spec = parse_std_spec(pctx, "f");
    REQUIRE(spec.sign == fmt_detail::sign_t::plus);
    REQUIRE(spec.alt);
    REQUIRE(spec.zero_pad);
    REQUIRE(spec.width.value == 10);
    REQUIRE(spec.precision.k == width_or_precision::kind::literal);
    REQUIRE(spec.precision.value == 3);
    REQUIRE(spec.type == 'f');
}

TEST_CASE("parse_std_spec reads dynamic width/precision via {}", "[truss][format][parser]") {
    format_parse_context pctx("{}.{}f");
    auto spec = parse_std_spec(pctx, "f");
    REQUIRE(spec.width.k == width_or_precision::kind::dynamic);
    REQUIRE(spec.width.value == 0);
    REQUIRE(spec.precision.k == width_or_precision::kind::dynamic);
    REQUIRE(spec.precision.value == 1);
}

TEST_CASE("parse_std_spec reads dynamic width via an explicit index {N}", "[truss][format][parser]") {
    format_parse_context pctx("{2}d");
    auto spec = parse_std_spec(pctx, "d");
    REQUIRE(spec.width.k == width_or_precision::kind::dynamic);
    REQUIRE(spec.width.value == 2);
}

TEST_CASE("parse_std_spec accepts an empty spec, matching field defaults", "[truss][format][parser]") {
    format_parse_context pctx("");
    auto spec = parse_std_spec(pctx, "d");
    REQUIRE(spec.type == '\0');
    REQUIRE(spec.align == align_t::none);
}

TEST_CASE("parse_std_spec accepts but ignores the locale flag", "[truss][format][parser]") {
    format_parse_context pctx("Ld");
    auto spec = parse_std_spec(pctx, "d");
    REQUIRE(spec.locale);
    REQUIRE(spec.type == 'd');
}

TEST_CASE("parse_std_spec throws on a type character not in the allowed set", "[truss][format][parser]") {
    format_parse_context pctx("z");
    REQUIRE_THROWS_AS(parse_std_spec(pctx, "dbxX"), format_error);
}

TEST_CASE("parse_width_or_precision_value rejects a literal 0 width but allows 0 precision",
          "[truss][format][parser]") {
    format_parse_context width_pctx("0");
    REQUIRE_THROWS_AS(fmt_detail::parse_width_or_precision_value(width_pctx, false), format_error);

    format_parse_context precision_pctx("0");
    auto result = fmt_detail::parse_width_or_precision_value(precision_pctx, true);
    REQUIRE(result.k == width_or_precision::kind::literal);
    REQUIRE(result.value == 0);
}

TEST_CASE("parse_std_spec: a zero-pad flag followed by a zero width throws, matching real std::format",
          "[truss][format][parser]") {
    // Cross-checked against real std::format ("{:00d}"): GCC's own
    // consteval diagnostic is "width must be non-zero in format
    // string", confirming the second '0' is routed into width parsing
    // (and rejected there) rather than being treated as an unrelated
    // invalid type character.
    format_parse_context pctx("00d");
    REQUIRE_THROWS_AS(parse_std_spec(pctx, "d"), format_error);
}

TEST_CASE("format_parse_context::next_arg_id increments and check_arg_id rejects mixing", "[truss][format][parser]") {
    format_parse_context pctx("");
    REQUIRE(pctx.next_arg_id() == 0);
    REQUIRE(pctx.next_arg_id() == 1);
    REQUIRE_THROWS_AS(pctx.check_arg_id(0), format_error);
}

TEST_CASE("format_parse_context::check_arg_id rejects switching back to automatic", "[truss][format][parser]") {
    format_parse_context pctx("");
    pctx.check_arg_id(0);
    REQUIRE_THROWS_AS(pctx.next_arg_id(), format_error);
}

TEST_CASE("format_context resolves dynamic width/precision via its dynamic_arg_source",
          "[truss][format][context]") {
    fmt_detail::dynamic_arg_source src([](std::size_t index) -> long long {
        if (index == 0) return 7;
        throw format_error("no such argument");
    });
    std::string out;
    bridge::truss::format_context<std::back_insert_iterator<std::string>> ctx(std::back_inserter(out), src);
    REQUIRE(ctx.arg_as_dynamic(0) == 7);

    width_or_precision literal{width_or_precision::kind::literal, 5};
    width_or_precision dynamic{width_or_precision::kind::dynamic, 0};
    REQUIRE(fmt_detail::resolve_width_or_precision(literal, src) == 5);
    REQUIRE(fmt_detail::resolve_width_or_precision(dynamic, src) == 7);
}

TEST_CASE("resolve_width_or_precision throws on a negative dynamic value", "[truss][format][context]") {
    fmt_detail::dynamic_arg_source src([](std::size_t) -> long long { return -1; });
    width_or_precision dynamic{width_or_precision::kind::dynamic, 0};
    REQUIRE_THROWS_AS(fmt_detail::resolve_width_or_precision(dynamic, src), format_error);
}

TEST_CASE("bridge::truss::formatter is disabled for an unspecialized type", "[truss][format]") {
    struct not_formattable {};
    static_assert(!std::is_default_constructible_v<bridge::truss::formatter<not_formattable>>);
    SUCCEED();
}
