/// @file msvc_stl.hpp
/// @brief MSVC STL implementation version Detector. **Not yet
///        implemented** — this file documents the intended shape so
///        it's visible before the real work happens.
///
/// The MSVC STL's version lives in `_MSVC_STL_VERSION` (and
/// `_MSVC_STL_UPDATE` for finer granularity) in modern releases.
/// Independent of which compiler is in use — `clang-cl` can be paired
/// with the MSVC STL — so this Detector must not assume the MSVC STL
/// just because `bridge::rivets::msvc::version` is nonzero, mirroring
/// the same compiler/STL independence noted in `libstdcxx.hpp` and
/// `libcxx.hpp`.
#pragma once
