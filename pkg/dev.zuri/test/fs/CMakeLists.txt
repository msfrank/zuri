enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    file_tests.cpp
    path_tests.cpp
    permission_tests.cpp
    )

# define test suite driver

add_executable(zuri_fs_package_testsuite ${TEST_CASES})
add_dependencies(zuri_fs_package_testsuite zuri-std-package zuri-fs-package)
target_include_directories(zuri_fs_package_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(zuri_fs_package_testsuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    "ZURI_FS_PACKAGE_PATH=\"${ZURI_FS_PACKAGE_PATH}\""
    )
target_link_libraries(zuri_fs_package_testsuite
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_utils
    zuri::zuri_test
    )
gtest_discover_tests(zuri_fs_package_testsuite)

# define test suite static library

add_library(ZuriFsPackageTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriFsPackageTestSuite zuri-std-package zuri-fs-package)
target_include_directories(ZuriFsPackageTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(ZuriFsPackageTestSuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    "ZURI_FS_PACKAGE_PATH=\"${ZURI_FS_PACKAGE_PATH}\""
    )
target_link_libraries(ZuriFsPackageTestSuite
    PUBLIC
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_utils
    zuri::zuri_test
    )
