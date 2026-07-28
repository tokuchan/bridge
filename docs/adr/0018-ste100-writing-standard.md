# The Manual Follows ASD-STE100 (Simplified Technical English)

## Context

Bridge's facility pages (`docs/pages/*.md`) and source `@brief`/
`@details` comments are the manual: the prose a consumer reads to
understand and use a facility, as distinct from ADRs (why the project
made a decision) or `CONTEXT.md` (the glossary of bridge's own
domain terms). That manual's prose has, until now, followed no
consistent grammar or vocabulary discipline beyond this session's
brief-concision pass and the Synopsis/Example/Divergences/Passthrough
template (docs/adr/0014's amendment) -- both about structure and
length, not about sentence-level clarity or word choice.

ASD-STE100 (the aerospace industry's Simplified Technical English
standard, used for aircraft maintenance manuals) exists to solve
exactly this: it defines a small set of writing rules (short simple
sentences, active voice, one fact per sentence, one approved meaning
per word) plus a controlled vocabulary of approximately 900 general
words. The user chose to adopt it for bridge's own manual, wanting the
same discipline applied to a very different domain.

## Decision

### Full vocabulary compliance, not just the grammar rules

Bridge adopts both ASD-STE100's writing rules and its approved-word
vocabulary, not just the grammar/style discipline loosely applied.
The standard's own dictionary is aerospace-maintenance-specific (it
has no entry for `optional`, `template`, `namespace`, or any other
C++/programming term) and cannot cover bridge's own domain directly.
ASD-STE100 anticipates exactly this gap: it permits a **technical name**
list, specific to the equipment or domain the manual covers, each name
pinned to one meaning and used consistently everywhere it appears.
`docs/pages/ste-glossary.md` (see below) is that list for bridge.

### Scope: facility pages and source comments, not ADRs

In scope:

- `docs/pages/<facility>.md`'s hand-written prose (Synopsis, Example
  narration, Divergences, Passthrough).
- Every promoted symbol's `@brief`/`@details` Doxygen comment in
  `include/{truss,deck}/cpp17/*.hpp` and Rivets' own headers.

Explicitly out of scope:

- **ADRs** (`docs/adr/*.md`). This project's ADRs are deliberately
  rationale-heavy and evidence-citing (`confirmed via probe, not
  assumed`, cross-references to other ADRs, discussion of rejected
  alternatives) -- a real, accepted tension with STE's simple-
  declarative style. Converting ADR prose to STE would strip exactly
  the nuance an ADR exists to preserve. Left alone, not fought.
- **`CONTEXT.md`**. Already has its own established, terser format
  (term, one-paragraph definition, `_Avoid_` line) that predates this
  decision and serves a different purpose (a glossary of bridge's own
  domain concepts, not a manual).
- **`CHANGELOG.md`**. Follows Keep a Changelog conventions already;
  STE's style would be a real departure from that established format
  for no corresponding benefit -- a changelog entry is read once, at
  release time, not consulted repeatedly as reference material.

### Vocabulary collisions: rewrite first, exception only if that fails

A word ASD-STE100 approves with one specific meaning sometimes already
has an established, different meaning in C++ or in bridge's own prose
(a real risk, given the dictionary wasn't designed for software). The
default response is to rewrite the sentence to avoid the collision
entirely -- STE's whole premise is that almost anything can be said
plainly if a writer is willing to restructure the sentence. Only when
a workable rewrite genuinely isn't available does this stop for an
explicit, one-off exception, decided with the user and recorded in
`docs/pages/ste-glossary.md` next to the term it exempts, along with
the reason. Exceptions are expected to be rare; if they aren't, that's
itself a signal this decision needs revisiting.

### Enforcement stays a manual convention

No checker script, no CI gate. Followed by hand during writing and
review, the same way this project's other prose conventions (the
brief-concision pass, the page template) already work without
tooling. A mechanical STE checker would need to verify grammar rules
(one fact per sentence, passive-voice detection, sentence length) that
are considerably harder to check automatically than a word list --
building and maintaining one isn't justified for this project's scale.

### The glossary: `docs/pages/ste-glossary.md`, a living page

A new, standalone page, not embedded in this ADR -- an ADR records the
*decision*, not an ever-growing word list that changes every time a
new facility is converted and a new technical term comes up. It's a
hand-written page like `docs/pages/index.md`/`rivets.md`: part of the
generated doc site (`docs/pages` is already in Doxygen's `INPUT`), but
not a registry-tracked facility, since it owns no headers of its own.
Records, per technical name: the one approved meaning/part of speech,
and any exception granted under the collision process above.

### Migration: incremental, one facility at a time

Matches this session's own doc-site-rewrite precedent: each commit
converts one facility's `docs/pages/<name>.md` prose and its header(s)'
`@brief`/`@details` comments together, and passes the full gate
(`./bridge test`, `./bridge docs`) on its own. `optional` goes first --
the smallest, simplest facility (4 symbols, minimal Divergences) --
specifically to prove out the glossary-building and exception process
before repeating it across the remaining seven facilities.

## Consequences

- Bridge's manual will read noticeably more repetitively than its
  current voice during the transition, and permanently once fully
  converted: STE deliberately favors several short, plain sentences
  over one nuanced compound one. Accepted as the actual cost of
  unambiguous, non-native-English-reader-friendly prose, not a
  regrettable side effect.
- ADRs and the manual will diverge stylistically going forward -- a
  reader moving from a facility page (STE) to the ADR it links to
  (this project's existing voice) will notice the shift. Disclosed
  here rather than silently inconsistent.
- Every future facility needs its `docs/pages/<name>.md` and header
  comments written in STE from the start, not just the eight that
  exist today -- this joins the existing "read every ADR before
  committing" `CLAUDE.md` check as something to verify.
- `docs/pages/ste-glossary.md` needs to stay current as new technical
  terms and exceptions come up; a term used in a facility page or
  source comment that isn't in the glossary and isn't a plain STE-
  approved word is itself a sign the glossary is out of date.
