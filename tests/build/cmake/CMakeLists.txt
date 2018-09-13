#
# jsoncons tests CMake file
#

cmake_minimum_required (VERSION 3.4)
# load global config
include (../../../build/cmake/config.cmake)

project (Tests CXX)
set(JSONCONS_PROJECT_DIR ${PROJECT_SOURCE_DIR}/../../..)
set(JSONCONS_TESTS_SOURCE_DIR ${JSONCONS_PROJECT_DIR}/tests/src)
set(JSONCONS_TESTS_DIR ${JSONCONS_PROJECT_DIR}/tests)
set(JSONCONS_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/include)
set(JSONCONS_THIRD_PARTY_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/third_party)

set(CTEST_OUTPUT_ON_FAILURE ON)

# load per-platform configuration
include (${JSONCONS_PROJECT_DIR}/build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

if (NO_DEPRECATED)
add_definitions(-DJSONCONS_NO_DEPRECATED)
endif()

set(CATCH_INCLUDE_DIR ${JSONCONS_THIRD_PARTY_INCLUDE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories (Catch INTERFACE ${CATCH_INCLUDE_DIR})

file(GLOB_RECURSE JSONCONS_TESTS_SOURCES ${JSONCONS_TESTS_SOURCE_DIR}/*.cpp)
add_executable(jsoncons_tests ${JSONCONS_TESTS_SOURCES})
target_compile_definitions (jsoncons_tests PUBLIC)
target_compile_options(jsoncons_tests PUBLIC
   "$<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>"
)
target_include_directories (jsoncons_tests PUBLIC ${JSONCONS_INCLUDE_DIR}
                                           PUBLIC ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

target_link_libraries(jsoncons_tests Catch)

if (NOT CMAKE_CXX_STANDARD)
set_target_properties (jsoncons_tests PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
  )
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_tests -Wl,-lstdc++ Catch)
endif()

enable_testing()
add_test(NAME jsoncons_tests WORKING_DIRECTORY ${JSONCONS_TESTS_DIR} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/jsoncons_tests)

 

