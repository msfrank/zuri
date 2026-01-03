
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriZpkRuntime static archive
add_library(ZuriZpkRuntime STATIC
    include/zuri_zpk/zpk_extract_command.h
    src/zpk_extract_command.cpp
    include/zuri_zpk/zpk_inspect_command.h
    src/zpk_inspect_command.cpp
    include/zuri_zpk/zpk_result.h
    src/zpk_result.cpp
    include/zuri_zpk/zuri_zpk.h
    src/zuri_zpk.cpp
    )

target_include_directories(ZuriZpkRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriZpkRuntime PRIVATE
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
    )

target_link_libraries(ZuriZpkRuntime
    lyric::lyric_object
    tempo::tempo_command
    zuri::zuri_tooling
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri-zpk program
add_executable(zuri-zpk src/main.cpp)

# ensure all output directories are created
add_dependencies(zuri-zpk make-output-directories)

set_target_properties(zuri-zpk PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-zpk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-zpk ZuriZpkRuntime)

install(TARGETS zuri-zpk EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
