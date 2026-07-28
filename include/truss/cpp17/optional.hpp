/// @file optional.hpp
/// @brief Truss's monadic methods for `std::optional`: `and_then`,
///        `or_else`, and `transform`. These functions match the C++23
///        methods with the same names.
///
///        These are free functions, not methods on a class. See
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace rule that this file follows. See
///        docs/adr/0008-best-effort-head-standard.md for the reason
///        that these are free functions here. Deck adds a class with
///        the same methods on top of these functions.
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

/// @brief If `opt` holds a value, this function calls `f` with the
///        value. If `opt` holds no value, this function does not call
///        `f`.
///
///        `f` must return a `std::optional`. This function returns
///        `f`'s result when it calls `f`. This function returns an
///        empty `std::optional` of the same type when it does not
///        call `f`.
/// @param opt The optional to check.
/// @param f A callable. `f` must return a `std::optional`.
/// @return `f`'s result, or an empty `std::optional` of the same type.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto and_then(std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), *opt));
    }
    return U{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value. If `opt` holds no value, this function does not call
///        `f`.
///
///        `f` must return a `std::optional`. This function returns
///        `f`'s result when it calls `f`. This function returns an
///        empty `std::optional` of the same type when it does not
///        call `f`.
/// @param opt The optional to check.
/// @param f A callable. `f` must return a `std::optional`.
/// @return `f`'s result, or an empty `std::optional` of the same type.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto and_then(const std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), *opt));
    }
    return U{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value. If `opt` holds no value, this function does not call
///        `f`.
///
///        `f` must return a `std::optional`. This function returns
///        `f`'s result when it calls `f`. This function returns an
///        empty `std::optional` of the same type when it does not
///        call `f`.
/// @param opt The optional to check.
/// @param f A callable. `f` must return a `std::optional`.
/// @return `f`'s result, or an empty `std::optional` of the same type.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto and_then(std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), std::move(*opt)));
    }
    return U{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value. If `opt` holds no value, this function does not call
///        `f`.
///
///        `f` must return a `std::optional`. This function returns
///        `f`'s result when it calls `f`. This function returns an
///        empty `std::optional` of the same type when it does not
///        call `f`.
/// @param opt The optional to check.
/// @param f A callable. `f` must return a `std::optional`.
/// @return `f`'s result, or an empty `std::optional` of the same type.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto and_then(const std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&&>>;
    static_assert(is_optional_v<U>, "F must return a std::optional specialization");
    if (opt.has_value()) {
        return U(std::invoke(std::forward<F>(f), std::move(*opt)));
    }
    return U{};
}

/// @brief If `opt` holds a value, this function returns a copy of
///        `opt`. If `opt` holds no value, this function calls `f` and
///        returns `f`'s result.
///
///        `f` takes no arguments. `f`'s result must convert to
///        `std::optional<T>`.
/// @param opt The optional to check.
/// @param f A callable. `f` takes no arguments. `f`'s result must
///          convert to `std::optional<T>`.
/// @return A copy of `opt`, or `f`'s result.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr std::optional<T> or_else(const std::optional<T>& opt, F&& f) {
    static_assert(std::is_convertible_v<std::invoke_result_t<F>, std::optional<T>>,
                  "F must return something convertible to std::optional<T>");
    if (opt.has_value()) {
        return opt;
    }
    return std::forward<F>(f)();
}

/// @brief If `opt` holds a value, this function moves the value out
///        of `opt` and returns it. If `opt` holds no value, this
///        function calls `f` and returns `f`'s result.
///
///        `f` takes no arguments. `f`'s result must convert to
///        `std::optional<T>`.
/// @param opt The optional to check.
/// @param f A callable. `f` takes no arguments. `f`'s result must
///          convert to `std::optional<T>`.
/// @return `opt`'s value, moved, or `f`'s result.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr std::optional<T> or_else(std::optional<T>&& opt, F&& f) {
    static_assert(std::is_convertible_v<std::invoke_result_t<F>, std::optional<T>>,
                  "F must return something convertible to std::optional<T>");
    if (opt.has_value()) {
        return std::move(opt);
    }
    return std::forward<F>(f)();
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value and returns the result in a new `std::optional<U>`.
///        If `opt` holds no value, this function returns an empty
///        `std::optional<U>`.
///
///        `f` must not return `void`.
/// @param opt The optional to check.
/// @param f A callable. `f` must not return `void`.
/// @return `f`'s result in a new `std::optional`, or an empty one.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto transform(std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), *opt)};
    }
    return std::optional<U>{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value and returns the result in a new `std::optional<U>`.
///        If `opt` holds no value, this function returns an empty
///        `std::optional<U>`.
///
///        `f` must not return `void`.
/// @param opt The optional to check.
/// @param f A callable. `f` must not return `void`.
/// @return `f`'s result in a new `std::optional`, or an empty one.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto transform(const std::optional<T>& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), *opt)};
    }
    return std::optional<U>{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value and returns the result in a new `std::optional<U>`.
///        If `opt` holds no value, this function returns an empty
///        `std::optional<U>`.
///
///        `f` must not return `void`.
/// @param opt The optional to check.
/// @param f A callable. `f` must not return `void`.
/// @return `f`'s result in a new `std::optional`, or an empty one.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto transform(std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, T&&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), std::move(*opt))};
    }
    return std::optional<U>{};
}

/// @brief If `opt` holds a value, this function calls `f` with the
///        value and returns the result in a new `std::optional<U>`.
///        If `opt` holds no value, this function returns an empty
///        `std::optional<U>`.
///
///        `f` must not return `void`.
/// @param opt The optional to check.
/// @param f A callable. `f` must not return `void`.
/// @return `f`'s result in a new `std::optional`, or an empty one.
/// @see https://en.cppreference.com/w/cpp/utility/optional
template <class T, class F>
constexpr auto transform(const std::optional<T>&& opt, F&& f) {
    using U = std::decay_t<std::invoke_result_t<F, const T&&>>;
    static_assert(!std::is_same_v<U, void>, "F must not return void");
    if (opt.has_value()) {
        return std::optional<U>{std::invoke(std::forward<F>(f), std::move(*opt))};
    }
    return std::optional<U>{};
}

/// @brief This namespace promotes `and_then`, `or_else`, and `transform`
///        to `bridge::exports::truss::optional`.
namespace exports {
using bridge::detail::truss::cpp17::optional::and_then;
using bridge::detail::truss::cpp17::optional::or_else;
using bridge::detail::truss::cpp17::optional::transform;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::optional

/// @brief This is the Exports namespace for `optional`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
namespace bridge::exports::truss {
inline namespace cpp17 {
inline namespace optional {
using namespace bridge::detail::truss::cpp17::optional::exports;
} // namespace optional
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::and_then;
using bridge::exports::truss::or_else;
using bridge::exports::truss::transform;
} // namespace bridge::truss
