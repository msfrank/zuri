
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriRunRuntime static archive
add_library(ZuriRunRuntime STATIC
    include/zuri_run/abstract_session.h
    include/zuri_run/ephemeral_session.h
    src/ephemeral_session.cpp
    include/zuri_run/fragment_store.h
    src/fragment_store.cpp
    include/zuri_run/read_eval_print_loop.h
    src/read_eval_print_loop.cpp
    include/zuri_run/run_interactive_command.h
    src/run_interactive_command.cpp
    include/zuri_run/run_package_command.h
    src/run_package_command.cpp
    include/zuri_run/run_result.h
    src/run_result.cpp
    include/zuri_run/zuri_run.h
    src/zuri_run.cpp
    )

target_include_directories(ZuriRunRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriRunRuntime PRIVATE
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    )

target_link_libraries(ZuriRunRuntime
    lyric::lyric_build
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_command
    zuri::zuri_tooling
    libedit::libedit
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri program
add_executable(zuri-run src/main.cpp)

set_target_properties(zuri-run PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-run ZuriRunRuntime)

install(TARGETS zuri-run EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )