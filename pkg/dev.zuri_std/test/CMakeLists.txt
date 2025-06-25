enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    datetime_tests.cpp
    instant_tests.cpp
    future_tests.cpp
    log_tests.cpp
    port_tests.cpp
    queue_tests.cpp
    system_tests.cpp
    text_tests.cpp
    time_tests.cpp
    timezone_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_package_testsuite ${TEST_CASES})
add_dependencies(zuri_std_package_testsuite zuri-std-package)
target_include_directories(zuri_std_package_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(zuri_std_package_testsuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    )
target_link_libraries(zuri_std_package_testsuite
    lyric::lyric_test
    tempo::tempo_test
    zuri::zuri_test
    )
gtest_discover_tests(zuri_std_package_testsuite)

# define test suite static library

add_library(ZuriStdPackageTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdPackageTestSuite zuri-std-package)
target_include_directories(ZuriStdPackageTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(ZuriStdPackageTestSuite PRIVATE
    "ZURI_STD_PACKAGE_PATH=\"${ZURI_STD_PACKAGE_PATH}\""
    )
target_link_libraries(ZuriStdPackageTestSuite
    PUBLIC
    lyric::lyric_test
    tempo::tempo_test
    zuri::zuri_test
    )
