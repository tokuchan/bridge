add_executable(bridge_deck_tests cpp17/version_test.cpp cpp17/optional_test.cpp cpp17/expected_test.cpp
                                  cpp17/format_test.cpp)
target_link_libraries(bridge_deck_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
catch_discover_tests(bridge_deck_tests)

# Dedicated C++23 build of the optional tests to exercise the real
# std::optional passthrough path (docs/adr/0008-best-effort-head-
# standard.md). The default build above compiles at each toolchain's
# default mode, which -- verified empirically against this project's
# compiler matrix floor, not assumed -- predates native monadic optional
# support, so without this target the passthrough branch would never
# actually be compiled or tested.
add_executable(bridge_deck_optional_cpp23_tests cpp17/optional_test.cpp)
target_link_libraries(bridge_deck_optional_cpp23_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_optional_cpp23_tests PRIVATE cxx_std_23)
catch_discover_tests(bridge_deck_optional_cpp23_tests)

# Same reasoning as bridge_deck_optional_cpp23_tests, for expected's own
# passthrough path -- plus the differential trait test (docs/adr/0010),
# which specifically needs std::expected to exist in this translation
# unit alongside bridge::truss::expected to compare them directly.
add_executable(bridge_deck_expected_cpp23_tests cpp17/expected_test.cpp cpp17/expected_differential_test.cpp)
target_link_libraries(bridge_deck_expected_cpp23_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_expected_cpp23_tests PRIVATE cxx_std_23)
catch_discover_tests(bridge_deck_expected_cpp23_tests)

# Same reasoning as bridge_deck_optional_cpp23_tests, for format's own
# passthrough path -- forced to C++20 specifically, not C++23, since
# format (unlike expected/optional/print) crosses its real threshold
# there (docs/adr/0012).
add_executable(bridge_deck_format_cpp20_tests cpp17/format_test.cpp)
target_link_libraries(bridge_deck_format_cpp20_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_format_cpp20_tests PRIVATE cxx_std_20)
catch_discover_tests(bridge_deck_format_cpp20_tests)
