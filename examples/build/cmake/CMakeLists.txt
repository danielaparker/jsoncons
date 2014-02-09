
#
# jsoncons examples CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../../build/cmake/config.cmake)


project (Examples CXX)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

include_directories (../../src
                     ../../../src)

add_executable (jsoncons_examples ../../src/array_examples.cpp
                                  ../../src/csv_examples.cpp
                                  ../../src/custom_data_examples.cpp
                                  ../../src/examples.cpp)

# special link option on Linux because llvm stl rely on GNU stl
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_link_libraries (jsoncons_examples -Wl,-lstdc++)
endif()
