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
plus a flat, auto-updated index of every documented facility further
down for quick lookup.

## Rivets -- detection

\subpage page_rivets "Rivets: the sparse Cartesian product" ties
together Rivets' three facilities (\ref page_detectors, \ref
page_feature_tests, \ref page_diagnostics -- linked from there, not
`\subpage`d a second time here, since a page can only have one
`\subpage` parent).

## Truss/Deck -- polyfilled facilities

Each of these is one conceptual division spanning however many headers
it actually takes (docs/adr/0014-documentation-site-architecture.md) --
not necessarily one page per header:

- \subpage page_optional
- \subpage page_expected
- \subpage page_format_print

## Everything else

- \subpage page_version
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
- \ref page_span "Span"
- \ref page_version "Version"
<!-- BRIDGE-DOCS:END -->
