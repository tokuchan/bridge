add_executable(bridge_deck_tests cpp17/version_test.cpp cpp17/optional_test.cpp cpp17/expected_test.cpp
                                  cpp17/format_test.cpp cpp17/print_test.cpp cpp17/format_differential_test.cpp
                                  cpp17/span_test.cpp cpp17/jthread_test.cpp)
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
# there (docs/adr/0012). Includes the differential test comparing both
# formatter<T> specializations of a user-defined type against each
# engine directly, and bridge::format against real std::format for
# built-in types.
add_executable(bridge_deck_format_cpp20_tests cpp17/format_test.cpp cpp17/format_differential_test.cpp)
target_link_libraries(bridge_deck_format_cpp20_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_format_cpp20_tests PRIVATE cxx_std_20)
catch_discover_tests(bridge_deck_format_cpp20_tests)

# Same reasoning as bridge_deck_expected_cpp23_tests, for print's own
# passthrough path -- print crosses its real threshold at C++23, same
# as expected/optional (docs/adr/0012). Includes the differential test
# comparing bridge::print/bridge::truss::print output against real
# std::print/std::println directly.
add_executable(bridge_deck_print_cpp23_tests cpp17/print_test.cpp cpp17/print_differential_test.cpp)
target_link_libraries(bridge_deck_print_cpp23_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_print_cpp23_tests PRIVATE cxx_std_23)
catch_discover_tests(bridge_deck_print_cpp23_tests)

# Same reasoning as bridge_deck_format_cpp20_tests, for span's own
# passthrough path -- span crosses its real threshold at C++20, same
# as format (docs/adr/0015). Includes the differential test comparing
# bridge::truss::span against real std::span directly, and
# bridge::span against std::span for the passthrough alias itself.
add_executable(bridge_deck_span_cpp20_tests cpp17/span_test.cpp cpp17/span_differential_test.cpp)
target_link_libraries(bridge_deck_span_cpp20_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_span_cpp20_tests PRIVATE cxx_std_20)
catch_discover_tests(bridge_deck_span_cpp20_tests)

# Same reasoning as bridge_deck_span_cpp20_tests, for jthread/stop_token's
# own passthrough path -- crosses its real threshold at C++20, gated on
# BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH (docs/adr/0017's Detector-backed
# override), not a bare Feature Test. Includes the differential test
# comparing bridge::truss::jthread/stop_token against real
# std::jthread/std::stop_token directly.
add_executable(bridge_deck_jthread_cpp20_tests cpp17/jthread_test.cpp cpp17/jthread_differential_test.cpp)
target_link_libraries(bridge_deck_jthread_cpp20_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
target_compile_features(bridge_deck_jthread_cpp20_tests PRIVATE cxx_std_20)
catch_discover_tests(bridge_deck_jthread_cpp20_tests)
