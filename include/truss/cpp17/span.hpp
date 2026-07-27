/// @file span.hpp
/// @brief Truss's from-scratch `std::span<T, Extent>` polyfill for
///        C++17. `span` doesn't exist at all before C++20 — like
///        `expected` (docs/adr/0010-expected-truss-owns-the-class.md),
///        there's no pre-existing C++17 type for Truss to attach free
///        functions onto, so Truss owns a complete class instead. See
///        docs/adr/0015-span-truss-owns-the-class.md for the full
///        fidelity scope: both static and dynamic `Extent` are
///        supported in full (including the storage-layout difference
///        and the static/dynamic conversion rules), the
///        range/container-constructing overloads are implemented for
///        the specific named cases real code overwhelmingly needs
///        (not a generic SFINAE net approximating `ranges::
///        contiguous_range`), and the C++23 tuple-like interface
///        (`get<I>`, structured bindings) is a disclosed, deferred
///        follow-up.
///
/// `bridge::truss::span<T, Extent>` is unconditionally this polyfill,
/// regardless of standard or toolchain — that selection happens
/// exactly once, in Deck (`deck/cpp17/span.hpp`), independent of
/// whatever other facility Deck is selecting elsewhere.
#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

namespace bridge::detail::truss::cpp17::span {

/// @brief Sentinel `Extent` value meaning "the size is only known at
///        runtime." Matches real `std::dynamic_extent`.
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

/// @brief A non-owning view over a contiguous sequence of `T`, with
///        an `Extent` known either at compile time or only at
///        runtime. Matches real `std::span<T, Extent>`'s shape:
///        trivially copyable, never allocates, never extends the
///        viewed sequence's lifetime.
/// @tparam T The element type.
/// @tparam Extent The compile-time extent, or `dynamic_extent` (the
///         default) when only known at runtime.
/// @see https://en.cppreference.com/w/cpp/container/span
template <class T, std::size_t Extent = dynamic_extent>
class span : private storage_detail::span_storage<T, Extent> {
    using base = storage_detail::span_storage<T, Extent>;

public:
    /// @brief The element type.
    using element_type = T;
    /// @brief `T` with cv-qualification stripped.
    using value_type = std::remove_cv_t<T>;
    /// @brief The type `size()`/`size_bytes()` return.
    using size_type = std::size_t;
    /// @brief The type iterator differences are expressed in.
    using difference_type = std::ptrdiff_t;
    /// @brief Pointer to an element.
    using pointer = T*;
    /// @brief Pointer to a const-qualified element.
    using const_pointer = const T*;
    /// @brief Reference to an element.
    using reference = T&;
    /// @brief Reference to a const-qualified element.
    using const_reference = const T&;
    /// @brief A random-access iterator over the span's elements.
    using iterator = pointer;
    /// @brief A reverse random-access iterator over the span's elements.
    using reverse_iterator = std::reverse_iterator<iterator>;

    /// @brief The extent this specialization was instantiated with.
    static constexpr std::size_t extent = Extent;

    /// @brief Default-constructs an empty span. Only available when
    ///        `Extent == dynamic_extent || Extent == 0` -- matching
    ///        real `std::span`'s constraint, confirmed via probe
    ///        (`std::is_default_constructible_v` both ways against
    ///        `std::span<int,3>`/`std::span<int,0>`) rather than
    ///        assumed from the standard's prose alone.
    template <std::size_t E = Extent, class = std::enable_if_t<E == dynamic_extent || E == 0>>
    constexpr span() noexcept {}

    /// @brief Constructs from a pointer and an element count.
    /// @param first Pointer to the first element.
    /// @param count The number of elements.
    constexpr span(pointer first, size_type count) noexcept : base(first, count) {}

    /// @brief Constructs from a pair of random-access iterators.
    /// @param first An iterator to the first element.
    /// @param last An iterator one past the last element.
    template <class It, class = std::enable_if_t<std::is_convertible_v<
                             typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>>>
    constexpr span(It first, It last) noexcept : base(&*first, static_cast<size_type>(last - first)) {}

    /// @brief Constructs from a C array.
    /// @param arr The array. Must outlive this span.
    template <std::size_t N>
    constexpr span(element_type (&arr)[N]) noexcept : base(arr, N) {}

    /// @brief Constructs from a mutable `std::array`. Constrained the
    ///        same way as the C array/vector/string overloads below
    ///        (`U(*)[]` convertible to `element_type(*)[]`) -- without
    ///        this, `is_constructible_v` reports true for combinations
    ///        that would need to silently discard `const` in the
    ///        constructor body, confirmed by hitting exactly that
    ///        false-positive `is_constructible_v` result before adding
    ///        the constraint, not assumed.
    /// @param arr The array. Must outlive this span.
    template <class U, std::size_t N, class = std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]>>>
    constexpr span(std::array<U, N>& arr) noexcept : base(arr.data(), N) {}

    /// @brief Constructs from a const `std::array`, yielding a span
    ///        over const elements.
    /// @param arr The array. Must outlive this span.
    template <class U, std::size_t N,
              class = std::enable_if_t<std::is_convertible_v<const U (*)[], element_type (*)[]>>>
    constexpr span(const std::array<U, N>& arr) noexcept : base(arr.data(), N) {}

    /// @brief Constructs from a mutable `std::vector`.
    /// @param vec The vector to view. Must outlive this span.
    template <class U, class Alloc, class = std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]>>>
    constexpr span(std::vector<U, Alloc>& vec) noexcept : base(vec.data(), vec.size()) {}

    /// @brief Constructs from a const `std::vector`, yielding a span
    ///        over const elements.
    /// @param vec The vector to view. Must outlive this span.
    template <class U, class Alloc,
              class = std::enable_if_t<std::is_convertible_v<const U (*)[], element_type (*)[]>>>
    constexpr span(const std::vector<U, Alloc>& vec) noexcept : base(vec.data(), vec.size()) {}

    /// @brief Constructs from a mutable `std::string`.
    /// @param str The string to view. Must outlive this span.
    template <class E = element_type, class = std::enable_if_t<std::is_convertible_v<char (*)[], E (*)[]>>>
    constexpr span(std::string& str) noexcept : base(str.data(), str.size()) {}

    /// @brief Constructs from a const `std::string`, yielding a span
    ///        over const elements.
    /// @param str The string to view. Must outlive this span.
    template <class E = element_type, class = std::enable_if_t<std::is_convertible_v<const char (*)[], E (*)[]>>>
    constexpr span(const std::string& str) noexcept : base(str.data(), str.size()) {}

    /// @brief Converting constructor from a span of a different
    ///        extent (and possibly cv-qualification). Constrained on
    ///        array-pointer convertibility -- `U(*)[]` convertible to
    ///        `element_type(*)[]` -- matching real `std::span`'s own
    ///        element-type constraint exactly: true only when `U` and
    ///        `T` are the same type up to cv-qualification, never for
    ///        a derived-to-base element type, confirmed via probe
    ///        (`span<Derived>` does not convert to, nor is even
    ///        explicitly constructible into, `span<Base>`) rather than
    ///        assumed.
    ///
    ///        Real `std::span` is conditionally `explicit` here --
    ///        `explicit(Extent != dynamic_extent && OtherExtent ==
    ///        dynamic_extent)`, i.e. only when narrowing from an
    ///        unknown-at-compile-time source extent to a known one --
    ///        which needs C++20's conditional `explicit(bool)` to
    ///        express directly. Reproduced exactly anyway, without
    ///        needing that C++20 feature or accepting `expected`-style
    ///        always-explicit conservatism (docs/adr/0010): the two
    ///        conditions are mutually exclusive, so two separate
    ///        constructor templates -- one plain, one `explicit`, each
    ///        SFINAE-enabled on the opposite half of the same
    ///        condition -- give the identical result, confirmed via
    ///        probe against real `std::span` (implicit for same-or-
    ///        compatible extents regardless of cv-change; explicit-only
    ///        for dynamic-to-fixed) before settling on this shape. The
    ///        SFINAE condition is deliberately on an extra defaulted
    ///        *function* parameter, not a defaulted template type
    ///        parameter -- two constructor templates with otherwise
    ///        identical template-parameter-list shapes are rejected as
    ///        "cannot be overloaded" even when their `enable_if`
    ///        conditions differ, confirmed by hitting exactly that
    ///        error with a first draft using the template-parameter
    ///        form, not assumed.
    /// @param other The span to convert from.
    template <class U, std::size_t OtherExtent>
    constexpr span(const span<U, OtherExtent>& other,
                   std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]> &&
                                         (Extent == dynamic_extent || OtherExtent != dynamic_extent),
                                     int> = 0) noexcept
        : base(other.data(), other.size()) {}

    /// @brief `explicit` counterpart to the converting constructor
    ///        above, for the one case real `std::span` requires it:
    ///        narrowing from an unknown-at-compile-time source extent
    ///        to a known one.
    /// @param other The span to convert from.
    template <class U, std::size_t OtherExtent>
    explicit constexpr span(const span<U, OtherExtent>& other,
                             std::enable_if_t<std::is_convertible_v<U (*)[], element_type (*)[]> &&
                                                   (Extent != dynamic_extent && OtherExtent == dynamic_extent),
                                               int> = 0) noexcept
        : base(other.data(), other.size()) {}

    /// @brief Copy constructor.
    constexpr span(const span&) noexcept = default;
    /// @brief Copy assignment.
    /// @return `*this`.
    constexpr span& operator=(const span&) noexcept = default;

    /// @brief The number of elements.
    /// @return The count.
    constexpr size_type size() const noexcept { return base::extent_value; }

    /// @brief The number of bytes the viewed elements occupy.
    /// @return The byte count.
    constexpr size_type size_bytes() const noexcept { return base::extent_value * sizeof(element_type); }

    /// @brief Whether the span is empty.
    /// @return `true` if `size() == 0`.
    constexpr bool empty() const noexcept { return size() == 0; }

    /// @brief Pointer to the first element, or a value that shall not
    ///        be dereferenced if `empty()`.
    /// @return The pointer.
    constexpr pointer data() const noexcept { return base::data_; }

    /// @brief The first element. Precondition: not `empty()`.
    /// @return A reference to the first element.
    constexpr reference front() const { return base::data_[0]; }

    /// @brief The last element. Precondition: not `empty()`.
    /// @return A reference to the last element.
    constexpr reference back() const { return base::data_[size() - 1]; }

    /// @brief The element at `idx`. Precondition: `idx < size()`.
    /// @param idx The index.
    /// @return A reference to the element at `idx`.
    constexpr reference operator[](size_type idx) const { return base::data_[idx]; }

    /// @brief An iterator to the first element.
    /// @return The iterator.
    constexpr iterator begin() const noexcept { return base::data_; }
    /// @brief An iterator one past the last element.
    /// @return The iterator.
    constexpr iterator end() const noexcept { return base::data_ + size(); }
    /// @brief A reverse iterator to the last element.
    /// @return The iterator.
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    /// @brief A reverse iterator one before the first element.
    /// @return The iterator.
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    /// @brief The first `Count` elements, as a fixed-extent span.
    ///        Matches real `std::span::first<Count>()`.
    /// @tparam Count The number of elements, known at compile time.
    /// @return The sub-span.
    template <std::size_t Count>
    constexpr span<element_type, Count> first() const {
        return span<element_type, Count>(data(), Count);
    }

    /// @brief The first `count` elements, as a dynamic-extent span --
    ///        always `dynamic_extent`, even called on a fixed-extent
    ///        span, matching real `std::span::first(size_type)`
    ///        (confirmed via probe, not assumed).
    /// @param count The number of elements.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> first(size_type count) const {
        return span<element_type, dynamic_extent>(data(), count);
    }

    /// @brief The last `Count` elements, as a fixed-extent span.
    /// @tparam Count The number of elements, known at compile time.
    /// @return The sub-span.
    template <std::size_t Count>
    constexpr span<element_type, Count> last() const {
        return span<element_type, Count>(data() + (size() - Count), Count);
    }

    /// @brief The last `count` elements, as a dynamic-extent span --
    ///        always `dynamic_extent`, matching real
    ///        `std::span::last(size_type)`.
    /// @param count The number of elements.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> last(size_type count) const {
        return span<element_type, dynamic_extent>(data() + (size() - count), count);
    }

    /// @brief A sub-view starting at `Offset`, of `Count` elements (or
    ///        the remainder, if `Count == dynamic_extent`). The result
    ///        extent is computed at compile time, matching real
    ///        `std::span::subspan<Offset, Count>()` exactly (confirmed
    ///        via probe, not assumed).
    /// @tparam Offset The starting offset, known at compile time.
    /// @tparam Count The number of elements, or `dynamic_extent`
    ///         (the default) for the remainder.
    /// @return The sub-span.
    template <std::size_t Offset, std::size_t Count = dynamic_extent>
    constexpr span<element_type, storage_detail::subspan_extent(Extent, Offset, Count)> subspan() const {
        constexpr std::size_t resolved = storage_detail::subspan_extent(Extent, Offset, Count);
        return span<element_type, resolved>(data() + Offset, Count == dynamic_extent ? size() - Offset : Count);
    }

    /// @brief A sub-view starting at `offset`, of `count` elements (or
    ///        the remainder, if `count == dynamic_extent`, the
    ///        default) -- always `dynamic_extent`, matching real
    ///        `std::span::subspan(offset, count)`.
    /// @param offset The starting offset.
    /// @param count The number of elements, or `dynamic_extent` (the
    ///        default) for the remainder.
    /// @return The sub-span.
    constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const {
        return span<element_type, dynamic_extent>(data() + offset, count == dynamic_extent ? size() - offset : count);
    }
};

/// @brief Reinterprets `s`'s elements as a read-only view of their
///        underlying bytes. Matches real `std::as_bytes`.
/// @tparam T The source span's element type.
/// @tparam Extent The source span's extent.
/// @param s The span to reinterpret.
/// @return A `span<const std::byte, ByteExtent>` over the same memory,
///         where `ByteExtent` is `s.size_bytes()` when `Extent` is
///         known at compile time, or `dynamic_extent` otherwise --
///         confirmed via probe that real `std::as_bytes`'s byte count
///         is `size() * sizeof(T)`, not assumed.
/// @see https://en.cppreference.com/w/cpp/container/span/as_bytes
template <class T, std::size_t Extent>
span<const std::byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)> as_bytes(span<T, Extent> s) {
    return {reinterpret_cast<const std::byte*>(s.data()), s.size_bytes()};
}

/// @brief Reinterprets `s`'s elements as a writable view of their
///        underlying bytes. Matches real `std::as_writable_bytes`.
///        Only participates when `T` isn't itself const-qualified,
///        matching the real function's constraint.
/// @tparam T The source span's element type.
/// @tparam Extent The source span's extent.
/// @param s The span to reinterpret.
/// @return A `span<std::byte, ByteExtent>` over the same memory.
/// @see https://en.cppreference.com/w/cpp/container/span/as_bytes
template <class T, std::size_t Extent, class = std::enable_if_t<!std::is_const_v<T>>>
span<std::byte, Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T)> as_writable_bytes(span<T, Extent> s) {
    return {reinterpret_cast<std::byte*>(s.data()), s.size_bytes()};
}

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::span::dynamic_extent;
using bridge::detail::truss::cpp17::span::span;
using bridge::detail::truss::cpp17::span::as_bytes;
using bridge::detail::truss::cpp17::span::as_writable_bytes;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::span

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace span { ... }` wrapper here (same reason as
/// truss/cpp17/expected.hpp's exports): this header's primary export
/// is a type named `span`, and nesting it inside an inline namespace
/// of the identical name makes that inline namespace's own qualified
/// name reachable at this same scope, colliding with the promoted
/// type. Promoting straight from the `cpp17` inline namespace avoids
/// the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::span::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::dynamic_extent;
using bridge::exports::truss::span;
using bridge::exports::truss::as_bytes;
using bridge::exports::truss::as_writable_bytes;
} // namespace bridge::truss
