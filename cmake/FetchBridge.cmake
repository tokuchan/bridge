# FetchBridge.cmake — pulls bridge directly via FetchContent, for
# projects that want bridge::rivets/truss/deck without installing a
# package first. See docs/adr/0009-packaging-via-cpack.md.
#
# Usage:
#   include(FetchBridge.cmake)
#   bridge_fetch(VERSION 26.7.0)
#   target_link_libraries(myapp PRIVATE bridge::deck)
#
# Options:
#   VERSION <tag>      Git tag to fetch (a CalVer release tag, e.g. 26.7.0).
#   GIT_TAG <ref>      Exact git ref (branch/tag/commit); overrides VERSION.
#   REPOSITORY <url>   Override the default GitHub URL.
function(bridge_fetch)
    set(options "")
    set(oneValueArgs VERSION GIT_TAG REPOSITORY)
    set(multiValueArgs "")
    cmake_parse_arguments(BRIDGE_FETCH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT BRIDGE_FETCH_REPOSITORY)
        set(BRIDGE_FETCH_REPOSITORY "https://github.com/tokuchan/bridge.git")
    endif()

    if(BRIDGE_FETCH_GIT_TAG)
        set(_bridge_fetch_tag "${BRIDGE_FETCH_GIT_TAG}")
    elseif(BRIDGE_FETCH_VERSION)
        set(_bridge_fetch_tag "${BRIDGE_FETCH_VERSION}")
    else()
        message(FATAL_ERROR "bridge_fetch(): VERSION or GIT_TAG is required")
    endif()

    include(FetchContent)
    FetchContent_Declare(bridge
        GIT_REPOSITORY "${BRIDGE_FETCH_REPOSITORY}"
        GIT_TAG "${_bridge_fetch_tag}"
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(bridge)
endfunction()
