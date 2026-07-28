/// @file stop_token.hpp
/// @brief This file holds Truss's `stop_token` and `stop_source`
///        polyfill, for standards that predate C++20. This polyfill
///        is built from scratch.
///
///        Like `expected` and `span`, there is no C++17 type to
///        attach free functions onto, so Truss owns complete classes
///        here. `stop_callback` is not implemented. This is a
///        disclosed follow-up (see
///        docs/adr/0017-jthread-stop-token-truss-owns-the-class.md).
///        Its destructor has a real concurrent-correctness contract.
///        This destructor must block, when its callback is running
///        on another thread. This destructor must not deadlock, when
///        something destroys it from inside its own callback. This
///        is real, high-risk concurrent code, and this pass does not
///        attempt it. See docs/adr/0001-namespace-and-export-
///        scheme.md for the namespace rule this file follows.
///
/// `bridge::truss::stop_token` and `stop_source` are always this
/// polyfill, regardless of standard or toolchain. Truss never itself
/// passes through to `std::stop_token` or `std::stop_source`, even
/// under C++20 or C++23, where the real types are available. Deck
/// makes that choice exactly once instead: `truss/cpp17/jthread.hpp`
/// builds `jthread` directly on these classes.
#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

namespace bridge::detail::truss::cpp17::stop_token {

/// @brief This struct holds the cancellation state that a
///        `stop_token`/`stop_source` pair shares. This state holds
///        whether a stop has been requested, and how many live
///        `stop_source`s could still request one.
struct stop_state {
    /// @brief This is true when `request_stop()` has been called, on
    ///        any `stop_source` sharing this state.
    std::atomic<bool> requested{false};
    /// @brief This is the number of live `stop_source`s currently
    ///        sharing this state. `stop_token::stop_possible()` needs
    ///        this count: it must know whether stopping is still
    ///        possible, not only whether a stop already happened.
    std::atomic<std::size_t> source_count{0};
};

class stop_source;

/// @brief This class is a handle to shared cancellation state. This
///        class matches `std::stop_token`.
///
///        A default-constructed token has no state at all; its
///        `stop_possible()` returns false. Otherwise, you get a
///        token from `stop_source::get_token()`.
/// @see https://en.cppreference.com/w/cpp/thread/stop_token
class stop_token {
public:
    /// @brief This constructor builds a token with no associated
    ///        stop-state.
    stop_token() noexcept = default;

    /// @brief This checks whether a stop has been requested, on the
    ///        associated stop-state.
    /// @return `false` if this token has no stop-state at all.
    bool stop_requested() const noexcept {
        return state_ && state_->requested.load(std::memory_order_acquire);
    }

    /// @brief This checks whether stopping is possible. Stopping is
    ///        possible when a stop is already requested, or when a
    ///        live `stop_source` could still request one.
    /// @return `false` if this token has no stop-state, or its
    ///         stop-state has no live `stop_source` left and stop was
    ///         never requested (stopping is now permanently
    ///         impossible through this token).
    bool stop_possible() const noexcept {
        if (!state_) {
            return false;
        }
        return state_->requested.load(std::memory_order_acquire) ||
               state_->source_count.load(std::memory_order_acquire) > 0;
    }

    /// @brief This swaps this token's stop-state with `other`'s.
    /// @param other The token to swap with.
    void swap(stop_token& other) noexcept { state_.swap(other.state_); }

    /// @brief This checks whether two tokens refer to the same
    ///        stop-state, or whether both have none.
    /// @param lhs The first token.
    /// @param rhs The second token.
    /// @return Whether `lhs` and `rhs` share the same stop-state.
    friend bool operator==(const stop_token& lhs, const stop_token& rhs) noexcept { return lhs.state_ == rhs.state_; }
    /// @brief This is the negation of `operator==`.
    /// @param lhs The first token.
    /// @param rhs The second token.
    /// @return Whether `lhs` and `rhs` refer to different stop-states.
    friend bool operator!=(const stop_token& lhs, const stop_token& rhs) noexcept { return !(lhs == rhs); }

private:
    friend class stop_source;
    friend struct ::std::hash<stop_token>;

    explicit stop_token(std::shared_ptr<stop_state> state) noexcept : state_(std::move(state)) {}

    std::shared_ptr<stop_state> state_;
};

/// @brief This swaps `a`'s and `b`'s stop-state.
/// @param a The first token.
/// @param b The second token.
inline void swap(stop_token& a, stop_token& b) noexcept { a.swap(b); }

/// @brief This tag type selects `stop_source`'s no-state constructor.
///        This tag type matches `std::nostopstate_t`.
struct nostopstate_t {
    /// @brief This default constructor is `explicit`. This matches
    ///        the real type. This prevents `stop_source{}` from
    ///        ambiguously selecting this tag, through aggregate
    ///        initialization.
    explicit nostopstate_t() = default;
};

/// @brief This is the canonical `nostopstate_t` instance. This
///        matches `std::nostopstate`.
inline constexpr nostopstate_t nostopstate{};

/// @brief This class owns a share of cancellation state. This class
///        can request a stop. This class matches `std::stop_source`.
/// @see https://en.cppreference.com/w/cpp/thread/stop_source
class stop_source {
public:
    /// @brief This constructor builds a new, independent stop-state.
    stop_source() : state_(std::make_shared<stop_state>()) { state_->source_count.fetch_add(1, std::memory_order_relaxed); }

    /// @brief This constructor builds a source with no stop-state at
    ///        all. You select this constructor by tag. This
    ///        constructor matches
    ///        `std::stop_source(std::nostopstate_t)`.
    explicit stop_source(nostopstate_t) noexcept {}

    /// @brief This constructor copies `other`. The new source shares
    ///        the same stop-state.
    /// @param other The source to copy.
    stop_source(const stop_source& other) noexcept : state_(other.state_) { add_ref(); }

    /// @brief This constructor moves `other`'s stop-state into the
    ///        new source. `other` is left with no stop-state, as if
    ///        `other` had been constructed via `nostopstate_t`.
    /// @param other The source to move from.
    stop_source(stop_source&& other) noexcept = default;

    /// @brief This copies `other`. The two sources share `other`'s
    ///        stop-state afterward.
    /// @param other The source to copy.
    /// @return `*this`.
    stop_source& operator=(const stop_source& other) noexcept {
        if (state_ != other.state_) {
            release();
            state_ = other.state_;
            add_ref();
        }
        return *this;
    }

    /// @brief This moves `other`'s stop-state into `*this`. `other`
    ///        is left with no stop-state.
    /// @param other The source to move from.
    /// @return `*this`.
    stop_source& operator=(stop_source&& other) noexcept {
        if (this != &other) {
            release();
            state_ = std::move(other.state_);
        }
        return *this;
    }

    ~stop_source() { release(); }

    /// @brief This checks whether this source has a stop-state at
    ///        all.
    /// @return `false` only if constructed via `nostopstate_t`, or
    ///         moved-from.
    bool stop_possible() const noexcept { return static_cast<bool>(state_); }

    /// @brief This checks whether a stop has been requested, on this
    ///        source's stop-state.
    /// @return `false` if this source has no stop-state.
    bool stop_requested() const noexcept { return state_ && state_->requested.load(std::memory_order_acquire); }

    /// @brief This requests a stop, when this source has a stop-state
    ///        and has not already requested one.
    /// @return Whether this call is the one that actually transitioned
    ///         the stop-state to requested. This is `false` when
    ///         there is no stop-state, or when a stop was already
    ///         requested, by this source or by another source sharing
    ///         the same state.
    bool request_stop() noexcept {
        if (!state_) {
            return false;
        }
        bool expected = false;
        return state_->requested.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }

    /// @brief This returns a token that refers to this source's
    ///        stop-state.
    /// @return A `stop_token` sharing this source's stop-state. This
    ///         token has no state at all, when this source has none.
    stop_token get_token() const noexcept { return stop_token(state_); }

    /// @brief This swaps this source's stop-state with `other`'s.
    /// @param other The source to swap with.
    void swap(stop_source& other) noexcept { state_.swap(other.state_); }

    /// @brief This checks whether two sources refer to the same
    ///        stop-state, or whether both have none.
    /// @param lhs The first source.
    /// @param rhs The second source.
    /// @return Whether `lhs` and `rhs` share the same stop-state.
    friend bool operator==(const stop_source& lhs, const stop_source& rhs) noexcept {
        return lhs.state_ == rhs.state_;
    }
    /// @brief This is the negation of `operator==`.
    /// @param lhs The first source.
    /// @param rhs The second source.
    /// @return Whether `lhs` and `rhs` refer to different stop-states.
    friend bool operator!=(const stop_source& lhs, const stop_source& rhs) noexcept { return !(lhs == rhs); }

private:
    void add_ref() noexcept {
        if (state_) {
            state_->source_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
    void release() noexcept {
        if (state_) {
            state_->source_count.fetch_sub(1, std::memory_order_acq_rel);
        }
    }

    std::shared_ptr<stop_state> state_;
};

/// @brief This swaps `a`'s and `b`'s stop-state.
/// @param a The first source.
/// @param b The second source.
inline void swap(stop_source& a, stop_source& b) noexcept { a.swap(b); }

/// @brief This namespace promotes `stop_token`, `swap`,
///        `nostopstate_t`, `nostopstate`, and `stop_source` to
///        `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::stop_token::stop_token;
using bridge::detail::truss::cpp17::stop_token::swap;
using bridge::detail::truss::cpp17::stop_token::nostopstate_t;
using bridge::detail::truss::cpp17::stop_token::nostopstate;
using bridge::detail::truss::cpp17::stop_token::stop_source;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::stop_token

// std::hash specialization for bridge::truss::stop_token, matching
// std::hash<std::stop_token>. Hashes the underlying stop-state's
// identity, same as operator==. \cond'd out: Doxygen can't resolve
// std::hash's scope without BUILTIN_STL_SUPPORT (a broader config
// change not worth making for one specialization), same reason
// field_writers-style internal helpers elsewhere in this codebase are
// excluded rather than fought with.
/// \cond BRIDGE_DETAIL
template <>
struct std::hash<bridge::detail::truss::cpp17::stop_token::stop_token> {
    std::size_t operator()(const bridge::detail::truss::cpp17::stop_token::stop_token& token) const noexcept {
        return std::hash<std::shared_ptr<bridge::detail::truss::cpp17::stop_token::stop_state>>{}(token.state_);
    }
};
/// \endcond

/// @brief This is the Exports namespace for `stop_token`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace stop_token { ... }`
/// wrapper, for the same reason as truss/cpp17/expected.hpp's Exports
/// namespace. This header's primary export is a type named
/// `stop_token`. The wrapper's name would be `stop_token` too, and
/// the two names would collide. This namespace promotes `stop_token`
/// straight from the `cpp17` inline namespace instead, and avoids the
/// collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::stop_token::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::stop_token;
using bridge::exports::truss::nostopstate_t;
using bridge::exports::truss::nostopstate;
using bridge::exports::truss::stop_source;
} // namespace bridge::truss
