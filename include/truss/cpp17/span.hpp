/// @file span.hpp
/// @brief This file holds Truss's `std::span<T, Extent>` polyfill for
///        C++17. This class is built from scratch, not attached to an
///        existing type.
///
///        `span` does not exist at all before C++20. Like `expected`
///        (docs/adr/0010-expected-truss-owns-the-class.md), there is
///        no C++17 type for Truss to attach free functions onto. This
///        is why Truss owns a complete class instead. See
///        docs/adr/0015-span-truss-owns-the-class.md for the full
///        scope. Both static and dynamic `Extent` are supported in
///        full, including the storage-layout difference and the
///        static/dynamic conversion rules. The range/container-
///        constructing overloads cover the specific named cases real
///        code needs most, not a generic check for any contiguous
///        range. The C++23 tuple-like interface is a disclosed,
///        deferred follow-up. This interface is `get<I>` plus
///        structured bindings.
///
///        `bridge::truss::span<T, Extent>` is always this polyfill,
///        regardless of standard or toolchain. Deck makes the
///        passthrough-or-polyfill choice exactly once, in
///        `deck/cpp17/span.hpp`. This choice does not depend on
///        whatever other facility Deck is selecting elsewhere.
#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

namespace bridge::detail::truss::cpp17::span {

/// @brief This is the sentinel `Extent` value. It means the size is
///        only known at runtime. This value matches real
///        `std::dynamic_extent`.
inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

template <class T, std::size_t Extent>
class span;

/// \cond BRIDGE_DETAIL
///
/// Storage layout split by Extent -- pure implementation plumbing,
/// never part of the public API. Excluded from the documentation-
/// coverage gate for the same reason truss/cpp17/format.hpp's
/// field_writers/engine namespaces are. A fixed Extent stores only
/// the pointer (the size is `Extent` itself, known at compile time);
/// dynamic_extent stores both -- confirmed via probe that real
/// `sizeof(std::span<int,4>)` is exactly half `sizeof(std::span<int>)`
/// on this project's compiler pair, not assumed.
namespace storage_detail {

template <class T, std::size_t Extent>
class span_storage {
protected:
    constexpr span_storage() noexcept = default;
    constexpr span_storage(T* p, std::size_t) noexcept : data_(p) {}

    T* data_ = nullptr;
    static constexpr std::size_t extent_value = Extent;
};

template <class T>
class span_storage<T, dynamic_extent> {
protected:
    constexpr span_storage() noexcept = default;
    constexpr span_storage(T* p, std::size_t n) noexcept : data_(p), extent_value(n) {}

    T* data_ = nullptr;
    std::size_t extent_value = 0;
};

/// @brief Computes `subspan<Offset, Count>()`'s resulting Extent at
///        compile time. Matches real `std::span::subspan`'s template
///        form: `Count` wins if given explicitly; otherwise, for a
///        fixed source Extent, the remainder `Extent - Offset`;
///        otherwise (a dynamic source), still dynamic.
constexpr std::size_t subspan_extent(std::size_t extent, std::size_t offset, std::size_t count) {
    if (count != dynamic_extent) return count;
    if (extent != dynamic_extent) return extent - offset;
    return dynamic_extent;
}

/// @brief True for `std::array<U, N>`/`const std::array<U, N>`, for
///        excluding the ambiguous const-array-of-const-elements case
///        from the wrong constructor overload.
template <class T>
struct is_std_array : std::false_type {};
template <class U, std::size_t N>
struct is_std_array<std::array<U, N>> : std::true_type {};

} // namespace storage_detail
/// \endcond

/// @brief This class is a non-owning view over a contiguous sequence
///        of `T`. `Extent` is known either at compile time or only at
///        runtime.
///
///        This class matches real `std::span<T, Extent>`'s shape.
///        This class is trivially copyable. This class never
///        allocates. This class never extends the lifetime of the
///        sequence it views.
/// @tparam T The element type.
/// @tparam Extent The compile-time extent. The default is
///         `dynamic_extent`, for when `Extent` is only known at
///         runtime.
/// @see https://en.cppreference.com/w/cpp/container/span
template <class T, std::size_t Extent = dynamic_extent>
class span : private storage_detail::span_storage<T, Extent> {
    using base = storage_detail::span_storage<T, Extent>;

public:
    /// @brief This is the element type.
    using element_type = T;
    /// @brief This is `T`, with cv-qualification stripped.
    using value_type = std::remove_cv_t<T>;
    /// @brief This is the type that `size()` and `size_bytes()`
    ///        return.
    using size_type = std::size_t;
    /// @brief This is the type that iterator differences use.
    using difference_type = std::ptrdiff_t;
    /// @brief This is a pointer to an element.
    using pointer = T*;
    /// @brief This is a pointer to a const-qualified element.
    using const_pointer = const T*;
    /// @brief This is a reference to an element.
    using reference = T&;
    /// @brief This is a reference to a const-qualified element.
    using const_reference = const T&;
    /// @brief This is a random-access iterator over the span's
    ///        elements.
    using iterator = pointer;
    /// @brief This is a reverse random-access iterator over the
    ///        span's elements.
    using reverse_iterator = std::reverse_iterator<iterator>;

    /// @brief This is the extent this specialization was built with.
    static constexpr std::size_t extent = Extent;

    /// @brief This constructor builds an empty span. This
    ///        constructor is only available when `Extent ==
    ///        dynamic_extent || Extent == 0`.
    ///
    ///        This constructor matches real `std::span`'s
    ///        constraint. A probe confirmed this match. The probe
    ///        checked `std::is_default_constructible_v` both ways,
    ///        against both `std::span<int, 3>` and
    ///        `std::span<int, 0>`.
    template <std::size_t E = Extent, class = std::enable_if_t<E == dynamic_extent || E == 0>>
    constexpr span() noexcept {}

    /// @brief This constructor builds a span from a pointer and an element count.
    /// @param first Pointer to the first element.
    /// @param count The number of elements.
    constexpr span(pointer first, size_type count) noexcept : base(first, count) {}

    /// @brief This constructor builds a span from a pair of
    ///        random-access iterators.
    /// @param first An iterator to the first element.
    /// @param last An iterator one past the last element.
    template <class It, class = std::enable_if_t<std::is_convertible_v<
                             typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>>>
    constexpr span(It first, It last) noexcept : base(&*first, static_cast<size_type>(last - first)) {}

    /// @brief This constructor builds a span from a C array.
    /// @param arr The array. Must outlive this span.
    template <std::size_t N>
    constexpr span(element_type (&arr)[N]) noexcept : base(arr, N) {}

    /// @brief This constructor builds a span from a mutable
    ///        `std::array`.
    ///
    ///        This constructor is constrained the same way as the C
    ///        array, vector, and string overloads below: `U(*)[]`
    ///        must convert to `element_type(*)[]`. Without this
    ///        constraint, `is_constructible_v` reports true for
    ///        combinations that would need to silently discard
    ///        `const` in the constructor body.
    /// @param arr The array. Must outlive this span.
    template <class U, std::size_t N, class = std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]>>>
    constexpr span(std::array<U, N>& arr) noexcept : base(arr.data(), N) {}

    /// @brief This constructor builds a span from a const
    ///        `std::array`. The new span is over const elements.
    /// @param arr The array. Must outlive this span.
    template <class U, std::size_t N,
              class = std::enable_if_t<std::is_convertible_v<const U (*)[], element_type (*)[]>>>
    constexpr span(const std::array<U, N>& arr) noexcept : base(arr.data(), N) {}

    /// @brief This constructor builds a span from a mutable
    ///        `std::vector`.
    /// @param vec The vector to view. Must outlive this span.
    template <class U, class Alloc, class = std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]>>>
    constexpr span(std::vector<U, Alloc>& vec) noexcept : base(vec.data(), vec.size()) {}

    /// @brief This constructor builds a span from a const
    ///        `std::vector`. The new span is over const elements.
    /// @param vec The vector to view. Must outlive this span.
    template <class U, class Alloc,
              class = std::enable_if_t<std::is_convertible_v<const U (*)[], element_type (*)[]>>>
    constexpr span(const std::vector<U, Alloc>& vec) noexcept : base(vec.data(), vec.size()) {}

    /// @brief This constructor builds a span from a mutable
    ///        `std::string`.
    /// @param str The string to view. Must outlive this span.
    template <class E = element_type, class = std::enable_if_t<std::is_convertible_v<char (*)[], E (*)[]>>>
    constexpr span(std::string& str) noexcept : base(str.data(), str.size()) {}

    /// @brief This constructor builds a span from a const
    ///        `std::string`. The new span is over const elements.
    /// @param str The string to view. Must outlive this span.
    template <class E = element_type, class = std::enable_if_t<std::is_convertible_v<const char (*)[], E (*)[]>>>
    constexpr span(const std::string& str) noexcept : base(str.data(), str.size()) {}

    /// @brief This converting constructor takes a span of a
    ///        different extent.
    ///
    ///        This constructor requires `U(*)[]` to convert to
    ///        `element_type(*)[]`. This matches real `std::span`'s
    ///        own element-type constraint exactly: true only when `U`
    ///        and `T` are the same type up to cv-qualification, never
    ///        for a derived-to-base element type. A probe confirmed
    ///        this: `span<Derived>` does not convert to, and is not
    ///        even explicitly constructible into, `span<Base>`.
    ///
    ///        This constructor is implicit. The next constructor
    ///        below is its `explicit` counterpart, for the one case
    ///        real `std::span` requires `explicit`: narrowing from a
    ///        dynamic source extent to a known destination extent.
    ///
    ///        Real `std::span` expresses this rule with C++20's
    ///        conditional `explicit(bool)`, a feature this project's
    ///        C++17 floor does not have. This constructor and its
    ///        `explicit` counterpart below reproduce the same rule
    ///        with two separate constructor templates instead, one
    ///        plain and one `explicit`, each SFINAE-enabled on the
    ///        opposite half of the same condition. A probe confirmed
    ///        this gives the identical result as real `std::span`:
    ///        implicit for same-or-compatible extents regardless of
    ///        cv-change, explicit-only for dynamic-to-fixed.
    ///
    ///        The SFINAE condition sits on an extra defaulted
    ///        *function* parameter here, not a defaulted template
    ///        type parameter. Two constructor templates with
    ///        otherwise identical template-parameter-list shapes are
    ///        rejected as "cannot be overloaded," even when their
    ///        `enable_if` conditions differ. A first draft using the
    ///        template-parameter form hit exactly that error.
    /// @param other The span to convert from.
    template <class U, std::size_t OtherExtent>
    constexpr span(const span<U, OtherExtent>& other,
                   std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]> &&
                                         (Extent == dynamic_extent || OtherExtent != dynamic_extent),
                                     int> = 0) noexcept
        : base(other.data(), other.size()) {}

    /// @brief This is the `explicit` counterpart to the converting
    ///        constructor above.
    ///
    ///        Real `std::span` requires `explicit` for one case:
    ///        narrowing from a dynamic source extent to a known
    ///        destination extent. This constructor handles that case.
    /// @param other The span to convert from.
    template <class U, std::size_t OtherExtent>
    explicit constexpr span(const span<U, OtherExtent>& other,
                             std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]> &&
                                                   (Extent != dynamic_extent && OtherExtent == dynamic_extent),
                                               int> = 0) noexcept
        : base(other.data(), other.size()) {}

    /// @brief This is the copy constructor.
    constexpr span(const span&) noexcept = default;
    /// @brief This is copy assignment.
    /// @return `*this`.
    constexpr span& operator=(const span&) noexcept = default;

    /// @brief This is the number of elements.
    /// @return The count.
    constexpr size_type size() const noexcept { return base::extent_value; }

    /// @brief This is the number of bytes the viewed elements occupy.
    /// @return The byte count.
    constexpr size_type size_bytes() const noexcept { return base::extent_value * sizeof(element_type); }

    /// @brief This checks whether the span is empty.
    /// @return `true` if `size() == 0`.
    constexpr bool empty() const noexcept { return size() == 0; }

    /// @brief This is a pointer to the first element. Do not
    ///        dereference this pointer when `empty()` is true.
    /// @return The pointer.
    constexpr pointer data() const noexcept { return base::data_; }

    /// @brief This is the first element. `empty()` must be false.
    /// @return A reference to the first element.
    constexpr reference front() const { return base::data_[0]; }

    /// @brief This is the last element. `empty()` must be false.
    /// @return A reference to the last element.
    constexpr reference back() const { return base::data_[size() - 1]; }

    /// @brief This is the element at `idx`. `idx` must be less than
    ///        `size()`.
    /// @param idx The index.
    /// @return A reference to the element at `idx`.
    constexpr reference operator[](size_type idx) const { return base::data_[idx]; }

    /// @brief This is an iterator to the first element.
    /// @return The iterator.
    constexpr iterator begin() const noexcept { return base::data_; }
    /// @brief This is an iterator one past the last element.
    /// @return The iterator.
    constexpr iterator end() const noexcept { return base::data_ + size(); }
    /// @brief This is a reverse iterator to the last element.
    /// @return The iterator.
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    /// @brief This is a reverse iterator one before the first
    ///        element.
    /// @return The iterator.
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    /// @brief This returns the first `Count` elements, as a
    ///        fixed-extent span. This matches real
    ///        `std::span::first<Count>()`.
    /// @tparam Count The number of elements, known at compile time.
    /// @return The sub-span.
    template <std::size_t Count>
    constexpr span<element_type, Count> first() const {
        return span<element_type, Count>(data(), Count);
    }

    /// @brief This returns the first `count` elements. The result is
    ///        always `dynamic_extent`, even when you call this on a
    ///        fixed-extent span. This matches real
    ///        `std::span::first(size_type)`.
    /// @param count The number of elements.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> first(size_type count) const {
        return span<element_type, dynamic_extent>(data(), count);
    }

    /// @brief This returns the last `Count` elements, as a
    ///        fixed-extent span.
    /// @tparam Count The number of elements, known at compile time.
    /// @return The sub-span.
    template <std::size_t Count>
    constexpr span<element_type, Count> last() const {
        return span<element_type, Count>(data() + (size() - Count), Count);
    }

    /// @brief This returns the last `count` elements, as a
    ///        dynamic-extent span. The result is always
    ///        `dynamic_extent`. This matches real
    ///        `std::span::last(size_type)`.
    /// @param count The number of elements.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> last(size_type count) const {
        return span<element_type, dynamic_extent>(data() + (size() - count), count);
    }

    /// @brief This returns a sub-view starting at `Offset`, of
    ///        `Count` elements. When `Count == dynamic_extent`, the
    ///        sub-view holds the remainder instead. This method
    ///        computes the result extent at compile time, matching
    ///        real `std::span::subspan<Offset, Count>()` exactly.
    /// @tparam Offset The starting offset, known at compile time.
    /// @tparam Count The number of elements, or `dynamic_extent`
    ///         (the default) for the remainder.
    /// @return The sub-span.
    template <std::size_t Offset, std::size_t Count = dynamic_extent>
    constexpr span<element_type, storage_detail::subspan_extent(Extent, Offset, Count)> subspan() const {
        constexpr std::size_t resolved = storage_detail::subspan_extent(Extent, Offset, Count);
        return span<element_type, resolved>(data() + Offset, Count == dynamic_extent ? size() - Offset : Count);
    }

    /// @brief This returns a sub-view starting at `offset`, of
    ///        `count` elements. The default for `count` is
    ///        `dynamic_extent`, for the remainder. The result is
    ///        always `dynamic_extent`, matching real
    ///        `std::span::subspan(offset, count)`.
    /// @param offset The starting offset.
    /// @param count The number of elements, or `dynamic_extent` (the
    ///        default) for the remainder.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const {
        return span<element_type, dynamic_extent>(data() + offset, count == dynamic_extent ? size() - offset : count);
    }
};

/// @brief This function reinterprets `s`'s elements as a read-only
///        view of their underlying bytes. This matches real
///        `std::as_bytes`.
/// @tparam T The source span's element type.
/// @tparam Extent The source span's extent.
/// @param s The span to reinterpret.
/// @return A `span<const std::byte, ByteExtent>` over the same
///         memory. `ByteExtent` is `s.size_bytes()` when `Extent` is
///         known at compile time. `ByteExtent` is `dynamic_extent`
///         otherwise. A probe confirmed that real `std::as_bytes`'s
///         byte count is `size() * sizeof(T)`.
/// @see https://en.cppreference.com/w/cpp/container/span/as_bytes
template <class T, std::size_t Extent>
span<const std::byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)> as_bytes(span<T, Extent> s) {
    return {reinterpret_cast<const std::byte*>(s.data()), s.size_bytes()};
}

/// @brief This function reinterprets `s`'s elements as a writable
///        view of their underlying bytes. This matches real
///        `std::as_writable_bytes`.
///
///        This function only participates when `T` is not itself
///        const-qualified. This matches the real function's
///        constraint.
/// @tparam T The source span's element type.
/// @tparam Extent The source span's extent.
/// @param s The span to reinterpret.
/// @return A `span<std::byte, ByteExtent>` over the same memory.
/// @see https://en.cppreference.com/w/cpp/container/span/as_bytes
template <class T, std::size_t Extent, class = std::enable_if_t<!std::is_const_v<T>>>
span<std::byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)> as_writable_bytes(span<T, Extent> s) {
    return {reinterpret_cast<std::byte*>(s.data()), s.size_bytes()};
}

/// @brief This namespace promotes `dynamic_extent`, `span`,
///        `as_bytes`, and `as_writable_bytes` to
///        `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::span::dynamic_extent;
using bridge::detail::truss::cpp17::span::span;
using bridge::detail::truss::cpp17::span::as_bytes;
using bridge::detail::truss::cpp17::span::as_writable_bytes;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::span

/// @brief This is the Exports namespace for `span`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace span { ... }` wrapper, for
/// the same reason as truss/cpp17/expected.hpp's Exports namespace.
/// This header's primary export is a type named `span`. The wrapper's
/// name would be `span` too, and the two names would collide. This
/// namespace promotes `span` straight from the `cpp17` inline
/// namespace instead, and avoids the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::span::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::dynamic_extent;
using bridge::exports::truss::span;
using bridge::exports::truss::as_bytes;
using bridge::exports::truss::as_writable_bytes;
} // namespace bridge::truss
