enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    attr_tests.cpp
    element_tests.cpp
    operation_tests.cpp
    port_tests.cpp
    queue_tests.cpp
    system_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_system_testsuite ${TEST_CASES})
add_dependencies(zuri_std_system_testsuite ZuriStdSystem)
target_include_directories(zuri_std_system_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(zuri_std_system_testsuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(zuri_std_system_testsuite PUBLIC lyric::lyric_test)
gtest_discover_tests(zuri_std_system_testsuite)

# define test suite static library

add_library(ZuriStdSystemTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdSystemTestSuite ZuriStdSystem)
target_include_directories(ZuriStdSystemTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(ZuriStdSystemTestSuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
    )
target_link_libraries(ZuriStdSystemTestSuite PUBLIC lyric::lyric_test)
