/// @file msvc.hpp
/// @brief This file will hold the MSVC compiler version Detector.
///        This Detector is not yet implemented. This file documents
///        the intended shape, so the design is visible before the
///        real work happens.
///
/// MSVC's version lives in `_MSC_VER`. Unlike `__GNUC__` and
/// `__clang_major__`, `_MSC_VER` is not a clean major-version int.
/// `_MSC_VER` encodes a specific toolset build, for example `1939`
/// for one particular VS2022 update. Real-world code conventionally
/// compares `_MSC_VER` directly against known thresholds, for
/// example `1920` for VS2019 and `1930` for VS2022's first release,
/// rather than extracting a "major version" the way GCC and Clang
/// detection does.
///
/// Before implementing this Detector, decide whether `n` in
/// `bridge::rivets::msvc::ge(n)` means the raw `_MSC_VER` value
/// directly, or some translated "toolset major version" more
/// analogous to the other compiler Entities. Record this decision in
/// an ADR if the answer is not obvious in hindsight. The two options
/// give very different-looking call sites. The choice is not forced
/// the way it was for GCC and Clang.
#pragma once
