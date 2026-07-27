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
#include <functional>
#include <initializer_list>
#include <memory>
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
/// @see https://en.cppreference.com/w/cpp/utility/expected
struct unexpect_t {
    /// @brief Explicit so `unexpect_t{}` can't happen via copy-list-
    ///        initialization from `{}` in a context expecting some
    ///        other tag type.
    explicit unexpect_t() = default;
};
/// @brief The canonical `unexpect_t` instance, passed to select
///        `expected`'s in-place-error constructors. Matches
///        `std::unexpect`.
/// @see https://en.cppreference.com/w/cpp/utility/expected
inline constexpr unexpect_t unexpect{};

/// @brief Wraps an error value of type `E`, matching `std::unexpected`.
///        Constructed explicitly and passed to `expected`'s converting
///        constructors, or returned directly from a function reporting
///        failure.
/// @tparam E The error type. Must be a non-array, non-cv-qualified
///         object type, and must not itself be a specialization of
///         `unexpected`.
/// @see https://en.cppreference.com/w/cpp/utility/expected
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

    /// @brief Compares the wrapped errors for inequality.
    ///
    ///        `std::unexpected` only defines `operator==`, relying on
    ///        C++20's automatic `!=` rewriting from it -- unavailable
    ///        to code compiled as C++17, this header's floor, so this
    ///        polyfill defines `!=` explicitly to match usability
    ///        under passthrough (confirmed `!=` alone genuinely fails
    ///        to compile without this under `-std=c++17`, not
    ///        assumed).
    /// @param lhs The left-hand `unexpected`.
    /// @param rhs The right-hand `unexpected`, possibly of a different
    ///            error type.
    /// @return Whether `lhs.error() != rhs.error()`.
    template <class E2>
    friend constexpr bool operator!=(const unexpected& lhs, const unexpected<E2>& rhs) {
        return !(lhs == rhs);
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
/// @see https://en.cppreference.com/w/cpp/utility/expected
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

/// \cond BRIDGE_DETAIL
///
/// Layered-base implementation of expected<T,E>'s conditionally-deleted
/// special members (docs/adr/0010-expected-truss-owns-the-class.md's
/// "match deletion conditions" fidelity target). Each layer customizes
/// exactly one special member (copy ctor, move ctor, copy assign, move
/// assign) via a bool template parameter computed from the exact
/// std::expected Mandates/Constraints for that member (verified against
/// eel.is/c++draft's expected.object.cons/expected.object.assign, not
/// assumed from memory); every other special member in that layer is
/// `= default`, so deletion propagates transitively down the chain the
/// same way it does for any class with a deleted base special member.
/// Verified empirically (a standalone probe checking is_copy_assignable_v
/// etc. against hand-picked throwing-move/non-copyable types) before
/// wiring this into the real header, not assumed to work from the
/// pattern alone. Pure implementation plumbing, never part of the
/// public API -- excluded from the documentation-coverage gate here
/// rather than padded with one-line doc comments that carry no reader
/// value.
namespace layers {

/// @brief Tri-state discriminant for @ref storage's union: `empty` is a
///        transient state that only exists mid-construction/mid-reinit
///        (never observable after a constructor or assignment operator
///        returns normally), so that an exception during placement-new
///        leaves the union safely destructible instead of reading an
///        unconstructed member.
enum class state : unsigned char { empty, value, error };

/// @brief Owns the actual `T`/`E` union, the state discriminant, and
///        every placement-new/destroy primitive the layers above build
///        on. Never copied/moved/assigned directly -- always through
///        exactly one of the conditionally-enabled layers below, which
///        call these named helpers from hand-written bodies.
template <class T, class E>
struct storage {
    /// @brief Selects the tag constructor that leaves the union in the
    ///        transient `state::empty` state, for a derived layer to
    ///        placement-new into immediately after.
    struct empty_tag {};

    union expected_union {
        // Not constexpr: a union constructor that initializes no member
        // is only constexpr-legal from C++20 on (confirmed by Clang's
        // -Wc++20-extensions on this exact line before removing it, not
        // assumed) -- this header's floor is C++17, and nothing here
        // actually needs compile-time construction of the union itself.
        expected_union() noexcept {}
        ~expected_union() {}
        T val;
        E err;
    };

    state state_;
    expected_union storage_;

    explicit storage(empty_tag) noexcept : state_(state::empty) {}

    template <class... Args>
    explicit storage(std::in_place_t, Args&&... args) : state_(state::empty) {
        ::new (std::addressof(storage_.val)) T(std::forward<Args>(args)...);
        state_ = state::value;
    }
    template <class... Args>
    explicit storage(unexpect_t, Args&&... args) : state_(state::empty) {
        ::new (std::addressof(storage_.err)) E(std::forward<Args>(args)...);
        state_ = state::error;
    }

    storage() = delete;
    storage(const storage&) = delete;
    storage(storage&&) = delete;
    storage& operator=(const storage&) = delete;
    storage& operator=(storage&&) = delete;

    void destroy() noexcept {
        if (state_ == state::value) {
            storage_.val.~T();
        } else if (state_ == state::error) {
            storage_.err.~E();
        }
        state_ = state::empty;
    }
    ~storage() { destroy(); }

    void copy_construct_from(const storage& other) {
        if (other.state_ == state::value) {
            ::new (std::addressof(storage_.val)) T(other.storage_.val);
            state_ = state::value;
        } else if (other.state_ == state::error) {
            ::new (std::addressof(storage_.err)) E(other.storage_.err);
            state_ = state::error;
        }
    }
    void move_construct_from(storage&& other) {
        if (other.state_ == state::value) {
            ::new (std::addressof(storage_.val)) T(std::move(other.storage_.val));
            state_ = state::value;
        } else if (other.state_ == state::error) {
            ::new (std::addressof(storage_.err)) E(std::move(other.storage_.err));
            state_ = state::error;
        }
    }

    // docs/adr/0010's disclosed exception-safety simplification: destroy-
    // then-construct, not the standard's full two-stage "reinit-expected"
    // technique. When switching alternative and the *new* alternative's
    // constructor throws, *this is left with neither value nor error
    // active (state::empty persists past the call) -- a documented
    // precondition violation for every subsequent observer, unlike real
    // std::expected which guarantees this can never happen.
    void copy_assign_from(const storage& other) {
        if (state_ == other.state_) {
            if (state_ == state::value) {
                storage_.val = other.storage_.val;
            } else if (state_ == state::error) {
                storage_.err = other.storage_.err;
            }
        } else {
            destroy();
            copy_construct_from(other);
        }
    }
    void move_assign_from(storage&& other) {
        if (state_ == other.state_) {
            if (state_ == state::value) {
                storage_.val = std::move(other.storage_.val);
            } else if (state_ == state::error) {
                storage_.err = std::move(other.storage_.err);
            }
        } else {
            destroy();
            move_construct_from(std::move(other));
        }
    }
};

/// @brief Layer customizing the copy constructor: deleted unless
///        `is_copy_constructible_v<T> && is_copy_constructible_v<E>`.
template <class T, class E, bool = std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E>>
struct copy_ctor_layer : storage<T, E> {
    using storage<T, E>::storage;
    copy_ctor_layer() = delete;
    copy_ctor_layer(const copy_ctor_layer& other) : storage<T, E>(typename storage<T, E>::empty_tag{}) {
        this->copy_construct_from(other);
    }
    copy_ctor_layer(copy_ctor_layer&&) = default;
    copy_ctor_layer& operator=(const copy_ctor_layer&) = default;
    copy_ctor_layer& operator=(copy_ctor_layer&&) = default;
};
/// @copydoc copy_ctor_layer
template <class T, class E>
struct copy_ctor_layer<T, E, false> : storage<T, E> {
    using storage<T, E>::storage;
    copy_ctor_layer() = delete;
    copy_ctor_layer(const copy_ctor_layer&) = delete;
    copy_ctor_layer(copy_ctor_layer&&) = default;
    copy_ctor_layer& operator=(const copy_ctor_layer&) = default;
    copy_ctor_layer& operator=(copy_ctor_layer&&) = default;
};

/// @brief Layer customizing the move constructor: available whenever
///        `is_move_constructible_v<T> && is_move_constructible_v<E>`.
template <class T, class E, bool = std::is_move_constructible_v<T> && std::is_move_constructible_v<E>>
struct move_ctor_layer : copy_ctor_layer<T, E> {
    using copy_ctor_layer<T, E>::copy_ctor_layer;
    move_ctor_layer() = delete;
    move_ctor_layer(const move_ctor_layer&) = default;
    move_ctor_layer(move_ctor_layer&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_move_constructible_v<E>)
        : copy_ctor_layer<T, E>(typename storage<T, E>::empty_tag{}) {
        this->move_construct_from(std::move(other));
    }
    move_ctor_layer& operator=(const move_ctor_layer&) = default;
    move_ctor_layer& operator=(move_ctor_layer&&) = default;
};
/// @copydoc move_ctor_layer
template <class T, class E>
struct move_ctor_layer<T, E, false> : copy_ctor_layer<T, E> {
    using copy_ctor_layer<T, E>::copy_ctor_layer;
    move_ctor_layer() = delete;
    move_ctor_layer(const move_ctor_layer&) = default;
    move_ctor_layer(move_ctor_layer&&) = delete;
    move_ctor_layer& operator=(const move_ctor_layer&) = default;
    move_ctor_layer& operator=(move_ctor_layer&&) = default;
};

/// @brief Layer customizing the copy-assignment operator: deleted
///        unless `T` and `E` are both copy-constructible and
///        copy-assignable, *and* at least one of them is
///        nothrow-move-constructible (the exact condition
///        `std::expected` uses to guarantee reassignment across a
///        changed alternative can't leave the object valueless --
///        docs/adr/0010's fidelity target).
template <class T, class E,
          bool = std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<E> &&
                 std::is_copy_assignable_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>)>
struct copy_assign_layer : move_ctor_layer<T, E> {
    using move_ctor_layer<T, E>::move_ctor_layer;
    copy_assign_layer() = delete;
    copy_assign_layer(const copy_assign_layer&) = default;
    copy_assign_layer(copy_assign_layer&&) = default;
    copy_assign_layer& operator=(const copy_assign_layer& other) {
        this->copy_assign_from(other);
        return *this;
    }
    copy_assign_layer& operator=(copy_assign_layer&&) = default;
};
/// @copydoc copy_assign_layer
template <class T, class E>
struct copy_assign_layer<T, E, false> : move_ctor_layer<T, E> {
    using move_ctor_layer<T, E>::move_ctor_layer;
    copy_assign_layer() = delete;
    copy_assign_layer(const copy_assign_layer&) = default;
    copy_assign_layer(copy_assign_layer&&) = default;
    copy_assign_layer& operator=(const copy_assign_layer&) = delete;
    copy_assign_layer& operator=(copy_assign_layer&&) = default;
};

/// @brief Layer customizing the move-assignment operator: same shape
///        of condition as @ref copy_assign_layer, with move-
///        constructible/assignable in place of copy-constructible/
///        assignable.
template <class T, class E,
          bool = std::is_move_constructible_v<T> && std::is_move_assignable_v<T> && std::is_move_constructible_v<E> &&
                 std::is_move_assignable_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>)>
struct move_assign_layer : copy_assign_layer<T, E> {
    using copy_assign_layer<T, E>::copy_assign_layer;
    move_assign_layer() = delete;
    move_assign_layer(const move_assign_layer&) = default;
    move_assign_layer(move_assign_layer&&) = default;
    move_assign_layer& operator=(const move_assign_layer&) = default;
    move_assign_layer& operator=(move_assign_layer&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_move_assignable_v<T>&&
            std::is_nothrow_move_constructible_v<E>&& std::is_nothrow_move_assignable_v<E>) {
        this->move_assign_from(std::move(other));
        return *this;
    }
};
/// @copydoc move_assign_layer
template <class T, class E>
struct move_assign_layer<T, E, false> : copy_assign_layer<T, E> {
    using copy_assign_layer<T, E>::copy_assign_layer;
    move_assign_layer() = delete;
    move_assign_layer(const move_assign_layer&) = default;
    move_assign_layer(move_assign_layer&&) = default;
    move_assign_layer& operator=(const move_assign_layer&) = default;
    move_assign_layer& operator=(move_assign_layer&&) = delete;
};

/// @brief Storage for the `expected<void,E>` partial specialization:
///        same tri-state discriminant as @ref storage, but there's
///        never a `T` alternative to hold -- `state::value` carries no
///        payload at all.
template <class E>
struct void_storage {
    struct empty_tag {};
    union expected_union {
        expected_union() noexcept {}
        ~expected_union() {}
        E err;
    };

    state state_;
    expected_union storage_;

    explicit void_storage(empty_tag) noexcept : state_(state::empty) {}
    explicit void_storage(std::in_place_t) noexcept : state_(state::value) {}
    template <class... Args>
    explicit void_storage(unexpect_t, Args&&... args) : state_(state::empty) {
        ::new (std::addressof(storage_.err)) E(std::forward<Args>(args)...);
        state_ = state::error;
    }

    void_storage() = delete;
    void_storage(const void_storage&) = delete;
    void_storage(void_storage&&) = delete;
    void_storage& operator=(const void_storage&) = delete;
    void_storage& operator=(void_storage&&) = delete;

    void destroy() noexcept {
        if (state_ == state::error) {
            storage_.err.~E();
        }
        state_ = state::empty;
    }
    ~void_storage() { destroy(); }

    void copy_construct_from(const void_storage& other) {
        if (other.state_ == state::value) {
            state_ = state::value;
        } else if (other.state_ == state::error) {
            ::new (std::addressof(storage_.err)) E(other.storage_.err);
            state_ = state::error;
        }
    }
    void move_construct_from(void_storage&& other) {
        if (other.state_ == state::value) {
            state_ = state::value;
        } else if (other.state_ == state::error) {
            ::new (std::addressof(storage_.err)) E(std::move(other.storage_.err));
            state_ = state::error;
        }
    }
    // Same decision #4 destroy-then-construct simplification as storage.
    void copy_assign_from(const void_storage& other) {
        if (state_ == other.state_) {
            if (state_ == state::error) {
                storage_.err = other.storage_.err;
            }
        } else {
            destroy();
            copy_construct_from(other);
        }
    }
    void move_assign_from(void_storage&& other) {
        if (state_ == other.state_) {
            if (state_ == state::error) {
                storage_.err = std::move(other.storage_.err);
            }
        } else {
            destroy();
            move_construct_from(std::move(other));
        }
    }
};

/// @brief Layer customizing the copy constructor for `expected<void,E>`:
///        deleted unless `is_copy_constructible_v<E>` (no `T` condition
///        -- there's no value alternative to copy).
template <class E, bool = std::is_copy_constructible_v<E>>
struct void_copy_ctor_layer : void_storage<E> {
    using void_storage<E>::void_storage;
    void_copy_ctor_layer() = delete;
    void_copy_ctor_layer(const void_copy_ctor_layer& other) : void_storage<E>(typename void_storage<E>::empty_tag{}) {
        this->copy_construct_from(other);
    }
    void_copy_ctor_layer(void_copy_ctor_layer&&) = default;
    void_copy_ctor_layer& operator=(const void_copy_ctor_layer&) = default;
    void_copy_ctor_layer& operator=(void_copy_ctor_layer&&) = default;
};
/// @copydoc void_copy_ctor_layer
template <class E>
struct void_copy_ctor_layer<E, false> : void_storage<E> {
    using void_storage<E>::void_storage;
    void_copy_ctor_layer() = delete;
    void_copy_ctor_layer(const void_copy_ctor_layer&) = delete;
    void_copy_ctor_layer(void_copy_ctor_layer&&) = default;
    void_copy_ctor_layer& operator=(const void_copy_ctor_layer&) = default;
    void_copy_ctor_layer& operator=(void_copy_ctor_layer&&) = default;
};

/// @brief Layer customizing the move constructor for `expected<void,E>`.
template <class E, bool = std::is_move_constructible_v<E>>
struct void_move_ctor_layer : void_copy_ctor_layer<E> {
    using void_copy_ctor_layer<E>::void_copy_ctor_layer;
    void_move_ctor_layer() = delete;
    void_move_ctor_layer(const void_move_ctor_layer&) = default;
    void_move_ctor_layer(void_move_ctor_layer&& other) noexcept(std::is_nothrow_move_constructible_v<E>)
        : void_copy_ctor_layer<E>(typename void_storage<E>::empty_tag{}) {
        this->move_construct_from(std::move(other));
    }
    void_move_ctor_layer& operator=(const void_move_ctor_layer&) = default;
    void_move_ctor_layer& operator=(void_move_ctor_layer&&) = default;
};
/// @copydoc void_move_ctor_layer
template <class E>
struct void_move_ctor_layer<E, false> : void_copy_ctor_layer<E> {
    using void_copy_ctor_layer<E>::void_copy_ctor_layer;
    void_move_ctor_layer() = delete;
    void_move_ctor_layer(const void_move_ctor_layer&) = default;
    void_move_ctor_layer(void_move_ctor_layer&&) = delete;
    void_move_ctor_layer& operator=(const void_move_ctor_layer&) = default;
    void_move_ctor_layer& operator=(void_move_ctor_layer&&) = default;
};

/// @brief Layer customizing the copy-assignment operator for
///        `expected<void,E>`: deleted unless `E` is copy-constructible,
///        copy-assignable, and nothrow-move-constructible (no `T`
///        alternative means no either-side-nothrow OR condition -- `E`
///        alone must be nothrow-move-constructible).
template <class E, bool = std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E> &&
                           std::is_nothrow_move_constructible_v<E>>
struct void_copy_assign_layer : void_move_ctor_layer<E> {
    using void_move_ctor_layer<E>::void_move_ctor_layer;
    void_copy_assign_layer() = delete;
    void_copy_assign_layer(const void_copy_assign_layer&) = default;
    void_copy_assign_layer(void_copy_assign_layer&&) = default;
    void_copy_assign_layer& operator=(const void_copy_assign_layer& other) {
        this->copy_assign_from(other);
        return *this;
    }
    void_copy_assign_layer& operator=(void_copy_assign_layer&&) = default;
};
/// @copydoc void_copy_assign_layer
template <class E>
struct void_copy_assign_layer<E, false> : void_move_ctor_layer<E> {
    using void_move_ctor_layer<E>::void_move_ctor_layer;
    void_copy_assign_layer() = delete;
    void_copy_assign_layer(const void_copy_assign_layer&) = default;
    void_copy_assign_layer(void_copy_assign_layer&&) = default;
    void_copy_assign_layer& operator=(const void_copy_assign_layer&) = delete;
    void_copy_assign_layer& operator=(void_copy_assign_layer&&) = default;
};

/// @brief Layer customizing the move-assignment operator for
///        `expected<void,E>`.
template <class E, bool = std::is_move_constructible_v<E> && std::is_move_assignable_v<E> &&
                           std::is_nothrow_move_constructible_v<E>>
struct void_move_assign_layer : void_copy_assign_layer<E> {
    using void_copy_assign_layer<E>::void_copy_assign_layer;
    void_move_assign_layer() = delete;
    void_move_assign_layer(const void_move_assign_layer&) = default;
    void_move_assign_layer(void_move_assign_layer&&) = default;
    void_move_assign_layer& operator=(const void_move_assign_layer&) = default;
    void_move_assign_layer& operator=(void_move_assign_layer&& other) noexcept(
        std::is_nothrow_move_constructible_v<E>&& std::is_nothrow_move_assignable_v<E>) {
        this->move_assign_from(std::move(other));
        return *this;
    }
};
/// @copydoc void_move_assign_layer
template <class E>
struct void_move_assign_layer<E, false> : void_copy_assign_layer<E> {
    using void_copy_assign_layer<E>::void_copy_assign_layer;
    void_move_assign_layer() = delete;
    void_move_assign_layer(const void_move_assign_layer&) = default;
    void_move_assign_layer(void_move_assign_layer&&) = default;
    void_move_assign_layer& operator=(const void_move_assign_layer&) = default;
    void_move_assign_layer& operator=(void_move_assign_layer&&) = delete;
};

} // namespace layers
/// \endcond

/// @brief True for `expected<U,G>` for any `U`/`G`, false otherwise.
///        Used by the monadic operations (added in a follow-up commit)
///        to constrain "F must return a specialization of expected",
///        mirroring `truss/cpp17/optional.hpp`'s `is_optional`.
template <class T>
struct is_expected : std::false_type {};

template <class T, class E>
class expected;

/// @copydoc is_expected
template <class U, class G>
struct is_expected<expected<U, G>> : std::true_type {};
/// @brief Convenience value for @ref is_expected.
template <class T>
inline constexpr bool is_expected_v = is_expected<T>::value;

/// @brief Truss's polyfilled `expected<T,E>`, matching C++23's
///        `std::expected<T,E>` for standards that predate it. See the
///        file-level docs and docs/adr/0010-expected-truss-owns-the-
///        class.md for the fidelity scope this implements: deletion
///        conditions on the special members are matched exactly;
///        conditional triviality and the standard's full two-stage
///        exception-safe reassignment are explicitly out of scope.
///
///        The converting constructors that accept `unexpected<G>`, a
///        raw value `U`, or another `expected<U,G>` are unconditionally
///        `explicit` here rather than conditionally explicit based on
///        convertibility (a C++20 `explicit(bool)` feature unavailable
///        on this header's C++17 floor). This is a strictly more
///        conservative surface than `std::expected` -- it only ever
///        refuses an implicit conversion `std::expected` would allow,
///        never the reverse -- so it fails loudly at the polyfill's own
///        compile time rather than silently diverging once passthrough
///        activates, the one direction docs/adr/0010 actually guards
///        against.
/// @tparam T The value type. Must not be a reference, `void` (see the
///         `expected<void,E>` partial specialization instead), an
///         array, `in_place_t`, `unexpect_t`, or a specialization of
///         `unexpected`.
/// @tparam E The error type, wrapped internally in `unexpected<E>`
///         wherever an error is stored or reported.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class T, class E>
class expected : private layers::move_assign_layer<T, E> {
    static_assert(!std::is_reference_v<T>, "expected<T,E>: T must not be a reference type");
    static_assert(!std::is_function_v<T>, "expected<T,E>: T must not be a function type");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>,
                  "expected<T,E>: T must not be std::in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "expected<T,E>: T must not be unexpect_t");
    static_assert(!is_unexpected_v<std::remove_cv_t<T>>,
                  "expected<T,E>: T must not be a specialization of unexpected");

    using base = layers::move_assign_layer<T, E>;

public:
    /// @brief The wrapped value type.
    using value_type = T;
    /// @brief The wrapped error type.
    using error_type = E;
    /// @brief The `unexpected` specialization matching this `expected`.
    using unexpected_type = unexpected<E>;
    /// @brief `expected<U,E>` -- the same error type, a different value
    ///        type, matching `std::expected::rebind`.
    template <class U>
    using rebind = expected<U, error_type>;

    // Deliberately not `using base::base;`: base's default constructor is
    // explicitly deleted at every layer (so deletion propagates when T
    // isn't default-constructible), and inheriting it verbatim alongside
    // this class's own templated default constructor below produces a
    // genuine ambiguity/deletion conflict between the two -- confirmed by
    // hitting exactly that compile error before switching to explicit
    // forwarding constructors, not assumed. Only the two tag constructors
    // actually used by expected's own public API are forwarded below.

    /// @brief Default-constructs the contained value. Only participates
    ///        in overload resolution when `T` is default-constructible.
    template <class T2 = T, class = std::enable_if_t<std::is_default_constructible_v<T2>>>
    constexpr expected() : base(std::in_place) {}

    /// @brief Constructs a value in place from `args`.
    /// @param args Forwarded to `T`'s constructor.
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    constexpr explicit expected(std::in_place_t, Args&&... args) : base(std::in_place, std::forward<Args>(args)...) {}
    /// @brief Constructs a value in place from an initializer list plus
    ///        `args`.
    /// @param il Forwarded to `T`'s constructor as the first argument.
    /// @param args Forwarded to `T`'s constructor after `il`.
    template <class U, class... Args,
              class = std::enable_if_t<std::is_constructible_v<T, std::initializer_list<U>&, Args...>>>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
        : base(std::in_place, il, std::forward<Args>(args)...) {}

    /// @brief Constructs an error in place from `args`.
    /// @param args Forwarded to `E`'s constructor.
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<E, Args...>>>
    constexpr explicit expected(unexpect_t, Args&&... args) : base(unexpect_t{}, std::forward<Args>(args)...) {}
    /// @brief Constructs an error in place from an initializer list plus
    ///        `args`.
    /// @param il Forwarded to `E`'s constructor as the first argument.
    /// @param args Forwarded to `E`'s constructor after `il`.
    template <class U, class... Args,
              class = std::enable_if_t<std::is_constructible_v<E, std::initializer_list<U>&, Args...>>>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
        : base(unexpect_t{}, il, std::forward<Args>(args)...) {}

    /// @brief Copies `T` or `E`, matching whichever `other` holds.
    ///        Deleted unless both `T` and `E` are copy-constructible.
    expected(const expected&) = default;
    /// @brief Moves `T` or `E`, matching whichever `other` holds.
    ///        Deleted unless both `T` and `E` are move-constructible.
    expected(expected&&) = default;

    /// @brief Constructs an error from `u`'s wrapped error.
    /// @param u The `unexpected` to construct the error from.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, const G&>>>
    constexpr explicit expected(const unexpected<G>& u) : base(unexpect_t{}, u.error()) {}
    /// @brief Constructs an error by moving `u`'s wrapped error.
    /// @param u The `unexpected` to construct the error from.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, G&&>>>
    constexpr explicit expected(unexpected<G>&& u) : base(unexpect_t{}, std::move(u).error()) {}

    /// @brief Converting constructor from an `expected<U,G>` holding a
    ///        value or error convertible to `T`/`E`. A documented,
    ///        simplified subset of the standard's Constraints: matches
    ///        `is_constructible_v<T, const U&>`/`is_constructible_v<E,
    ///        const G&>`, but not the additional defensive guards the
    ///        standard adds against ambiguity with `T = bool` or a
    ///        source constructible from `expected<U,G>` itself --
    ///        omitted here as out of scope, same spirit as the
    ///        exception-safety and triviality trims in docs/adr/0010.
    /// @param other The `expected` to convert from.
    template <class U, class G,
              class = std::enable_if_t<std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&>>>
    explicit expected(const expected<U, G>& other) : base(typename layers::storage<T, E>::empty_tag{}) {
        if (other.has_value()) {
            ::new (std::addressof(this->storage_.val)) T(*other);
            this->state_ = layers::state::value;
        } else {
            ::new (std::addressof(this->storage_.err)) E(other.error());
            this->state_ = layers::state::error;
        }
    }
    /// @copydoc expected(const expected<U,G>&)
    template <class U, class G,
              class = std::enable_if_t<std::is_constructible_v<T, U&&> && std::is_constructible_v<E, G&&>>>
    explicit expected(expected<U, G>&& other) : base(typename layers::storage<T, E>::empty_tag{}) {
        if (other.has_value()) {
            ::new (std::addressof(this->storage_.val)) T(std::move(*other));
            this->state_ = layers::state::value;
        } else {
            ::new (std::addressof(this->storage_.err)) E(std::move(other).error());
            this->state_ = layers::state::error;
        }
    }

    /// @brief Constructs a value from `v`.
    /// @param v The value to construct `T` from.
    template <class U = T,
              class = std::enable_if_t<!std::is_same_v<remove_cvref_t<U>, expected> &&
                                        !std::is_same_v<remove_cvref_t<U>, std::in_place_t> &&
                                        !is_unexpected_v<remove_cvref_t<U>> && std::is_constructible_v<T, U>>>
    constexpr explicit expected(U&& v) : base(std::in_place, std::forward<U>(v)) {}

    /// @brief Copy-assigns whichever alternative `other` holds. Deleted
    ///        unless both `T` and `E` are copy-constructible and
    ///        copy-assignable, and at least one is
    ///        nothrow-move-constructible.
    /// @return `*this`.
    expected& operator=(const expected&) = default;
    /// @brief Move-assigns whichever alternative `other` holds. Deleted
    ///        unless both `T` and `E` are move-constructible and
    ///        move-assignable, and at least one is
    ///        nothrow-move-constructible.
    /// @return `*this`.
    expected& operator=(expected&&) = default;

    /// @brief Assigns a value constructed from `v`, replacing whatever
    ///        this held before.
    /// @param v The value to assign from.
    /// @return `*this`.
    template <class U = T,
              class = std::enable_if_t<!std::is_same_v<remove_cvref_t<U>, expected> && !is_unexpected_v<remove_cvref_t<U>> &&
                                        std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>>>
    constexpr expected& operator=(U&& v) {
        if (has_value()) {
            **this = std::forward<U>(v);
        } else {
            this->destroy();
            ::new (std::addressof(this->storage_.val)) T(std::forward<U>(v));
            this->state_ = layers::state::value;
        }
        return *this;
    }

    /// @brief Assigns an error constructed from `u`'s wrapped error,
    ///        replacing whatever this held before.
    /// @param u The `unexpected` to assign the error from.
    /// @return `*this`.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>>>
    constexpr expected& operator=(const unexpected<G>& u) {
        if (!has_value()) {
            error() = u.error();
        } else {
            this->destroy();
            ::new (std::addressof(this->storage_.err)) E(u.error());
            this->state_ = layers::state::error;
        }
        return *this;
    }
    /// @brief Assigns an error by moving `u`'s wrapped error, replacing
    ///        whatever this held before.
    /// @param u The `unexpected` to assign the error from.
    /// @return `*this`.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, G&&> && std::is_assignable_v<E&, G&&>>>
    constexpr expected& operator=(unexpected<G>&& u) {
        if (!has_value()) {
            error() = std::move(u).error();
        } else {
            this->destroy();
            ::new (std::addressof(this->storage_.err)) E(std::move(u).error());
            this->state_ = layers::state::error;
        }
        return *this;
    }

    /// @brief Whether this holds a value (as opposed to an error).
    /// @return `true` if this holds a value.
    constexpr bool has_value() const noexcept { return this->state_ == layers::state::value; }
    /// @copydoc has_value
    constexpr explicit operator bool() const noexcept { return has_value(); }

    /// @brief Lvalue access to the contained value. Precondition:
    ///        `has_value()`; violating it is undefined behavior,
    ///        matching `std::expected`.
    /// @return A pointer to the contained value.
    constexpr T* operator->() { return std::addressof(this->storage_.val); }
    /// @copydoc operator->()
    constexpr const T* operator->() const { return std::addressof(this->storage_.val); }

    /// @copydoc operator->()
    /// @return A reference to the contained value.
    constexpr T& operator*() & { return this->storage_.val; }
    /// @copydoc operator*()&
    constexpr const T& operator*() const& { return this->storage_.val; }
    /// @copydoc operator*()&
    constexpr T&& operator*() && { return std::move(this->storage_.val); }
    /// @copydoc operator*()&
    constexpr const T&& operator*() const&& { return std::move(this->storage_.val); }

    /// @brief Lvalue access to the contained value, checked.
    /// @return A reference to the contained value.
    /// @throws bad_expected_access<E> if `!has_value()`, constructed
    ///         from a copy of the contained error.
    constexpr T& value() & {
        if (!has_value()) throw bad_expected_access<E>(error());
        return this->storage_.val;
    }
    /// @copydoc value()&
    constexpr const T& value() const& {
        if (!has_value()) throw bad_expected_access<E>(error());
        return this->storage_.val;
    }
    /// @copydoc value()&
    constexpr T&& value() && {
        if (!has_value()) throw bad_expected_access<E>(error());
        return std::move(this->storage_.val);
    }
    /// @copydoc value()&
    constexpr const T&& value() const&& {
        if (!has_value()) throw bad_expected_access<E>(error());
        return std::move(this->storage_.val);
    }

    /// @brief Lvalue access to the contained error. Precondition:
    ///        `!has_value()`; violating it is undefined behavior,
    ///        matching `std::expected` (this is *not* the throwing
    ///        accessor -- see @ref value()).
    /// @return A reference to the contained error.
    constexpr E& error() & { return this->storage_.err; }
    /// @copydoc error()&
    constexpr const E& error() const& { return this->storage_.err; }
    /// @copydoc error()&
    constexpr E&& error() && { return std::move(this->storage_.err); }
    /// @copydoc error()&
    constexpr const E&& error() const&& { return std::move(this->storage_.err); }

    /// @brief The contained value, or `v` converted to `T` if this
    ///        holds an error.
    /// @param v The fallback value.
    /// @return A copy of the contained value, or `v` converted to `T`.
    template <class U>
    constexpr T value_or(U&& v) const& {
        return has_value() ? **this : static_cast<T>(std::forward<U>(v));
    }
    /// @copydoc value_or(U&&)const&
    template <class U>
    constexpr T value_or(U&& v) && {
        return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(v));
    }

    /// @brief The contained error, or `g` converted to `E` if this
    ///        holds a value.
    ///
    ///        Not part of the original `std::expected` proposal
    ///        (P0323R12 explicitly excluded it), but present under the
    ///        same `__cpp_lib_expected` value as the rest of the type
    ///        on every ecosystem this project's compiler matrix covers
    ///        -- confirmed by direct compile probe before relying on
    ///        it, not assumed. See docs/adr/0010-expected-truss-owns-the-class.md.
    /// @param g The fallback error.
    /// @return A copy of the contained error, or `g` converted to `E`.
    template <class G>
    constexpr E error_or(G&& g) const& {
        return has_value() ? static_cast<E>(std::forward<G>(g)) : error();
    }
    /// @copydoc error_or(G&&)const&
    template <class G>
    constexpr E error_or(G&& g) && {
        return has_value() ? static_cast<E>(std::forward<G>(g)) : std::move(error());
    }

    /// @brief If this holds a value, invoke `f` with it and return the
    ///        result (which must itself be a specialization of
    ///        `expected` with a matching `error_type`); otherwise
    ///        return an error copy of that same type.
    /// @param f A callable returning a specialization of `expected`.
    /// @return `f`'s result, or an error copy of its `expected` type.
    template <class F>
    constexpr auto and_then(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f), **this);
        }
        return U(unexpect_t{}, error());
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f), **this);
        }
        return U(unexpect_t{}, error());
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f), std::move(**this));
        }
        return U(unexpect_t{}, std::move(error()));
    }
    /// @copydoc and_then(F&&)&
    template <class F>
    constexpr auto and_then(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f), std::move(**this));
        }
        return U(unexpect_t{}, std::move(error()));
    }

    /// @brief If this holds an error, invoke `f` with it and return the
    ///        result (which must itself be a specialization of
    ///        `expected` with a matching `value_type`); otherwise
    ///        return a value copy of that same type.
    /// @param f A callable returning a specialization of `expected`.
    /// @return `f`'s result, or a value copy of its `expected` type.
    template <class F>
    constexpr auto or_else(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::value_type, T>, "F's expected<T, ...> must use this expected's T");
        if (has_value()) {
            return U(std::in_place, **this);
        }
        return std::invoke(std::forward<F>(f), error());
    }
    /// @copydoc or_else(F&&)&
    template <class F>
    constexpr auto or_else(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::value_type, T>, "F's expected<T, ...> must use this expected's T");
        if (has_value()) {
            return U(std::in_place, **this);
        }
        return std::invoke(std::forward<F>(f), error());
    }
    /// @copydoc or_else(F&&)&
    template <class F>
    constexpr auto or_else(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::value_type, T>, "F's expected<T, ...> must use this expected's T");
        if (has_value()) {
            return U(std::in_place, std::move(**this));
        }
        return std::invoke(std::forward<F>(f), std::move(error()));
    }
    /// @copydoc or_else(F&&)&
    template <class F>
    constexpr auto or_else(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::value_type, T>, "F's expected<T, ...> must use this expected's T");
        if (has_value()) {
            return U(std::in_place, std::move(**this));
        }
        return std::invoke(std::forward<F>(f), std::move(error()));
    }

    /// @brief If this holds a value, invoke `f` with it and return
    ///        `expected<U,E>` containing the result (or `expected<void,E>`
    ///        if `f` returns `void`, matching `std::expected` -- initially
    ///        missing here, fixed once `expected<void,E>` existed to
    ///        chain to); otherwise return an error copy.
    /// @param f A callable, invoked with the contained value.
    /// @return `f`'s result wrapped in `expected<U,E>`, or an error copy.
    template <class F>
    constexpr auto transform(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f), **this);
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, error());
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f), **this));
            }
            return expected<U, E>(unexpect_t{}, error());
        }
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f), **this);
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, error());
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f), **this));
            }
            return expected<U, E>(unexpect_t{}, error());
        }
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f), std::move(**this));
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, std::move(error()));
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f), std::move(**this)));
            }
            return expected<U, E>(unexpect_t{}, std::move(error()));
        }
    }
    /// @copydoc transform(F&&)&
    template <class F>
    constexpr auto transform(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f), std::move(**this));
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, std::move(error()));
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f), std::move(**this)));
            }
            return expected<U, E>(unexpect_t{}, std::move(error()));
        }
    }

    /// @brief If this holds an error, invoke `f` with it and return
    ///        `expected<T,G>` containing the result wrapped in
    ///        `unexpected`; otherwise return a value copy of
    ///        `expected<T,G>`.
    /// @param f A callable returning a non-`void` value.
    /// @return `f`'s result wrapped in `expected<T,G>`'s error, or a
    ///         value copy.
    template <class F>
    constexpr auto transform_error(F&& f) & {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<T, G>(std::in_place, **this);
        }
        return expected<T, G>(unexpect_t{}, std::invoke(std::forward<F>(f), error()));
    }
    /// @copydoc transform_error(F&&)&
    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<T, G>(std::in_place, **this);
        }
        return expected<T, G>(unexpect_t{}, std::invoke(std::forward<F>(f), error()));
    }
    /// @copydoc transform_error(F&&)&
    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<T, G>(std::in_place, std::move(**this));
        }
        return expected<T, G>(unexpect_t{}, std::invoke(std::forward<F>(f), std::move(error())));
    }
    /// @copydoc transform_error(F&&)&
    template <class F>
    constexpr auto transform_error(F&& f) const&& {
        using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<T, G>(std::in_place, std::move(**this));
        }
        return expected<T, G>(unexpect_t{}, std::invoke(std::forward<F>(f), std::move(error())));
    }

    /// @brief Destroys whatever this held and constructs a new value
    ///        in place from `args`. Constrained to
    ///        `is_nothrow_constructible_v<T, Args...>`, matching
    ///        `std::expected` exactly.
    ///
    ///        Destroying the old alternative before constructing the
    ///        new one is only exception-safe when the new construction
    ///        can't throw, which is exactly what this constraint
    ///        guarantees -- confirmed necessary by hitting a real
    ///        compile error against the real type without it (this
    ///        polyfill was, incorrectly, unconditionally permissive
    ///        here), not assumed.
    /// @param args Forwarded to `T`'s constructor.
    /// @return A reference to the newly-constructed value.
    template <class... Args, class = std::enable_if_t<std::is_nothrow_constructible_v<T, Args...>>>
    constexpr T& emplace(Args&&... args) noexcept {
        this->destroy();
        ::new (std::addressof(this->storage_.val)) T(std::forward<Args>(args)...);
        this->state_ = layers::state::value;
        return this->storage_.val;
    }

    /// @brief Compares two `expected`s: equal if both hold a value and
    ///        those values compare equal, or both hold an error and
    ///        those errors compare equal; never equal if one holds a
    ///        value and the other an error.
    /// @param x The left-hand `expected`.
    /// @param y The right-hand `expected`, possibly with different
    ///           value/error types.
    /// @return Whether `x` and `y` are equal, as described above.
    template <class T2, class E2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value()) return false;
        if (x.has_value()) return static_cast<bool>(*x == *y);
        return static_cast<bool>(x.error() == y.error());
    }
    /// @brief `std::expected` relies on C++20's automatic `!=` rewriting
    ///        from `operator==`, unavailable under this header's C++17
    ///        floor -- see the `unexpected::operator!=` comment for why
    ///        this is defined explicitly here and throughout.
    /// @param x The left-hand `expected`.
    /// @param y The right-hand `expected`, possibly with different
    ///           value/error types.
    /// @return Whether `x` and `y` are unequal.
    template <class T2, class E2>
    friend constexpr bool operator!=(const expected& x, const expected<T2, E2>& y) {
        return !(x == y);
    }

    /// @brief Compares against a raw value: equal only when `x` holds a
    ///        value and it compares equal to `v`.
    /// @param x The `expected`.
    /// @param v The value to compare against.
    /// @return Whether `x` holds a value equal to `v`.
    template <class U, class = std::enable_if_t<!is_expected_v<std::decay_t<U>> && !is_unexpected_v<std::decay_t<U>>>>
    friend constexpr auto operator==(const expected& x, const U& v) -> decltype(std::declval<const T&>() == v, bool{}) {
        return x.has_value() && static_cast<bool>(*x == v);
    }
    /// @copydoc operator==(const expected&,const U&)
    template <class U, class = std::enable_if_t<!is_expected_v<std::decay_t<U>> && !is_unexpected_v<std::decay_t<U>>>>
    friend constexpr auto operator==(const U& v, const expected& x) -> decltype(std::declval<const T&>() == v, bool{}) {
        return x == v;
    }
    /// @brief Compares against a raw value for inequality (see the
    ///        `unexpected::operator!=` comment for why this is defined
    ///        explicitly rather than relying on C++20 rewriting).
    /// @param x The `expected`.
    /// @param v The value to compare against.
    /// @return Whether `x` does not hold a value equal to `v`.
    template <class U, class = std::enable_if_t<!is_expected_v<std::decay_t<U>> && !is_unexpected_v<std::decay_t<U>>>>
    friend constexpr auto operator!=(const expected& x, const U& v) -> decltype(std::declval<const T&>() == v, bool{}) {
        return !(x == v);
    }
    /// @copydoc operator!=(const expected&,const U&)
    template <class U, class = std::enable_if_t<!is_expected_v<std::decay_t<U>> && !is_unexpected_v<std::decay_t<U>>>>
    friend constexpr auto operator!=(const U& v, const expected& x) -> decltype(std::declval<const T&>() == v, bool{}) {
        return !(x == v);
    }

    /// @brief Compares against an `unexpected<G>`: equal only when `x`
    ///        holds an error and it compares equal to `e`'s.
    /// @param x The `expected`.
    /// @param e The `unexpected` to compare against.
    /// @return Whether `x` holds an error equal to `e.error()`.
    template <class G>
    friend constexpr bool operator==(const expected& x, const unexpected<G>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }
    /// @copydoc operator==(const expected&,const unexpected<G>&)
    template <class G>
    friend constexpr bool operator==(const unexpected<G>& e, const expected& x) {
        return x == e;
    }
    /// @brief Compares against an `unexpected<G>` for inequality (see
    ///        the `unexpected::operator!=` comment for why this is
    ///        defined explicitly rather than relying on C++20
    ///        rewriting).
    /// @param x The `expected`.
    /// @param e The `unexpected` to compare against.
    /// @return Whether `x` does not hold an error equal to `e.error()`.
    template <class G>
    friend constexpr bool operator!=(const expected& x, const unexpected<G>& e) {
        return !(x == e);
    }
    /// @copydoc operator!=(const expected&,const unexpected<G>&)
    template <class G>
    friend constexpr bool operator!=(const unexpected<G>& e, const expected& x) {
        return !(x == e);
    }

    /// @brief Swaps the contents of `*this` and `other`, including
    ///        across differing alternatives (a value swapped with an
    ///        error).
    /// @param other The `expected` to swap with.
    void swap(expected& other) noexcept(
        std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_swappable_v<T>&&
            std::is_nothrow_move_constructible_v<E>&& std::is_nothrow_swappable_v<E>) {
        using std::swap;
        if (has_value() && other.has_value()) {
            swap(**this, *other);
        } else if (!has_value() && !other.has_value()) {
            swap(error(), other.error());
        } else {
            // Same-class private access applies to any object of this
            // class, not just `this` -- with_value/with_error name the
            // same two objects as *this/other, so reaching into their
            // inherited-as-private storage_/state_/destroy() here needs
            // no friend declaration.
            expected& with_value = has_value() ? *this : other;
            expected& with_error = has_value() ? other : *this;
            E moved_error(std::move(with_error.error()));
            with_error.destroy();
            ::new (std::addressof(with_error.storage_.val)) T(std::move(*with_value));
            with_error.state_ = layers::state::value;
            with_value.destroy();
            ::new (std::addressof(with_value.storage_.err)) E(std::move(moved_error));
            with_value.state_ = layers::state::error;
        }
    }
};

/// @brief Partial specialization of `expected` for `T = void`: no
///        value alternative to store, so construction, `operator*`,
///        `value()`, `value_or` (dropped entirely -- there's no value to
///        fall back to), and the monadic operations all drop their
///        value-carrying parameters accordingly. Otherwise mirrors the
///        primary template's API and fidelity scope member-for-member
///        (constructors, assignment, `error()`/`error_or`, `and_then`/
///        `or_else`/`transform`/`transform_error`, comparisons,
///        `emplace`, `swap`) -- see the primary template's own doc
///        comments for the contract each corresponding member follows.
///
/// Per-member documentation isn't separately generated for this
/// specialization: Doxygen's handling of class-template partial
/// specializations conflates member lookup with the primary template
/// (confirmed by isolating it -- removing this specialization entirely
/// made an unrelated "found documented return type that does not
/// return anything" error on the *primary* template's `value_or`
/// disappear, meaning Doxygen was resolving `T -> void` against the
/// primary's members while processing this specialization, not a
/// defect in this code). Excluded from the documentation-coverage gate
/// below for that reason, same posture as the layered-base
/// implementation detail earlier in this file.
/// @tparam E The error type, same as the primary template.
/// \cond BRIDGE_DETAIL
template <class E>
class expected<void, E> : private layers::void_move_assign_layer<E> {
    using base = layers::void_move_assign_layer<E>;

public:
    /// @brief The (absent) wrapped value type.
    using value_type = void;
    /// @brief The wrapped error type.
    using error_type = E;
    /// @brief The `unexpected` specialization matching this `expected`.
    using unexpected_type = unexpected<E>;
    /// @brief `expected<U,E>` -- the same error type, a different value
    ///        type, matching `std::expected::rebind`.
    template <class U>
    using rebind = expected<U, error_type>;

    /// @brief Default-constructs the (empty) value. Unlike the primary
    ///        template, always available -- there's no `T` to require
    ///        default-constructibility of.
    constexpr expected() noexcept : base(std::in_place) {}

    /// @brief Marks this as holding a value. Matches the primary
    ///        template's `expected(std::in_place_t, Args&&...)`, with
    ///        no `Args` since there's nothing to construct.
    constexpr explicit expected(std::in_place_t) noexcept : base(std::in_place) {}

    /// @brief Copies whichever alternative `other` holds. Deleted
    ///        unless `E` is copy-constructible.
    expected(const expected&) = default;
    /// @brief Moves whichever alternative `other` holds. Deleted unless
    ///        `E` is move-constructible.
    expected(expected&&) = default;

    /// @brief Constructs an error from `u`'s wrapped error.
    /// @param u The `unexpected` to construct the error from.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, const G&>>>
    constexpr explicit expected(const unexpected<G>& u) : base(unexpect_t{}, u.error()) {}
    /// @brief Constructs an error by moving `u`'s wrapped error.
    /// @param u The `unexpected` to construct the error from.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, G&&>>>
    constexpr explicit expected(unexpected<G>&& u) : base(unexpect_t{}, std::move(u).error()) {}

    /// @brief Constructs an error in place from `args`.
    /// @param args Forwarded to `E`'s constructor.
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<E, Args...>>>
    constexpr explicit expected(unexpect_t, Args&&... args) : base(unexpect_t{}, std::forward<Args>(args)...) {}
    /// @brief Constructs an error in place from an initializer list plus
    ///        `args`.
    /// @param il Forwarded to `E`'s constructor as the first argument.
    /// @param args Forwarded to `E`'s constructor after `il`.
    template <class U, class... Args,
              class = std::enable_if_t<std::is_constructible_v<E, std::initializer_list<U>&, Args...>>>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
        : base(unexpect_t{}, il, std::forward<Args>(args)...) {}

    /// @brief Converting constructor from an `expected<U,G>` where `U`
    ///        is also (possibly cv-qualified) `void`, and `G` converts
    ///        to `E`. Same documented simplification as the primary
    ///        template's converting constructor: omits the standard's
    ///        extra defensive SFINAE guards.
    /// @param other The `expected` to convert from.
    template <class U, class G,
              class = std::enable_if_t<std::is_void_v<U> && std::is_constructible_v<E, const G&>>>
    explicit expected(const expected<U, G>& other) : base(typename layers::void_storage<E>::empty_tag{}) {
        if (other.has_value()) {
            this->state_ = layers::state::value;
        } else {
            ::new (std::addressof(this->storage_.err)) E(other.error());
            this->state_ = layers::state::error;
        }
    }
    /// @copydoc expected(const expected<U,G>&)
    template <class U, class G, class = std::enable_if_t<std::is_void_v<U> && std::is_constructible_v<E, G&&>>>
    explicit expected(expected<U, G>&& other) : base(typename layers::void_storage<E>::empty_tag{}) {
        if (other.has_value()) {
            this->state_ = layers::state::value;
        } else {
            ::new (std::addressof(this->storage_.err)) E(std::move(other).error());
            this->state_ = layers::state::error;
        }
    }

    /// @brief Copy-assigns whichever alternative `other` holds. Deleted
    ///        unless `E` is copy-constructible, copy-assignable, and
    ///        nothrow-move-constructible.
    /// @return `*this`.
    expected& operator=(const expected&) = default;
    /// @brief Move-assigns whichever alternative `other` holds. Deleted
    ///        unless `E` is move-constructible, move-assignable, and
    ///        nothrow-move-constructible.
    /// @return `*this`.
    expected& operator=(expected&&) = default;

    /// @brief Assigns an error constructed from `u`'s wrapped error,
    ///        replacing whatever this held before.
    /// @param u The `unexpected` to assign the error from.
    /// @return `*this`.
    template <class G = E,
              class = std::enable_if_t<std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>>>
    constexpr expected& operator=(const unexpected<G>& u) {
        if (!has_value()) {
            error() = u.error();
        } else {
            this->destroy();
            ::new (std::addressof(this->storage_.err)) E(u.error());
            this->state_ = layers::state::error;
        }
        return *this;
    }
    /// @brief Assigns an error by moving `u`'s wrapped error, replacing
    ///        whatever this held before.
    /// @param u The `unexpected` to assign the error from.
    /// @return `*this`.
    template <class G = E, class = std::enable_if_t<std::is_constructible_v<E, G&&> && std::is_assignable_v<E&, G&&>>>
    constexpr expected& operator=(unexpected<G>&& u) {
        if (!has_value()) {
            error() = std::move(u).error();
        } else {
            this->destroy();
            ::new (std::addressof(this->storage_.err)) E(std::move(u).error());
            this->state_ = layers::state::error;
        }
        return *this;
    }

    /// @brief Whether this holds a value (as opposed to an error).
    /// @return `true` if this holds a value.
    constexpr bool has_value() const noexcept { return this->state_ == layers::state::value; }
    /// @copydoc has_value
    constexpr explicit operator bool() const noexcept { return has_value(); }

    /// @brief No-op accessor, present only for generic-code uniformity
    ///        with the primary template's `operator*`. Precondition:
    ///        `has_value()`; violating it is undefined behavior,
    ///        matching `std::expected`.
    constexpr void operator*() const noexcept {}

    /// @brief Checked access, present only for generic-code uniformity.
    /// @throws bad_expected_access<E> if `!has_value()`, constructed
    ///         from a copy of the contained error.
    constexpr void value() const& {
        if (!has_value()) throw bad_expected_access<E>(error());
    }
    /// @copydoc value()const&
    constexpr void value() && {
        if (!has_value()) throw bad_expected_access<E>(std::move(error()));
    }

    /// @brief Lvalue access to the contained error. Precondition:
    ///        `!has_value()`; violating it is undefined behavior,
    ///        matching `std::expected`.
    /// @return A reference to the contained error.
    constexpr E& error() & { return this->storage_.err; }
    /// @copydoc error()&
    constexpr const E& error() const& { return this->storage_.err; }
    /// @copydoc error()&
    constexpr E&& error() && { return std::move(this->storage_.err); }
    /// @copydoc error()&
    constexpr const E&& error() const&& { return std::move(this->storage_.err); }

    /// @brief The contained error, or `g` converted to `E` if this
    ///        holds a value. See the primary template's `error_or` for
    ///        the same `__cpp_lib_expected`-verified rationale.
    /// @param g The fallback error.
    /// @return A copy of the contained error, or `g` converted to `E`.
    template <class G>
    constexpr E error_or(G&& g) const& {
        return has_value() ? static_cast<E>(std::forward<G>(g)) : error();
    }
    /// @copydoc error_or(G&&)const&
    template <class G>
    constexpr E error_or(G&& g) && {
        return has_value() ? static_cast<E>(std::forward<G>(g)) : std::move(error());
    }

    /// @brief If this holds a value, invoke `f` with no arguments and
    ///        return the result (which must itself be a specialization
    ///        of `expected` with a matching `error_type`); otherwise
    ///        return an error copy of that same type.
    /// @param f A callable, invoked with no arguments, returning a
    ///          specialization of `expected`.
    /// @return `f`'s result, or an error copy of its `expected` type.
    template <class F>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f));
        }
        return U(unexpect_t{}, error());
    }
    /// @copydoc and_then(F&&)const&
    template <class F>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>, "F's expected<..., E> must use this expected's E");
        if (has_value()) {
            return std::invoke(std::forward<F>(f));
        }
        return U(unexpect_t{}, std::move(error()));
    }

    /// @brief If this holds an error, invoke `f` with it and return the
    ///        result (which must itself be a specialization of
    ///        `expected` with a `void` value type); otherwise return a
    ///        value copy of that same type.
    /// @param f A callable returning a specialization of `expected`.
    /// @return `f`'s result, or a value copy of its `expected` type.
    template <class F>
    constexpr auto or_else(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_void_v<typename U::value_type>, "F's expected<void, ...> must have a void value_type");
        if (has_value()) {
            return U(std::in_place);
        }
        return std::invoke(std::forward<F>(f), error());
    }
    /// @copydoc or_else(F&&)const&
    template <class F>
    constexpr auto or_else(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
        static_assert(is_expected_v<U>, "F must return a specialization of expected");
        static_assert(std::is_void_v<typename U::value_type>, "F's expected<void, ...> must have a void value_type");
        if (has_value()) {
            return U(std::in_place);
        }
        return std::invoke(std::forward<F>(f), std::move(error()));
    }

    /// @brief If this holds a value, invoke `f` with no arguments and
    ///        return `expected<U,E>` containing the result (or
    ///        `expected<void,E>` if `f` returns `void`); otherwise
    ///        return an error copy.
    /// @param f A callable, invoked with no arguments.
    /// @return `f`'s result wrapped in `expected<U,E>`, or an error copy.
    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f));
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, error());
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f)));
            }
            return expected<U, E>(unexpect_t{}, error());
        }
    }
    /// @copydoc transform(F&&)const&
    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (std::is_void_v<U>) {
            if (has_value()) {
                std::invoke(std::forward<F>(f));
                return expected<void, E>(std::in_place);
            }
            return expected<void, E>(unexpect_t{}, std::move(error()));
        } else {
            if (has_value()) {
                return expected<U, E>(std::in_place, std::invoke(std::forward<F>(f)));
            }
            return expected<U, E>(unexpect_t{}, std::move(error()));
        }
    }

    /// @brief If this holds an error, invoke `f` with it and return
    ///        `expected<void,G>` containing the result wrapped in
    ///        `unexpected`; otherwise return a value copy of
    ///        `expected<void,G>`.
    /// @param f A callable returning a non-`void` value.
    /// @return `f`'s result wrapped in `expected<void,G>`'s error, or a
    ///         value copy.
    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<void, G>(std::in_place);
        }
        return expected<void, G>(unexpect_t{}, std::invoke(std::forward<F>(f), error()));
    }
    /// @copydoc transform_error(F&&)const&
    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
        static_assert(!std::is_same_v<G, void>, "F must not return void");
        if (has_value()) {
            return expected<void, G>(std::in_place);
        }
        return expected<void, G>(unexpect_t{}, std::invoke(std::forward<F>(f), std::move(error())));
    }

    /// @brief Destroys whatever this held and marks this as holding a
    ///        (empty) value.
    constexpr void emplace() noexcept {
        this->destroy();
        this->state_ = layers::state::value;
    }

    /// @brief Swaps the contents of `*this` and `other`, including
    ///        across differing alternatives.
    /// @param other The `expected` to swap with.
    void swap(expected& other) noexcept(std::is_nothrow_move_constructible_v<E>&& std::is_nothrow_swappable_v<E>) {
        using std::swap;
        if (has_value() && other.has_value()) {
            // Nothing to exchange -- a value alternative carries no data.
        } else if (!has_value() && !other.has_value()) {
            swap(error(), other.error());
        } else {
            expected& with_value = has_value() ? *this : other;
            expected& with_error = has_value() ? other : *this;
            E moved_error(std::move(with_error.error()));
            with_error.destroy();
            with_error.state_ = layers::state::value;
            with_value.destroy();
            ::new (std::addressof(with_value.storage_.err)) E(std::move(moved_error));
            with_value.state_ = layers::state::error;
        }
    }

    /// @brief Compares two `expected`s: equal if both hold a value (a
    ///        value holds no data to differ on), or both hold an error
    ///        and those errors compare equal.
    /// @param x The left-hand `expected`.
    /// @param y The right-hand `expected`, possibly with a different
    ///           error type.
    /// @return Whether `x` and `y` are equal, as described above.
    template <class E2>
    friend constexpr bool operator==(const expected& x, const expected<void, E2>& y) {
        if (x.has_value() != y.has_value()) return false;
        if (x.has_value()) return true;
        return static_cast<bool>(x.error() == y.error());
    }
    /// @brief See the primary template's `operator!=` for why this is
    ///        defined explicitly rather than relying on C++20 rewriting.
    /// @param x The left-hand `expected`.
    /// @param y The right-hand `expected`, possibly with a different
    ///           error type.
    /// @return Whether `x` and `y` are unequal.
    template <class E2>
    friend constexpr bool operator!=(const expected& x, const expected<void, E2>& y) {
        return !(x == y);
    }

    /// @brief Compares against an `unexpected<G>`: equal only when `x`
    ///        holds an error and it compares equal to `e`'s.
    /// @param x The `expected`.
    /// @param e The `unexpected` to compare against.
    /// @return Whether `x` holds an error equal to `e.error()`.
    template <class G>
    friend constexpr bool operator==(const expected& x, const unexpected<G>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }
    /// @copydoc operator==(const expected&,const unexpected<G>&)
    template <class G>
    friend constexpr bool operator==(const unexpected<G>& e, const expected& x) {
        return x == e;
    }
    /// @brief See the primary template's `operator!=` for why this is
    ///        defined explicitly rather than relying on C++20 rewriting.
    /// @param x The `expected`.
    /// @param e The `unexpected` to compare against.
    /// @return Whether `x` does not hold an error equal to `e.error()`.
    template <class G>
    friend constexpr bool operator!=(const expected& x, const unexpected<G>& e) {
        return !(x == e);
    }
    /// @copydoc operator!=(const expected&,const unexpected<G>&)
    template <class G>
    friend constexpr bool operator!=(const unexpected<G>& e, const expected& x) {
        return !(x == e);
    }
};
/// \endcond

/// @brief ADL swap, forwarding to `expected`'s member `swap`.
/// @param lhs The first `expected`.
/// @param rhs The second `expected`.
template <class T, class E>
void swap(expected<T, E>& lhs, expected<T, E>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::expected::unexpect_t;
using bridge::detail::truss::cpp17::expected::unexpect;
using bridge::detail::truss::cpp17::expected::unexpected;
using bridge::detail::truss::cpp17::expected::bad_expected_access;
using bridge::detail::truss::cpp17::expected::expected;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::expected

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace expected { ... }` wrapper here (same reason as
/// deck/cpp17/optional.hpp's exports): this header's primary export is
/// a type named `expected`, and nesting it inside an inline namespace
/// of the identical name makes that inline namespace's own qualified
/// name reachable at this same scope, colliding with the promoted
/// type. Promoting straight from the `cpp17` inline namespace avoids
/// the collision.
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
using bridge::exports::truss::expected;
} // namespace bridge::truss
