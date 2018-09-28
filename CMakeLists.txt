cmake_minimum_required(VERSION 3.1)
project(jsoncons)

set(JSONCONS_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(JSONCONS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versioning
# ==========

file(STRINGS "${JSONCONS_INCLUDE_DIR}/jsoncons/version.hpp" jsoncons_version_defines
     REGEX "#define JSONCONS_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${jsoncons_version_defines})
    if(ver MATCHES "#define JSONCONS_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(JSONCONS_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${JSONCONS_VERSION_MAJOR}.${JSONCONS_VERSION_MINOR}.${JSONCONS_VERSION_PATCH})
message(STATUS "jsoncons v${${PROJECT_NAME}_VERSION}")

# Build
# =====

set(JSONCONS_HEADERS
    ${JSONCONS_INCLUDE_DIR}/jsoncons/json.hpp
)

add_library(jsoncons INTERFACE)
target_include_directories(jsoncons INTERFACE $<BUILD_INTERFACE:${JSONCONS_INCLUDE_DIR}>
                                           $<INSTALL_INTERFACE:include>)

OPTION(ENABLE_FALLBACK "build tests/benchmarks with fallback implementation" OFF)
OPTION(ENABLE_XTL_COMPLEX "enables support for xcomplex defined in xtl" OFF)
OPTION(BUILD_TESTS "jsoncons test suite" OFF)

if(ENABLE_FALLBACK)
    add_definitions(-DJSONCONS_ENABLE_FALLBACK=1)
endif()

if(ENABLE_XTL_COMPLEX)
    add_definitions(-DJSONCONS_ENABLE_XTL_COMPLEX=1)
    find_package(xtl 0.4.11 REQUIRED)
    target_link_libraries(jsoncons INTERFACE xtl)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS jsoncons
        EXPORT ${PROJECT_NAME}-targets)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

install(DIRECTORY ${JSONCONS_INCLUDE_DIR}/jsoncons
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
set(JSONCONS_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for jsonconsConfig.cmake")

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${JSONCONS_CMAKECONFIG_INSTALL_DIR})

# jsoncons is header-only and does not depend on the architecture.
# Remove CMAKE_SIZEOF_VOID_P from xtensorConfigVersion.cmake so that an xtensorConfig.cmake
# generated for a 64 bit target can be used for 32 bit targets and vice versa.
set(_XTENSOR_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${JSONCONS_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${JSONCONS_CMAKECONFIG_INSTALL_DIR})

