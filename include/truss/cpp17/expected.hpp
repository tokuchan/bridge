/// @file expected.hpp
/// @brief Truss's from-scratch `expected<T,E>` polyfill for standards
///        that predate C++23 — unlike `std::optional`, `std::expected`
///        doesn't exist at all before C++23, so there is no pre-existing
///        STL type for Truss to add free functions onto (compare
///        truss/cpp17/optional.hpp). This header is the exception to
///        that shape: Truss owns a complete class here. See
///        docs/adr/0010-expected-truss-owns-the-class.md for the full
///        rationale, docs/adr/0001-namespace-and-export-scheme.md for
///        the namespace scheme this follows, and docs/adr/0008-best-
///        effort-head-standard.md for the "behaviorally
///        indistinguishable" bar Deck's alias-selection (deck/cpp17/
///        expected.hpp) has to clear.
///
/// `bridge::truss::expected<T,E>` is unconditionally this polyfill,
/// regardless of standard or toolchain — Truss never itself passes
/// through to `std::expected`, even under C++23 where the real type is
/// available. That selection happens exactly once, in Deck.
#pragma once

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace bridge::detail::truss::cpp17::expected {

/// @brief C++17-compatible stand-in for `std::remove_cvref_t` (a C++20
///        addition), used only to disambiguate `unexpected`'s
///        forwarding constructor from its copy/move constructors.
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class E>
class unexpected;

/// @brief True for `unexpected<E>` for any `E`, false otherwise. Used to
///        reject `E`-being-a-specialization-of-`unexpected` at
///        `unexpected`'s own definition, matching `std::unexpected`'s
///        constraint.
template <class T>
struct is_unexpected : std::false_type {};
/// @copydoc is_unexpected
template <class E>
struct is_unexpected<unexpected<E>> : std::true_type {};
/// @brief Convenience value for @ref is_unexpected.
template <class T>
inline constexpr bool is_unexpected_v = is_unexpected<T>::value;

/// @brief Tag type selecting `expected`'s error-constructing
///        constructor overloads, mirroring `std::in_place_t`. Matches
///        `std::unexpect_t`.
struct unexpect_t {
    /// @brief Explicit so `unexpect_t{}` can't happen via copy-list-
    ///        initialization from `{}` in a context expecting some
    ///        other tag type.
    explicit unexpect_t() = default;
};
/// @brief The canonical `unexpect_t` instance, passed to select
///        `expected`'s in-place-error constructors. Matches
///        `std::unexpect`.
inline constexpr unexpect_t unexpect{};

/// @brief Wraps an error value of type `E`, matching `std::unexpected`.
///        Constructed explicitly and passed to `expected`'s converting
///        constructors, or returned directly from a function reporting
///        failure.
/// @tparam E The error type. Must be a non-array, non-cv-qualified
///         object type, and must not itself be a specialization of
///         `unexpected`.
template <class E>
class unexpected {
    static_assert(std::is_object_v<E>, "unexpected<E>: E must be an object type");
    static_assert(!std::is_array_v<E>, "unexpected<E>: E must not be an array type");
    static_assert(!std::is_const_v<E>, "unexpected<E>: E must not be const-qualified");
    static_assert(!std::is_volatile_v<E>, "unexpected<E>: E must not be volatile-qualified");
    static_assert(!is_unexpected_v<E>, "unexpected<E>: E must not itself be a specialization of unexpected");

public:
    /// @brief Copies the wrapped error.
    constexpr unexpected(const unexpected&) = default;
    /// @brief Moves the wrapped error.
    constexpr unexpected(unexpected&&) = default;

    /// @brief Constructs the wrapped error directly from `e`.
    /// @param e The error value to wrap, forwarded into `E`'s
    ///          constructor.
    template <class Err = E,
              class = std::enable_if_t<!std::is_same_v<remove_cvref_t<Err>, unexpected> &&
                                        !std::is_same_v<remove_cvref_t<Err>, std::in_place_t> &&
                                        std::is_constructible_v<E, Err>>>
    constexpr explicit unexpected(Err&& e) : val_(std::forward<Err>(e)) {}

    /// @brief Constructs the wrapped error in place from `args`.
    /// @param args Forwarded to `E`'s constructor.
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<E, Args...>>>
    constexpr explicit unexpected(std::in_place_t, Args&&... args) : val_(std::forward<Args>(args)...) {}

    /// @brief Constructs the wrapped error in place from an
    ///        initializer list plus `args`.
    /// @param il Forwarded to `E`'s constructor as the first argument.
    /// @param args Forwarded to `E`'s constructor after `il`.
    template <class U, class... Args,
              class = std::enable_if_t<std::is_constructible_v<E, std::initializer_list<U>&, Args...>>>
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
        : val_(il, std::forward<Args>(args)...) {}

    /// @brief Copy-assigns the wrapped error.
    /// @return `*this`.
    constexpr unexpected& operator=(const unexpected&) = default;
    /// @brief Move-assigns the wrapped error.
    /// @return `*this`.
    constexpr unexpected& operator=(unexpected&&) = default;

    /// @brief Const-lvalue access to the wrapped error.
    /// @return A const reference to the wrapped error.
    constexpr const E& error() const& noexcept { return val_; }
    /// @brief Lvalue access to the wrapped error.
    /// @return A reference to the wrapped error.
    constexpr E& error() & noexcept { return val_; }
    /// @brief Const-rvalue access to the wrapped error.
    /// @return A const rvalue reference to the wrapped error.
    constexpr const E&& error() const&& noexcept { return std::move(val_); }
    /// @brief Rvalue access to the wrapped error.
    /// @return An rvalue reference to the wrapped error.
    constexpr E&& error() && noexcept { return std::move(val_); }

    /// @brief Swaps the wrapped error with `other`'s.
    /// @param other The `unexpected` to swap with.
    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
        using std::swap;
        swap(val_, other.val_);
    }

    /// @brief Compares the wrapped errors for equality.
    /// @param lhs The left-hand `unexpected`.
    /// @param rhs The right-hand `unexpected`, possibly of a different
    ///            error type.
    /// @return Whether `lhs.error() == rhs.error()`.
    template <class E2>
    friend constexpr bool operator==(const unexpected& lhs, const unexpected<E2>& rhs) {
        return lhs.val_ == rhs.error();
    }

    /// @brief ADL swap, forwarding to the member @ref swap.
    /// @param x The first `unexpected`.
    /// @param y The second `unexpected`.
    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

private:
    E val_;
};

/// @brief Deduces `unexpected<E>` from a single constructor argument,
///        matching `std::unexpected`'s deduction guide.
template <class E>
unexpected(E) -> unexpected<E>;

/// @brief Thrown by `expected<T,E>::value()` when accessed without a
///        value; carries a copy of the error that caused the access to
///        fail. Matches `std::bad_expected_access<E>`. Forward-declared
///        here so the `void` specialization below (its common base) can
///        reference it; defined for real further down.
/// @tparam E The wrapped error type.
template <class E>
class bad_expected_access;

/// @brief Common base of `bad_expected_access<E>`, holding the
///        exception message. Matches `std::bad_expected_access<void>`.
///        Constructible only by a derived `bad_expected_access<E>`.
template <>
class bad_expected_access<void> : public std::exception {
protected:
    /// @brief Default-constructs the base.
    bad_expected_access() noexcept = default;
    /// @brief Copies the base.
    bad_expected_access(const bad_expected_access&) = default;
    /// @brief Moves the base.
    bad_expected_access(bad_expected_access&&) = default;
    /// @brief Copy-assigns the base.
    /// @return `*this`.
    bad_expected_access& operator=(const bad_expected_access&) = default;
    /// @brief Move-assigns the base.
    /// @return `*this`.
    bad_expected_access& operator=(bad_expected_access&&) = default;
    /// @brief Destroys the base.
    ~bad_expected_access() override = default;

public:
    /// @brief The exception message. Always `"bad expected access"`.
    /// @return The string `"bad expected access"`.
    const char* what() const noexcept override { return "bad expected access"; }
};

/// @brief Alias for @ref bad_expected_access "bad_expected_access<void>",
///        used only as the general template's base below. Doxygen's
///        static analysis of a `template<class E> class X : public
///        X<void>` shape (textually identical base/derived names)
///        misreports this as a "recursive class relation" — a known
///        Doxygen false-positive on this exact idiom, not an actual
///        cycle: `bad_expected_access<void>` is a concrete,
///        already-defined type by the time the general template below
///        is parsed. Routing through a differently-named alias avoids
///        the false positive without changing the real inheritance.
using bad_expected_access_void_base = bad_expected_access<void>;

/// @brief Definition of @ref bad_expected_access. Publicly inherits from
///        @ref bad_expected_access "bad_expected_access<void>" (via
///        `bad_expected_access_void_base`), so it can be caught without
///        knowing `E`.
template <class E>
class bad_expected_access : public bad_expected_access_void_base {
public:
    /// @brief Constructs from an error value, moving it in.
    /// @param e The error value to carry.
    explicit bad_expected_access(E e) : val_(std::move(e)) {}

    /// @brief Lvalue access to the carried error.
    /// @return A reference to the carried error.
    E& error() & noexcept { return val_; }
    /// @brief Const-lvalue access to the carried error.
    /// @return A const reference to the carried error.
    const E& error() const& noexcept { return val_; }
    /// @brief Rvalue access to the carried error.
    /// @return An rvalue reference to the carried error.
    E&& error() && noexcept { return std::move(val_); }
    /// @brief Const-rvalue access to the carried error.
    /// @return A const rvalue reference to the carried error.
    const E&& error() const&& noexcept { return std::move(val_); }

private:
    E val_;
};

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::expected::unexpect_t;
using bridge::detail::truss::cpp17::expected::unexpect;
using bridge::detail::truss::cpp17::expected::unexpected;
using bridge::detail::truss::cpp17::expected::bad_expected_access;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::expected

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace expected { ... }` wrapper here (same reason as
/// deck/cpp17/optional.hpp's exports): this header's primary export
/// will be a type named `expected` once expected<T,E> itself lands, and
/// nesting it inside an inline namespace of the identical name makes
/// that inline namespace's own qualified name reachable at this same
/// scope, colliding with the promoted type. Promoting straight from the
/// `cpp17` inline namespace avoids the collision up front rather than
/// hitting it later.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::expected::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::unexpect_t;
using bridge::exports::truss::unexpect;
using bridge::exports::truss::unexpected;
using bridge::exports::truss::bad_expected_access;
} // namespace bridge::truss
