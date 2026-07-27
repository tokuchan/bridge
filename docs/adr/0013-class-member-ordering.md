# Class Member Ordering: Type Aliases, Then Data, Then Everything Else

## Context

C++ classes conventionally list their public interface first
(constructors, then public methods) with data members tucked away at
the bottom, often under a `private:` section a reader has to scroll
past the whole interface to reach. That ordering optimizes for "what
can I call on this," at the cost of "what does this actually hold" —
and the latter is frequently the faster way to build a correct mental
model of what a class does and how its pieces relate, especially for
the kind of small, focused types this codebase tends to produce
(`format_context`, `parsed_std_spec`, `counted_output_iterator`, and
so on).

## Decision

Order class/struct members as:

1. **Public type aliases** (`using`/typedefs) — the vocabulary the
   rest of the class is defined in terms of.
2. **Private data members** — what the class actually holds.
3. **Everything else** — constructors, public methods, private
   methods — in whatever order reads best for that particular class.

Rationale: data held by a class usually communicates what it *does*
more directly and more quickly than its function signatures do.
Putting type aliases and data members up front means a reader forms
that picture before wading into behavior, rather than after.

This is a **documentation/style convention**, not something a compiler
or a new gate enforces — there's no linter check backing it, and it
isn't being retrofitted onto every existing class in one sweep.
Existing classes get reordered opportunistically (next time they're
touched for an unrelated reason), not as a dedicated repo-wide pass —
a mechanical reorder of every class in the codebase is a separate,
riskier undertaking nobody asked for here, and the risk/reward doesn't
favor doing it as a single disruptive commit.

## Consequences

- New classes/structs follow this ordering from the start.
- Code review is the enforcement mechanism, not tooling — same as
  most of this project's other prose-level style conventions.
- A reader encountering an older class that doesn't follow this
  ordering shouldn't read anything into it beyond "written before this
  ADR" — it's not a signal of a different, deliberate design choice
  for that specific class.
