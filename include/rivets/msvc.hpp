/// @file msvc.hpp
/// @brief MSVC compiler version Detector. **Not yet implemented** — this
///        file documents the intended shape so it's visible before the
///        real work happens.
///
/// MSVC's version lives in `_MSC_VER`, but unlike `__GNUC__`/
/// `__clang_major__` it isn't a clean major-version int — it encodes a
/// specific toolset build (e.g. `1939` for one particular VS2022
/// update), and real-world code conventionally compares it directly
/// against known thresholds (`1920` for VS2019, `1930` for VS2022's
/// first release, etc.) rather than extracting a "major version" the
/// way GCC/Clang detection does.
///
/// Before implementing this Detector, decide (and record in an ADR if
/// the answer isn't obvious in hindsight) whether `n` in
/// `bridge::rivets::msvc::ge(n)` means the raw `_MSC_VER` value directly,
/// or some translated "toolset major version" more analogous to the
/// other compiler Entities — the two give very different-looking call
/// sites and the choice isn't forced the way it was for GCC/Clang.
#pragma once
