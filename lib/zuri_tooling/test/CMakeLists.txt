
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    )

# define test suite driver

add_executable(zuri_tooling_testsuite ${TEST_CASES})
target_include_directories(zuri_tooling_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(zuri_tooling_testsuite PUBLIC
    zuri::zuri_tooling
    zuri::zuri_test
    gtest::gtest
    )
gtest_discover_tests(zuri_tooling_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriToolingTestSuite OBJECT ${TEST_CASES})
target_include_directories(ZuriToolingTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(ZuriToolingTestSuite PUBLIC
    zuri::zuri_tooling
    zuri::zuri_test
    gtest::gtest
    )
