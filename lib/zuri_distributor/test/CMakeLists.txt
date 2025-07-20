
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    dependency_set_tests.cpp
    dependency_selector_tests.cpp
    package_cache_tests.cpp
    package_fetcher_tests.cpp
    )

# define test suite driver

add_executable(zuri_distributor_testsuite ${TEST_CASES} test_http_server.cpp test_http_server.h)
target_include_directories(zuri_distributor_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(zuri_distributor_testsuite PUBLIC
    zuri::zuri_distributor
    zuri::zuri_test
    gtest::gtest
    Boost::headers
    )
gtest_discover_tests(zuri_distributor_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriDistributorTestSuite OBJECT ${TEST_CASES} test_http_server.cpp test_http_server.h)
target_include_directories(ZuriDistributorTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(ZuriDistributorTestSuite PUBLIC
    zuri::zuri_distributor
    zuri::zuri_test
    gtest::gtest
    Boost::headers
    )
