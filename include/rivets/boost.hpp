/// @file boost.hpp
/// @brief This file will hold the Boost version Detector. This
///        Detector is not yet implemented. This file documents the
///        intended shape, so the design is visible before the real
///        work happens.
///
/// Boost's version lives in `BOOST_VERSION`, from
/// `<boost/version.hpp>` or any other Boost header. `BOOST_VERSION`
/// is only defined once code has actually included something from
/// Boost. `BOOST_VERSION` is encoded as
/// `major * 100000 + minor * 100 + patch`.
///
/// Unlike GCC and Clang, Boost is the one Entity whose number is
/// naturally two-component: major and minor, for example `ge_1_87`.
/// This matches how people actually discuss Boost versions, rather
/// than a single major-version int. See
/// docs/adr/0006-detector-naming-calculus.md. Layer 1 here will need
/// `ge(int major, int minor)`-shaped comparators, not the
/// single-`int` shape every other entity uses. Layer 2 will need a
/// distinct `BRIDGE_RIVETS_DEFINE_DETECTOR_MM` generator to match.
/// Neither exists yet.
///
/// Per docs/adr/0002-boost-optional-feature-detected.md, Boost is
/// optional project-wide. This header must not require a Boost
/// installation, to be `#include`d safely. The real implementation
/// needs to gate the seed on `__has_include(<boost/version.hpp>)`,
/// or an equivalent CMake-injected signal reaching the `rivets`
/// target. This signal does not currently propagate
/// `BRIDGE_HAS_BOOST` the way `truss` and `deck` do; that
/// propagation is itself part of the unfinished work here. The seed
/// must be `0` when Boost is not present, exactly like
/// `bridge::rivets::gcc::version` seeds to `0` under a non-GCC
/// compiler.
#pragma once
