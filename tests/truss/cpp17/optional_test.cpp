#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <utility>

#include <truss/cpp17/optional.hpp>

TEST_CASE("bridge::truss::and_then invokes f when opt has a value", "[truss][optional]") {
    std::optional<int> opt{21};
    auto result = bridge::truss::and_then(opt, [](int v) { return std::optional<int>{v * 2}; });
    REQUIRE(result == std::optional<int>{42});
}

TEST_CASE("bridge::truss::and_then returns empty when opt is empty", "[truss][optional]") {
    std::optional<int> opt{};
    auto result = bridge::truss::and_then(opt, [](int v) { return std::optional<int>{v * 2}; });
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("bridge::truss::and_then works across all value categories", "[truss][optional]") {
    auto f = [](const std::string& s) { return std::optional<std::size_t>{s.size()}; };

    std::optional<std::string> lvalue{"hello"};
    REQUIRE(bridge::truss::and_then(lvalue, f) == std::optional<std::size_t>{5});

    const std::optional<std::string> const_lvalue{"hello"};
    REQUIRE(bridge::truss::and_then(const_lvalue, f) == std::optional<std::size_t>{5});

    REQUIRE(bridge::truss::and_then(std::optional<std::string>{"hello"}, f) == std::optional<std::size_t>{5});

    const std::optional<std::string> const_rvalue_source{"hello"};
    REQUIRE(bridge::truss::and_then(std::move(const_rvalue_source), f) == std::optional<std::size_t>{5});
}

TEST_CASE("bridge::truss::and_then chains and short-circuits on empty", "[truss][optional]") {
    auto half = [](int v) -> std::optional<int> {
        if (v % 2 != 0) return std::nullopt;
        return v / 2;
    };
    std::optional<int> start{8};
    auto step1 = bridge::truss::and_then(start, half);  // 4
    auto step2 = bridge::truss::and_then(step1, half);  // 2
    auto step3 = bridge::truss::and_then(step2, half);  // 1
    auto step4 = bridge::truss::and_then(step3, half);  // 1 is odd -> empty
    REQUIRE(step1 == std::optional<int>{4});
    REQUIRE(step2 == std::optional<int>{2});
    REQUIRE(step3 == std::optional<int>{1});
    REQUIRE_FALSE(step4.has_value());
}

TEST_CASE("bridge::truss::or_else returns a copy without calling f when opt has a value", "[truss][optional]") {
    const std::optional<int> opt{7};
    bool called = false;
    auto result = bridge::truss::or_else(opt, [&called]() {
        called = true;
        return std::optional<int>{0};
    });
    REQUIRE(result == std::optional<int>{7});
    REQUIRE_FALSE(called);
}

TEST_CASE("bridge::truss::or_else invokes f when opt is empty", "[truss][optional]") {
    std::optional<int> opt{};
    auto result = bridge::truss::or_else(opt, []() { return std::optional<int>{99}; });
    REQUIRE(result == std::optional<int>{99});
}

TEST_CASE("bridge::truss::or_else moves opt when given an rvalue", "[truss][optional]") {
    std::optional<std::string> opt{"hello"};
    auto result = bridge::truss::or_else(std::move(opt), []() { return std::optional<std::string>{"fallback"}; });
    REQUIRE(result == std::optional<std::string>{"hello"});
}

TEST_CASE("bridge::truss::transform wraps f's result when opt has a value", "[truss][optional]") {
    std::optional<int> opt{21};
    auto result = bridge::truss::transform(opt, [](int v) { return v * 2; });
    REQUIRE(result == std::optional<int>{42});
}

TEST_CASE("bridge::truss::transform returns empty when opt is empty", "[truss][optional]") {
    std::optional<int> opt{};
    auto result = bridge::truss::transform(opt, [](int v) { return v * 2; });
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("bridge::truss::transform can change the contained type", "[truss][optional]") {
    std::optional<std::string> opt{"hello"};
    auto result = bridge::truss::transform(opt, [](const std::string& s) { return s.size(); });
    REQUIRE(result == std::optional<std::size_t>{5});
}

TEST_CASE("bridge::truss::transform works across all value categories", "[truss][optional]") {
    auto f = [](const std::string& s) { return s.size(); };

    std::optional<std::string> lvalue{"hello"};
    REQUIRE(bridge::truss::transform(lvalue, f) == std::optional<std::size_t>{5});

    const std::optional<std::string> const_lvalue{"hello"};
    REQUIRE(bridge::truss::transform(const_lvalue, f) == std::optional<std::size_t>{5});

    REQUIRE(bridge::truss::transform(std::optional<std::string>{"hello"}, f) == std::optional<std::size_t>{5});

    const std::optional<std::string> const_rvalue_source{"hello"};
    REQUIRE(bridge::truss::transform(std::move(const_rvalue_source), f) == std::optional<std::size_t>{5});
}
