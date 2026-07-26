#include <catch2/catch_test_macros.hpp>

#include <string>

#include <truss/cpp17/expected.hpp>

TEST_CASE("bridge::truss::expected comparisons against another expected", "[truss][expected][comparison]") {
    bridge::truss::expected<int, std::string> a{1};
    bridge::truss::expected<int, std::string> b{1};
    bridge::truss::expected<int, std::string> c{2};
    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
    REQUIRE(a != c);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("bridge::truss::expected comparisons: value vs error are never equal",
          "[truss][expected][comparison]") {
    bridge::truss::expected<int, std::string> value_holder{1};
    bridge::truss::expected<int, std::string> error_holder{bridge::truss::unexpect, "x"};
    REQUIRE(value_holder != error_holder);
    REQUIRE_FALSE(value_holder == error_holder);
}

TEST_CASE("bridge::truss::expected comparisons against another expected, both holding errors",
          "[truss][expected][comparison]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "x"};
    bridge::truss::expected<int, std::string> b{bridge::truss::unexpect, "x"};
    bridge::truss::expected<int, std::string> c{bridge::truss::unexpect, "y"};
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("bridge::truss::expected comparisons against a raw value", "[truss][expected][comparison]") {
    bridge::truss::expected<int, std::string> a{1};
    REQUIRE(a == 1);
    REQUIRE(1 == a);
    REQUIRE(a != 2);
    REQUIRE(2 != a);

    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "x"};
    REQUIRE(e != 1);
    REQUIRE(1 != e);
    REQUIRE_FALSE(e == 1);
}

TEST_CASE("bridge::truss::expected comparisons against unexpected<G>", "[truss][expected][comparison]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "x"};
    REQUIRE(e == bridge::truss::unexpected<std::string>{"x"});
    REQUIRE(bridge::truss::unexpected<std::string>{"x"} == e);
    REQUIRE(e != bridge::truss::unexpected<std::string>{"y"});
    REQUIRE(bridge::truss::unexpected<std::string>{"y"} != e);

    bridge::truss::expected<int, std::string> v{1};
    REQUIRE(v != bridge::truss::unexpected<std::string>{"x"});
    REQUIRE_FALSE(v == bridge::truss::unexpected<std::string>{"x"});
}

TEST_CASE("bridge::truss::unexpected comparisons for inequality", "[truss][expected][comparison]") {
    REQUIRE(bridge::truss::unexpected<int>{1} == bridge::truss::unexpected<int>{1});
    REQUIRE(bridge::truss::unexpected<int>{1} != bridge::truss::unexpected<int>{2});
}
