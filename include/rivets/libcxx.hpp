/// @file libcxx.hpp
/// @brief libc++ (LLVM) STL implementation version Detector. **Not yet
///        implemented** — this file documents the intended shape so
///        it's visible before the real work happens.
///
/// libc++'s version lives in `_LIBCPP_VERSION`, encoded (in modern
/// releases) as `major * 10000 + minor * 100 + patch`. The exact
/// encoding has shifted across libc++'s history, so this needs
/// verification against current LLVM source rather than assumption when
/// implemented. Independent of which compiler is in use — Clang can be
/// paired with either libc++ or libstdc++ — so this Detector must not
/// assume libc++ just because `bridge::rivets::clang::version` is
/// nonzero.
#pragma once
