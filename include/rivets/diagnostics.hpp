/// @file diagnostics.hpp
/// @brief `BRIDGE_RIVETS_DIVERGENCE_NOTE`: a compiler-visible note for
///        points where a Truss-owned polyfill's behavior diverges from
///        the real standard facility it backports, in a way a user
///        could reasonably be surprised by. See
///        docs/adr/0011-warn-on-surprising-facility-divergences.md for
///        the policy this exists to serve, and why a doc comment or
///        ADR entry alone isn't enough — those require the user to
///        already be reading them at the moment they'd help.
#pragma once

/// @def BRIDGE_RIVETS_PRAGMA
/// @brief Turns a token sequence into a `_Pragma` string-literal
///        argument. Exposition-only: use @ref BRIDGE_RIVETS_DIVERGENCE_NOTE
///        directly instead.
#define BRIDGE_RIVETS_PRAGMA(x) _Pragma(#x)

/// @def BRIDGE_RIVETS_DIVERGENCE_NOTE
/// @brief Emits `#pragma message` with `msg` at the point of expansion.
///
///        `msg` must be a single string-literal argument (e.g.
///        `BRIDGE_RIVETS_DIVERGENCE_NOTE("some message")`) — passing
///        unquoted, comma-containing tokens splits across multiple
///        macro arguments and fails to compile, confirmed by hitting
///        exactly that preprocessor error before settling on this
///        shape, not assumed.
///
///        Place this where the *diverging* code path is actually
///        selected for a given translation unit (e.g. inside a Deck
///        header's polyfill branch), not unconditionally inside
///        Truss's own header — Truss's header is typically included
///        regardless of which path Deck ultimately selects, and the
///        note is only relevant to a consumer who's actually on the
///        polyfill path.
///
///        Verified empirically that this doesn't break a `-Werror`
///        build on either GCC (where `#pragma message` is a note, not
///        a warning, and so is immune to `-Werror` by construction) or
///        Clang (categorized as a warning under `-W#pragma-messages`,
///        but did not trigger `-Werror` in practice as tested).
#define BRIDGE_RIVETS_DIVERGENCE_NOTE(msg) BRIDGE_RIVETS_PRAGMA(message msg)
