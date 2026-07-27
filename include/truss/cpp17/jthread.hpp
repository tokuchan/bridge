/// @file jthread.hpp
/// @brief Truss's from-scratch `jthread` polyfill for standards that
///        predate C++20.
///
///        Like `expected`/`span`/`stop_token`, there is no
///        pre-existing C++17 type to attach free functions onto, so
///        Truss owns a complete class here, built directly on
///        `std::thread` (a real C++11 facility) plus this same
///        library's own `stop_token.hpp` -- no platform-specific code
///        needed. See docs/adr/0017-jthread-stop-token-truss-owns-the-
///        class.md for the full design and docs/adr/0001-namespace-
///        and-export-scheme.md for the namespace scheme this follows.
///
/// `bridge::truss::jthread` is unconditionally this polyfill,
/// regardless of standard or toolchain -- Truss never itself passes
/// through to `std::jthread`, even under C++20/23 where the real type
/// is available. That selection happens exactly once, in Deck.
#pragma once

#include <thread>
#include <type_traits>
#include <utility>

#include <truss/cpp17/stop_token.hpp>

namespace bridge::detail::truss::cpp17::jthread {

/// @brief A joining, cooperatively-cancellable thread, matching
///        `std::jthread`. Unlike `std::thread`, automatically requests
///        a stop and joins on destruction if still joinable -- never
///        terminates the program for an un-joined, un-detached thread
///        the way `std::thread`'s own destructor does.
/// @see https://en.cppreference.com/w/cpp/thread/jthread
class jthread {
public:
    /// @brief The underlying OS thread handle type.
    using native_handle_type = std::thread::native_handle_type;
    /// @brief The type `get_id()` returns.
    using id = std::thread::id;

    /// @brief Constructs a `jthread` with no associated thread of
    ///        execution.
    jthread() noexcept : stop_source_(bridge::truss::nostopstate) {}

    /// @brief Constructs a `jthread` running `f(args...)` on a new
    ///        thread. If `f` is invocable with a leading `stop_token`
    ///        (i.e. `f(get_stop_token(), args...)` would be
    ///        well-formed), that token is prepended automatically;
    ///        otherwise `f` runs exactly as `std::thread` would call
    ///        it.
    /// @tparam F The callable type.
    /// @tparam Args The argument types.
    /// @param f The callable to run on the new thread.
    /// @param args The arguments to invoke `f` with.
    template <class F, class... Args, class = std::enable_if_t<!std::is_same_v<std::decay_t<F>, jthread>>>
    explicit jthread(F&& f, Args&&... args) {
        if constexpr (std::is_invocable_v<std::decay_t<F>, bridge::truss::stop_token, std::decay_t<Args>...>) {
            thread_ = std::thread(
                [](bridge::truss::stop_token st, std::decay_t<F> fn, std::decay_t<Args>... a) {
                    std::move(fn)(std::move(st), std::move(a)...);
                },
                stop_source_.get_token(), std::forward<F>(f), std::forward<Args>(args)...);
        } else {
            thread_ = std::thread(std::forward<F>(f), std::forward<Args>(args)...);
        }
    }

    jthread(const jthread&) = delete;
    jthread& operator=(const jthread&) = delete;

    /// @brief Move-constructs, taking over `other`'s thread and
    ///        stop-state. `other` is left with no associated thread.
    /// @param other The `jthread` to move from.
    jthread(jthread&& other) noexcept = default;

    /// @brief Move-assigns, taking over `other`'s thread and
    ///        stop-state. If `*this` is still joinable, requests a
    ///        stop and joins it first, same as the destructor.
    /// @param other The `jthread` to move from.
    /// @return `*this`.
    jthread& operator=(jthread&& other) noexcept {
        if (this != &other) {
            if (joinable()) {
                request_stop();
                join();
            }
            thread_ = std::move(other.thread_);
            stop_source_ = std::move(other.stop_source_);
        }
        return *this;
    }

    /// @brief If joinable, requests a stop and joins before
    ///        destroying -- the core difference from `std::thread`,
    ///        whose destructor terminates the program in this same
    ///        situation instead.
    ~jthread() {
        if (joinable()) {
            request_stop();
            join();
        }
    }

    /// @brief Swaps this `jthread`'s thread and stop-state with
    ///        `other`'s.
    /// @param other The `jthread` to swap with.
    void swap(jthread& other) noexcept {
        thread_.swap(other.thread_);
        stop_source_.swap(other.stop_source_);
    }

    /// @brief Whether this `jthread` has an associated thread of
    ///        execution.
    /// @return Whether `join()`/`detach()` may be called.
    bool joinable() const noexcept { return thread_.joinable(); }

    /// @brief Blocks until the associated thread completes.
    void join() { thread_.join(); }

    /// @brief Separates the associated thread from this object,
    ///        allowing it to continue running independently.
    void detach() { thread_.detach(); }

    /// @brief The associated thread's id, or a default-constructed
    ///        `id` if none.
    /// @return The thread id.
    id get_id() const noexcept { return thread_.get_id(); }

    /// @brief The underlying implementation-defined thread handle.
    /// @return The native handle.
    native_handle_type native_handle() { return thread_.native_handle(); }

    /// @brief A `stop_source` sharing this `jthread`'s stop-state.
    /// @return The stop source.
    bridge::truss::stop_source get_stop_source() const noexcept { return stop_source_; }

    /// @brief A `stop_token` sharing this `jthread`'s stop-state.
    /// @return The stop token.
    bridge::truss::stop_token get_stop_token() const noexcept { return stop_source_.get_token(); }

    /// @brief Requests a stop via this `jthread`'s `stop_source`.
    /// @return Whether this call is the one that actually requested it
    ///         (matches `stop_source::request_stop()`).
    bool request_stop() noexcept { return stop_source_.request_stop(); }

    /// @brief The number of concurrent threads supported by the
    ///        implementation, matching `std::thread::hardware_concurrency()`.
    /// @return The hint, or `0` if not computable/well-defined.
    static unsigned hardware_concurrency() noexcept { return std::thread::hardware_concurrency(); }

private:
    bridge::truss::stop_source stop_source_{};
    std::thread thread_;
};

/// @brief Swaps `a`'s and `b`'s thread and stop-state.
/// @param a The first `jthread`.
/// @param b The second `jthread`.
inline void swap(jthread& a, jthread& b) noexcept { a.swap(b); }

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::jthread::jthread;
using bridge::detail::truss::cpp17::jthread::swap;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::jthread

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace jthread { ... }` wrapper here (same reason as
/// truss/cpp17/expected.hpp's exports): this header's primary export
/// is a type named `jthread`, and nesting it inside an inline
/// namespace of the identical name makes that inline namespace's own
/// qualified name reachable at this same scope, colliding with the
/// promoted type. Promoting straight from the `cpp17` inline
/// namespace avoids the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::jthread::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::jthread;
} // namespace bridge::truss
