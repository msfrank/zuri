
# generate config files

configure_file(zuri.config.in ${CMAKE_CURRENT_BINARY_DIR}/zuri.config @ONLY)
configure_file(zuri-build.config.in ${CMAKE_CURRENT_BINARY_DIR}/zuri-build.config @ONLY)

# install config files

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zuri.config DESTINATION ${CONFIG_DIR_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zuri-build.config DESTINATION ${CONFIG_DIR_PREFIX})
