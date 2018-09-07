#
# jsoncons tests CMake file
#

cmake_minimum_required (VERSION 3.4)

# load global config
include (../../../build/cmake/config.cmake)

project (Tests CXX)
set(JSONCONS_PROJECT_DIR ${PROJECT_SOURCE_DIR}/../../..)
set(JSONCONS_TESTS_SOURCE_DIR ${JSONCONS_PROJECT_DIR}/tests/src)
set(JSONCONS_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/include)
set(JSONCONS_THIRD_PARTY_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/third_party)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

file(GLOB_RECURSE JSONCONS_TESTS_SOURCES ${JSONCONS_TESTS_SOURCE_DIR}/*.cpp)

add_executable(jsoncons_tests ${JSONCONS_TESTS_SOURCES})

target_compile_definitions (jsoncons_tests PUBLIC)

if (NO_DEPRECATED)
add_definitions(-DJSONCONS_NO_DEPRECATED)
endif()

target_include_directories (jsoncons_tests PUBLIC ${JSONCONS_INCLUDE_DIR}
                                           PUBLIC ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

# target_link_libraries (jsoncons_tests)

target_link_libraries (jsoncons_tests)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_tests -Wl,-lstdc++)
endif()
