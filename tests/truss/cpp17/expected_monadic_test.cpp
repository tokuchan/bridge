#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <truss/cpp17/expected.hpp>

TEST_CASE("bridge::truss::expected::and_then invokes f when it holds a value", "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{21};
    auto result = a.and_then([](int v) { return bridge::truss::expected<int, std::string>{v * 2}; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

TEST_CASE("bridge::truss::expected::and_then short-circuits on error", "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    auto result = a.and_then([](int v) { return bridge::truss::expected<int, std::string>{v * 2}; });
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "boom");
}

TEST_CASE("bridge::truss::expected::and_then works across all value categories", "[truss][expected][monadic]") {
    auto f = [](const std::string& s) { return bridge::truss::expected<std::size_t, int>{s.size()}; };

    bridge::truss::expected<std::string, int> lvalue{std::string{"hello"}};
    REQUIRE(*lvalue.and_then(f) == 5);

    const bridge::truss::expected<std::string, int> const_lvalue{std::string{"hello"}};
    REQUIRE(*const_lvalue.and_then(f) == 5);

    REQUIRE(*bridge::truss::expected<std::string, int>{std::string{"hello"}}.and_then(f) == 5);

    const bridge::truss::expected<std::string, int> const_rvalue_source{std::string{"hello"}};
    REQUIRE(*std::move(const_rvalue_source).and_then(f) == 5);
}

TEST_CASE("bridge::truss::expected::and_then chains and short-circuits on error", "[truss][expected][monadic]") {
    auto half = [](int v) -> bridge::truss::expected<int, std::string> {
        if (v % 2 != 0) return bridge::truss::expected<int, std::string>{bridge::truss::unexpect, "odd"};
        return bridge::truss::expected<int, std::string>{v / 2};
    };
    bridge::truss::expected<int, std::string> start{8};
    auto step1 = start.and_then(half); // 4
    auto step2 = step1.and_then(half); // 2
    auto step3 = step2.and_then(half); // 1
    auto step4 = step3.and_then(half); // 1 is odd -> error
    REQUIRE(*step1 == 4);
    REQUIRE(*step2 == 2);
    REQUIRE(*step3 == 1);
    REQUIRE_FALSE(step4.has_value());
    REQUIRE(step4.error() == "odd");
}

TEST_CASE("bridge::truss::expected::or_else returns a copy without calling f when it holds a value",
          "[truss][expected][monadic]") {
    const bridge::truss::expected<int, std::string> a{7};
    bool called = false;
    auto result = a.or_else([&called](const std::string&) {
        called = true;
        return bridge::truss::expected<int, std::string>{0};
    });
    REQUIRE(result.has_value());
    REQUIRE(*result == 7);
    REQUIRE_FALSE(called);
}

TEST_CASE("bridge::truss::expected::or_else invokes f when it holds an error", "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    auto result = a.or_else([](const std::string&) { return bridge::truss::expected<int, std::string>{99}; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 99);
}

TEST_CASE("bridge::truss::expected::transform wraps f's result when it holds a value",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{21};
    auto result = a.transform([](int v) { return v * 2; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

TEST_CASE("bridge::truss::expected::transform returns an error copy when it holds an error",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    auto result = a.transform([](int v) { return v * 2; });
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "boom");
}

TEST_CASE("bridge::truss::expected::transform with a void-returning F produces expected<void,E>",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{5};
    bool called = false;
    auto result = a.transform([&called](int) { called = true; });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<void, std::string>>);
    REQUIRE(result.has_value());
    REQUIRE(called);

    bridge::truss::expected<int, std::string> b{bridge::truss::unexpect, "boom"};
    bool called_on_error = false;
    auto error_result = b.transform([&called_on_error](int) { called_on_error = true; });
    REQUIRE_FALSE(error_result.has_value());
    REQUIRE(error_result.error() == "boom");
    REQUIRE_FALSE(called_on_error);
}

TEST_CASE("bridge::truss::expected::transform can change the contained type", "[truss][expected][monadic]") {
    bridge::truss::expected<std::string, int> a{std::string{"hello"}};
    auto result = a.transform([](const std::string& s) { return s.size(); });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<std::size_t, int>>);
    REQUIRE(*result == 5);
}

TEST_CASE("bridge::truss::expected::transform works across all value categories", "[truss][expected][monadic]") {
    auto f = [](const std::string& s) { return s.size(); };

    bridge::truss::expected<std::string, int> lvalue{std::string{"hello"}};
    REQUIRE(*lvalue.transform(f) == 5);

    const bridge::truss::expected<std::string, int> const_lvalue{std::string{"hello"}};
    REQUIRE(*const_lvalue.transform(f) == 5);

    REQUIRE(*bridge::truss::expected<std::string, int>{std::string{"hello"}}.transform(f) == 5);

    const bridge::truss::expected<std::string, int> const_rvalue_source{std::string{"hello"}};
    REQUIRE(*std::move(const_rvalue_source).transform(f) == 5);
}

TEST_CASE("bridge::truss::expected::transform_error wraps f's result when it holds an error",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    auto result = a.transform_error([](const std::string& e) { return e.size(); });
    static_assert(std::is_same_v<decltype(result), bridge::truss::expected<int, std::size_t>>);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == 4);
}

TEST_CASE("bridge::truss::expected::transform_error returns a value copy when it holds a value",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{21};
    auto result = a.transform_error([](const std::string& e) { return e.size(); });
    REQUIRE(result.has_value());
    REQUIRE(*result == 21);
}

TEST_CASE("bridge::truss::expected::error_or returns the error when present", "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    REQUIRE(a.error_or("fallback") == "boom");
}

TEST_CASE("bridge::truss::expected::error_or returns the fallback when it holds a value",
          "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{21};
    REQUIRE(a.error_or("fallback") == "fallback");
}

TEST_CASE("bridge::truss::expected monadic methods can be chained end to end", "[truss][expected][monadic]") {
    bridge::truss::expected<int, std::string> a{5};
    auto result = a.transform([](int v) { return v * 2; })
                      .and_then([](int v) { return bridge::truss::expected<int, std::string>{v + 1}; })
                      .or_else([](const std::string&) { return bridge::truss::expected<int, std::string>{-1}; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 11);
}
