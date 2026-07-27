#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>

#include <deck/cpp17/span.hpp>

TEST_CASE("bridge::span constructs from a C array and a std::vector", "[deck][span]") {
    int arr[3] = {1, 2, 3};
    bridge::span<int> s(arr);
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == 1);

    std::vector<int> vec = {4, 5, 6, 7};
    bridge::span<int> sv(vec);
    REQUIRE(sv.size() == 4);
}

TEST_CASE("bridge::span supports both static and dynamic extent", "[deck][span]") {
    std::array<int, 3> arr = {10, 20, 30};
    bridge::span<int, 3> fixed(arr);
    REQUIRE(fixed.size() == 3);
    bridge::span<int> dyn = fixed;
    REQUIRE(dyn.size() == 3);
}

TEST_CASE("bridge::span::first/last/subspan", "[deck][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    bridge::span<int> s(arr);
    REQUIRE(s.first(2)[1] == 2);
    REQUIRE(s.last(2)[0] == 4);
    REQUIRE(s.subspan(1, 2)[0] == 2);
}

TEST_CASE("bridge::as_bytes/as_writable_bytes", "[deck][span]") {
    int arr[2] = {1, 2};
    bridge::span<int> s(arr);
    REQUIRE(bridge::as_bytes(s).size() == 2 * sizeof(int));
    REQUIRE(bridge::as_writable_bytes(s).size() == 2 * sizeof(int));
}

TEST_CASE("bridge::dynamic_extent is usable as a template argument", "[deck][span]") {
    bridge::span<int, bridge::dynamic_extent> s;
    REQUIRE(s.empty());
}
