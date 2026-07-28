/// @file diagnostics.hpp
/// @brief `BRIDGE_RIVETS_DIVERGENCE_NOTE` is a compiler-visible note.
///        It marks a point where a Truss polyfill's behavior diverges
///        from the real standard facility it backports, in a way that
///        could surprise a user.
///
///        See docs/adr/0011-warn-on-surprising-facility-divergences.md
///        for the policy behind this macro. A doc comment or an ADR
///        entry alone is not enough. Both need the user to already be
///        reading them at the right moment. This note does not need
///        that: the compiler shows it directly.
#pragma once

/// @def BRIDGE_RIVETS_PRAGMA
/// @brief This macro turns a token sequence into a `_Pragma`
///        string-literal argument.
///
///        Do not use this macro directly. Use @ref
///        BRIDGE_RIVETS_DIVERGENCE_NOTE instead. This macro exists
///        only to explain how @ref BRIDGE_RIVETS_DIVERGENCE_NOTE
///        works.
#define BRIDGE_RIVETS_PRAGMA(x) _Pragma(#x)

/// @def BRIDGE_RIVETS_DIVERGENCE_NOTE
/// @brief This macro emits `#pragma message` with `msg`, at the point
///        where this macro expands.
///
///        `msg` must be one string-literal argument, for example
///        `BRIDGE_RIVETS_DIVERGENCE_NOTE("some message")`. A message
///        with an unquoted comma splits across more than one macro
///        argument, and fails to compile.
///
///        Place this macro where the diverging code path is actually
///        selected for a given translation unit, for example inside a
///        Deck header's polyfill branch. Do not place this macro
///        unconditionally inside Truss's own header. Truss's header
///        is usually included on every path, whether Deck selects the
///        polyfill or not. This note only matters to a user who is
///        actually on the polyfill path.
///
///        This macro does not break a `-Werror` build. On GCC,
///        `#pragma message` is a note, not a warning, so `-Werror`
///        does not apply to it. On Clang, `#pragma message` is a
///        warning under `-W#pragma-messages`, but it does not trigger
///        `-Werror`.
#define BRIDGE_RIVETS_DIVERGENCE_NOTE(msg) BRIDGE_RIVETS_PRAGMA(message msg)
