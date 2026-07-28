/// @file libcxx.hpp
/// @brief This file will hold the libc++ (LLVM) STL implementation
///        version Detector. This Detector is not yet implemented.
///        This file documents the intended shape, so the design is
///        visible before the real work happens.
///
/// libc++'s version lives in `_LIBCPP_VERSION`. In modern releases,
/// this value is encoded as `major * 10000 + minor * 100 + patch`.
/// The exact encoding has shifted across libc++'s history. So this
/// needs verification against current LLVM source, rather than
/// assumption, when implemented. Which standard library is active is
/// independent of which compiler is in use: Clang can be paired with
/// either libc++ or libstdc++. So this Detector must not assume
/// libc++ just because `bridge::rivets::clang::version` is nonzero.
#pragma once
