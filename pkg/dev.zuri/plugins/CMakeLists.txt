
define_property(DIRECTORY PROPERTY PLUGIN_NAME)
define_property(DIRECTORY PROPERTY PLUGIN_FILENAME)
define_property(DIRECTORY PROPERTY PLUGIN_FILEPATH)

add_subdirectory(collections)
add_subdirectory(file)
add_subdirectory(log)
add_subdirectory(path)
add_subdirectory(process)
add_subdirectory(system)
add_subdirectory(text)
add_subdirectory(time)
