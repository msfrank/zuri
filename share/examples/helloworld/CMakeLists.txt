

set(SOURCE_DIRECTORY        "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PACKAGE_DIRECTORY       "${CMAKE_CURRENT_BINARY_DIR}")

configure_file(workspace.config.in ${CMAKE_CURRENT_BINARY_DIR}/workspace.config @ONLY)

# build the helloworld example package

add_custom_command (
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/install/examples_helloworld-${FULL_VERSION}.zpk
    COMMAND
      zuri-build -v
        -W ${CMAKE_CURRENT_BINARY_DIR}
        -B ${CMAKE_CURRENT_BINARY_DIR}
        -I ${PACKAGE_DIRECTORY}
        package:
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      zuri-build
      ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
)

add_custom_target(examples-helloworld DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/install/examples_helloworld-${FULL_VERSION}.zpk)
