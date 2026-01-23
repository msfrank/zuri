
# run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

# build ZuriProjectRuntime static archive
add_library(ZuriProjectRuntime STATIC
    include/zuri_project/add_target.h
    src/add_target.cpp
    include/zuri_project/project_add_command.h
    src/project_add_command.cpp
    include/zuri_project/project_conversions.h
    src/project_conversions.cpp
    include/zuri_project/project_link_command.h
    src/project_link_command.cpp
    include/zuri_project/project_new_command.h
    src/project_new_command.cpp
    include/zuri_project/project_result.h
    src/project_result.cpp
    include/zuri_project/template.h
    src/template.cpp
    include/zuri_project/template_config.h
    src/template_config.cpp
    include/zuri_project/zuri_project.h
    src/zuri_project.cpp
    )

target_include_directories(ZuriProjectRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DISTRIBUTION_ROOT ${ZURI_BUILD_OUTPUT_ROOT})
cmake_path(RELATIVE_PATH DISTRIBUTION_ROOT BASE_DIRECTORY ${ZURI_BUILD_BIN_DIR})

target_compile_definitions(ZuriProjectRuntime PRIVATE
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "ZURI_RUNTIME_BIN_DIR=\"${ZURI_RUNTIME_BIN_DIR}\""
    "ZURI_RUNTIME_LIB_DIR=\"${ZURI_RUNTIME_LIB_DIR}\""
    "ZURI_RUNTIME_PACKAGES_DIR=\"${ZURI_RUNTIME_PACKAGES_DIR}\""
    "ZURI_RUNTIME_ENVIRONMENTS_DIR=\"${ZURI_RUNTIME_ENVIRONMENTS_DIR}\""
    "ZURI_RUNTIME_CONFIG_DIR=\"${ZURI_RUNTIME_CONFIG_DIR}\""
    )

target_link_libraries(ZuriProjectRuntime
    lyric::lyric_build
    lyric::lyric_parser
    lyric::lyric_runtime
    tempo::tempo_command
    zuri::zuri_tooling
    mustache::mustache
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build zuri-project program
add_executable(zuri-project src/main.cpp)

# ensure all output directories are created
add_dependencies(zuri-project make-output-directories)

set_target_properties(zuri-project PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ZURI_BUILD_BIN_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BIN_RPATH}
    )

target_include_directories(zuri-project PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(zuri-project ZuriProjectRuntime)

install(TARGETS zuri-project EXPORT zuri-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# add testing subdirectory
add_subdirectory(test)