\page page_jthread Jthread

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<thread>`](https://en.cppreference.com/w/cpp/thread/jthread) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/stop_token.hpp`
<!-- BRIDGE-DOCS:END -->

TODO: narrative prose for this facility.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `nostopstate` | variable | The canonical `nostopstate_t` instance, matching `std::nostopstate`. | [`<thread>::nostopstate`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `nostopstate_t` | struct | Tag type selecting `stop_source`'s no-state constructor, matching `std::nostopstate_t`. | [`<thread>::nostopstate_t`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_source` | class | Owns (a share of) cancellation state and can request a stop, matching `std::stop_source`. | [`<thread>::stop_source`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `stop_token` | class | A handle to shared cancellation state, matching `std::stop_token`. Default-constructed with no state at all (`stop_possible()` false); otherwise obtained from a `stop_source::get_token()`. | [`<thread>::stop_token`](https://en.cppreference.com/w/cpp/thread/jthread) |
| `swap` | function | Swaps `a`'s and `b`'s stop-state. | [`<thread>::swap`](https://en.cppreference.com/w/cpp/thread/jthread) |
<!-- BRIDGE-DOCS:END -->
