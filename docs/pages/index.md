\mainpage Bridge

Bridge the gaps between C++ standards, compilers, and STL implementations:
a suite of header-only libraries providing modern C++ features on older
standards (down to C++17), passing through to the real standard-library
facility once each ecosystem actually supports it. See CONTEXT.md's
glossary (in the repository, not part of this generated site) for the
precise terms this documentation uses -- **Rivets**, **Truss**, **Deck**,
**Facility**, and so on -- and docs/adr/ for the design decisions behind
each one.

This page is organized like cppreference.com: a curated hierarchy below,
generated from docs/pages/registry.yaml's own facility groups
(docs/adr/0020-cppreference-mirrored-facility-groups.md), plus a flat,
auto-updated index of every documented facility further down for quick
lookup.

<!-- BRIDGE-DOCS:BEGIN groups -->
## Containers library

- \subpage page_span

## Concurrency support library

- \subpage page_jthread

## General utilities library

- \subpage page_expected
- \subpage page_format_print
- \subpage page_optional
- \subpage page_source_location

## Rivets detection library

\subpage page_rivets

## Bridge metadata library

- \subpage page_version
<!-- BRIDGE-DOCS:END -->

## See also

- \subpage page_ste_glossary

## Full facility index

<!-- BRIDGE-DOCS:BEGIN facilities -->
- \ref page_detectors "Detectors"
- \ref page_diagnostics "Diagnostics"
- \ref page_expected "Expected"
- \ref page_feature_tests "Feature Tests"
- \ref page_format_print "Format Print"
- \ref page_jthread "Jthread"
- \ref page_optional "Optional"
- \ref page_source_location "Source Location"
- \ref page_span "Span"
- \ref page_version "Version"
<!-- BRIDGE-DOCS:END -->
