#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <thread>

#include <deck/cpp17/jthread.hpp>

TEST_CASE("bridge::jthread default-constructs with no thread", "[deck][jthread]") {
    bridge::jthread jt;
    REQUIRE_FALSE(jt.joinable());
}

TEST_CASE("bridge::jthread runs a plain callable, no stop_token", "[deck][jthread]") {
    std::atomic<int> counter{0};
    {
        bridge::jthread jt([&counter] { counter.fetch_add(1); });
        REQUIRE(jt.joinable());
        jt.join();
    }
    REQUIRE(counter.load() == 1);
}

TEST_CASE("bridge::jthread prepends a stop_token when the callable accepts one", "[deck][jthread]") {
    std::atomic<bool> saw_stop{false};
    {
        bridge::jthread jt([&saw_stop](bridge::stop_token st) {
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            saw_stop = true;
        });
        // Destructor requests stop and joins automatically.
    }
    REQUIRE(saw_stop.load());
}

TEST_CASE("bridge::jthread::request_stop lets a cooperative loop observe the request", "[deck][jthread]") {
    std::atomic<bool> saw_stop{false};
    bridge::jthread jt([&saw_stop](bridge::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        saw_stop = true;
    });
    REQUIRE(jt.request_stop());
    jt.join();
    REQUIRE(saw_stop.load());
}

TEST_CASE("bridge::jthread::get_stop_source/get_stop_token share the same stop-state", "[deck][jthread]") {
    bridge::jthread jt([](bridge::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    auto tok = jt.get_stop_token();
    REQUIRE_FALSE(tok.stop_requested());
    jt.request_stop();
    REQUIRE(tok.stop_requested());
    jt.join();
}

TEST_CASE("bridge::stop_source(bridge::nostopstate) has no stop-state", "[deck][jthread]") {
    bridge::stop_source src{bridge::nostopstate};
    REQUIRE_FALSE(src.stop_possible());
    REQUIRE_FALSE(src.request_stop());
}

TEST_CASE("bridge::jthread move-constructs, leaving the source not joinable", "[deck][jthread]") {
    bridge::jthread jt([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    bridge::jthread moved = std::move(jt);
    REQUIRE_FALSE(jt.joinable()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    REQUIRE(moved.joinable());
    moved.join();
}
