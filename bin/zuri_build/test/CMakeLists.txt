enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    target_builder_tests.cpp
    )

# define test suite driver

add_executable(zuri_build_testsuite ${TEST_CASES})
target_include_directories(zuri_build_testsuite PRIVATE ../include)
target_link_libraries(zuri_build_testsuite PUBLIC
    ZuriBuildRuntime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(zuri_build_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriBuildTestSuite OBJECT ${TEST_CASES})
target_include_directories(ZuriBuildTestSuite PRIVATE ../include)
target_link_libraries(ZuriBuildTestSuite PUBLIC
    ZuriBuildRuntime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
