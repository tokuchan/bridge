/// @file libstdcxx.hpp
/// @brief libstdc++ (GNU) STL implementation version Detector. **Not yet
///        implemented** — this file documents the intended shape so
///        it's visible before the real work happens.
///
/// libstdc++'s version lives in `_GLIBCXX_RELEASE` (added in GCC 7,
/// equal to the GCC major version that shipped that libstdc++ release —
/// simpler than the older `__GLIBCXX__` datestamp macro, which should
/// not be used for new code). Presence of `__GLIBCXX__` at all indicates
/// libstdc++ is the active standard library, which — per the entity
/// list in `CONTEXT.md` — is independent of which *compiler* is in use:
/// Clang can be paired with either libstdc++ or libc++, so this
/// Detector must not assume libstdc++ just because
/// `bridge::rivets::gcc::version` is nonzero.
#pragma once
