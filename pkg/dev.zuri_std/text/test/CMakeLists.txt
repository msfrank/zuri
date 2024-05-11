enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    Text_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_text_testsuite ${TEST_CASES})
add_dependencies(zuri_std_text_testsuite ZuriStdText)
target_include_directories(zuri_std_text_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(zuri_std_text_testsuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(zuri_std_text_testsuite PUBLIC lyric::lyric_test)
gtest_discover_tests(zuri_std_text_testsuite)

# define test suite static library

add_library(ZuriStdTextTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdTextTestSuite ZuriStdText)
target_include_directories(ZuriStdTextTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(ZuriStdTextTestSuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(ZuriStdTextTestSuite PUBLIC lyric::lyric_test)
