enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    generate_protocol_task_tests.cpp
    target_builder_tests.cpp
    )

# define test suite driver

add_executable(zuri_build_testsuite
    ${TEST_CASES}
    base_build_fixture.cpp base_build_fixture.h
    base_task_fixture.cpp base_task_fixture.h
    )
target_include_directories(zuri_build_testsuite PRIVATE ../include)
target_link_libraries(zuri_build_testsuite PUBLIC
    ZuriBuildRuntime
    zuri::zuri_test
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_test_main
    gtest::gtest
    )
gtest_discover_tests(zuri_build_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(ZuriBuildTestSuite OBJECT
    ${TEST_CASES}
    base_build_fixture.cpp base_build_fixture.h
    base_task_fixture.cpp base_task_fixture.h
    )
target_include_directories(ZuriBuildTestSuite PRIVATE ../include)
target_link_libraries(ZuriBuildTestSuite PUBLIC
    ZuriBuildRuntime
    zuri::zuri_test
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
