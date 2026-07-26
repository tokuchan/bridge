#include <catch2/catch_test_macros.hpp>

#include <string>

#include <deck/cpp17/expected.hpp>

TEST_CASE("bridge::expected behaves like std::expected for basic operations", "[deck][expected]") {
    bridge::expected<int, std::string> value{42};
    bridge::expected<int, std::string> error{bridge::unexpect, "boom"};

    REQUIRE(value.has_value());
    REQUIRE_FALSE(error.has_value());
    REQUIRE(*value == 42);
    REQUIRE(value.value() == 42);
    REQUIRE(error.error() == "boom");
    REQUIRE(error.value_or(-1) == -1);
}

TEST_CASE("bridge::expected supports emplace", "[deck][expected]") {
    // emplace requires is_nothrow_constructible_v<T, Args...> (matching
    // std::expected exactly), so this moves from an existing std::string
    // (nothrow) rather than constructing from a string literal (which
    // can throw via allocation).
    bridge::expected<std::string, int> e{bridge::unexpect, 1};
    std::string hello{"hello"};
    e.emplace(std::move(hello));
    REQUIRE(e.has_value());
    REQUIRE(*e == "hello");
}

TEST_CASE("bridge::expected comparisons", "[deck][expected]") {
    bridge::expected<int, std::string> a{1};
    bridge::expected<int, std::string> b{1};
    bridge::expected<int, std::string> c{2};

    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a == 1);
    REQUIRE(a == bridge::deck::expected<int, std::string>{1});
}

TEST_CASE("bridge::expected monadic methods can be chained", "[deck][expected]") {
    bridge::expected<int, std::string> e{5};
    auto result = e.transform([](int v) { return v * 2; })
                      .and_then([](int v) { return bridge::expected<int, std::string>{v + 1}; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 11);
}

TEST_CASE("bridge::expected<void,E> works via the same alias", "[deck][expected]") {
    bridge::expected<void, std::string> e;
    REQUIRE(e.has_value());
    bridge::expected<void, std::string> err{bridge::unexpect, "boom"};
    REQUIRE(err.error() == "boom");
}
