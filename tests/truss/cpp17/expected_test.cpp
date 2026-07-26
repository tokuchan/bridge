#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <truss/cpp17/expected.hpp>

TEST_CASE("bridge::truss::unexpected wraps and exposes its error", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<int> u{42};
    REQUIRE(u.error() == 42);
}

TEST_CASE("bridge::truss::unexpected::error works across all value categories", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<std::string> u{"boom"};
    REQUIRE(u.error() == "boom");

    const bridge::truss::unexpected<std::string> const_u{"boom"};
    REQUIRE(const_u.error() == "boom");

    REQUIRE(bridge::truss::unexpected<std::string>{"boom"}.error() == "boom");

    const bridge::truss::unexpected<std::string> const_rvalue_source{"boom"};
    REQUIRE(std::move(const_rvalue_source).error() == "boom");
}

TEST_CASE("bridge::truss::unexpected constructs in place", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<std::string> u{std::in_place, "hello", 3};
    REQUIRE(u.error() == "hel");
}

TEST_CASE("bridge::truss::unexpected constructs in place from an initializer list", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<std::vector<int>> u{std::in_place, {1, 2, 3}};
    REQUIRE(u.error() == std::vector<int>{1, 2, 3});
}

TEST_CASE("bridge::truss::unexpected copies and moves", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<std::string> u{"hello"};
    bridge::truss::unexpected<std::string> copy{u};
    REQUIRE(copy.error() == "hello");

    bridge::truss::unexpected<std::string> moved{std::move(u)};
    REQUIRE(moved.error() == "hello");
}

TEST_CASE("bridge::truss::unexpected swaps, member and ADL", "[truss][expected][unexpected]") {
    bridge::truss::unexpected<std::string> a{"a"};
    bridge::truss::unexpected<std::string> b{"b"};
    a.swap(b);
    REQUIRE(a.error() == "b");
    REQUIRE(b.error() == "a");

    using std::swap;
    swap(a, b);
    REQUIRE(a.error() == "a");
    REQUIRE(b.error() == "b");
}

TEST_CASE("bridge::truss::unexpected compares by wrapped error, including across error types",
          "[truss][expected][unexpected]") {
    bridge::truss::unexpected<int> a{1};
    bridge::truss::unexpected<int> b{1};
    bridge::truss::unexpected<int> c{2};
    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);

    bridge::truss::unexpected<long> d{1};
    REQUIRE(a == d);
}

TEST_CASE("bridge::truss::unexpected deduces its error type", "[truss][expected][unexpected]") {
    bridge::truss::unexpected u{std::string{"deduced"}};
    static_assert(std::is_same_v<decltype(u), bridge::truss::unexpected<std::string>>);
    REQUIRE(u.error() == "deduced");
}

TEST_CASE("bridge::truss::bad_expected_access carries the error and a message", "[truss][expected]") {
    bridge::truss::bad_expected_access<std::string> ex{"why it failed"};
    REQUIRE(ex.error() == "why it failed");
    REQUIRE(std::string{ex.what()} == "bad expected access");
}

TEST_CASE("bridge::truss::bad_expected_access::error works across all value categories", "[truss][expected]") {
    bridge::truss::bad_expected_access<std::string> ex{"why"};
    REQUIRE(ex.error() == "why");

    const bridge::truss::bad_expected_access<std::string> const_ex{"why"};
    REQUIRE(const_ex.error() == "why");

    REQUIRE(bridge::truss::bad_expected_access<std::string>{"why"}.error() == "why");
}

TEST_CASE("bridge::truss::bad_expected_access is catchable as std::exception", "[truss][expected]") {
    try {
        throw bridge::truss::bad_expected_access<int>{5};
    } catch (const std::exception& ex) {
        REQUIRE(std::string{ex.what()} == "bad expected access");
        return;
    }
    FAIL("expected bad_expected_access to be caught as std::exception");
}

TEST_CASE("bridge::truss::unexpect_t is a distinct, explicitly-constructed tag type", "[truss][expected]") {
    bridge::truss::unexpect_t tag{};
    (void)tag;
    static_assert(!std::is_convertible_v<int, bridge::truss::unexpect_t>);
    SUCCEED();
}
