enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    add_target_tests.cpp
    template_processor_tests.cpp
    update_requirements_tests.cpp
    )

set(TEST_TEMPLATE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test-template)

# define test suite driver

add_executable(zuri_project_testsuite ${TEST_CASES})
target_include_directories(zuri_project_testsuite PRIVATE ../include)
target_compile_definitions(zuri_project_testsuite PRIVATE
    "TEST_TEMPLATE_DIRECTORY=\"${TEST_TEMPLATE_DIRECTORY}\""
    )
target_link_libraries(zuri_project_testsuite PUBLIC
    ZuriProjectRuntime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(zuri_project_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriProjectTestSuite OBJECT ${TEST_CASES})
target_include_directories(ZuriProjectTestSuite PRIVATE ../include)
target_compile_definitions(ZuriProjectTestSuite PRIVATE
    "TEST_TEMPLATE_DIRECTORY=\"${TEST_TEMPLATE_DIRECTORY}\""
)
target_link_libraries(ZuriProjectTestSuite PUBLIC
    ZuriProjectRuntime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
