#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include <truss/cpp17/span.hpp>

using bridge::truss::as_bytes;
using bridge::truss::as_writable_bytes;
using bridge::truss::dynamic_extent;
using bridge::truss::span;

TEST_CASE("span<T,Extent> storage layout: fixed extent stores no runtime size", "[truss][span]") {
    // Cross-checked against real std::span: sizeof(span<int,4>) is
    // exactly half sizeof(span<int>) on this project's compiler pair.
    REQUIRE(sizeof(span<int, 4>) < sizeof(span<int>));
}

TEST_CASE("span constructs from a C array", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int> s(arr);
    REQUIRE(s.size() == 5);
    REQUIRE(s[0] == 1);
    REQUIRE(s[4] == 5);
}

TEST_CASE("span constructs from a mutable std::array", "[truss][span]") {
    std::array<int, 3> arr = {10, 20, 30};
    span<int, 3> s(arr);
    REQUIRE(s.size() == 3);
    REQUIRE(s[1] == 20);
}

TEST_CASE("span constructs from a const std::array, yielding const elements", "[truss][span]") {
    const std::array<int, 3> arr = {1, 2, 3};
    span<const int, 3> s(arr);
    REQUIRE(s.size() == 3);
    static_assert(!std::is_constructible_v<span<int, 3>, const std::array<int, 3>&>,
                  "span<int,3> must not be constructible from a const array<int,3>&");
}

TEST_CASE("span constructs from a mutable std::vector", "[truss][span]") {
    std::vector<int> vec = {7, 8, 9};
    span<int> s(vec);
    REQUIRE(s.size() == 3);
    s[0] = 70;
    REQUIRE(vec[0] == 70);
}

TEST_CASE("span constructs from a const std::vector, yielding const elements", "[truss][span]") {
    const std::vector<int> vec = {1, 2, 3};
    span<const int> s(vec);
    REQUIRE(s.size() == 3);
    static_assert(!std::is_constructible_v<span<int>, const std::vector<int>&>,
                  "span<int> must not be constructible from a const vector<int>&");
}

TEST_CASE("span constructs from a mutable std::string", "[truss][span]") {
    std::string str = "hello";
    span<char> s(str);
    REQUIRE(s.size() == 5);
}

TEST_CASE("span constructs from a const std::string, yielding const elements", "[truss][span]") {
    const std::string str = "hello";
    span<const char> s(str);
    REQUIRE(s.size() == 5);
    static_assert(!std::is_constructible_v<span<char>, const std::string&>,
                  "span<char> must not be constructible from a const string&");
}

TEST_CASE("span constructs from a pointer and count", "[truss][span]") {
    std::vector<int> vec = {1, 2, 3};
    span<int> s(vec.data(), vec.size());
    REQUIRE(s.size() == 3);
}

TEST_CASE("span constructs from an iterator pair", "[truss][span]") {
    std::vector<int> vec = {1, 2, 3, 4};
    span<int> s(vec.begin(), vec.end());
    REQUIRE(s.size() == 4);
}

TEST_CASE("span default-constructs an empty span", "[truss][span]") {
    span<int> s;
    REQUIRE(s.size() == 0);
    REQUIRE(s.empty());
    REQUIRE(s.data() == nullptr);
}

TEST_CASE("span's default constructor is only available for dynamic or zero extent", "[truss][span]") {
    static_assert(std::is_default_constructible_v<span<int>>, "dynamic extent must be default-constructible");
    static_assert(std::is_default_constructible_v<span<int, 0>>, "extent 0 must be default-constructible");
    static_assert(!std::is_default_constructible_v<span<int, 3>>, "extent 3 must not be default-constructible");
}

TEST_CASE("span is trivially copyable, both extents", "[truss][span]") {
    static_assert(std::is_trivially_copyable_v<span<int>>);
    static_assert(std::is_trivially_copyable_v<span<int, 5>>);
}

TEST_CASE("span converting constructor: mutable to const is implicit", "[truss][span]") {
    std::vector<int> vec = {1, 2, 3};
    span<int> mutable_span(vec);
    span<const int> const_span = mutable_span;
    REQUIRE(const_span.size() == 3);
}

TEST_CASE("span converting constructor: static to dynamic extent is implicit", "[truss][span]") {
    std::array<int, 3> arr = {1, 2, 3};
    span<int, 3> fixed(arr);
    span<int> dyn = fixed;
    REQUIRE(dyn.size() == 3);
}

TEST_CASE("span converting constructor: dynamic to static extent is explicit-only", "[truss][span]") {
    static_assert(!std::is_convertible_v<span<int>, span<int, 3>>,
                  "dynamic->static must not be implicitly convertible");
    static_assert(std::is_constructible_v<span<int, 3>, span<int>>, "dynamic->static must be explicitly constructible");

    std::vector<int> vec = {1, 2, 3};
    span<int> dyn(vec);
    span<int, 3> fixed(dyn);
    REQUIRE(fixed.size() == 3);
}

TEST_CASE("span converting constructor rejects derived-to-base element types", "[truss][span]") {
    struct Base {};
    struct Derived : Base {};
    static_assert(!std::is_convertible_v<span<Derived>, span<Base>>);
    static_assert(!std::is_constructible_v<span<Base>, span<Derived>>);
}

TEST_CASE("span front/back/operator[]/data", "[truss][span]") {
    int arr[3] = {10, 20, 30};
    span<int> s(arr);
    REQUIRE(s.front() == 10);
    REQUIRE(s.back() == 30);
    REQUIRE(s[1] == 20);
    REQUIRE(s.data() == arr);
}

TEST_CASE("span begin/end/rbegin/rend", "[truss][span]") {
    int arr[3] = {10, 20, 30};
    span<int> s(arr);
    REQUIRE(*s.begin() == 10);
    REQUIRE(*(s.end() - 1) == 30);
    REQUIRE(*s.rbegin() == 30);
    REQUIRE(*(s.rend() - 1) == 10);

    int sum = 0;
    for (int v : s) sum += v;
    REQUIRE(sum == 60);
}

TEST_CASE("span::size_bytes", "[truss][span]") {
    int arr[5] = {};
    span<int> s(arr);
    REQUIRE(s.size_bytes() == 5 * sizeof(int));
}

TEST_CASE("span::first(count)/last(count) always yield dynamic_extent", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int, 5> fixed(arr);
    auto f = fixed.first(2);
    auto l = fixed.last(2);
    static_assert(std::is_same_v<decltype(f), span<int, dynamic_extent>>);
    static_assert(std::is_same_v<decltype(l), span<int, dynamic_extent>>);
    REQUIRE(f.size() == 2);
    REQUIRE(f[0] == 1);
    REQUIRE(f[1] == 2);
    REQUIRE(l.size() == 2);
    REQUIRE(l[0] == 4);
    REQUIRE(l[1] == 5);
}

TEST_CASE("span::first<Count>()/last<Count>() on a fixed extent yield a fixed extent", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int, 5> fixed(arr);
    auto f = fixed.first<2>();
    auto l = fixed.last<2>();
    static_assert(std::is_same_v<decltype(f), span<int, 2>>);
    static_assert(std::is_same_v<decltype(l), span<int, 2>>);
    REQUIRE(f[0] == 1);
    REQUIRE(f[1] == 2);
    REQUIRE(l[0] == 4);
    REQUIRE(l[1] == 5);
}

TEST_CASE("span::subspan(offset,count) always yields dynamic_extent", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int> s(arr);
    auto sub = s.subspan(1, 2);
    static_assert(std::is_same_v<decltype(sub), span<int, dynamic_extent>>);
    REQUIRE(sub.size() == 2);
    REQUIRE(sub[0] == 2);
    REQUIRE(sub[1] == 3);

    auto rest = s.subspan(2);
    REQUIRE(rest.size() == 3);
    REQUIRE(rest[0] == 3);
}

TEST_CASE("span::subspan<Offset,Count>() on a fixed extent yields a fixed extent", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int, 5> fixed(arr);
    auto sub = fixed.subspan<1, 2>();
    static_assert(std::is_same_v<decltype(sub), span<int, 2>>);
    REQUIRE(sub[0] == 2);
    REQUIRE(sub[1] == 3);

    auto rest = fixed.subspan<1>();
    static_assert(std::is_same_v<decltype(rest), span<int, 4>>);
    REQUIRE(rest.size() == 4);
}

TEST_CASE("as_bytes reinterprets a span's elements as const std::byte", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int> s(arr);
    auto bytes = as_bytes(s);
    static_assert(std::is_same_v<decltype(bytes)::element_type, const std::byte>);
    REQUIRE(bytes.size() == 5 * sizeof(int));
}

TEST_CASE("as_writable_bytes reinterprets a span's elements as std::byte", "[truss][span]") {
    int arr[5] = {1, 2, 3, 4, 5};
    span<int> s(arr);
    auto bytes = as_writable_bytes(s);
    static_assert(std::is_same_v<decltype(bytes)::element_type, std::byte>);
    REQUIRE(bytes.size() == 5 * sizeof(int));
    bytes[0] = std::byte{0xFF};
    REQUIRE(reinterpret_cast<unsigned char&>(arr[0]) == 0xFFu);
}
