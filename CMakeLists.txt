cmake_minimum_required(VERSION 3.8)

##
## PROJECT
## name and version
##
project(jsoncons_json VERSION 0.109.0 LANGUAGES CXX)

##
## INCLUDE
##
##
include(ExternalProject)

##
## OPTIONS
##
option(JSON_BuildTests "Build the unit tests when BUILD_TESTING is enabled." ON)

##
## CONFIGURATION
##
set(JSONCONS_JSON_TARGET_NAME               ${PROJECT_NAME})
set(JSONCONS_JSON_CONFIG_INSTALL_DIR        "lib/cmake/${PROJECT_NAME}"
  CACHE INTERNAL "")
set(JSONCONS_JSON_INCLUDE_INSTALL_DIR       "include")
set(JSONCONS_JSON_TARGETS_EXPORT_NAME       "${PROJECT_NAME}Targets")
set(JSONCONS_JSON_CMAKE_CONFIG_TEMPLATE     "build/cmake/config.cmake.in")
set(JSONCONS_JSON_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}")
set(JSONCONS_JSON_CMAKE_VERSION_CONFIG_FILE "${JSONCONS_JSON_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(JSONCONS_JSON_CMAKE_PROJECT_CONFIG_FILE "${JSONCONS_JSON_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(JSONCONS_JSON_CMAKE_PROJECT_TARGETS_FILE "${JSONCONS_JSON_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")

set(JSONCONS_JSON_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include/")
message(STATUS "Using the multi-header code from ${JSONCONS_JSON_INCLUDE_BUILD_DIR}")

##
## TARGET
## create target and add include path
##
add_library(${JSONCONS_JSON_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${JSONCONS_JSON_TARGET_NAME} ALIAS ${JSONCONS_JSON_TARGET_NAME})
target_compile_features(${JSONCONS_JSON_TARGET_NAME} INTERFACE cxx_std_11)

target_include_directories(
    ${JSONCONS_JSON_TARGET_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${JSONCONS_JSON_INCLUDE_BUILD_DIR}>
    $<INSTALL_INTERFACE:include>
)

## add debug view definition file for msvc (natvis)
## if (MSVC)
##    set(JSONCONS_ADD_NATVIS TRUE)
##    set(JSONCONS_NATVIS_FILE "jsoncons_json.natvis")
##    target_sources(
##        ${JSONCONS_JSON_TARGET_NAME} 
##        INTERFACE 
##            $<INSTALL_INTERFACE:${JSONCONS_NATVIS_FILE}>
##            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${JSONCONS_NATVIS_FILE}>  
##    )
##endif()

##
## TESTS
## create and configure the unit test target
##
include(CTest) #adds option BUILD_TESTING (default ON)

if(BUILD_TESTING AND JSON_BuildTests)
    enable_testing()
    add_subdirectory(tests/build/cmake)
endif()

##
## INSTALL
## install header files, generate and install cmake config files for find_package()
##
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${JSONCONS_JSON_CMAKE_VERSION_CONFIG_FILE} COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    ${JSONCONS_JSON_CMAKE_CONFIG_TEMPLATE}
    ${JSONCONS_JSON_CMAKE_PROJECT_CONFIG_FILE}
    INSTALL_DESTINATION ${JSONCONS_JSON_CONFIG_INSTALL_DIR}
)

install(
    DIRECTORY ${JSONCONS_JSON_INCLUDE_BUILD_DIR}
    DESTINATION ${JSONCONS_JSON_INCLUDE_INSTALL_DIR}
)
install(
    FILES ${JSONCONS_JSON_CMAKE_PROJECT_CONFIG_FILE} ${JSONCONS_JSON_CMAKE_VERSION_CONFIG_FILE}
    DESTINATION ${JSONCONS_JSON_CONFIG_INSTALL_DIR}
)
## if (JSONCONS_ADD_NATVIS)
##    install(
##        FILES ${JSONCONS_NATVIS_FILE}
##        DESTINATION .
## )
##endif()

export(
    TARGETS ${JSONCONS_JSON_TARGET_NAME}
    NAMESPACE ${PROJECT_NAME}::
    FILE ${JSONCONS_JSON_CMAKE_PROJECT_TARGETS_FILE}
)
install(
    TARGETS ${JSONCONS_JSON_TARGET_NAME}
    EXPORT ${JSONCONS_JSON_TARGETS_EXPORT_NAME}
    INCLUDES DESTINATION ${JSONCONS_JSON_INCLUDE_INSTALL_DIR}
)
install(
    EXPORT ${JSONCONS_JSON_TARGETS_EXPORT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${JSONCONS_JSON_CONFIG_INSTALL_DIR}
)
export(PACKAGE ${PROJECT_NAME})