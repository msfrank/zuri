
#######################################################################
#
# declare options
#
#######################################################################

set(DOCKER_PROGRAM ""
    CACHE FILEPATH
    "Path to the docker command. If empty then the program path is auto-detected. Defaults to ''")

set(DOCKER_REQUIRES_SUDO "FALSE"
    CACHE BOOL
    "TRUE if the docker command requires sudo privileges. Defaults to 'FALSE'")

set(DOCKER_PLATFORM_ID ""
    CACHE STRING
    "Target platform for docker image. If empty then the platform is auto-detected. Defaults to ''")

set(DOCKER_BASE_IMAGE "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,timbre-debug:latest,timbre:latest>"
    CACHE STRING
    "The docker image to use as the base. Defaults to 'timbre-debug:latest' for a debug build, otherwise 'timbre:latest'")

set(DOCKER_TEMPO_IMAGE "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,tempo-pkgcache-debug:latest,tempo-pkgcache:latest>"
    CACHE STRING
    "The docker image which contains the Tempo cache save. Defaults to 'tempo-pkgcache-debug:latest' for a debug build, otherwise 'tempo-pkgcache:latest'")

set(DOCKER_LYRIC_IMAGE "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,lyric-pkgcache-debug:latest,lyric-pkgcache:latest>"
    CACHE STRING
    "The docker image which contains the Lyric cache save. Defaults to 'lyric-pkgcache-debug:latest' for a debug build, otherwise 'lyric-pkgcache:latest'")

set(DOCKER_REGISTRY ""
    CACHE STRING
    "The docker registry to export to. If empty then defaults to the local image store. Defaults to ''")


#######################################################################
#
# find required and optional dependencies
#
#######################################################################

# find docker CLI
if (DOCKER_PROGRAM)
    set(Docker ${DOCKER_PROGRAM})
    message(STATUS "Using Docker: ${Docker} (specified by DOCKER_PROGRAM)")
else()
    find_program(Docker docker)
    if (Docker EQUAL "Docker-NOTFOUND")
        message(STATUS "Docker not found, skipping generation of docker targets")
    else()
        message(STATUS "Found Docker: ${Docker}")
    endif()
endif()


#######################################################################
#
# configure tools and paths
#
#######################################################################

if (DOCKER_REQUIRES_SUDO)
    set(DOCKER "sudo;${Docker}")
else ()
    set(DOCKER ${Docker})
endif ()


#######################################################################
#
# define docker build targets
#
#######################################################################

if (DOCKER_REGISTRY)
    set(DOCKER_OUTPUT "type=registry,push=true")
else()
    set(DOCKER_OUTPUT "type=docker")
endif()

# set the PLATFORM_ID
if (DOCKER_PLATFORM_ID)
    set(PLATFORM_ID ${DOCKER_PLATFORM_ID})
else()
    set(PLATFORM_ID "linux/${CMAKE_SYSTEM_PROCESSOR}")
endif()
if (NOT PLATFORM_ID)
    message(FATAL_ERROR "failed to determine docker platform id")
endif()

set(PLATFORM_DOCKERFILE_PATH "${CMAKE_SOURCE_DIR}/docker/platform/Dockerfile")
set(PKGCACHE_DOCKERFILE_PATH "${CMAKE_SOURCE_DIR}/docker/pkgcache/Dockerfile")

# build the zuri image
set(PLATFORM_IMAGE_NAME "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,zuri-debug,zuri>")
add_custom_target(build_docker_image
    COMMAND ${DOCKER} build
      --platform ${PLATFORM_ID}
      --build-arg base_image=${DOCKER_BASE_IMAGE}
      --build-arg tempo_image=${DOCKER_TEMPO_IMAGE}
      --build-arg is_debug_build=$<BOOL:${IS_DEBUG_BUILD}>
      -o ${DOCKER_OUTPUT}
      -t "${DOCKER_REGISTRY}${PLATFORM_IMAGE_NAME}"
      -t "${DOCKER_REGISTRY}${PLATFORM_IMAGE_NAME}:${CMAKE_PROJECT_VERSION}"
      -t "${DOCKER_REGISTRY}${PLATFORM_IMAGE_NAME}:latest"
      -t "${DOCKER_REGISTRY}zuri-current-build"
      -f ${PLATFORM_DOCKERFILE_PATH}
      --label "org.opencontainers.image.version=${CMAKE_PROJECT_VERSION}"
      --label "org.opencontainers.image.source=${CMAKE_PROJECT_HOMEPAGE_URL}"
      --label "org.opencontainers.image.description=${CMAKE_PROJECT_DESCRIPTION}"
      --label "org.opencontainers.image.licenses=${ZURI_PROJECT_LICENSE}"
      .
    COMMENT "building zuri docker image"
    DEPENDS ${PLATFORM_DOCKERFILE_PATH}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

# build the zuri-pkgcache image
set(PKGCACHE_IMAGE_NAME "$<IF:$<BOOL:${IS_DEBUG_BUILD}>,zuri-pkgcache-debug,zuri-pkgcache>")
add_custom_target(build_docker_pkgcache_image
    COMMAND ${DOCKER} build
      --platform ${PLATFORM_ID}
      --build-arg zuri_image=${DOCKER_REGISTRY}${PLATFORM_IMAGE_NAME}
      -o ${DOCKER_OUTPUT}
      -t "${DOCKER_REGISTRY}${PKGCACHE_IMAGE_NAME}"
      -t "${DOCKER_REGISTRY}${PKGCACHE_IMAGE_NAME}:${CMAKE_PROJECT_VERSION}"
      -t "${DOCKER_REGISTRY}${PKGCACHE_IMAGE_NAME}:latest"
      -t "${DOCKER_REGISTRY}zuri-pkgcache-current-build"
      -f ${PKGCACHE_DOCKERFILE_PATH}
      --label "org.opencontainers.image.version=${CMAKE_PROJECT_VERSION}"
      --label "org.opencontainers.image.source=${CMAKE_PROJECT_HOMEPAGE_URL}"
      --label "org.opencontainers.image.description=${CMAKE_PROJECT_DESCRIPTION}"
      --label "org.opencontainers.image.licenses=${ZURI_PROJECT_LICENSE}"
      .
    COMMENT "building zuri-pkgcache docker image"
    DEPENDS ${PKGCACHE_DOCKERFILE_PATH}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
