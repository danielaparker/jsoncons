if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(jsoncons-test)

    find_package(jsoncons REQUIRED CONFIG)
    set(JSONCONS_INCLUDE_DIR ${jsoncons_INCLUDE_DIRS})
endif ()

if(NOT CMAKE_BUILD_TYPE)
message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

OPTION(JSONCONS_SANITIZE "sanitize" OFF)
option(JSONCONS_VALGRIND "Execute tests with valgrind" OFF)

if(JSONCONS_VALGRIND)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

include (CTest)

# load per-platform configuration
# include (${JSONCONS_PROJECT_DIR}/cmake/${CMAKE_SYSTEM_NAME}.cmake)

# -fsanitize=undefined clang issue
if (JSONCONS_SANITIZE)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fsanitize=address -fno-omit-frame-pointer")
        message(STATUS "SANITIZE ON")
    endif()
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

set(CTEST_OUTPUT_ON_FAILURE ON)

set(JSONCONS_TESTS_DIR ${JSONCONS_PROJECT_DIR}/tests)
set(JSONCONS_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/include)
set(JSONCONS_THIRD_PARTY_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/third_party)

set(CATCH_INCLUDE_DIR ${JSONCONS_THIRD_PARTY_INCLUDE_DIR}/catch)
add_library(catch INTERFACE)

if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
    target_compile_features(catch INTERFACE cxx_range_for)  # for C++11 - flags
else()
    target_compile_features(catch INTERFACE cxx_std_11)
endif()

target_include_directories (catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable(unit_tests 
               cbor/src/cbor_bitset_traits_tests.cpp
               cbor/src/cbor_cursor_tests.cpp
               cbor/src/cbor_encoder_tests.cpp
               cbor/src/cbor_json_visitor2_tests.cpp
               cbor/src/cbor_reader_tests.cpp
               cbor/src/cbor_tests.cpp
               cbor/src/decode_cbor_tests.cpp
               cbor/src/encode_cbor_tests.cpp
               src/main.cpp
)

if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
    target_compile_features(unit_tests INTERFACE cxx_range_for)  # for C++11 - flags
else()
    target_compile_features(unit_tests INTERFACE cxx_std_11)
endif()

target_compile_options(unit_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /EHsc /MP /bigobj /W4>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "8.0")
    message(STATUS Version " ${CMAKE_CXX_COMPILER_VERSION}")
    # older GCC versions don't support 
   target_compile_options(unit_tests PRIVATE
       $<$<CXX_COMPILER_ID:GNU>:-Wnon-virtual-dtor -Werror -Wall -Wextra -Wcast-align -Wcast-qual -Wimplicit-fallthrough -Wsign-compare -pedantic>
   )
endif()

if (NOT CMAKE_SYSTEM_NAME STREQUAL Windows)
target_compile_options(unit_tests PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Werror -Wall -Wextra -Wimplicit-fallthrough -Wcast-align -Wcast-qual -Wsign-compare -pedantic>
)
endif()

add_test(jsoncons unit_tests
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

target_include_directories (unit_tests 
                            PUBLIC ${JSONCONS_INCLUDE_DIR} 
                            PRIVATE ${JSONCONS_TESTS_DIR}
                            PRIVATE ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

target_link_libraries(unit_tests catch)

