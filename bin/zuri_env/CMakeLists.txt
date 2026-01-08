
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriEnvRuntime static archive
add_library(ZuriEnvRuntime STATIC
    include/zuri_env/env_create_command.h
    src/env_create_command.cpp
    include/zuri_env/env_result.h
    src/env_result.cpp
    include/zuri_env/zuri_env.h
    src/zuri_env.cpp
    )

target_include_directories(ZuriEnvRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriEnvRuntime PRIVATE
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "ZURI_RUNTIME_BIN_DIR=\"${ZURI_RUNTIME_BIN_DIR}\""
    "ZURI_RUNTIME_LIB_DIR=\"${ZURI_RUNTIME_LIB_DIR}\""
    "ZURI_RUNTIME_PACKAGES_DIR=\"${ZURI_RUNTIME_PACKAGES_DIR}\""
    "ZURI_RUNTIME_ENVIRONMENTS_DIR=\"${ZURI_RUNTIME_ENVIRONMENTS_DIR}\""
    "ZURI_RUNTIME_CONFIG_DIR=\"${ZURI_RUNTIME_CONFIG_DIR}\""
    )

target_link_libraries(ZuriEnvRuntime
    lyric::lyric_build
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_command
    zuri::zuri_tooling
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri-env program
add_executable(zuri-env src/main.cpp)

# ensure all output directories are created
add_dependencies(zuri-env make-output-directories)

set_target_properties(zuri-env PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-env PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-env ZuriEnvRuntime)

install(TARGETS zuri-env EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )