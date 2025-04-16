// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS2_JSON_TYPE_HPP
#define JSONCONS2_JSON_TYPE_HPP

namespace jsoncons2 {

enum class json_type : uint8_t
{
    none = 0,      // ____0000 
    null = 1,      // ____0001
    boolean = 2,   // ____0010
    uint64 = 3,    // ____0011
    int64 = 4,     // ____0100
    float64 = 5,   // ____0101
    string = 6,    // ____0110
    array = 14,    // ____1110
    object = 15    // ____1111
};    

struct null_type
{
    explicit null_type() = default;
};

struct null_arg_t
{
    explicit null_arg_t() = default; 
};

inline constexpr null_arg_t null_arg{};

struct byte_string_arg_t
{
    explicit byte_string_arg_t() = default; 
};

inline constexpr byte_string_arg_t byte_string_arg{};

struct half_float_arg_t
{
    explicit half_float_arg_t() = default; 
};

inline constexpr half_float_arg_t half_float_arg{};

struct array_arg_t
{
    explicit array_arg_t() = default; 
};

inline constexpr array_arg_t array_arg{};

struct object_arg_t
{
    explicit object_arg_t() = default; 
};

inline constexpr object_arg_t object_arg{};

struct raw_json_arg_t
{
    explicit raw_json_arg_t() = default; 
};

inline constexpr raw_json_arg_t raw_json_arg{};

struct noesc_arg_t
{
    explicit noesc_arg_t() = default; 
};


} // namespace jsoncons2

#endif // JSONCONS2_JSON_TYPE_HPP
