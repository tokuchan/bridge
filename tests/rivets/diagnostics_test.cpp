#include <catch2/catch_test_macros.hpp>

#include <rivets/diagnostics.hpp>

// BRIDGE_RIVETS_DIVERGENCE_NOTE's real "test" is that this file compiles
// at all -- it expands to a #pragma message, which has no runtime
// behavior to assert on. A broken macro shape (e.g. accepting an
// unquoted, comma-containing message) fails to compile, not fails a
// REQUIRE; this file compiling successfully, with the note visible in
// the build log, is the verification. See
// docs/adr/0011-warn-on-surprising-facility-divergences.md.
BRIDGE_RIVETS_DIVERGENCE_NOTE("bridge: this is a test divergence note from diagnostics_test.cpp, ignore")

TEST_CASE("BRIDGE_RIVETS_DIVERGENCE_NOTE compiles cleanly", "[rivets][diagnostics]") {
    SUCCEED();
}
