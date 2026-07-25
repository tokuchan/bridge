add_library(rivets INTERFACE)
add_library(bridge::rivets ALIAS rivets)

target_include_directories(rivets INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(rivets INTERFACE cxx_std_17)
