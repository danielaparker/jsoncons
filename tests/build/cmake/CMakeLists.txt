#
# jsoncons tests suite CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../../build/cmake/config.cmake)

project (Tests CXX)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

file(GLOB_RECURSE UnitTests_sources ../../src/*.cpp)

add_executable(jsoncons_tests
    ${UnitTests_sources}
)

target_compile_definitions (jsoncons_tests PUBLIC)

if (NO_DEPRECATED)
add_definitions(-DJSONCONS_NO_DEPRECATED)
endif()

target_include_directories (jsoncons_tests PUBLIC ../../../include
                                           PUBLIC ../../../third_party
                                           PRIVATE ../../include)

# target_link_libraries (jsoncons_tests)

target_link_libraries (jsoncons_tests)

    add_test(NAME "${testcase}_default"
      COMMAND ${testcase} ${CATCH_TEST_FILTER}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties("${testcase}_default" PROPERTIES LABELS "default")

    add_test(NAME "${testcase}_all"
      COMMAND ${testcase} ${CATCH_TEST_FILTER} "*"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties("${testcase}_all" PROPERTIES LABELS "all")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_tests -Wl,-lstdc++)
endif()
