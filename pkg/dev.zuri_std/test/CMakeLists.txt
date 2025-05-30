enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    log_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_log_testsuite ${TEST_CASES})
add_dependencies(zuri_std_log_testsuite zuri-std-package)
target_include_directories(zuri_std_log_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(zuri_std_log_testsuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    )
target_link_libraries(zuri_std_log_testsuite
    lyric::lyric_test
    tempo::tempo_test
    zuri::zuri_test
    )
gtest_discover_tests(zuri_std_log_testsuite)

# define test suite static library

add_library(ZuriStdLogTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdLogTestSuite zuri-std-package)
target_include_directories(ZuriStdLogTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(ZuriStdLogTestSuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    )
target_link_libraries(ZuriStdLogTestSuite
    PUBLIC
    lyric::lyric_test
    tempo::tempo_test
    zuri::zuri_test
    )