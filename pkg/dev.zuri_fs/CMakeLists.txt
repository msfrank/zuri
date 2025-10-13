
# declare plugin targets
add_subdirectory(plugin)

# get file paths to each plugin
get_directory_property(ZURI_FS_FILE_PLUGIN_FILEPATH DIRECTORY plugin/file PLUGIN_FILEPATH)

# create the workspace config file used to build package
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/workspace.config.in ${CMAKE_CURRENT_BINARY_DIR}/workspace.config @ONLY)

# build the fs package
add_custom_command (
    OUTPUT
        ${ZURI_FS_PACKAGE_PATH}
    COMMAND
        zuri-build -v
        -W ${CMAKE_CURRENT_SOURCE_DIR}
        --workspace-config-file ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
        -B ${CMAKE_CURRENT_BINARY_DIR}
        -I ${CMAKE_BINARY_DIR}
        --distribution-root ${ZURI_BUILD_OUTPUT_ROOT}
        "dev.zuri_fs"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        zuri-build
        ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
        zuri-std-package
        fs-file-plugin
    )

# define custom target and target properties for consuming fs
add_custom_target(zuri-fs-package DEPENDS ${ZURI_FS_PACKAGE_PATH})
set_target_properties(zuri-fs-package PROPERTIES
    ZURI_PKG_FILEPATH ${ZURI_FS_PACKAGE_PATH}
    ZURI_PKG_SPECIFIER "fs-${PROJECT_VERSION}@zuri.dev"
    )

# add testing subdirectory
add_subdirectory(test)