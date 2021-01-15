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

if (JSONCONS_SANITIZE)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
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
add_library(Catch INTERFACE)
target_include_directories (Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable(unit_tests 
               src/main.cpp
               src/jsoncons_tests.cpp
               src/JSONTestSuite_tests.cpp
               src/bigint_tests.cpp
               bson/src/bson_cursor_tests.cpp
               bson/src/bson_encoder_tests.cpp
               bson/src/bson_reader_tests.cpp
               bson/src/bson_test_suite.cpp
               bson/src/encode_decode_bson_tests.cpp
               src/byte_string_tests.cpp
               cbor/src/cbor_bitset_traits_tests.cpp
               cbor/src/cbor_cursor_tests.cpp
               cbor/src/cbor_encoder_tests.cpp
               cbor/src/cbor_json_visitor2_tests.cpp
               cbor/src/cbor_reader_tests.cpp
               cbor/src/cbor_tests.cpp
               cbor/src/cbor_typed_array_tests.cpp
               cbor/src/decode_cbor_tests.cpp
               cbor/src/encode_cbor_tests.cpp
               src/converter_tests.cpp
               csv/src/csv_cursor_tests.cpp
               csv/src/csv_subfield_tests.cpp
               csv/src/csv_tests.cpp
               csv/src/encode_decode_csv_tests.cpp
               src/decode_traits_tests.cpp
               src/detail/optional_tests.cpp
               src/detail/span_tests.cpp
               src/detail/string_view_tests.cpp
               src/detail/string_wrapper_tests.cpp
               src/detail/to_integer_tests.cpp
               src/double_round_trip_tests.cpp
               src/double_to_string_tests.cpp
               src/dtoa_tests.cpp
               src/encode_decode_json_tests.cpp
               src/error_recovery_tests.cpp
               fuzz_regression/src/fuzz_regression_tests.cpp
               jmespath/src/jmespath_tests.cpp
               src/json_array_tests.cpp
               src/json_as_tests.cpp
               src/json_bitset_traits_tests.cpp
               src/json_checker_tests.cpp
               src/json_comparator_tests.cpp
               src/json_const_pointer_tests.cpp
               src/json_constructor_tests.cpp
               src/json_cursor_tests.cpp
               src/json_encoder_tests.cpp
               src/json_exception_tests.cpp
               src/json_filter_tests.cpp
               src/json_in_place_update_tests.cpp
               src/json_integer_tests.cpp
               src/json_less_tests.cpp
               src/json_line_split_tests.cpp
               src/json_literal_operator_tests.cpp
               src/json_object_tests.cpp
               src/json_options_tests.cpp
               src/json_parse_error_tests.cpp
               src/json_parser_position_tests.cpp
               src/json_parser_tests.cpp
               src/json_proxy_tests.cpp
               src/json_push_back_tests.cpp
               src/json_reader_exception_tests.cpp
               src/json_reader_tests.cpp
               src/json_storage_tests.cpp
               src/json_swap_tests.cpp
               src/json_traits_macro_tests.cpp
               src/json_traits_name_macro_tests.cpp
               src/json_traits_macro_functional_tests.cpp
               src/json_type_traits_container_tests.cpp
               src/json_type_traits_chrono_tests.cpp
               src/json_type_traits_tests.cpp
               src/json_validation_tests.cpp
               jsonpatch/src/jsonpatch_tests.cpp
               jsonpath/src/JSONPathTestSuite_tests.cpp
               jsonpath/src/jsonpath_error_tests.cpp
               jsonpath/src/jsonpath_flatten_tests.cpp
               jsonpath/src/jsonpath_function_tests.cpp
               jsonpath/src/jsonpath_normalized_path_tests.cpp
               jsonpath/src/jsonpath_test_suite.cpp
               jsonpath/src/jsonpath_tests.cpp
               jsonpointer/src/jsonpointer_flatten_tests.cpp
               jsonpointer/src/jsonpointer_tests.cpp
               jsonschema/src/format_checker_tests.cpp
               jsonschema/src/jsonschema_output_format_tests.cpp
               jsonschema/src/jsonschema_defaults_tests.cpp
               jsonschema/src/jsonschema_tests.cpp
               msgpack/src/decode_msgpack_tests.cpp
               msgpack/src/encode_msgpack_tests.cpp
               msgpack/src/msgpack_bitset_traits_tests.cpp
               msgpack/src/msgpack_cursor_tests.cpp
               msgpack/src/msgpack_encoder_tests.cpp
               msgpack/src/msgpack_tests.cpp
               msgpack/src/msgpack_timestamp_tests.cpp
               src/ojson_tests.cpp
               src/order_preserving_json_object_tests.cpp
               src/parse_string_tests.cpp
               src/encode_traits_tests.cpp
               src/short_string_tests.cpp
               src/staj_iterator_tests.cpp
               src/stateful_allocator_tests.cpp
               src/string_to_double_tests.cpp
               ubjson/src/decode_ubjson_tests.cpp
               ubjson/src/encode_ubjson_tests.cpp
               ubjson/src/ubjson_cursor_tests.cpp
               ubjson/src/ubjson_encoder_tests.cpp
               src/unicode_tests.cpp
               src/wjson_tests.cpp
)

target_compile_features(unit_tests INTERFACE cxx_std_11)

target_compile_options(unit_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>: /EHsc /MP /bigobj /W4>
    $<$<CXX_COMPILER_ID:Clang>:-Werror -Wall -Wextra -Wimplicit-fallthrough -Wcast-align -Wcast-qual>
    $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra -Wimplicit-fallthrough -pedantic -Wcast-align -Wcast-qual>
)

add_test(jsoncons unit_tests
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

target_include_directories (unit_tests 
                            PUBLIC ${JSONCONS_INCLUDE_DIR} 
                            PRIVATE ${JSONCONS_TESTS_DIR}
                            PRIVATE ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

target_link_libraries(unit_tests Catch)

