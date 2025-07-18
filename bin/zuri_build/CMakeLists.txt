
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriBuildRuntime static archive
add_library(ZuriBuildRuntime STATIC
    src/build_graph.cpp
    include/zuri_build/build_graph.h
    src/collect_modules_task.cpp
    include/zuri_build/collect_modules_task.h
    src/import_resolver.cpp
    include/zuri_build/import_resolver.h
    src/import_store.cpp
    include/zuri_build/import_store.h
    src/workspace_config.cpp
    include/zuri_build/workspace_config.h
    src/target_builder.cpp
    include/zuri_build/target_builder.h
    src/target_cycle_detector.cpp
    include/zuri_build/target_cycle_detector.h
    src/target_store.cpp
    include/zuri_build/target_store.h
    src/target_writer.cpp
    include/zuri_build/target_writer.h
    src/zuri_build.cpp
    include/zuri_build/zuri_build.h
    )

target_include_directories(ZuriBuildRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriBuildRuntime PRIVATE
    "FULL_VERSION=\"${FULL_VERSION}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    )

target_link_libraries(ZuriBuildRuntime
    PUBLIC
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    zuri::zuri_distributor
    zuri::zuri_packager
    Boost::headers
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri-build program
add_executable(zuri-build src/main.cpp)

set_target_properties(zuri-build PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-build PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-build ZuriBuildRuntime)

# install targets
install(TARGETS zuri-build EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# add testing subdirectory
add_subdirectory(test)
