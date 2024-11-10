
#
# jsoncons examples CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../build/cmake/Config.cmake)


project (Examples CXX)

# load per-platform configuration
include (../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

include_directories (../../include
                     ../../../include)

file(GLOB_RECURSE Example_sources ../../src/*.cpp)

# Loop through each example file and create an executable for each
foreach(example_file ${Example_sources})
    # Extract the filename without path and extension
    get_filename_component(example_name ${example_file} NAME_WE)

    # Create an executable with the example name and file
    add_executable(${example_name} ${example_file})

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
      # special link option on Linux because llvm stl rely on GNU stl
      target_link_libraries(${example_name} -Wl,-lstdc++)
    endif()
endforeach()
