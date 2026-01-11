
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build zuri_tooling as a shared library
add_library(zuri_tooling SHARED)
add_library(zuri::zuri_tooling ALIAS zuri_tooling)

set(ZURI_TOOLING_INCLUDES
    include/zuri_tooling/build_graph.h
    include/zuri_tooling/build_tool_config.h
    include/zuri_tooling/core_config.h
    include/zuri_tooling/distribution.h
    include/zuri_tooling/environment.h
    include/zuri_tooling/environment_config.h
    include/zuri_tooling/home.h
    include/zuri_tooling/import_store.h
    include/zuri_tooling/logging_config.h
    #include/zuri_tooling/package_manager.h
    include/zuri_tooling/project.h
    include/zuri_tooling/project_config.h
    include/zuri_tooling/repository_store.h
    include/zuri_tooling/resolver_config.h
    include/zuri_tooling/target_store.h
    include/zuri_tooling/tooling_conversions.h
    include/zuri_tooling/tooling_result.h
    )
set_target_properties(zuri_tooling PROPERTIES PUBLIC_HEADER "${ZURI_TOOLING_INCLUDES}")

target_sources(zuri_tooling PRIVATE
    src/build_graph.cpp
    src/build_tool_config.cpp
    src/core_config.cpp
    src/distribution.cpp
    src/environment.cpp
    src/environment_config.cpp
    src/home.cpp
    src/import_store.cpp
    src/logging_config.cpp
    #src/package_manager.cpp
    src/project.cpp
    src/project_config.cpp
    src/repository_store.cpp
    src/resolver_config.cpp
    src/target_store.cpp
    src/tooling_conversions.cpp
    src/tooling_result.cpp

    include/zuri_tooling/internal/build_graph_impl.h
    )

# set the library version
set_target_properties(zuri_tooling PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(zuri_tooling PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(zuri_tooling PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${ZURI_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(zuri_tooling PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# make private headers visible internally
target_include_directories(zuri_tooling PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

set(RELATIVE_LIB_DIR "$<PATH:RELATIVE_PATH,${ZURI_RUNTIME_LIB_DIR},${ZURI_RUNTIME_BIN_DIR}>")
set(RELATIVE_PACKAGES_DIR "$<PATH:RELATIVE_PATH,${ZURI_RUNTIME_PACKAGES_DIR},${ZURI_RUNTIME_BIN_DIR}>")
set(RELATIVE_CONFIG_DIR "$<PATH:RELATIVE_PATH,${ZURI_RUNTIME_CONFIG_DIR},${ZURI_RUNTIME_BIN_DIR}>")

target_compile_definitions(zuri_tooling PRIVATE
    "PROJECT_VERSION_MAJOR=\"${PROJECT_VERSION_MAJOR}\""
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "RELATIVE_LIB_DIR=\"${RELATIVE_LIB_DIR}\""
    "RELATIVE_PACKAGES_DIR=\"${RELATIVE_PACKAGES_DIR}\""
    "RELATIVE_CONFIG_DIR=\"${RELATIVE_CONFIG_DIR}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "PACKAGES_DIR_PREFIX=\"${PACKAGES_DIR_PREFIX}\""
    "ZURI_PACKAGES_DIR_NAME=\"${ZURI_PACKAGES_DIR_NAME}\""
    )

target_link_libraries(zuri_tooling
    PUBLIC
    lyric::lyric_build
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_config
    tempo::tempo_utils
    zuri::zuri_distributor
    zuri::zuri_packager
    PRIVATE
    Boost::headers
    )

# install targets
install(TARGETS zuri_tooling EXPORT zuri-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zuri_tooling
    )

# add testing subdirectory
add_subdirectory(test)
