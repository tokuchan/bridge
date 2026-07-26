cmake_minimum_required(VERSION 3.20)
# See docs/adr/0005-calver-versioning.md. 0.0.0 until the first release is
# cut; CHANGELOG.md's [Unreleased] section is the current in-progress state.
project(bridge VERSION 0.0.0 LANGUAGES CXX)

# CMAKE_SOURCE_DIR is fixed to the *outermost* project's root for the
# whole cmake run; when bridge is pulled in via FetchContent/
# add_subdirectory (see cmake/FetchBridge.cmake), it means the
# consumer's root, not bridge's own -- confirmed by hitting a real
# "header not found" error via a FetchContent-based consumer before
# fixing this, not assumed. CMAKE_CURRENT_SOURCE_DIR, evaluated here at
# bridge's own top-level listfile, is bridge's own root in both cases.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BRIDGE_IS_TOP_LEVEL ON)
else()
    set(BRIDGE_IS_TOP_LEVEL OFF)
endif()
set(BRIDGE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Tests/install/packaging default off when bridge is just a dependency of
# a bigger project -- a consumer pulling bridge in via FetchContent
# shouldn't get bridge's own Catch2 suite built, or bridge's install()
# rules competing with the outer project's, as a side effect.
option(BRIDGE_BUILD_TESTS "Build the Catch2 test suite" ${BRIDGE_IS_TOP_LEVEL})
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
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
endif()

# Install/packaging only when bridge is the top-level project -- a
# consumer pulling bridge in via FetchContent owns its own install()/
# CPack story; bridge's shouldn't compete with or shadow it.
if(BRIDGE_IS_TOP_LEVEL)
    # See docs/adr/0009-packaging-via-cpack.md.
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS rivets truss deck EXPORT bridgeTargets)
    install(DIRECTORY ${BRIDGE_INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(EXPORT bridgeTargets
        FILE bridgeTargets.cmake
        NAMESPACE bridge::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bridge
    )

    # find_package(bridge) requires this file named exactly bridgeConfig.cmake
    # (matching the lowercase project() name) -- confirmed by hitting CMake's
    # "could not find a package configuration file" error with the capitalized
    # name first, not assumed.
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bridgeConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/bridgeConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bridge
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/bridgeConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/bridgeConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/bridgeConfigVersion.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchBridge.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bridge
    )

    set(CPACK_PACKAGE_NAME "bridge")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_VENDOR "tokuchan")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bridge the gaps between C++ standards, compilers, and STL implementations")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/tokuchan/bridge")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_GENERATOR "TBZ2;DEB;RPM")

    # Debian's lib*-dev convention for header-only C++ libraries; avoids
    # colliding with unrelated real-world packages named plain "bridge".
    set(CPACK_DEBIAN_PACKAGE_NAME "libbridge-dev")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Sean R. Spillane <sean@spillane.us>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")

    # Fedora's -devel convention, same reasoning.
    set(CPACK_RPM_PACKAGE_NAME "bridge-devel")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "noarch")

    include(CPack)
endif()
