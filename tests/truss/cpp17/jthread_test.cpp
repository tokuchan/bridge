#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

#include <truss/cpp17/jthread.hpp>

TEST_CASE("bridge::truss::jthread default-constructs with no thread", "[truss][jthread]") {
    bridge::truss::jthread jt;
    REQUIRE_FALSE(jt.joinable());
}

TEST_CASE("bridge::truss::jthread runs a plain callable, no stop_token", "[truss][jthread]") {
    std::atomic<int> counter{0};
    {
        bridge::truss::jthread jt([&counter] { counter.fetch_add(1); });
        REQUIRE(jt.joinable());
        jt.join();
    }
    REQUIRE(counter.load() == 1);
}

TEST_CASE("bridge::truss::jthread forwards extra arguments to a plain callable", "[truss][jthread]") {
    int sum = 0;
    bridge::truss::jthread jt([&sum](int a, int b) { sum = a + b; }, 2, 3);
    jt.join();
    REQUIRE(sum == 5);
}

TEST_CASE("bridge::truss::jthread prepends a stop_token when the callable accepts one", "[truss][jthread]") {
    std::atomic<bool> saw_stop{false};
    {
        bridge::truss::jthread jt([&saw_stop](bridge::truss::stop_token st) {
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            saw_stop = true;
        });
        // Destructor requests stop and joins automatically.
    }
    REQUIRE(saw_stop.load());
}

TEST_CASE("bridge::truss::jthread forwards extra arguments alongside a stop_token", "[truss][jthread]") {
    int sum = 0;
    bridge::truss::jthread jt([&sum](bridge::truss::stop_token, int a, int b) { sum = a + b; }, 4, 5);
    jt.join();
    REQUIRE(sum == 9);
}

TEST_CASE("bridge::truss::jthread::request_stop lets a cooperative loop observe the request", "[truss][jthread]") {
    std::atomic<bool> saw_stop{false};
    bridge::truss::jthread jt([&saw_stop](bridge::truss::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        saw_stop = true;
    });
    REQUIRE(jt.request_stop());
    jt.join();
    REQUIRE(saw_stop.load());
    REQUIRE_FALSE(jt.joinable());
}

TEST_CASE("bridge::truss::jthread destructor requests stop and joins if still joinable", "[truss][jthread]") {
    std::atomic<int> stopped_count{0};
    {
        bridge::truss::jthread jt([&stopped_count](bridge::truss::stop_token st) {
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            stopped_count.fetch_add(1);
        });
        // Not joined/detached explicitly -- destructor must not terminate
        // the program the way std::thread's would.
    }
    REQUIRE(stopped_count.load() == 1);
}

TEST_CASE("bridge::truss::jthread move-constructs, leaving the source not joinable", "[truss][jthread]") {
    bridge::truss::jthread jt([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    bridge::truss::jthread moved = std::move(jt);
    REQUIRE_FALSE(jt.joinable()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    REQUIRE(moved.joinable());
    moved.join();
}

TEST_CASE("bridge::truss::jthread move-assignment stops and joins the current thread first", "[truss][jthread]") {
    std::atomic<bool> old_stopped{false};
    bridge::truss::jthread jt([&old_stopped](bridge::truss::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        old_stopped = true;
    });

    bridge::truss::jthread replacement([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    jt = std::move(replacement);

    REQUIRE(old_stopped.load()); // the original thread was stopped+joined during the assignment
    jt.join();
}

TEST_CASE("bridge::truss::jthread::get_stop_source/get_stop_token share the same stop-state", "[truss][jthread]") {
    bridge::truss::jthread jt([](bridge::truss::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    auto src = jt.get_stop_source();
    auto tok = jt.get_stop_token();
    REQUIRE(src.get_token() == tok);
    REQUIRE_FALSE(tok.stop_requested());

    jt.request_stop();
    REQUIRE(tok.stop_requested());
    jt.join();
}

TEST_CASE("swap exchanges thread and stop-state between two jthreads", "[truss][jthread]") {
    bridge::truss::jthread a([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    bridge::truss::jthread b;
    REQUIRE(a.joinable());
    REQUIRE_FALSE(b.joinable());

    swap(a, b);
    REQUIRE_FALSE(a.joinable());
    REQUIRE(b.joinable());
    b.join();
}

TEST_CASE("bridge::truss::jthread::hardware_concurrency is callable", "[truss][jthread]") {
    // No behavioral guarantee on the value itself -- just confirms the
    // static forwarding member compiles and runs.
    (void)bridge::truss::jthread::hardware_concurrency();
    SUCCEED();
}
