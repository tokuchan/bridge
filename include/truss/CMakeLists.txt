add_library(truss INTERFACE)
add_library(bridge::truss ALIAS truss)

target_include_directories(truss INTERFACE
    $<BUILD_INTERFACE:${BRIDGE_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(truss INTERFACE bridge::rivets)
target_compile_features(truss INTERFACE cxx_std_17)

if(BRIDGE_HAS_BOOST)
    target_link_libraries(truss INTERFACE Boost::headers)
    target_compile_definitions(truss INTERFACE BRIDGE_HAS_BOOST=1)
endif()
