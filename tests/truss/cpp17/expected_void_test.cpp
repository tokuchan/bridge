#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <truss/cpp17/expected.hpp>

namespace {

struct throwing_move {
    throwing_move() = default;
    throwing_move(const throwing_move&) = default;
    throwing_move(throwing_move&&) noexcept(false) {}
    throwing_move& operator=(const throwing_move&) = default;
    throwing_move& operator=(throwing_move&&) noexcept(false) { return *this; }
};

static_assert(std::is_copy_constructible_v<bridge::truss::expected<void, throwing_move>>);
static_assert(!std::is_copy_assignable_v<bridge::truss::expected<void, throwing_move>>);
static_assert(!std::is_move_assignable_v<bridge::truss::expected<void, throwing_move>>);
static_assert(std::is_copy_assignable_v<bridge::truss::expected<void, int>>);
static_assert(std::is_move_assignable_v<bridge::truss::expected<void, int>>);

} // namespace

TEST_CASE("bridge::truss::expected<void,E> default-constructs holding a value", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    REQUIRE(e.has_value());
    REQUIRE(static_cast<bool>(e));
}

TEST_CASE("bridge::truss::expected<void,E> constructs an error via unexpect", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == "boom");
}

TEST_CASE("bridge::truss::expected<void,E> constructs an error from unexpected<G>", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpected<std::string>{"nope"}};
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == "nope");
}

TEST_CASE("bridge::truss::expected<void,E> copies and moves", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> a;
    bridge::truss::expected<void, std::string> b{a};
    REQUIRE(b.has_value());

    bridge::truss::expected<void, std::string> c{bridge::truss::unexpect, "boom"};
    bridge::truss::expected<void, std::string> d{std::move(c)};
    REQUIRE_FALSE(d.has_value());
    REQUIRE(d.error() == "boom");
}

TEST_CASE("bridge::truss::expected<void,E> copy-assignment switches alternative", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> value_holder;
    bridge::truss::expected<void, std::string> error_holder{bridge::truss::unexpect, "boom"};
    value_holder = error_holder;
    REQUIRE_FALSE(value_holder.has_value());
    REQUIRE(value_holder.error() == "boom");
}

TEST_CASE("bridge::truss::expected<void,E> assigns from unexpected<G>", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    e = bridge::truss::unexpected<std::string>{"boom"};
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == "boom");
}

TEST_CASE("bridge::truss::expected<void,E> converting constructor from expected<U,G>", "[truss][expected][void]") {
    bridge::truss::expected<void, int> src{bridge::truss::unexpect, 5};
    bridge::truss::expected<void, long> dst{src};
    REQUIRE_FALSE(dst.has_value());
    REQUIRE(dst.error() == 5L);
}

TEST_CASE("bridge::truss::expected<void,E>::value throws bad_expected_access when it holds an error",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE_THROWS_AS(e.value(), bridge::truss::bad_expected_access<std::string>);
}

TEST_CASE("bridge::truss::expected<void,E>::value does not throw when it holds a value",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    REQUIRE_NOTHROW(e.value());
}

TEST_CASE("bridge::truss::expected<void,E>::error_or falls back on value", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE(e.error_or("fallback") == "boom");

    bridge::truss::expected<void, std::string> v;
    REQUIRE(v.error_or("fallback") == "fallback");
}

TEST_CASE("bridge::truss::expected<void,E>::and_then invokes f with no arguments when it holds a value",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    auto result = e.and_then([]() { return bridge::truss::expected<int, std::string>{42}; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

TEST_CASE("bridge::truss::expected<void,E>::and_then short-circuits on error", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    auto result = e.and_then([]() { return bridge::truss::expected<int, std::string>{42}; });
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "boom");
}

TEST_CASE("bridge::truss::expected<void,E>::or_else invokes f when it holds an error",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    auto result = e.or_else([](const std::string&) { return bridge::truss::expected<void, std::string>{}; });
    REQUIRE(result.has_value());
}

TEST_CASE("bridge::truss::expected<void,E>::transform with a non-void result", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    auto result = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<int, std::string>>);
    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

TEST_CASE("bridge::truss::expected<void,E>::transform with a void result chains to expected<void,E>",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e;
    bool called = false;
    auto result = e.transform([&called]() { called = true; });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<void, std::string>>);
    REQUIRE(result.has_value());
    REQUIRE(called);
}

TEST_CASE("bridge::truss::expected<void,E>::transform propagates an error without calling f",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    bool called = false;
    auto result = e.transform([&called]() { called = true; });
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "boom");
    REQUIRE_FALSE(called);
}

TEST_CASE("bridge::truss::expected<void,E>::transform_error wraps f's result", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    auto result = e.transform_error([](const std::string& err) { return err.size(); });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<void, std::size_t>>);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == 4);
}

TEST_CASE("bridge::truss::expected<void,E>::emplace clears an error and marks a value",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> e{bridge::truss::unexpect, "boom"};
    e.emplace();
    REQUIRE(e.has_value());
}

TEST_CASE("bridge::truss::expected<void,E>::swap exchanges errors within the same alternative",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> a{bridge::truss::unexpect, "a"};
    bridge::truss::expected<void, std::string> b{bridge::truss::unexpect, "b"};
    a.swap(b);
    REQUIRE(a.error() == "b");
    REQUIRE(b.error() == "a");
}

TEST_CASE("bridge::truss::expected<void,E>::swap exchanges across differing alternatives",
          "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> a;
    bridge::truss::expected<void, std::string> b{bridge::truss::unexpect, "err"};
    a.swap(b);
    REQUIRE_FALSE(a.has_value());
    REQUIRE(a.error() == "err");
    REQUIRE(b.has_value());
}

TEST_CASE("bridge::truss::expected<void,E> comparisons", "[truss][expected][void]") {
    bridge::truss::expected<void, std::string> a;
    bridge::truss::expected<void, std::string> b;
    bridge::truss::expected<void, std::string> c{bridge::truss::unexpect, "x"};
    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(c == bridge::truss::unexpected<std::string>{"x"});
    REQUIRE(bridge::truss::unexpected<std::string>{"x"} == c);
}
