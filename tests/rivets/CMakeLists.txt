add_executable(bridge_rivets_tests standard_test.cpp gcc_test.cpp clang_test.cpp features_test.cpp)
target_link_libraries(bridge_rivets_tests PRIVATE bridge::rivets Catch2::Catch2WithMain)
catch_discover_tests(bridge_rivets_tests)
