/// @file boost.hpp
/// @brief Boost version Detector. **Not yet implemented** — this file
///        documents the intended shape so it's visible before the real
///        work happens.
///
/// Boost's version lives in `BOOST_VERSION` (from `<boost/version.hpp>`,
/// or any other Boost header — it's only defined once something from
/// Boost has actually been included), encoded as
/// `major * 100000 + minor * 100 + patch`.
///
/// Unlike GCC/Clang, Boost is the one Entity whose number is naturally
/// two-component (major.minor, e.g. `ge_1_87`, matching how Boost
/// versions are actually discussed) rather than a single major-version
/// int — see docs/adr/0006-detector-naming-calculus.md. Layer 1 here
/// will need `ge(int major, int minor)`-shaped comparators (not the
/// single-`int` shape every other entity uses), and Layer 2 will need a
/// distinct `BRIDGE_RIVETS_DEFINE_DETECTOR_MM` generator to match,
/// neither of which exist yet.
///
/// Per docs/adr/0002-boost-optional-feature-detected.md, Boost is
/// optional project-wide — this header must not require a Boost
/// installation to be `#include`d safely. The real implementation needs
/// to gate the seed on `__has_include(<boost/version.hpp>)` (or an
/// equivalent CMake-injected signal reaching the `rivets` target, which
/// doesn't currently propagate `BRIDGE_HAS_BOOST` the way `truss`/`deck`
/// do — that propagation is itself part of the unfinished work here),
/// seeding to `0` when Boost isn't present, exactly like
/// `bridge::rivets::gcc::version` seeds to `0` under a non-GCC compiler.
#pragma once
