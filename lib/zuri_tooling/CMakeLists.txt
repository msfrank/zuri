
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build zuri_tooling as a shared library
add_library(zuri_tooling SHARED)
add_library(zuri::zuri_tooling ALIAS zuri_tooling)

set(ZURI_TOOLING_INCLUDES
    include/zuri_tooling/import_store.h
    include/zuri_tooling/package_store.h
    include/zuri_tooling/target_store.h
    include/zuri_tooling/tooling_conversions.h
    include/zuri_tooling/tooling_result.h
    include/zuri_tooling/zuri_config.h
    )
set_target_properties(zuri_tooling PROPERTIES PUBLIC_HEADER "${ZURI_TOOLING_INCLUDES}")

target_sources(zuri_tooling PRIVATE
    src/import_store.cpp
    src/package_store.cpp
    src/target_store.cpp
    src/tooling_conversions.cpp
    src/tooling_result.cpp
    src/zuri_config.cpp
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

target_compile_definitions(zuri_tooling PRIVATE
    "DISTRIBUTION_ROOT=\"${DISTRIBUTION_ROOT}\""
    "CONFIG_DIR_PREFIX=\"${CONFIG_DIR_PREFIX}\""
    "VENDOR_CONFIG_DIR_PREFIX=\"${VENDOR_CONFIG_DIR_PREFIX}\""
)

target_link_libraries(zuri_tooling
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    zuri::zuri_packager
    )

# install targets
install(TARGETS zuri_tooling EXPORT zuri-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zuri_tooling
    )

# add testing subdirectory
#add_subdirectory(test)
