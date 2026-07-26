#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

#include <deck/cpp17/optional.hpp>

TEST_CASE("bridge::optional behaves like std::optional for basic operations", "[deck][optional]") {
    bridge::optional<int> empty{};
    bridge::optional<int> full{42};

    REQUIRE_FALSE(empty.has_value());
    REQUIRE(full.has_value());
    REQUIRE(*full == 42);
    REQUIRE(full.value() == 42);
    REQUIRE(empty.value_or(7) == 7);
}

TEST_CASE("bridge::optional supports emplace and reset", "[deck][optional]") {
    bridge::optional<std::string> opt;
    opt.emplace("hello");
    REQUIRE(opt.has_value());
    REQUIRE(*opt == "hello");
    opt.reset();
    REQUIRE_FALSE(opt.has_value());
}

TEST_CASE("bridge::optional comparisons against another bridge::optional", "[deck][optional]") {
    bridge::optional<int> a{1};
    bridge::optional<int> b{1};
    bridge::optional<int> c{2};

    REQUIRE(a == b);
    REQUIRE(a != c);
    REQUIRE(a < c);
    REQUIRE(a <= b);
    REQUIRE(c > a);
    REQUIRE(c >= a);
}

TEST_CASE("bridge::optional comparisons against std::nullopt", "[deck][optional]") {
    bridge::optional<int> empty{};
    bridge::optional<int> full{1};

    REQUIRE(empty == std::nullopt);
    REQUIRE(std::nullopt == empty);
    REQUIRE(full != std::nullopt);
    REQUIRE(std::nullopt != full);
}

TEST_CASE("bridge::optional comparisons against a raw value", "[deck][optional]") {
    bridge::optional<int> opt{1};

    REQUIRE(opt == 1);
    REQUIRE(1 == opt);
    REQUIRE(opt != 2);
    REQUIRE(opt < 2);
    REQUIRE(2 > opt);
}

TEST_CASE("bridge::optional comparisons against a plain std::optional", "[deck][optional]") {
    bridge::optional<int> opt{1};
    std::optional<int> plain{1};

    REQUIRE(opt == plain);
    REQUIRE(plain == opt);
}

TEST_CASE("bridge::optional::and_then works as a member method", "[deck][optional]") {
    bridge::optional<int> opt{21};

    auto doubled = opt.and_then([](int v) { return bridge::optional<int>{v * 2}; });
    REQUIRE(doubled.has_value());
    REQUIRE(*doubled == 42);

    bridge::optional<int> empty{};
    auto result = empty.and_then([](int v) { return bridge::optional<int>{v * 2}; });
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("bridge::optional::or_else works as a member method", "[deck][optional]") {
    bridge::optional<int> full{7};
    auto kept = full.or_else([]() { return bridge::optional<int>{0}; });
    REQUIRE(*kept == 7);

    bridge::optional<int> empty{};
    auto fallback = empty.or_else([]() { return bridge::optional<int>{99}; });
    REQUIRE(*fallback == 99);
}

TEST_CASE("bridge::optional::transform works as a member method", "[deck][optional]") {
    bridge::optional<int> opt{21};
    auto doubled = opt.transform([](int v) { return v * 2; });
    REQUIRE(*doubled == 42);

    bridge::optional<int> empty{};
    auto result = empty.transform([](int v) { return v * 2; });
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("bridge::optional monadic methods can be chained", "[deck][optional]") {
    bridge::optional<std::string> name{"hello"};

    auto result = name
                      .transform([](const std::string& s) { return s.size(); })
                      .and_then([](std::size_t n) { return bridge::optional<std::size_t>{n * 2}; });

    REQUIRE(result.has_value());
    REQUIRE(*result == 10);
}
