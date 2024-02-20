
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

target_compile_definitions(ZuriRuntime PRIVATE
    "FULL_VERSION=\"${FULL_VERSION}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    "ZURI_RUNTIME_DISTRIBUTION_ROOT=\"${ZURI_RUNTIME_DISTRIBUTION_ROOT}\""
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

set_target_properties(zuri PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(zuri PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri ZuriRuntime)

install(TARGETS zuri EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )