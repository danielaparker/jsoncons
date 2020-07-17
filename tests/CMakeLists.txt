cmake_minimum_required(VERSION 3.0.2)

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

################
# ARM SETTINGS #
################

OPTION(CROSS_COMPILE_ARM "cross compile for ARM targets" OFF)
OPTION(JSONCONS_SANITIZE "sanitize" OFF)
option(JSONCONS_VALGRIND "Execute tests with valgrind" OFF)

if(JSONCONS_VALGRIND)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

include (CTest)

# Note: to compile on ARM (or cross compile), you may need to add the following:
# -DTARGET_ARCH="armv8-a -mfpu=neon -mfloat-abi=softfp -target arm-linux-gnueabi"
set(TARGET_ARCH "native" CACHE STRING "Target architecture arguments")
set(ARM_ARCH_DIRECTORY "arm-linux-gnueabi" CACHE STRING "ARM arch header dir")
set(ARM_GCC_VER "4.7.3" CACHE STRING "ARM GCC header dir")

# load per-platform configuration
include (${JSONCONS_PROJECT_DIR}/build_files/cmake/${CMAKE_SYSTEM_NAME}.cmake)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    if (CROSS_COMPILE_ARM)
        # We're cross-compiling with clang++ on Travis, this is all pretty specific and just for testing
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
        set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
        set(CMAKE_THREAD_LIBS_INIT)

        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/c++/${ARM_GCC_VER}/${ARM_ARCH_DIRECTORY}/)
        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/c++/${ARM_GCC_VER}/)
        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${TARGET_ARCH} -Wunused-parameter -Wextra -Wreorder -std=c++11")
    endif()
endif()
if (JSONCONS_SANITIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP /bigobj")
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

set(CTEST_OUTPUT_ON_FAILURE ON)

set(JSONCONS_TESTS_DIR ${JSONCONS_PROJECT_DIR}/tests)
set(JSONCONS_TESTS_SOURCE_DIR ${JSONCONS_PROJECT_DIR}/tests/src)
set(JSONCONS_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/include)
set(JSONCONS_THIRD_PARTY_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/third_party)

set(CATCH_INCLUDE_DIR ${JSONCONS_THIRD_PARTY_INCLUDE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories (Catch INTERFACE ${CATCH_INCLUDE_DIR})

#file(GLOB_RECURSE JSONCONS_TESTS_SOURCES ${JSONCONS_TESTS_SOURCE_DIR}/*.cpp)
#message((${JSONCONS_TESTS_SOURCES}))

set(JSONCONS_TESTS_SOURCES
   ${JSONCONS_TESTS_SOURCE_DIR}/jsoncons_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/JSONTestSuite_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bigint_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bson/bson_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bson/bson_encoder_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bson/bson_reader_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bson/bson_test_suite.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/bson/encode_decode_bson_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/byte_string_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_encoder_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_json_visitor2_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_reader_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/cbor_typed_array_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/decode_cbor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/cbor/encode_cbor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/converter_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/csv/csv_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/csv/csv_subfield_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/csv/csv_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/csv/encode_decode_csv_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/deser_traits_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/detail/optional_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/detail/span_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/detail/string_view_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/detail/string_wrapper_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/detail/to_integer_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/double_round_trip_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/double_to_string_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/dtoa_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/encode_decode_json_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/error_recovery_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/fuzz_regression/fuzz_regression_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jmespath/jmespath_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_array_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_as_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_checker_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_comparator_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_constructor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_encoder_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_exception_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_filter_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_in_place_update_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_integer_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_less_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_line_split_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_literal_operator_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_object_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_options_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_parse_error_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_parser_position_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_parser_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_proxy_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_push_back_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_reader_exception_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_reader_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_storage_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_swap_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_traits_macro_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_traits_name_macro_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_type_traits_container_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_type_traits_string_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/json_type_traits_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpatch/jsonpatch_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/JSONPathTestSuite_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_error_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_flatten_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_function_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_normalized_path_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_test_suite.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpath/jsonpath_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpointer/jsonpointer_flatten_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/jsonpointer/jsonpointer_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/msgpack/decode_msgpack_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/msgpack/encode_msgpack_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/msgpack/msgpack_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/msgpack/msgpack_encoder_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/msgpack/msgpack_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ojson_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/order_preserving_json_object_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/parse_string_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ser_traits_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/short_string_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/staj_iterator_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/stateful_allocator_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/string_to_double_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/tests_main.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ubjson/decode_ubjson_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ubjson/encode_ubjson_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ubjson/ubjson_cursor_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/ubjson/ubjson_encoder_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/unicode_tests.cpp
   ${JSONCONS_TESTS_SOURCE_DIR}/wjson_tests.cpp
)
set(JSONCONS_TARGET test_jsoncons)
add_executable(${JSONCONS_TARGET} EXCLUDE_FROM_ALL ${JSONCONS_TESTS_SOURCES} ${JSONCONS_HEADERS})

add_test(example_test ${JSONCONS_TARGET})

target_include_directories (${JSONCONS_TARGET} PUBLIC ${JSONCONS_INCLUDE_DIR}
                                           PUBLIC ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

target_link_libraries(${JSONCONS_TARGET} Catch)

if (CROSS_COMPILE_ARM)
    add_custom_target(jtest COMMAND qemu-arm -L /usr/arm-linux-gnueabi/ test_jsoncons DEPENDS ${JSONCONS_TARGET})
else()
    add_custom_target(jtest COMMAND test_jsoncons DEPENDS ${JSONCONS_TARGET})
endif()

