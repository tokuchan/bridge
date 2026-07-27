/// @file optional.hpp
/// @brief The STL-shaped `optional<T>` Deck owns: a passthrough alias to
///        `std::optional<T>` when the detected ecosystem already has
///        monadic support, or a wrapper built on Truss's free functions
///        when it doesn't — selected so there is no detectable
///        difference between the two from the caller's side. See
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace scheme and docs/adr/0008-best-effort-head-standard.md
///        for why this selection exists per-ecosystem rather than on a
///        single global language-standard threshold.
#pragma once

#include <optional>
#include <utility>

#include <rivets/features.hpp>
#include <truss/cpp17/optional.hpp>

namespace bridge::detail::deck::cpp17::optional {

#if BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 202110L

/// @brief Passthrough: this ecosystem's `std::optional` already has
///        monadic methods, so bridge adds nothing.
template <class T>
using optional = std::optional<T>;

#else

/// @brief `std::optional<T>` plus monadic methods, for ecosystems whose
///        `std::optional` doesn't have them yet. Built on Truss's free
///        functions (include/truss/cpp17/optional.hpp); every other
///        member comes from `std::optional<T>` via public inheritance.
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
    /// @brief Constructs from a plain `std::optional<T>`, copying it.
    /// @param other The value to copy.
    constexpr optional(const std::optional<T>& other) : std::optional<T>(other) {}
    /// @brief Constructs from a plain `std::optional<T>`, moving it.
    /// @param other The value to move from.
    constexpr optional(std::optional<T>&& other) : std::optional<T>(std::move(other)) {}

    /// @brief See bridge::truss::and_then. Wraps the result back into
    ///        `optional` (even when `f` returned a plain `std::optional`)
    ///        so chained calls (`.and_then(...).and_then(...)`) keep
    ///        working.
    /// @param f A callable returning a `std::optional` specialization
    ///           (Truss's or Deck's own).
    /// @return `f`'s result, or an empty instance of its optional type.
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

    /// @brief See bridge::truss::or_else.
    /// @param f A callable, invoked with no arguments, returning
    ///           something convertible to `std::optional<T>`.
    /// @return A copy of `*this`, or `f`'s result.
    template <class F>
    constexpr optional or_else(F&& f) const& {
        return optional(bridge::truss::or_else(static_cast<const std::optional<T>&>(*this), std::forward<F>(f)));
    }
    /// @brief See bridge::truss::or_else.
    /// @param f A callable, invoked with no arguments, returning
    ///           something convertible to `std::optional<T>`.
    /// @return `*this`, moved, or `f`'s result.
    template <class F>
    constexpr optional or_else(F&& f) && {
        return optional(bridge::truss::or_else(static_cast<std::optional<T>&&>(*this), std::forward<F>(f)));
    }

    /// @brief See bridge::truss::transform. Wraps the result in
    ///        `optional` (Truss's free function always returns a plain
    ///        `std::optional`, since it has no notion of Deck's wrapper)
    ///        so chained calls keep working.
    /// @param f A callable returning a non-`void` value.
    /// @return `f`'s result wrapped in `optional`, or an empty one.
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
    /** @brief Forwards to std::optional's operator op. Comparing a      \
     *  plain std::optional<T> against optional<T> (same T, not just     \
     *  U deducible generically) needs its own exact-match overload:     \
     *  with only the generic-U overloads below, this specific direction \
     *  (plain on the left) hits a genuine 3-way ambiguity that partial  \
     *  ordering cannot break, between std::optional's own base-deduced  \
     *  candidate and the generic-U ones on both sides -- confirmed by   \
     *  hitting that exact compile error, not assumed upfront. */        \
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
     *  comparison is actually well-formed. Unconstrained, this overload \
     *  is greedy enough to get pulled in via ADL for unrelated types    \
     *  that happen to mention optional<T> (Catch2's own internal        \
     *  expression-template machinery hit exactly this during testing —  \
     *  confirmed empirically, not a hypothetical concern). */           \
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

/// @brief Symbols promoted to `bridge::exports::deck::optional`.
namespace exports {
using bridge::detail::deck::cpp17::optional::optional;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::optional

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace optional { ... }` wrapper here (unlike
/// truss/cpp17/optional.hpp's exports, where the leaf segment name
/// "optional" never collides with any symbol it exports): this header's
/// primary export is *also* named `optional`, and nesting it inside an
/// inline namespace of the identical name makes that inline namespace's
/// own qualified name reachable at this same scope, colliding with the
/// promoted type. Confirmed by hitting the actual compile error before
/// working around it, not assumed. Promoting straight from the `cpp17`
/// inline namespace avoids the collision without losing the collapse to
/// `bridge::exports::deck::optional` cpp17-inline gives every other
/// header.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::optional::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::optional;
} // namespace bridge::deck

/// @brief Bridge's public API surface — optional is flattened all the
///        way to bridge::optional, matching docs/adr/0001's own
///        worked example.
namespace bridge {
using bridge::deck::optional;
} // namespace bridge
