/// @file stop_token.hpp
/// @brief Truss's from-scratch `stop_token`/`stop_source` polyfill for
///        standards that predate C++20.
///
///        Like `expected` and `span`, there is no pre-existing C++17
///        type to attach free functions onto, so Truss owns complete
///        classes here. `stop_callback` is not implemented (disclosed
///        follow-up, see docs/adr/0017-jthread-stop-token-truss-owns-
///        the-class.md): its destructor's concurrent-correctness
///        contract (must block if its callback is running on another
///        thread, must not deadlock if destroyed from inside its own
///        callback) is real, high-risk concurrent code this pass
///        doesn't attempt. See docs/adr/0001-namespace-and-export-
///        scheme.md for the namespace scheme this follows.
///
/// `bridge::truss::stop_token`/`stop_source` are unconditionally this
/// polyfill, regardless of standard or toolchain -- Truss never itself
/// passes through to `std::stop_token`/`std::stop_source`, even under
/// C++20/23 where the real types are available. That selection
/// happens exactly once, in Deck (truss/cpp17/jthread.hpp builds
/// `jthread` directly on these).
#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

namespace bridge::detail::truss::cpp17::stop_token {

/// @brief Shared cancellation state a `stop_token`/`stop_source` pair
///        refers to: whether a stop has been requested, and how many
///        live `stop_source`s could still request one.
struct stop_state {
    /// @brief Whether `request_stop()` has been called on any
    ///        `stop_source` sharing this state.
    std::atomic<bool> requested{false};
    /// @brief The number of live `stop_source`s currently sharing this
    ///        state -- needed for `stop_token::stop_possible()`, which
    ///        must know whether stopping is still *possible*, not just
    ///        whether it's already happened.
    std::atomic<std::size_t> source_count{0};
};

class stop_source;

/// @brief A handle to shared cancellation state, matching
///        `std::stop_token`. Default-constructed with no state at all
///        (`stop_possible()` false); otherwise obtained from a
///        `stop_source::get_token()`.
/// @see https://en.cppreference.com/w/cpp/thread/stop_token
class stop_token {
public:
    /// @brief Constructs a token with no associated stop-state.
    stop_token() noexcept = default;

    /// @brief Whether stop has been requested on the associated
    ///        stop-state.
    /// @return `false` if this token has no stop-state at all.
    bool stop_requested() const noexcept {
        return state_ && state_->requested.load(std::memory_order_acquire);
    }

    /// @brief Whether stopping is possible: either already requested,
    ///        or a live `stop_source` could still request it.
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

    /// @brief Swaps this token's stop-state with `other`'s.
    /// @param other The token to swap with.
    void swap(stop_token& other) noexcept { state_.swap(other.state_); }

    /// @brief Whether two tokens refer to the same stop-state (or both
    ///        have none).
    /// @param lhs The first token.
    /// @param rhs The second token.
    /// @return Whether `lhs` and `rhs` share the same stop-state.
    friend bool operator==(const stop_token& lhs, const stop_token& rhs) noexcept { return lhs.state_ == rhs.state_; }
    /// @brief The negation of `operator==`.
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

/// @brief Swaps `a`'s and `b`'s stop-state.
/// @param a The first token.
/// @param b The second token.
inline void swap(stop_token& a, stop_token& b) noexcept { a.swap(b); }

/// @brief Tag type selecting `stop_source`'s no-state constructor,
///        matching `std::nostopstate_t`.
struct nostopstate_t {
    /// @brief Explicit default constructor, matching the real type
    ///        (prevents `stop_source{}` from ambiguously selecting
    ///        this tag via aggregate initialization).
    explicit nostopstate_t() = default;
};

/// @brief The canonical `nostopstate_t` instance, matching
///        `std::nostopstate`.
inline constexpr nostopstate_t nostopstate{};

/// @brief Owns (a share of) cancellation state and can request a
///        stop, matching `std::stop_source`.
/// @see https://en.cppreference.com/w/cpp/thread/stop_source
class stop_source {
public:
    /// @brief Constructs a new, independent stop-state.
    stop_source() : state_(std::make_shared<stop_state>()) { state_->source_count.fetch_add(1, std::memory_order_relaxed); }

    /// @brief Constructs a source with no stop-state at all, selected
    ///        by tag, matching `std::stop_source(std::nostopstate_t)`.
    explicit stop_source(nostopstate_t) noexcept {}

    /// @brief Copy-constructs, sharing the same stop-state.
    /// @param other The source to copy.
    stop_source(const stop_source& other) noexcept : state_(other.state_) { add_ref(); }

    /// @brief Move-constructs, taking over `other`'s stop-state.
    ///        `other` is left with no stop-state, as if constructed
    ///        via `nostopstate_t`.
    /// @param other The source to move from.
    stop_source(stop_source&& other) noexcept = default;

    /// @brief Copy-assigns, sharing `other`'s stop-state.
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

    /// @brief Move-assigns, taking over `other`'s stop-state. `other`
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

    /// @brief Whether this source has a stop-state at all.
    /// @return `false` only if constructed via `nostopstate_t`, or
    ///         moved-from.
    bool stop_possible() const noexcept { return static_cast<bool>(state_); }

    /// @brief Whether stop has been requested on this source's
    ///        stop-state.
    /// @return `false` if this source has no stop-state.
    bool stop_requested() const noexcept { return state_ && state_->requested.load(std::memory_order_acquire); }

    /// @brief Requests a stop, if this source has a stop-state and
    ///        hasn't already requested one.
    /// @return Whether this call is the one that actually transitioned
    ///         the stop-state to requested (`false` if there's no
    ///         stop-state, or one was already requested -- by this
    ///         source or another sharing the same state).
    bool request_stop() noexcept {
        if (!state_) {
            return false;
        }
        bool expected = false;
        return state_->requested.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }

    /// @brief Returns a token referring to this source's stop-state.
    /// @return A `stop_token` sharing this source's stop-state (with
    ///         no state at all if this source has none).
    stop_token get_token() const noexcept { return stop_token(state_); }

    /// @brief Swaps this source's stop-state with `other`'s.
    /// @param other The source to swap with.
    void swap(stop_source& other) noexcept { state_.swap(other.state_); }

    /// @brief Whether two sources refer to the same stop-state (or
    ///        both have none).
    /// @param lhs The first source.
    /// @param rhs The second source.
    /// @return Whether `lhs` and `rhs` share the same stop-state.
    friend bool operator==(const stop_source& lhs, const stop_source& rhs) noexcept {
        return lhs.state_ == rhs.state_;
    }
    /// @brief The negation of `operator==`.
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

/// @brief Swaps `a`'s and `b`'s stop-state.
/// @param a The first source.
/// @param b The second source.
inline void swap(stop_source& a, stop_source& b) noexcept { a.swap(b); }

/// @brief Symbols promoted to `bridge::exports::truss`.
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

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace stop_token { ... }` wrapper here (same reason
/// as truss/cpp17/expected.hpp's exports): this header's primary
/// export is a type named `stop_token`, and nesting it inside an
/// inline namespace of the identical name makes that inline
/// namespace's own qualified name reachable at this same scope,
/// colliding with the promoted type. Promoting straight from the
/// `cpp17` inline namespace avoids the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::stop_token::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::stop_token;
using bridge::exports::truss::nostopstate_t;
using bridge::exports::truss::nostopstate;
using bridge::exports::truss::stop_source;
} // namespace bridge::truss
