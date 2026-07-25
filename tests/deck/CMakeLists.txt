add_executable(bridge_deck_tests cpp17/version_test.cpp)
target_link_libraries(bridge_deck_tests PRIVATE bridge::deck Catch2::Catch2WithMain)
catch_discover_tests(bridge_deck_tests)
