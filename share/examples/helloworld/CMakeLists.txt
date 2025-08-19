

get_target_property(ZURI_STD_FILEPATH zuri-std-package ZURI_PKG_FILEPATH)

configure_file(workspace.config.in ${CMAKE_CURRENT_BINARY_DIR}/workspace.config @ONLY)

# build the helloworld example package

add_custom_command (
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/examples_helloworld-${PROJECT_VERSION}.zpk
    COMMAND
      zuri-build -vv
        -W ${CMAKE_CURRENT_SOURCE_DIR}
        --workspace-config-file ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
        -B ${CMAKE_CURRENT_BINARY_DIR}
        -I ${CMAKE_CURRENT_BINARY_DIR}
        --distribution-root ${ZURI_BUILD_OUTPUT_ROOT}
        "helloworld"
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      zuri-build
      ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
      zuri-std-package
    )

add_custom_target(examples-helloworld DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/examples_helloworld-${PROJECT_VERSION}.zpk
    )
