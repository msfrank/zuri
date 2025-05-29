
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build zuri_test as a shared library
add_library(zuri_test SHARED)
add_library(zuri::zuri_test ALIAS zuri_test)

set(ZURI_TEST_INCLUDES
    include/zuri_test/placeholder_loader.h
    include/zuri_test/zuri_tester.h
    )
set_target_properties(zuri_test PROPERTIES PUBLIC_HEADER "${ZURI_TEST_INCLUDES}")

target_sources(zuri_test PRIVATE
    src/placeholder_loader.cpp
    src/zuri_tester.cpp
    ${ZURI_TEST_INCLUDES}
    )

# set the library version
set_target_properties(zuri_test PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(zuri_test PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(zuri_test PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${ZURI_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(zuri_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(zuri_test
    PUBLIC
    lyric::lyric_build
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_utils
    zuri::zuri_distributor
    zuri::zuri_packager
    gtest::gtest
    )

# install targets
install(TARGETS zuri_test EXPORT zuri-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zuri_test
    )
