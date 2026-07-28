/// @file optional.hpp
/// @brief Deck's `optional<T>`. This type acts the same way in every
///        ecosystem.
///
///        If the ecosystem's `std::optional` already has monadic
///        methods, `optional<T>` is a passthrough alias to
///        `std::optional<T>`. If the ecosystem's `std::optional` does
///        not have monadic methods yet, `optional<T>` is a polyfill
///        class that wraps Truss's free functions. See
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace rule. See docs/adr/0008-best-effort-head-
///        standard.md for the reason Deck makes this choice for each
///        ecosystem, not from one language-standard number for every
///        ecosystem.
#pragma once

#include <optional>
#include <utility>

#include <rivets/features.hpp>
#include <truss/cpp17/optional.hpp>

namespace bridge::detail::deck::cpp17::optional {

#if BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 202110L

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::optional` already has monadic methods. Bridge adds
///        nothing here.
template <class T>
using optional = std::optional<T>;

#else

/// @brief This class is a polyfill. It adds monadic methods to
///        `std::optional<T>`, for an ecosystem whose `std::optional`
///        does not have them yet.
///
///        This class inherits from `std::optional<T>`. This class
///        gets the monadic methods from Truss's free functions
///        (include/truss/cpp17/optional.hpp). This class gets every
///        other method from `std::optional<T>` through inheritance.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T>
class optional : public std::optional<T> {
public:
    using std::optional<T>::optional;
    using std::optional<T>::operator=;

    // std::optional<T>'s own copy/move constructors are *not* brought in
    // by the using-declaration above (inherited constructors explicitly
    // exclude a base's copy/move constructors), so without these,
    // there's no way to construct this wrapper from a plain
    // std::optional<T> value -- needed below for or_else, whose
    // underlying bridge::truss::or_else necessarily returns the base
    // type. Verified empirically that omitting these fails to compile.
    /// @brief This constructor builds a new `optional<T>` from a plain
    ///        `std::optional<T>`. It copies the value.
    /// @param other The value to copy.
    constexpr optional(const std::optional<T>& other) : std::optional<T>(other) {}
    /// @brief This constructor builds a new `optional<T>` from a plain
    ///        `std::optional<T>`. It moves the value.
    /// @param other The value to move.
    constexpr optional(std::optional<T>&& other) : std::optional<T>(std::move(other)) {}

    /// @brief This method does the same work as `bridge::truss::and_then`.
    ///
    ///        This method puts the result back into `optional`, even
    ///        when `f` returns a plain `std::optional`. This lets you
    ///        chain `and_then` calls together.
    /// @param f A callable. `f` must return a `std::optional` or an
    ///          `optional`.
    /// @return `f`'s result in `optional`, or an empty `optional`.
    template <class F>
    constexpr auto and_then(F&& f) & {
        using Raw = std::decay_t<std::invoke_result_t<F, T&>>;
        return optional<typename Raw::value_type>(
            bridge::truss::and_then(static_cast<std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) const& {
        using Raw = std::decay_t<std::invoke_result_t<F, const T&>>;
        return optional<typename Raw::value_type>(
            bridge::truss::and_then(static_cast<const std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) && {
        using Raw = std::decay_t<std::invoke_result_t<F, T&&>>;
        return optional<typename Raw::value_type>(
            bridge::truss::and_then(static_cast<std::optional<T>&&>(*this), std::forward<F>(f)));
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) const&& {
        using Raw = std::decay_t<std::invoke_result_t<F, const T&&>>;
        return optional<typename Raw::value_type>(
            bridge::truss::and_then(static_cast<const std::optional<T>&&>(*this), std::forward<F>(f)));
    }

    /// @brief This method does the same work as `bridge::truss::or_else`.
    /// @param f A callable. `f` takes no arguments. `f`'s result must
    ///          convert to `std::optional<T>`.
    /// @return A copy of `*this`, or `f`'s result.
    template <class F>
    constexpr optional or_else(F&& f) const& {
        return optional(bridge::truss::or_else(static_cast<const std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @brief This method does the same work as `bridge::truss::or_else`.
    /// @param f A callable. `f` takes no arguments. `f`'s result must
    ///          convert to `std::optional<T>`.
    /// @return `*this`, moved, or `f`'s result.
    template <class F>
    constexpr optional or_else(F&& f) && {
        return optional(bridge::truss::or_else(static_cast<std::optional<T>&&>(*this), std::forward<F>(f)));
    }

    /// @brief This method does the same work as `bridge::truss::transform`.
    ///
    ///        This method puts the result back into `optional`. This lets
    ///        you chain `transform` calls together.
    /// @param f A callable. `f` must not return `void`.
    /// @return `f`'s result in `optional`, or an empty `optional`.
    template <class F>
    constexpr auto transform(F&& f) & {
        using U = std::decay_t<std::invoke_result_t<F, T&>>;
        return optional<U>(bridge::truss::transform(static_cast<std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::decay_t<std::invoke_result_t<F, const T&>>;
        return optional<U>(bridge::truss::transform(static_cast<const std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::decay_t<std::invoke_result_t<F, T&&>>;
        return optional<U>(bridge::truss::transform(static_cast<std::optional<T>&&>(*this), std::forward<F>(f)));
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) const&& {
        using U = std::decay_t<std::invoke_result_t<F, const T&&>>;
        return optional<U>(bridge::truss::transform(static_cast<const std::optional<T>&&>(*this), std::forward<F>(f)));
    }
};

// Comparison operators: template argument deduction against
// std::optional<T>/<U> succeeds through public inheritance (derived-to-
// base deduction), but ambiguously — it competes with std::optional's
// own "compare against a generic value" overloads, which also accept an
// optional<T> argument in their unconstrained value slot. An operator
// declared to take exactly optional<T>/optional<U> wins over both by
// partial ordering, resolving the ambiguity. Verified empirically before
// writing this (three-way ambiguity without these; clean resolution
// with them) rather than assumed.
/// \cond BRIDGE_DETAIL
///
/// @def BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON
/// @brief Defines every `operator op` overload `optional<T>` needs
///        (against another `optional`, `std::nullopt_t`, a plain
///        `std::optional<T>`, and a raw comparable value, both
///        directions where relevant) for one comparison operator.
///        Undefined again immediately after its six invocations below.
///        Pure code-generation plumbing, not part of the public API --
///        excluded from the documentation-coverage gate for the same
///        reason truss/cpp17/format.hpp's field_writers/engine
///        namespaces are.
/// @param op The operator token, e.g. `==`.
#define BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(op)                                             \
    /** @brief Forwards to std::optional's operator op. */                                     \
    template <class T, class U>                                                                \
    constexpr bool operator op(const optional<T>& lhs, const optional<U>& rhs) {                \
        return static_cast<const std::optional<T>&>(lhs) op static_cast<const std::optional<U>&>(rhs); \
    }                                                                                          \
    /** @brief Forwards to std::optional's operator op. */                                     \
    template <class T>                                                                         \
    constexpr bool operator op(const optional<T>& lhs, std::nullopt_t rhs) noexcept {           \
        return static_cast<const std::optional<T>&>(lhs) op rhs;                                \
    }                                                                                          \
    /** @brief Forwards to std::optional's operator op. */                                     \
    template <class T>                                                                         \
    constexpr bool operator op(std::nullopt_t lhs, const optional<T>& rhs) noexcept {           \
        return lhs op static_cast<const std::optional<T>&>(rhs);                                \
    }                                                                                          \
    /** @brief Forwards to std::optional's operator op.                  \
     *                                                                   \
     *  Comparing a plain std::optional<T> against optional<T> (same T,  \
     *  not just U deducible generically) needs its own exact-match      \
     *  overload: with only the generic-U overloads below, this specific \
     *  direction (plain on the left) hits a genuine 3-way ambiguity     \
     *  that partial ordering cannot break, between std::optional's own  \
     *  base-deduced candidate and the generic-U ones on both sides --   \
     *  confirmed by hitting that exact compile error, not assumed       \
     *  upfront. */                                                      \
    template <class T>                                                                         \
    constexpr bool operator op(const optional<T>& lhs, const std::optional<T>& rhs) {           \
        return static_cast<const std::optional<T>&>(lhs) op rhs;                                \
    }                                                                                          \
    /** @copydoc operator op(const optional<T>&,const std::optional<T>&) */                     \
    template <class T>                                                                         \
    constexpr bool operator op(const std::optional<T>& lhs, const optional<T>& rhs) {           \
        return lhs op static_cast<const std::optional<T>&>(rhs);                                \
    }                                                                                          \
    /** @brief Forwards to std::optional's operator op, only when that   \
     *  comparison is actually well-formed.                              \
     *                                                                   \
     *  Unconstrained, this overload is greedy enough to get pulled in   \
     *  via ADL for unrelated types that happen to mention optional<T>   \
     *  (Catch2's own internal expression-template machinery hit         \
     *  exactly this during testing -- confirmed empirically, not a      \
     *  hypothetical concern). */                                        \
    template <class T, class U>                                                                \
    constexpr auto operator op(const optional<T>& lhs, const U& rhs)                            \
        -> decltype(std::declval<const std::optional<T>&>() op rhs, bool{}) {                   \
        return static_cast<const std::optional<T>&>(lhs) op rhs;                                \
    }                                                                                          \
    /** @copydoc operator op(const optional<T>&,const U&) */                                    \
    template <class T, class U>                                                                \
    constexpr auto operator op(const U& lhs, const optional<T>& rhs)                            \
        -> decltype(lhs op std::declval<const std::optional<T>&>(), bool{}) {                   \
        return lhs op static_cast<const std::optional<T>&>(rhs);                                \
    }

BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(==)
BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(!=)
BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(<)
BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(<=)
BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(>)
BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON(>=)

#undef BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON
/// \endcond

#endif // BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL

/// @brief This namespace promotes `optional` to
///        `bridge::exports::deck::optional`.
namespace exports {
using bridge::detail::deck::cpp17::optional::optional;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::optional

/// @brief This is the Exports namespace for `optional`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
///        This namespace has no `inline namespace optional { ... }`
///        wrapper. Other headers add this wrapper around their Exports
///        namespace. This header cannot do the same. The wrapper's
///        name would be `optional`. The type this header promotes is
///        also named `optional`. The two names would collide. This
///        namespace promotes `optional` straight from the `cpp17`
///        inline namespace instead, and avoids the collision. The
///        result is still `bridge::exports::deck::optional`, the same
///        result every other header's wrapper gives.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::optional::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::optional;
} // namespace bridge::deck

/// @brief This is bridge's public API. `optional` reaches all the way
///        to `bridge::optional` here. ADR-0001 shows this same
///        example.
namespace bridge {
using bridge::deck::optional;
} // namespace bridge
