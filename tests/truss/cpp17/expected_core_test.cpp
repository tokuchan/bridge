#include <catch2/catch_test_macros.hpp>

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

struct not_copyable {
    not_copyable() = default;
    not_copyable(const not_copyable&) = delete;
    not_copyable(not_copyable&&) = default;
    not_copyable& operator=(const not_copyable&) = delete;
    not_copyable& operator=(not_copyable&&) = default;
};

// docs/adr/0010's "match deletion conditions" fidelity target, checked
// at compile time so a regression here fails the build, not just a test.
static_assert(!std::is_copy_constructible_v<bridge::truss::expected<not_copyable, int>>);
static_assert(std::is_move_constructible_v<bridge::truss::expected<not_copyable, int>>);
static_assert(!std::is_copy_assignable_v<bridge::truss::expected<not_copyable, int>>);
static_assert(std::is_move_assignable_v<bridge::truss::expected<not_copyable, int>>);

static_assert(!std::is_nothrow_move_constructible_v<throwing_move>);
static_assert(std::is_copy_constructible_v<bridge::truss::expected<throwing_move, throwing_move>>);
static_assert(std::is_move_constructible_v<bridge::truss::expected<throwing_move, throwing_move>>);
static_assert(!std::is_copy_assignable_v<bridge::truss::expected<throwing_move, throwing_move>>);
static_assert(!std::is_move_assignable_v<bridge::truss::expected<throwing_move, throwing_move>>);

static_assert(std::is_nothrow_move_constructible_v<int>);
static_assert(std::is_copy_assignable_v<bridge::truss::expected<throwing_move, int>>);
static_assert(std::is_move_assignable_v<bridge::truss::expected<throwing_move, int>>);

} // namespace

TEST_CASE("bridge::truss::expected default-constructs the contained value", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e;
    REQUIRE(e.has_value());
    REQUIRE(*e == 0);
}

TEST_CASE("bridge::truss::expected constructs a value from U", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{42};
    REQUIRE(e.has_value());
    REQUIRE(static_cast<bool>(e));
    REQUIRE(*e == 42);
}

TEST_CASE("bridge::truss::expected constructs an error via unexpect", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE_FALSE(e.has_value());
    REQUIRE_FALSE(static_cast<bool>(e));
    REQUIRE(e.error() == "boom");
}

TEST_CASE("bridge::truss::expected constructs an error from unexpected<G>", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpected<std::string>{"nope"}};
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == "nope");

    bridge::truss::unexpected<std::string> u{"moved"};
    bridge::truss::expected<int, std::string> moved_in{std::move(u)};
    REQUIRE(moved_in.error() == "moved");
}

TEST_CASE("bridge::truss::expected copies a value", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{42};
    bridge::truss::expected<int, std::string> b{a};
    REQUIRE(b.has_value());
    REQUIRE(*b == 42);
}

TEST_CASE("bridge::truss::expected copies an error", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    bridge::truss::expected<int, std::string> b{a};
    REQUIRE_FALSE(b.has_value());
    REQUIRE(b.error() == "boom");
}

TEST_CASE("bridge::truss::expected moves a value", "[truss][expected]") {
    bridge::truss::expected<std::string, int> a{std::string{"hello"}};
    bridge::truss::expected<std::string, int> b{std::move(a)};
    REQUIRE(b.has_value());
    REQUIRE(*b == "hello");
}

TEST_CASE("bridge::truss::expected moves an error", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "boom"};
    bridge::truss::expected<int, std::string> b{std::move(a)};
    REQUIRE_FALSE(b.has_value());
    REQUIRE(b.error() == "boom");
}

TEST_CASE("bridge::truss::expected copy-assignment switches alternative", "[truss][expected]") {
    bridge::truss::expected<int, std::string> value_holder{42};
    bridge::truss::expected<int, std::string> error_holder{bridge::truss::unexpect, "boom"};
    value_holder = error_holder;
    REQUIRE_FALSE(value_holder.has_value());
    REQUIRE(value_holder.error() == "boom");
}

TEST_CASE("bridge::truss::expected copy-assignment within the same alternative", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{1};
    bridge::truss::expected<int, std::string> b{2};
    a = b;
    REQUIRE(a.has_value());
    REQUIRE(*a == 2);

    bridge::truss::expected<int, std::string> c{bridge::truss::unexpect, "a"};
    bridge::truss::expected<int, std::string> d{bridge::truss::unexpect, "b"};
    c = d;
    REQUIRE_FALSE(c.has_value());
    REQUIRE(c.error() == "b");
}

TEST_CASE("bridge::truss::expected move-assignment switches alternative", "[truss][expected]") {
    bridge::truss::expected<int, std::string> value_holder{42};
    bridge::truss::expected<int, std::string> error_holder{bridge::truss::unexpect, "boom"};
    value_holder = std::move(error_holder);
    REQUIRE_FALSE(value_holder.has_value());
    REQUIRE(value_holder.error() == "boom");
}

TEST_CASE("bridge::truss::expected assigns from a raw value", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "boom"};
    e = 42;
    REQUIRE(e.has_value());
    REQUIRE(*e == 42);

    bridge::truss::expected<int, std::string> f{1};
    f = 2;
    REQUIRE(f.has_value());
    REQUIRE(*f == 2);
}

TEST_CASE("bridge::truss::expected assigns from unexpected<G>", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{1};
    e = bridge::truss::unexpected<std::string>{"boom"};
    REQUIRE_FALSE(e.has_value());
    REQUIRE(e.error() == "boom");

    bridge::truss::expected<int, std::string> f{bridge::truss::unexpect, "old"};
    f = bridge::truss::unexpected<std::string>{"new"};
    REQUIRE_FALSE(f.has_value());
    REQUIRE(f.error() == "new");
}

TEST_CASE("bridge::truss::expected converting constructor from expected<U,G>", "[truss][expected]") {
    bridge::truss::expected<int, std::string> src{42};
    bridge::truss::expected<long, std::string> dst{src};
    REQUIRE(dst.has_value());
    REQUIRE(*dst == 42L);

    bridge::truss::expected<int, std::string> err_src{bridge::truss::unexpect, "boom"};
    bridge::truss::expected<long, std::string> err_dst{err_src};
    REQUIRE_FALSE(err_dst.has_value());
    REQUIRE(err_dst.error() == "boom");

    bridge::truss::expected<long, std::string> moved_dst{std::move(src)};
    REQUIRE(moved_dst.has_value());
    REQUIRE(*moved_dst == 42L);
}

TEST_CASE("bridge::truss::expected::value throws bad_expected_access carrying the error", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE_THROWS_AS(e.value(), bridge::truss::bad_expected_access<std::string>);
    try {
        (void)e.value();
        FAIL("expected value() to throw");
    } catch (const bridge::truss::bad_expected_access<std::string>& ex) {
        REQUIRE(ex.error() == "boom");
    }
}

TEST_CASE("bridge::truss::expected::value returns the value across value categories", "[truss][expected]") {
    bridge::truss::expected<std::string, int> e{std::string{"hi"}};
    REQUIRE(e.value() == "hi");

    const bridge::truss::expected<std::string, int> const_e{std::string{"hi"}};
    REQUIRE(const_e.value() == "hi");

    REQUIRE(bridge::truss::expected<std::string, int>{std::string{"hi"}}.value() == "hi");
}

TEST_CASE("bridge::truss::expected::value_or falls back on error", "[truss][expected]") {
    bridge::truss::expected<int, std::string> e{bridge::truss::unexpect, "boom"};
    REQUIRE(e.value_or(-1) == -1);

    bridge::truss::expected<int, std::string> v{42};
    REQUIRE(v.value_or(-1) == 42);
}

TEST_CASE("bridge::truss::expected::operator-> accesses the value's members", "[truss][expected]") {
    bridge::truss::expected<std::string, int> e{std::string{"hi"}};
    REQUIRE(e->size() == 2);
}

TEST_CASE("bridge::truss::expected::emplace replaces the contents with a new value", "[truss][expected]") {
    bridge::truss::expected<std::string, int> e{bridge::truss::unexpect, 1};
    e.emplace("hello");
    REQUIRE(e.has_value());
    REQUIRE(*e == "hello");
}

TEST_CASE("bridge::truss::expected::swap exchanges values within the same alternative", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{1};
    bridge::truss::expected<int, std::string> b{2};
    a.swap(b);
    REQUIRE(*a == 2);
    REQUIRE(*b == 1);
}

TEST_CASE("bridge::truss::expected::swap exchanges errors within the same alternative", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{bridge::truss::unexpect, "a"};
    bridge::truss::expected<int, std::string> b{bridge::truss::unexpect, "b"};
    a.swap(b);
    REQUIRE(a.error() == "b");
    REQUIRE(b.error() == "a");
}

TEST_CASE("bridge::truss::expected::swap exchanges across differing alternatives", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{1};
    bridge::truss::expected<int, std::string> b{bridge::truss::unexpect, "err"};
    a.swap(b);
    REQUIRE_FALSE(a.has_value());
    REQUIRE(a.error() == "err");
    REQUIRE(b.has_value());
    REQUIRE(*b == 1);
}

TEST_CASE("bridge::truss::expected ADL swap forwards to the member swap", "[truss][expected]") {
    bridge::truss::expected<int, std::string> a{1};
    bridge::truss::expected<int, std::string> b{bridge::truss::unexpect, "err"};
    using std::swap;
    swap(a, b);
    REQUIRE_FALSE(a.has_value());
    REQUIRE(b.has_value());
}
