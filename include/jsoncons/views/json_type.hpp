// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_HPP
#define JSONCONS_JSON_TYPE_HPP

namespace jsoncons {

enum class json_type : uint8_t
{
    null_value = 1,      // ____0001
    bool_value = 2,      // ____0010
    uint64_value = 3,    // ____0011
    int64_value = 4,     // ____0100
    double_value = 5,    // ____0101
    string_value = 6,    // ____0110
    array_value = 14,    // ____1110
    object_value = 15    // ____1111
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

struct json_array_arg_t
{
    explicit json_array_arg_t() = default; 
};

inline constexpr json_array_arg_t json_array_arg{};

struct json_object_arg_t
{
    explicit json_object_arg_t() = default; 
};

inline constexpr json_object_arg_t json_object_arg{};

struct raw_json_arg_t
{
    explicit raw_json_arg_t() = default; 
};

inline constexpr raw_json_arg_t raw_json_arg{};

struct noesc_arg_t
{
    explicit noesc_arg_t() = default; 
};


} // namespace jsoncons

#endif // JSONCONS_JSON_TYPE_HPP
