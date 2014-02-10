
#
# jsoncons tests suite CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../../build/cmake/config.cmake)


project (Tests CXX)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)


set (Boost_NO_BOOST_CMAKE ON)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)

add_executable (jsoncons_tests ../../src/csv_tests.cpp
                               ../../src/double_to_string_tests.cpp
                               ../../src/json_accessor_tests.cpp
                               ../../src/json_array_tests.cpp
                               ../../src/json_construction_tests.cpp
                               ../../src/json_equals_tests.cpp
                               ../../src/json_object_tests.cpp
                               ../../src/json_parser_test.cpp
                               ../../src/json_reader_exception_tests.cpp
                               ../../src/json_serializer_tests.cpp
                               ../../src/jsoncons_test.cpp
                               ../../src/string_to_double_tests.cpp)

target_compile_definitions (jsoncons_tests PUBLIC BOOST_DYN_LINK BOOST_TEST_DYN_LINK)

target_include_directories (jsoncons_tests PUBLIC ${Boost_INCLUDE_DIRS}
                                           PUBLIC ../../../src
                                           PRIVATE ../../src)

target_link_libraries (jsoncons_tests ${Boost_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_tests -Wl,-lstdc++)
endif()
