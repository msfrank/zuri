find_program(SPHINX_BUILD NAMES sphinx-build
    HINTS $ENV{SPHINX_DIR}
    PATH_SUFFIXES bin
    DOC "Sphinx documentation generator"
    )

if (NOT ${SPHINX_BUILD} MATCHES "SPHINX_BUILD-NOTFOUND")

    message(STATUS "found sphinx-build at ${SPHINX_BUILD}")
                 
    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()
      
    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()

    set(SPHINX_CONF_DIR "${CMAKE_CURRENT_BINARY_DIR}/conf")
    set(SPHINX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SPHINX_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/build")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
        "${SPHINX_CONF_DIR}/conf.py"
        @ONLY)

    add_custom_target(devguide
        #COMMAND
        #    ${CMAKE_COMMAND}
        #        -E copy_directory
        #        "${CMAKE_CURRENT_SOURCE_DIR}/_static" "${CMAKE_CURRENT_SOURCE_DIR}/_templates"
        #        ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND
            ${SPHINX_BUILD}
                -q -b html -c "${SPHINX_CONF_DIR}"
                "${SPHINX_SOURCE_DIR}"
                "${SPHINX_OUTPUT_DIR}"
        COMMENT "Building UserGuide HTML documentation"
        )

else()
    message(WARNING "Failed to find sphinx-build program, devguide will not be built")
endif()
