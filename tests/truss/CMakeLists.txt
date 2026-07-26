add_executable(bridge_truss_tests cpp17/version_test.cpp cpp17/optional_test.cpp cpp17/expected_test.cpp)
target_link_libraries(bridge_truss_tests PRIVATE bridge::truss Catch2::Catch2WithMain)
catch_discover_tests(bridge_truss_tests)
