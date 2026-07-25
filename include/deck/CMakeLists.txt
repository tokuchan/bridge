add_library(deck INTERFACE)
add_library(bridge::deck ALIAS deck)

target_include_directories(deck INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(deck INTERFACE bridge::truss)
target_compile_features(deck INTERFACE cxx_std_17)

if(BRIDGE_HAS_BOOST)
    target_link_libraries(deck INTERFACE Boost::headers)
    target_compile_definitions(deck INTERFACE BRIDGE_HAS_BOOST=1)
endif()
