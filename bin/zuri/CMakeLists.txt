
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriRuntime static archive
add_library(ZuriRuntime OBJECT
    include/zuri/ephemeral_session.h
    src/ephemeral_session.cpp
    include/zuri/fragment_store.h
    src/fragment_store.cpp
    include/zuri/read_eval_print_loop.h
    src/read_eval_print_loop.cpp
    include/zuri/load_config.h
    src/load_config.cpp
    include/zuri/zuri.h
    src/zuri.cpp
    )

target_include_directories(ZuriRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriRuntime PRIVATE
    "FULL_VERSION=\"${FULL_VERSION}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    )

target_link_libraries(ZuriRuntime
    lyric::lyric_build
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_command
    cppterminal::cppterminal
    libedit::libedit
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri program
add_executable(zuri src/main.cpp)

set_target_properties(zuri PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri ZuriRuntime)

install(TARGETS zuri EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )