
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriBuildRuntime static archive
add_library(ZuriBuildRuntime OBJECT
    src/load_config.cpp
    include/zuri_build/load_config.h
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
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
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

install(TARGETS zuri-build EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )