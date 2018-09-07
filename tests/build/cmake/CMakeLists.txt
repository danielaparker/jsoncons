#
# jsoncons tests CMake file
#

cmake_minimum_required (VERSION 3.4)

# load global config
include (../../../build/cmake/config.cmake)

project (Tests CXX)
# set(JSONCONS_TESTS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../src)
set(JSONCONS_TESTS_SOURCE_DIR ../../src)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

file(GLOB_RECURSE JSONCONS_TESTS_SOURCES ${JSONCONS_TESTS_SOURCE_DIR}/*.cpp)

add_executable(JSONCONS_TESTS ${JSONCONS_TESTS_SOURCES})

target_compile_definitions (JSONCONS_TESTS PUBLIC)

if (NO_DEPRECATED)
add_definitions(-DJSONCONS_NO_DEPRECATED)
endif()

target_include_directories (JSONCONS_TESTS PUBLIC ../../../include
                                           PUBLIC ../../../third_party
                                           PRIVATE ../../include)

# target_link_libraries (JSONCONS_TESTS)

target_link_libraries (JSONCONS_TESTS)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (JSONCONS_TESTS -Wl,-lstdc++)
endif()
