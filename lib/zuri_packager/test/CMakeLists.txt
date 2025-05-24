
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    #package_loader_tests.cpp
    package_reader_tests.cpp
    package_specifier_tests.cpp
    package_writer_tests.cpp
    )

# define test suite driver

add_executable(zuri_packager_testsuite ${TEST_CASES})
target_include_directories(zuri_packager_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(zuri_packager_testsuite PUBLIC
    zuri::zuri_packager
    gtest::gtest
    )
gtest_discover_tests(zuri_packager_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriPackagerTestSuite OBJECT ${TEST_CASES})
target_include_directories(ZuriPackagerTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(ZuriPackagerTestSuite PUBLIC
    zuri::zuri_packager
    gtest::gtest
    )
