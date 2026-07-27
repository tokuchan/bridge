#include <catch2/catch_test_macros.hpp>

#include <unordered_set>
#include <utility>

#include <truss/cpp17/stop_token.hpp>

TEST_CASE("bridge::truss::stop_source default-constructs with a real stop-state", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    REQUIRE(src.stop_possible());
    REQUIRE_FALSE(src.stop_requested());
}

TEST_CASE("bridge::truss::stop_source::get_token shares the same stop-state", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    auto tok = src.get_token();
    REQUIRE(tok.stop_possible());
    REQUIRE_FALSE(tok.stop_requested());
}

TEST_CASE("bridge::truss::stop_source::request_stop propagates to every shared token", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    auto tok = src.get_token();

    REQUIRE(src.request_stop());
    REQUIRE(src.stop_requested());
    REQUIRE(tok.stop_requested());
    REQUIRE_FALSE(src.request_stop()); // already requested -- second call reports false
}

TEST_CASE("bridge::truss::stop_source(nostopstate) has no stop-state", "[truss][stop_token]") {
    bridge::truss::stop_source src{bridge::truss::nostopstate};
    REQUIRE_FALSE(src.stop_possible());
    REQUIRE_FALSE(src.request_stop());

    auto tok = src.get_token();
    REQUIRE_FALSE(tok.stop_possible());
    REQUIRE_FALSE(tok.stop_requested());
}

TEST_CASE("bridge::truss::stop_token default-constructs with no stop-state", "[truss][stop_token]") {
    bridge::truss::stop_token tok;
    REQUIRE_FALSE(tok.stop_possible());
    REQUIRE_FALSE(tok.stop_requested());
}

TEST_CASE("bridge::truss::stop_token::stop_possible becomes false once every source is gone", "[truss][stop_token]") {
    bridge::truss::stop_token tok;
    {
        bridge::truss::stop_source src;
        tok = src.get_token();
        REQUIRE(tok.stop_possible());
    }
    // src is destroyed; stop was never requested, so it's now permanently impossible.
    REQUIRE_FALSE(tok.stop_possible());
    REQUIRE_FALSE(tok.stop_requested());
}

TEST_CASE("bridge::truss::stop_token::stop_possible stays true if stop was already requested", "[truss][stop_token]") {
    bridge::truss::stop_token tok;
    {
        bridge::truss::stop_source src;
        tok = src.get_token();
        REQUIRE(src.request_stop());
    }
    // src is gone, but stop was already requested -- still true.
    REQUIRE(tok.stop_possible());
    REQUIRE(tok.stop_requested());
}

TEST_CASE("bridge::truss::stop_source copies share the same stop-state", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    bridge::truss::stop_source copy = src; // NOLINT(performance-unnecessary-copy-initialization)
    REQUIRE(src == copy);

    REQUIRE(copy.request_stop());
    REQUIRE(src.stop_requested());
}

TEST_CASE("bridge::truss::stop_source moves leave the source behind with no stop-state", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    bridge::truss::stop_source moved = std::move(src);
    REQUIRE(moved.stop_possible());
    REQUIRE_FALSE(src.stop_possible()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
}

TEST_CASE("bridge::truss::stop_token equality compares stop-state identity", "[truss][stop_token]") {
    bridge::truss::stop_source a;
    bridge::truss::stop_source b;

    REQUIRE(a.get_token() == a.get_token());
    REQUIRE_FALSE(a.get_token() == b.get_token());
    REQUIRE(a.get_token() != b.get_token());
}

TEST_CASE("bridge::truss::stop_token is usable as an unordered_set key", "[truss][stop_token]") {
    bridge::truss::stop_source src;
    auto tok = src.get_token();

    std::unordered_set<bridge::truss::stop_token> tokens;
    tokens.insert(tok);
    REQUIRE(tokens.count(tok) == 1);
    REQUIRE(tokens.count(bridge::truss::stop_token{}) == 0);
}

TEST_CASE("swap exchanges stop-state between two sources/tokens", "[truss][stop_token]") {
    bridge::truss::stop_source a;
    bridge::truss::stop_source b{bridge::truss::nostopstate};
    REQUIRE(a.request_stop());

    swap(a, b);
    REQUIRE_FALSE(a.stop_possible());
    REQUIRE(b.stop_possible());
    REQUIRE(b.stop_requested());
}
