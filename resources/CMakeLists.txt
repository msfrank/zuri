
# generate resources

configure_file(config/zuri.config.in ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_PREFIX}/zuri.config @ONLY)
configure_file(config/zuri-build.config.in ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_PREFIX}/zuri-build.config @ONLY)

# install resources

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_PREFIX}/zuri.config DESTINATION ${CONFIG_DIR_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_PREFIX}/zuri-build.config DESTINATION ${CONFIG_DIR_PREFIX})