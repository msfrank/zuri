cmake_minimum_required(VERSION 3.27)

# read project metadata
file(READ ${CMAKE_SOURCE_DIR}/meta/version ZURI_PROJECT_VERSION)
string(STRIP ${ZURI_PROJECT_VERSION} ZURI_PROJECT_VERSION)
file(READ ${CMAKE_SOURCE_DIR}/meta/url ZURI_PROJECT_URL)
string(STRIP ${ZURI_PROJECT_URL} ZURI_PROJECT_URL)
file(READ ${CMAKE_SOURCE_DIR}/meta/description ZURI_PROJECT_DESCRIPTION)
string(STRIP ${ZURI_PROJECT_DESCRIPTION} ZURI_PROJECT_DESCRIPTION)
file(READ ${CMAKE_SOURCE_DIR}/meta/license ZURI_PROJECT_LICENSE)
string(STRIP ${ZURI_PROJECT_LICENSE} ZURI_PROJECT_LICENSE)

# declare the project
project(zuri
    VERSION ${ZURI_PROJECT_VERSION}
    HOMEPAGE_URL ${ZURI_PROJECT_URL}
    DESCRIPTION ${ZURI_PROJECT_DESCRIPTION}
)

# if build type was not explicitly defined then default to Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type not specified so defaulting to Debug" FORCE)
endif()

# define IS_DEBUG_BUILD boolean to make it easier to check whether we are in debug mode
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(IS_DEBUG_BUILD ON)
else()
    set(IS_DEBUG_BUILD OFF)
endif()


#######################################################################
#
# enable/disable build targets
#
#######################################################################

option(DISABLE_LOCAL_BUILD "Disable local build" FALSE)
option(ENABLE_DOCKER_BUILD "Enable docker build" FALSE)

# if docker build is enabled then add targets from docker directory
if (ENABLE_DOCKER_BUILD)
    add_subdirectory(docker)
endif()

# if local build is disabled then don't process any further targets
if (DISABLE_LOCAL_BUILD)
    return()
endif()


#######################################################################
#
# declare options
#
#######################################################################

set(RUNTIME_DISTRIBUTION_ROOT "${CMAKE_INSTALL_PREFIX}"
    CACHE STRING
    "The runtime distribution root directory. Defaults to CMAKE_INSTALL_PREFIX.")
message(STATUS "runtime distribution root is ${RUNTIME_DISTRIBUTION_ROOT}")

option(ENABLE_SANITIZER "Enable sanitizer library." OFF)

set(SANITIZER ""
    CACHE STRING
    "The sanitizer to use. Must be one of 'address', 'thread', 'memory', 'ub', or 'leak'.")

set(SANITIZER_IGNORELIST ""
    CACHE PATH
    "Instruct sanitizer to ignore symbols specified in ignorelist.")

option(ENABLE_PROFILER "Enable gperftools profiler." OFF)


#######################################################################
#
# configure compiler and linker
#
#######################################################################

# configure common compiler flags
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wno-deprecated-declarations")

# by default do not specify -O
set (OPTIMIZATION_LEVEL "")

# enable one of the sanitizer libraries if specified
if (ENABLE_SANITIZER)
    if ("${SANITIZER}" STREQUAL "address")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling address sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "thread")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling thread sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "memory")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling memory sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "ub")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling ub sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "leak")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling leak sanitizer library")
    else()
        message(WARNING "skipping unknown sanitizer ${SANITIZER}")
        set (SANITIZER "")
    endif ("${SANITIZER}" STREQUAL "address")
    # if a sanitizer is enabled and an ignorelist is specified, then add it to the cxx flags
    if (SANITIZER)
        if (SANITIZER_IGNORELIST)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-ignorelist=${SANITIZER_IGNORELIST}")
            message(STATUS "using sanitizer ignorelist ${SANITIZER_IGNORELIST}")
        endif (SANITIZER_IGNORELIST)
    endif (SANITIZER)
else()
    message(STATUS "sanitizer not enabled")
endif (ENABLE_SANITIZER)

#
if (NOT OPTIMIZATION_LEVEL)
    set(OPTIMIZATION_LEVEL "-O0 -fno-eliminate-unused-debug-types")
endif()

# set the optimization level
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATION_LEVEL}")

# define the rpath for executable and library targets
if (APPLE)
    set(BIN_RPATH "@executable_path/../lib")
    set(LIB_RPATH "@loader_path")
elseif(WIN32)
    # do nothing
else()
    set(BIN_RPATH "$ORIGIN/../lib")
    set(LIB_RPATH "$ORIGIN/")
endif()


#######################################################################
#
# find required and optional dependencies
#
#######################################################################

# find required Tempo dependency
find_package(tempo REQUIRED)

# find required Lyric dependency
find_package(lyric REQUIRED)

# find required Abseil dependency
find_package(absl REQUIRED)

# find required Boost dependency
find_package(Boost REQUIRED)

# find required cppterminal dependency
find_package(cppterminal REQUIRED)

# find required libedit dependency
find_package(libedit REQUIRED)

# find required Flatbuffers dependency
find_package(flatbuffers REQUIRED)

# find required Antlr dependency
find_package(antlr REQUIRED)

# find required Curl dependency
find_package(CURL REQUIRED)

# find required Openssl dependency
find_package(openssl REQUIRED)

# find required Uv dependency
find_package(uv REQUIRED)

# find required Fmt dependency
find_package(fmt REQUIRED)

# find required ICU dependency
find_package(icu REQUIRED)

# find required Rocksdb dependency
find_package(rocksdb REQUIRED)

# find required Sqlite dependency
find_package(sqlite REQUIRED)

# find java build tool dependency for antlr
find_package(Java REQUIRED)

# find required google test dependency
find_package(gtest REQUIRED)

# link to gperftools profiler library if specified
set (PROFILER_LIBRARIES "")
if (${ENABLE_PROFILER})
    set (PROFILER_LIBRARIES "-lprofiler")
    message(STATUS "enabling gperftools profiler")
else()
    message(STATUS "gperftools profiler not enabled")
endif(${ENABLE_PROFILER})

# include needed CMake features
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)
include(GoogleTest)


#######################################################################
#
# configure tools and paths
#
#######################################################################

# define path to flatc
set(FLATC ${FLATBUFFERS_FLATC})
message(STATUS "flatc command is ${FLATC}")

# define path to antlr tool
set(ANTLR
    ${Java_JAVA_EXECUTABLE}
    -Xmx500M
    -cp ${ANTLR_TOOL_JAR}
    org.antlr.v4.Tool
)
message(STATUS "antlr command is ${ANTLR}")


#######################################################################
#
# define build constants
#
#######################################################################

set(FULL_PLATFORM "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
message(STATUS "full platform is ${FULL_PLATFORM}")

# platform id is formed from<system name/os>-<processor/arch>
set(ZURI_PLATFORM_ID                        "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# zuri package group
set(ZURI_PACKAGE_GROUP                      "dev.zuri")

# zuri std package
set(ZURI_STD_PACKAGE_NAME                   "std")
set(ZURI_STD_PACKAGE_VERSION                "${PROJECT_VERSION}")
set(ZURI_STD_PACKAGE_ID                     "${ZURI_PACKAGE_GROUP}_${ZURI_STD_PACKAGE_NAME}-${ZURI_STD_PACKAGE_VERSION}")
set(ZURI_STD_PACKAGE_AUTHORITY              "${ZURI_STD_PACKAGE_NAME}-${ZURI_STD_PACKAGE_VERSION}@zuri.dev")
set(ZURI_STD_PACKAGE_URL                    "dev.zuri.pkg://${ZURI_STD_PACKAGE_AUTHORITY}")

# special directory names
set(ZURI_PACKAGES_DIR_NAME                  "zuri-packages-${PROJECT_VERSION_MAJOR}")

# install paths
set(PACKAGES_DIR_PREFIX                     "${CMAKE_INSTALL_LIBDIR}/${ZURI_PACKAGES_DIR_NAME}")
set(DOC_DIR_PREFIX                          "${CMAKE_INSTALL_DATAROOTDIR}/doc/zuri-${PROJECT_VERSION}")
set(CONFIG_DIR_PREFIX                       "${CMAKE_INSTALL_DATAROOTDIR}/zuri-${PROJECT_VERSION}/config")
set(VENDOR_CONFIG_DIR_PREFIX                "${CMAKE_INSTALL_DATAROOTDIR}/zuri-${PROJECT_VERSION}/vendor-config")

# build paths
set(ZURI_BUILD_OUTPUT_ROOT                  "${CMAKE_BINARY_DIR}/output")
set(ZURI_BUILD_BIN_DIR                      "${ZURI_BUILD_OUTPUT_ROOT}/${CMAKE_INSTALL_BINDIR}")
set(ZURI_BUILD_LIB_DIR                      "${ZURI_BUILD_OUTPUT_ROOT}/${CMAKE_INSTALL_LIBDIR}")
set(ZURI_BUILD_PACKAGES_DIR                 "${ZURI_BUILD_OUTPUT_ROOT}/${PACKAGES_DIR_PREFIX}")
set(ZURI_BUILD_CONFIG_DIR                   "${ZURI_BUILD_OUTPUT_ROOT}/${CONFIG_DIR_PREFIX}")
set(ZURI_BUILD_VENDOR_CONFIG_DIR            "${ZURI_BUILD_OUTPUT_ROOT}/${VENDOR_CONFIG_DIR_PREFIX}")
set(ZURI_BUILD_DOC_DIR                      "${ZURI_BUILD_OUTPUT_ROOT}/${DOC_DIR_PREFIX}")

# runtime paths
set(ZURI_RUNTIME_DISTRIBUTION_ROOT          "${CMAKE_INSTALL_PREFIX}")
set(ZURI_RUNTIME_PACKAGES_DIR               "${CMAKE_INSTALL_PREFIX}/${PACKAGES_DIR_PREFIX}")
set(ZURI_RUNTIME_CONFIG_DIR                 "${CMAKE_INSTALL_PREFIX}/${CONFIG_DIR_PREFIX}")
set(ZURI_RUNTIME_VENDOR_CONFIG_DIR          "${CMAKE_INSTALL_PREFIX}/${VENDOR_CONFIG_DIR_PREFIX}")

# used to initialize tester
set(TESTER_CONFIG_PATH                      "${CMAKE_BINARY_DIR}/zuri_std/tester.config")

# empty cpp file is used for targets which don't have any source files
set(EMPTY_CPP "${CMAKE_BINARY_DIR}/empty.cpp")


#######################################################################
#
# add library targets from subdirectories
#
#######################################################################

add_subdirectory(bin)
add_subdirectory(lib)
add_subdirectory(pkg)
add_subdirectory(share)


#######################################################################
#
# declare the package test suite target
#
#######################################################################

add_custom_command(
    OUTPUT ${EMPTY_CPP}
    COMMAND cmake -E touch ${EMPTY_CPP}
)

add_executable(zuri_testsuite ${EMPTY_CPP})
target_link_libraries(zuri_testsuite
    PUBLIC gtest::gtest
    PRIVATE
    ZuriBuildTestSuite
    ZuriDistributorTestSuite
    ZuriPackagerTestSuite
    ZuriStdPackageTestSuite
)
gtest_discover_tests(zuri_testsuite)


#######################################################################
#
# create cmake package config
#
#######################################################################

configure_package_config_file(
    cmake/zuri-config.cmake.in
    zuri-config.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zuri"
    PATH_VARS
      PACKAGES_DIR_PREFIX
      DOC_DIR_PREFIX
      CONFIG_DIR_PREFIX
      VENDOR_CONFIG_DIR_PREFIX
    )

write_basic_package_version_file(zuri-config-version.cmake
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
    )

install(EXPORT zuri-targets
    COMPONENT Devel
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zuri"
    NAMESPACE zuri::
    )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/zuri-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/zuri-config-version.cmake"
    COMPONENT Devel
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zuri"
    )
