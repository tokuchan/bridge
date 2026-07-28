\page page_jthread Jthread

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<thread>`](https://en.cppreference.com/w/cpp/thread/jthread) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/stop_token.hpp`
- `include/truss/cpp17/jthread.hpp`
- `include/deck/cpp17/stop_token.hpp`
- `include/deck/cpp17/jthread.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::jthread` is a joining, cooperatively-cancellable thread,
matching C++20's `std::jthread`: unlike `std::thread`, its destructor
requests a stop and joins automatically if still joinable, rather than
terminating the program. `bridge::stop_token`/`bridge::stop_source`
are the cancellation primitives it's built on -- `std::jthread` doesn't
exist at all before C++20, and neither does a pre-existing type for
Truss to attach free functions onto (like `optional`), so Truss owns
complete classes for all three, built directly on `std::thread` (a
real C++11 facility) plus `<atomic>`/`<mutex>`/`<condition_variable>`
-- no platform-specific code needed.

## Example

```cpp
#include <deck/cpp17/jthread.hpp>
#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>

int main() {
    std::atomic<int> ticks{0};
    bridge::jthread worker([&ticks](bridge::stop_token st) {
        while (!st.stop_requested()) {
            ++ticks;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    worker.request_stop();
    worker.join();

    assert(ticks.load() > 0);
}
```

## Divergences

- **`stop_callback` is not implemented.** Its destructor has a real
  concurrent-correctness contract (must block if its callback is
  currently running on another thread, must not deadlock if destroyed
  from inside its own callback) -- genuinely the highest-risk,
  hardest-to-verify code this facility would need, deferred rather
  than rushed. `stop_token`/`stop_source`/`jthread` alone already cover
  cooperative cancellation's common case (a `jthread` polling
  `stop_requested()`); callback-based cancellation is rarer. Not a
  disclosed silent divergence -- attempting to use `bridge::stop_callback`
  simply fails to compile, the same loud, immediate way any genuinely
  absent symbol would.
- **`nostopstate_t`'s default constructor is `explicit`.** Matches
  real `std::nostopstate_t`, but is easy to miss: `stop_source src{};`
  cannot ambiguously resolve to the tag constructor.

See [ADR-0017](https://github.com/tokuchan/bridge/blob/master/docs/adr/0017-jthread-stop-token-truss-owns-the-class.md)
for the full fidelity scope and rationale.

## Passthrough

Deck's passthrough condition is a Detector-backed override, not a bare
Feature Test check: `BRIDGE_RIVETS_FEATURES_LIB_JTHREAD >= 201911L` is
always required, and `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN >= 201907L`
is honored normally *unless* libstdc++ is confirmed active
(`bridge::rivets::libstdcxx`), where the macro is never published even
though `stop_token` itself works -- confirmed by direct compiler probe
on GCC 13-15 and Clang 20, not assumed. `jthread` and `stop_token`
always select the same path together (one shared condition): a
callable expecting `bridge::stop_token` would be a genuine type
mismatch against real `std::jthread` if the two selections ever
disagreed.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH` | define | Whether `stop_token`/`stop_source`/`jthread` should pass through to the real `std::` types. A Detector-backed override, not a bare Feature Test check -- see docs/adr/0017-jthread-stop-token-truss-owns-the-class.md. | [`<thread>::BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `jthread` | class | A joining, cooperatively-cancellable thread, matching `std::jthread`. Unlike `std::thread`, automatically requests a stop and joins on destruction if still joinable -- never terminates the program for an un-joined, un-detached thread the way `std::thread`'s own destructor does. | [`<thread>::jthread`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `nostopstate` | variable | Polyfill companion to stop_token. | [`<thread>::nostopstate`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `nostopstate_t` | struct | Tag type selecting `stop_source`'s no-state constructor, matching `std::nostopstate_t`. | [`<thread>::nostopstate_t`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_source` | class | Owns (a share of) cancellation state and can request a stop, matching `std::stop_source`. | [`<thread>::stop_source`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_token` | class | A handle to shared cancellation state, matching `std::stop_token`. Default-constructed with no state at all (`stop_possible()` false); otherwise obtained from a `stop_source::get_token()`. | [`<thread>::stop_token`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `swap` | function | Swaps `a`'s and `b`'s thread and stop-state. | [`<thread>::swap`](https://en.cppreference.com/w/cpp/thread/jthread) |
<!-- BRIDGE-DOCS:END -->
