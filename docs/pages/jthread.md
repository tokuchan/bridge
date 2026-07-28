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

`bridge::jthread` is a joining, cooperatively-cancellable thread. It
matches C++20's `std::jthread`. Unlike `std::thread`, its destructor
requests a stop and joins automatically, when the thread is still
joinable. `std::thread`'s own destructor terminates the program in
that case instead.

`bridge::stop_token` and `bridge::stop_source` are the cancellation
primitives `jthread` is built on. `std::jthread` does not exist at
all before C++20. There is also no C++17 type for Truss to attach
free functions onto, the way `optional` has `std::optional`. So
Truss owns complete classes for all three: `jthread`, `stop_token`,
and `stop_source`. These classes are built directly on `std::thread`,
a real C++11 facility, plus `<atomic>`, `<mutex>`, and
`<condition_variable>`. This facility needs no platform-specific
code.

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
  concurrent-correctness contract. This destructor must block, when
  its callback is currently running on another thread. This
  destructor must not deadlock, when something destroys it from
  inside its own callback. This is genuinely the highest-risk,
  hardest-to-verify code this facility would need. This facility
  defers this work, rather than rushing it. `stop_token`,
  `stop_source`, and `jthread` alone already cover cooperative
  cancellation's common case: a `jthread` that polls
  `stop_requested()`. Callback-based cancellation is rarer. This is
  not a disclosed silent divergence. Code that tries to use
  `bridge::stop_callback` simply fails to compile, the same loud,
  immediate way any genuinely absent symbol would.
- **`nostopstate_t`'s default constructor is `explicit`.** This
  matches real `std::nostopstate_t`. This is easy to miss:
  `stop_source src{};` cannot ambiguously resolve to the tag
  constructor.

See [ADR-0017](https://github.com/tokuchan/bridge/blob/master/docs/adr/0017-jthread-stop-token-truss-owns-the-class.md)
for the full fidelity scope and rationale.

## Passthrough

Deck's passthrough choice is a Detector-backed override, not a bare
Feature Test check. `BRIDGE_RIVETS_FEATURES_LIB_JTHREAD >= 201911L`
is always required. `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN >= 201907L`
is honored normally, unless libstdc++ is confirmed active
(`bridge::rivets::libstdcxx`). When libstdc++ is active, this facility
never sees `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN` published, even
though `stop_token` itself works. A direct compiler probe on GCC 13-15
and Clang 20 confirmed this.

`jthread` and `stop_token` always select the same path together, from
one shared condition. If the two selections ever disagreed, a
callable expecting `bridge::stop_token` would be a genuine type
mismatch against real `std::jthread`.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH` | define | This macro tells you whether `stop_token`, `stop_source`, and `jthread` should pass through to the real `std::` types. This macro is a Detector-backed override, not a bare Feature Test check. See docs/adr/0017-jthread-stop-token-truss-owns-the-class.md. | [`<thread>::BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `jthread` | class | This class is a joining, cooperatively-cancellable thread. This class matches `std::jthread`. | [`<thread>::jthread`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `nostopstate` | variable | This is the polyfill companion to stop_token. | [`<thread>::nostopstate`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `nostopstate_t` | struct | This tag type selects `stop_source`'s no-state constructor. This tag type matches `std::nostopstate_t`. | [`<thread>::nostopstate_t`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_source` | class | This class owns a share of cancellation state. This class can request a stop. This class matches `std::stop_source`. | [`<thread>::stop_source`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_token` | class | This class is a handle to shared cancellation state. This class matches `std::stop_token`. | [`<thread>::stop_token`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `swap` | function | This swaps `a`'s and `b`'s thread and stop-state. | [`<thread>::swap`](https://en.cppreference.com/w/cpp/thread/jthread) |
<!-- BRIDGE-DOCS:END -->
