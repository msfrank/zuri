enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    hashmap_tests.cpp
    option_tests.cpp
    treemap_tests.cpp
    treeset_tests.cpp
    vector_tests.cpp
    )

# define test suite driver

add_executable(zuri_std_collections_testsuite ${TEST_CASES})
add_dependencies(zuri_std_collections_testsuite ZuriStdCollections)
target_include_directories(zuri_std_collections_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)
target_compile_definitions(zuri_std_collections_testsuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
)
target_link_libraries(zuri_std_collections_testsuite PUBLIC lyric::lyric_test)
gtest_discover_tests(zuri_std_collections_testsuite)

# define test suite static library

add_library(ZuriStdCollectionsTestSuite OBJECT ${TEST_CASES})
add_dependencies(ZuriStdCollectionsTestSuite ZuriStdCollections)
target_include_directories(ZuriStdCollectionsTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
    )
target_compile_definitions(ZuriStdCollectionsTestSuite PRIVATE
    "TESTER_CONFIG_PATH=\"${TESTER_CONFIG_PATH}\""
    )
target_link_libraries(ZuriStdCollectionsTestSuite PUBLIC lyric::lyric_test)
