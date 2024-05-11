enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    Datetime_tests.cpp
    Instant_tests.cpp
    Timezone_tests.cpp
    time_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_time_testsuite ${TEST_CASES})
add_dependencies(zuri_std_time_testsuite ZuriStdTime)
target_include_directories(zuri_std_time_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(zuri_std_time_testsuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(zuri_std_time_testsuite PUBLIC lyric::lyric_test)
gtest_discover_tests(zuri_std_time_testsuite)

# define test suite static library

add_library(ZuriStdTimeTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdTimeTestSuite ZuriStdTime)
target_include_directories(ZuriStdTimeTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(ZuriStdTimeTestSuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(ZuriStdTimeTestSuite PUBLIC lyric::lyric_test)
