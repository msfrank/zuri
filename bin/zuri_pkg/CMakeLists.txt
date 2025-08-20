
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriPkgRuntime static archive
add_library(ZuriPkgRuntime STATIC
    include/zuri_pkg/install_solver.h
    src/install_solver.cpp
    include/zuri_pkg/pkg_cache_command.h
    src/pkg_cache_command.cpp
    include/zuri_pkg/pkg_install_command.h
    src/pkg_install_command.cpp
    include/zuri_pkg/pkg_result.h
    src/pkg_result.cpp
    include/zuri_pkg/zuri_pkg.h
    src/zuri_pkg.cpp
    )

target_include_directories(ZuriPkgRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriPkgRuntime PRIVATE
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    )

target_link_libraries(ZuriPkgRuntime
    lyric::lyric_build
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_command
    zuri::zuri_tooling
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri program
add_executable(zuri-pkg src/main.cpp)

set_target_properties(zuri-pkg PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-pkg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-pkg ZuriPkgRuntime)

install(TARGETS zuri-pkg EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )