# Facility Groups Mirror cppreference's Own Top-Level Categories

## Context

`docs/pages/index.md` already claimed to be "organized like
cppreference.com," but its actual top-level hierarchy grouped
facilities by bridge's own architecture layers instead (`Rivets --
detection` / `Truss/Deck -- polyfilled facilities` / `Everything
else`), not by any real cppreference category. That hierarchy was also
hand-written and had already drifted stale: `span`, `jthread`, and
`source_location` existed as pages but were never `\subpage`d from any
of those three sections, reachable only through the flat, auto-generated
"Full facility index" further down the same page.

Confirmed against cppreference directly, not assumed from memory: the
real top-level categories relevant to bridge's current facilities are
"Containers library," "Concurrency support library" (the actual page
title -- the standard-library overview page's own prose calls the same
thing "Thread support library," but a reader clicking through lands on
the page titled "Concurrency support library"), and "General utilities
library." The last of these is broad: it covers `std::optional`/
`std::expected` under its own "Sum types and type-erased wrappers"
subcategory, and both `std::source_location` and the library
feature-test macros under its "Language support" subcategory. Also
confirmed: `std::format` lives under Utility library, but
`std::print`/`std::println` live under a *different* top-level category
entirely, Input/output library -- and Doxygen's `\subpage` command
allows exactly one parent per page (a hard crash, not a warning, if
violated -- see [ADR-0014](0014-documentation-site-architecture.md)),
so bridge's single `format-print` page genuinely cannot `\subpage`
under two top-level headings at once.

## Decision

### Registry-driven, generated grouping

`docs/pages/registry.yaml` gains a top-level `groups:` list (`id`,
`name`, optional `overview_page`) and a `group:` field on every
facility. `scripts/docs-pages.py` generates the mainpage's top-level
hierarchy from this data into a new `<!-- BRIDGE-DOCS:BEGIN groups
-->` marker block in `docs/pages/index.md`, the same regenerate-in-place
pattern every other machine-owned block already uses. This was chosen
over keeping the hierarchy hand-written: every other generated block in
this project exists specifically because a hand-maintained equivalent
"drifts silently... over time" (ADR-0014's own words), and the
just-replaced hierarchy is direct proof of exactly that failure mode.

### Top-level categories only, not cppreference's own subcategories

The registry's groups use cppreference's top-level category names only
-- not its second-level subcategories (e.g. "Sum types and type-erased
wrappers," "Language support"), even where those would give more
precise, less crowded buckets. A single "General utilities library"
group holding `optional`, `expected`, `source_location`, and
`format-print` honestly reflects where the real library keeps them, and
keeps the group list small and low-maintenance for bridge's current
facility count -- the more granular subcategory split is a real
alternative, revisitable if bridge's own catalog grows enough that one
group holding most of its facilities stops being useful.

### `format-print`'s split is a disclosed, single-value compromise

`format-print` is assigned `group: utility`, matching `format`'s own
category rather than `print`'s. This mirrors the exact shape of
compromise ADR-0014 already made for this same facility's single
`cppreference` URL (pointed at `format`'s page, not `print`'s, with the
simplification disclosed in the page's own prose): one value, not a
list, with the gap called out rather than silently chosen. A
list-valued `group` field was considered and rejected for the same
reason ADR-0014 rejected a list-valued `cppreference` field: it reopens
exactly the "which row/heading gets which value" complexity a
single-value field with a disclosed compromise avoids.

### Rivets' three facilities stay together as one bridge-only group

`detectors`, `feature-tests`, and `diagnostics` are grouped together
under `rivets` (display name "Rivets detection library"), even though
`feature-tests` alone has a real cppreference mapping
(`cpp/feature_test`, under Utility library's own "Language support"
subcategory) that `detectors` and `diagnostics` (both `cppreference:
null`) don't share. Splitting `feature-tests` out into Utility library
would break `rivets.md`'s existing "sparse Cartesian product" narrative
across two different top-level sections, for a consistency gain
("group strictly by cppreference mapping") that costs more in
coherence than it buys in taxonomic purity -- these three facilities
are one conceptual unit in bridge's own architecture first, and only
incidentally individually mappable (or not) onto cppreference's own
taxonomy.

### Bridge-only groups follow cppreference's "X library" naming

Groups with no real cppreference equivalent (`rivets`, `bridge-metadata`)
still get an "X library"-styled display name ("Rivets detection
library," "Bridge metadata library"), not bridge's bare existing
terminology ("Rivets"). This reads naturally alongside real category
names in the same generated list, matching the stated goal of
presenting a bridge-only group with the same structural weight as a
real cppreference one, "pretending it's just like the cppreference
groups."

### `version` gets its own bridge-only group, separate from Rivets

`version` also has `cppreference: null`, but it's a Truss/Deck facility
(C++ standard/compiler version metadata), architecturally unrelated to
Rivets' detection concepts. It gets its own group (`bridge-metadata`)
rather than being folded into `rivets` under a looser "anything with no
cppreference mapping" definition -- keeping each group semantically
honest establishes the right pattern for any future non-Rivets facility
that also happens to have no real cppreference counterpart.

### The registry supports an optional per-group overview page

A group may declare `overview_page` (only `rivets` does today, pointing
at `rivets.md`). When set, the generated hierarchy `\subpage`s that one
hand-written page under the group's heading, rather than `\subpage`ing
every member facility directly -- `rivets.md` is already the sole
`\subpage` parent of `detectors`/`feature-tests`/`diagnostics`
(confirmed by reading it directly: it already `\subpage`s all three
itself), and a second `\subpage` reference to any of them from the new
groups block would recreate the exact two-parent crash ADR-0014
describes hitting once before. A group with no `overview_page`
`\subpage`s its member facilities directly instead, sorted
alphabetically -- matching `render_facility_index_block`'s own existing
ordering, for the same determinism reason.

### No CONTEXT.md entry

CONTEXT.md is glossary-only, by its own established convention (terms
and their meanings, nothing else). "Which cppreference category a
facility belongs to" is a documentation-site organizational decision,
not a term definition -- this ADR is the sole record of it.

## Consequences

- Every future facility needs a `group:` value in its registry entry
  alongside everything ADR-0014 already requires (`page`, `headers`,
  `cppreference`, `header`, `example_headers`) -- `./bridge docs` fails
  loudly (same severity as an unassigned header) if a facility
  references an unknown group id, or if a declared group ends up used
  by no facility at all.
- Adding bridge's next few backlogged facilities (`ranges`+format
  support, `flat_map`/`flat_set`, `move_only_function`) will grow
  existing groups (`containers` gains `flat_map`/`flat_set`; `utility`
  likely gains `move_only_function`) rather than needing new ones --
  expected, and not itself a signal anything needs rebalancing.
- A future facility that itself spans two real cppreference categories
  will hit the same `format-print`-shaped choice: pick one value,
  disclose the compromise in the page's own prose, don't reach for a
  list-valued `group` field.
- `docs/pages/index.md`'s hierarchy is now fully generated between its
  hand-written intro paragraph and its hand-written "See also"/"Full
  facility index" sections -- a contributor editing the mainpage's
  category structure now edits `docs/pages/registry.yaml`, not
  `index.md` directly.
