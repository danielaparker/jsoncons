
#
# jsoncons examples CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../../build/cmake/config.cmake)


project (Examples CXX)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

include_directories (../../include
                     ../../../include)

file(GLOB_RECURSE Example_sources ../../src/*.cpp)

add_executable (jsoncons_examples ${Example_sources})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_examples -Wl,-lstdc++)
endif()
