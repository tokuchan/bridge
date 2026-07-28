#include <catch2/catch_test_macros.hpp>

// This file only exercises anything meaningful when compiled under a
// standard/toolchain where std::jthread/std::stop_token are actually
// available -- gated on the same condition deck/cpp17/stop_token.hpp
// and deck/cpp17/jthread.hpp use to select passthrough
// (BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH, a Detector-backed override, not
// a bare Feature Test -- see docs/adr/0017), so this never tries to
// #include <thread>/<stop_token> on a toolchain that doesn't have
// them. See tests/deck/CMakeLists.txt: only added to the C++20-forced
// target, matching how bridge_deck_span_cpp20_tests exercises span's
// own passthrough path.
#include <deck/cpp17/jthread.hpp>

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH

#    include <atomic>
#    include <chrono>
#    include <stop_token>
#    include <thread>
#    include <type_traits>

namespace {

// bridge::deck::jthread/stop_token (and bridge::jthread/stop_token)
// are plain aliases to the real types under this passthrough
// condition -- not another distinct type, confirming the passthrough
// path is actually selected here.
static_assert(std::is_same_v<bridge::deck::jthread, std::jthread>);
static_assert(std::is_same_v<bridge::jthread, std::jthread>);
static_assert(std::is_same_v<bridge::deck::stop_token, std::stop_token>);
static_assert(std::is_same_v<bridge::stop_token, std::stop_token>);
static_assert(std::is_same_v<bridge::stop_source, std::stop_source>);
static_assert(std::is_same_v<bridge::nostopstate_t, std::nostopstate_t>);

} // namespace

TEST_CASE("bridge::jthread is a passthrough alias to std::jthread under C++20", "[deck][jthread][differential]") {
    std::atomic<bool> ran{false};
    bridge::jthread jt([&ran] { ran = true; });
    static_assert(std::is_same_v<decltype(jt), std::jthread>);
    jt.join();
    REQUIRE(ran.load());
}

TEST_CASE("bridge::truss::stop_token and real std::stop_token agree on behavior directly",
          "[deck][jthread][differential]") {
    bridge::truss::stop_source truss_src;
    std::stop_source std_src;

    REQUIRE(truss_src.stop_possible() == std_src.stop_possible());
    REQUIRE(truss_src.stop_requested() == std_src.stop_requested());

    auto truss_tok = truss_src.get_token();
    auto std_tok = std_src.get_token();
    REQUIRE(truss_tok.stop_possible() == std_tok.stop_possible());

    REQUIRE(truss_src.request_stop() == std_src.request_stop());
    REQUIRE(truss_tok.stop_requested() == std_tok.stop_requested());
    REQUIRE(truss_src.request_stop() == std_src.request_stop()); // both already requested -- both false
}

TEST_CASE("bridge::truss::jthread and real std::jthread agree on cooperative-cancellation behavior",
          "[deck][jthread][differential]") {
    std::atomic<bool> truss_saw_stop{false};
    std::atomic<bool> std_saw_stop{false};

    bridge::truss::jthread truss_jt([&truss_saw_stop](bridge::truss::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        truss_saw_stop = true;
    });
    std::jthread std_jt([&std_saw_stop](std::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std_saw_stop = true;
    });

    REQUIRE(truss_jt.request_stop());
    REQUIRE(std_jt.request_stop());
    truss_jt.join();
    std_jt.join();

    REQUIRE(truss_saw_stop.load() == std_saw_stop.load());
    REQUIRE_FALSE(truss_jt.joinable());
    REQUIRE_FALSE(std_jt.joinable());
}

#else

TEST_CASE("jthread/stop_token differential trait checks skipped: passthrough condition unmet here",
          "[deck][jthread][differential]") {
    SUCCEED();
}

#endif
