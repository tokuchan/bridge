/// @file optional.hpp
/// @brief Free-function monadic operations for `std::optional`, matching
///        C++23's `and_then`/`or_else`/`transform`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the namespace
///        scheme this follows, and docs/adr/0008-best-effort-head-
///        standard.md for why these are free functions here (Deck owns
///        the STL-shaped wrapper type built on top of them).
#pragma once

#include <functional>
#include <optional>
#include <type_traits>

namespace bridge::detail::truss::cpp17::optional {

/// @brief Implementation detail of @ref is_optional. Pointer-conversion
///        SFINAE: matches a `std::optional<U>` specialization *or*
///        anything publicly derived from one (e.g. Deck's `optional<T>`
///        wrapper), since a derived-class pointer converts to a
///        base-class pointer implicitly. A callback chaining
///        `bridge::optional` operations naturally returns Deck's
///        wrapper type, not `std::optional` itself, and this needs to
///        recognize that as valid.
template <class T>
struct is_optional_impl {
    /// @brief Selected when `U*` converts to `const std::optional<V>*`
    ///        for some deducible `V`.
    /// @return `std::true_type`; never actually called, only used
    ///         inside `decltype`.
    template <class V>
    static std::true_type test(const std::optional<V>*);
    /// @brief Selected otherwise.
    /// @return `std::false_type`; never actually called, only used
    ///         inside `decltype`.
    static std::false_type test(...);
    /// @brief The result of overload resolution between the two above.
    using type = decltype(test(std::declval<T*>()));
};
/// @brief True for `std::optional<U>`, or anything publicly derived
///        from a `std::optional<U>` specialization.
template <class T>
using is_optional = typename is_optional_impl<T>::type;
/// @brief Convenience value for @ref is_optional.
template <class T>
inline constexpr bool is_optional_v = is_optional<T>::value;

/// @brief If `opt` has a value, invoke `f` with it and return the
///        result (which must itself be a `std::optional`); otherwise
///        return an empty result of that same type.
/// @param opt The optional to inspect.
/// @param f A callable returning a `std::optional` specialization.
/// @return `f`'s result, or an empty instance of its optional type.
template <class T, class F>
constexpr auto and_then(std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), *opt));
    }
    return U{};
}

/// @brief If `opt` has a value, invoke `f` with it and return the
///        result (which must itself be a `std::optional`); otherwise
///        return an empty result of that same type.
/// @param opt The optional to inspect.
/// @param f A callable returning a `std::optional` specialization.
/// @return `f`'s result, or an empty instance of its optional type.
template <class T, class F>
constexpr auto and_then(const std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), *opt));
    }
    return U{};
}

/// @brief If `opt` has a value, invoke `f` with it and return the
///        result (which must itself be a `std::optional`); otherwise
///        return an empty result of that same type.
/// @param opt The optional to inspect.
/// @param f A callable returning a `std::optional` specialization.
/// @return `f`'s result, or an empty instance of its optional type.
template <class T, class F>
constexpr auto and_then(std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), std::move(*opt)));
    }
    return U{};
}

/// @brief If `opt` has a value, invoke `f` with it and return the
///        result (which must itself be a `std::optional`); otherwise
///        return an empty result of that same type.
/// @param opt The optional to inspect.
/// @param f A callable returning a `std::optional` specialization.
/// @return `f`'s result, or an empty instance of its optional type.
template <class T, class F>
constexpr auto and_then(const std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), std::move(*opt)));
    }
    return U{};
}

/// @brief If `opt` has a value, return a copy of it; otherwise invoke
///        `f` with no arguments and return its result.
/// @param opt The optional to inspect.
/// @param f A callable, invoked with no arguments, returning something
///           convertible to `std::optional<T>`.
/// @return A copy of `opt`, or `f`'s result.
template <class T, class F>
constexpr std::optional<T> or_else(const std::optional<T>& opt, F&& f) {
    static_assert(std::is_convertible_v<std::invoke_result_t<F>, std::optional<T>>,
                  "F must return something convertible to std::optional<T>");
    if (opt.has_value()) {
        return opt;
    }
    return std::forward<F>(f)();
}

/// @brief If `opt` has a value, return it moved-from; otherwise invoke
///        `f` with no arguments and return its result.
/// @param opt The optional to inspect.
/// @param f A callable, invoked with no arguments, returning something
///           convertible to `std::optional<T>`.
/// @return `opt`, moved, or `f`'s result.
template <class T, class F>
constexpr std::optional<T> or_else(std::optional<T>&& opt, F&& f) {
    static_assert(std::is_convertible_v<std::invoke_result_t<F>, std::optional<T>>,
                  "F must return something convertible to std::optional<T>");
    if (opt.has_value()) {
        return std::move(opt);
    }
    return std::forward<F>(f)();
}

/// @brief If `opt` has a value, invoke `f` with it and return
///        `std::optional<U>` containing the result; otherwise return an
///        empty `std::optional<U>`.
/// @param opt The optional to inspect.
/// @param f A callable returning a non-`void` value.
/// @return `f`'s result wrapped in `std::optional`, or an empty one.
template <class T, class F>
constexpr auto transform(std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), *opt)};
    }
    return std::optional<U>{};
}

/// @brief If `opt` has a value, invoke `f` with it and return
///        `std::optional<U>` containing the result; otherwise return an
///        empty `std::optional<U>`.
/// @param opt The optional to inspect.
/// @param f A callable returning a non-`void` value.
/// @return `f`'s result wrapped in `std::optional`, or an empty one.
template <class T, class F>
constexpr auto transform(const std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), *opt)};
    }
    return std::optional<U>{};
}

/// @brief If `opt` has a value, invoke `f` with it and return
///        `std::optional<U>` containing the result; otherwise return an
///        empty `std::optional<U>`.
/// @param opt The optional to inspect.
/// @param f A callable returning a non-`void` value.
/// @return `f`'s result wrapped in `std::optional`, or an empty one.
template <class T, class F>
constexpr auto transform(std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), std::move(*opt))};
    }
    return std::optional<U>{};
}

/// @brief If `opt` has a value, invoke `f` with it and return
///        `std::optional<U>` containing the result; otherwise return an
///        empty `std::optional<U>`.
/// @param opt The optional to inspect.
/// @param f A callable returning a non-`void` value.
/// @return `f`'s result wrapped in `std::optional`, or an empty one.
template <class T, class F>
constexpr auto transform(const std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), std::move(*opt))};
    }
    return std::optional<U>{};
}

/// @brief Symbols promoted to `bridge::exports::truss::optional`.
namespace exports {
using bridge::detail::truss::cpp17::optional::and_then;
using bridge::detail::truss::cpp17::optional::or_else;
using bridge::detail::truss::cpp17::optional::transform;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::optional

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
namespace bridge::exports::truss {
inline namespace cpp17 {
inline namespace optional {
using namespace bridge::detail::truss::cpp17::optional::exports;
} // namespace optional
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::and_then;
using bridge::exports::truss::or_else;
using bridge::exports::truss::transform;
} // namespace bridge::truss
