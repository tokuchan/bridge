/// @file jthread.hpp
/// @brief This file holds Truss's `jthread` polyfill, for standards
///        that predate C++20. This polyfill is built from scratch.
///
///        Like `expected`, `span`, and `stop_token`, there is no
///        C++17 type to attach free functions onto, so Truss owns a
///        complete class here. This class is built directly on
///        `std::thread`, a real C++11 facility, plus this same
///        library's own `stop_token.hpp`. This facility needs no
///        platform-specific code. See
///        docs/adr/0017-jthread-stop-token-truss-owns-the-class.md
///        for the full design. See
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace rule this file follows.
///
/// `bridge::truss::jthread` is always this polyfill, regardless of
/// standard or toolchain. Truss never itself passes through to
/// `std::jthread`, even under C++20 or C++23, where the real type is
/// available. Deck makes that choice exactly once instead.
#pragma once

#include <thread>
#include <type_traits>
#include <utility>

#include <truss/cpp17/stop_token.hpp>

namespace bridge::detail::truss::cpp17::jthread {

/// @brief This class is a joining, cooperatively-cancellable thread.
///        This class matches `std::jthread`.
///
///        Unlike `std::thread`, this class automatically requests a
///        stop and joins on destruction, when the thread is still
///        joinable. `std::thread`'s own destructor terminates the
///        program in that same situation instead.
/// @see https://en.cppreference.com/w/cpp/thread/jthread
class jthread {
public:
    /// @brief This is the underlying OS thread handle type.
    using native_handle_type = std::thread::native_handle_type;
    /// @brief This is the type that `get_id()` returns.
    using id = std::thread::id;

    /// @brief This constructor builds a `jthread` with no associated
    ///        thread of execution.
    jthread() noexcept : stop_source_(bridge::truss::nostopstate) {}

    /// @brief This constructor builds a `jthread` running
    ///        `f(args...)` on a new thread.
    ///
    ///        When `f` is invocable with a leading `stop_token` (when
    ///        `f(get_stop_token(), args...)` would be well-formed),
    ///        this constructor prepends that token automatically.
    ///        Otherwise, this constructor runs `f` exactly as
    ///        `std::thread` would call it.
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

    /// @brief This constructor moves `other`'s thread and stop-state
    ///        into the new `jthread`. `other` is left with no
    ///        associated thread.
    /// @param other The `jthread` to move from.
    jthread(jthread&& other) noexcept = default;

    /// @brief This moves `other`'s thread and stop-state into
    ///        `*this`. When `*this` is still joinable, this method
    ///        requests a stop and joins it first, the same as the
    ///        destructor.
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

    /// @brief This destructor requests a stop and joins first, when
    ///        the thread is joinable. This is the core difference
    ///        from `std::thread`. `std::thread`'s own destructor
    ///        terminates the program in this same situation instead.
    ~jthread() {
        if (joinable()) {
            request_stop();
            join();
        }
    }

    /// @brief This swaps this `jthread`'s thread and stop-state with
    ///        `other`'s.
    /// @param other The `jthread` to swap with.
    void swap(jthread& other) noexcept {
        thread_.swap(other.thread_);
        stop_source_.swap(other.stop_source_);
    }

    /// @brief This checks whether this `jthread` has an associated
    ///        thread of execution.
    /// @return Whether `join()`/`detach()` may be called.
    bool joinable() const noexcept { return thread_.joinable(); }

    /// @brief This blocks until the associated thread completes.
    void join() { thread_.join(); }

    /// @brief This separates the associated thread from this object.
    ///        The thread continues running independently.
    void detach() { thread_.detach(); }

    /// @brief This is the associated thread's id. This is a
    ///        default-constructed `id` when there is no associated
    ///        thread.
    /// @return The thread id.
    id get_id() const noexcept { return thread_.get_id(); }

    /// @brief This is the underlying implementation-defined thread
    ///        handle.
    /// @return The native handle.
    native_handle_type native_handle() { return thread_.native_handle(); }

    /// @brief This is a `stop_source` that shares this `jthread`'s
    ///        stop-state.
    /// @return The stop source.
    bridge::truss::stop_source get_stop_source() const noexcept { return stop_source_; }

    /// @brief This is a `stop_token` that shares this `jthread`'s
    ///        stop-state.
    /// @return The stop token.
    bridge::truss::stop_token get_stop_token() const noexcept { return stop_source_.get_token(); }

    /// @brief This requests a stop, through this `jthread`'s
    ///        `stop_source`.
    /// @return Whether this call is the one that actually requested
    ///         it. This matches `stop_source::request_stop()`.
    bool request_stop() noexcept { return stop_source_.request_stop(); }

    /// @brief This is the number of concurrent threads the
    ///        implementation supports. This matches
    ///        `std::thread::hardware_concurrency()`.
    /// @return The hint, or `0` if not computable/well-defined.
    static unsigned hardware_concurrency() noexcept { return std::thread::hardware_concurrency(); }

private:
    bridge::truss::stop_source stop_source_{};
    std::thread thread_;
};

/// @brief This swaps `a`'s and `b`'s thread and stop-state.
/// @param a The first `jthread`.
/// @param b The second `jthread`.
inline void swap(jthread& a, jthread& b) noexcept { a.swap(b); }

/// @brief This namespace promotes `jthread` and `swap` to
///        `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::jthread::jthread;
using bridge::detail::truss::cpp17::jthread::swap;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::jthread

/// @brief This is the Exports namespace for `jthread`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace jthread { ... }` wrapper,
/// for the same reason as truss/cpp17/expected.hpp's Exports
/// namespace. This header's primary export is a type named
/// `jthread`. The wrapper's name would be `jthread` too, and the two
/// names would collide. This namespace promotes `jthread` straight
/// from the `cpp17` inline namespace instead, and avoids the
/// collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::jthread::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::jthread;
} // namespace bridge::truss
