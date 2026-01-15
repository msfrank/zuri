
add_library(test_utils STATIC)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_dir_paths.h.in ${CMAKE_CURRENT_BINARY_DIR}/lib_dir_paths.h @ONLY)

target_sources(test_utils PRIVATE
    test_utils.cpp
    test_utils.h
    ${CMAKE_CURRENT_BINARY_DIR}/lib_dir_paths.h
    )

set_target_properties(test_utils PROPERTIES
    PUBLIC_HEADER test_utils.h
    LIBRARY_OUTPUT_DIRECTORY ${ZURI_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

target_include_directories(test_utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

target_link_libraries(test_utils
    PUBLIC
    zuri::zuri_distributor
    zuri::zuri_tooling
    zuri::zuri_test
    gtest::gtest
    )

add_subdirectory(std)
#add_subdirectory(fs)