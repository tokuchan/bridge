cmake_minimum_required(VERSION 3.20)
# See docs/adr/0005-calver-versioning.md. 0.0.0 until the first release is
# cut; CHANGELOG.md's [Unreleased] section is the current in-progress state.
project(bridge VERSION 0.0.0 LANGUAGES CXX)

option(BRIDGE_BUILD_TESTS "Build the Catch2 test suite" ON)
option(BRIDGE_BUILD_DOCS "Build Doxygen documentation" OFF)
set(BRIDGE_WITH_BOOST "AUTO" CACHE STRING "Use Boost where available: ON, OFF, or AUTO")
set_property(CACHE BRIDGE_WITH_BOOST PROPERTY STRINGS AUTO ON OFF)

# See docs/adr/0002-boost-optional-feature-detected.md.
if(NOT BRIDGE_WITH_BOOST STREQUAL "OFF")
    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 NEW) # use upstream BoostConfig.cmake, not CMake's FindBoost
    endif()
    find_package(Boost QUIET)
    if(Boost_FOUND)
        set(BRIDGE_HAS_BOOST ON)
    elseif(BRIDGE_WITH_BOOST STREQUAL "ON")
        message(FATAL_ERROR "BRIDGE_WITH_BOOST=ON but Boost was not found")
    else()
        set(BRIDGE_HAS_BOOST OFF)
    endif()
else()
    set(BRIDGE_HAS_BOOST OFF)
endif()

add_subdirectory(include/rivets)
add_subdirectory(include/truss)
add_subdirectory(include/deck)

if(BRIDGE_BUILD_TESTS)
    enable_testing()
    # See docs/adr/0003-nix-provided-build-tooling.md: Catch2 comes from the
    # Nix devShell, not FetchContent.
    find_package(Catch2 3 REQUIRED)
    include(Catch)
    add_subdirectory(tests/rivets)
    add_subdirectory(tests/truss)
    add_subdirectory(tests/deck)
endif()

if(BRIDGE_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
endif()
