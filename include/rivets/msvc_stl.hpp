/// @file msvc_stl.hpp
/// @brief This file will hold the MSVC STL implementation version
///        Detector. This Detector is not yet implemented. This file
///        documents the intended shape, so the design is visible
///        before the real work happens.
///
/// In modern releases, the MSVC STL's version lives in
/// `_MSVC_STL_VERSION`, plus `_MSVC_STL_UPDATE` for finer
/// granularity. Which standard library is active is independent of
/// which compiler is in use: `clang-cl` can be paired with the MSVC
/// STL. So this Detector must not assume the MSVC STL just because
/// `bridge::rivets::msvc::version` is nonzero. This mirrors the same
/// compiler/STL independence noted in `libstdcxx.hpp` and
/// `libcxx.hpp`.
#pragma once
