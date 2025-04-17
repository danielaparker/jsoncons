// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_UBJSON_UBJSON_TYPE_HPP
#define JSONCONS_EXT_UBJSON_UBJSON_TYPE_HPP

#include <cstdint>

#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 
namespace ubjson { 

    namespace ubjson_type
    {
        JSONCONS_INLINE_CONSTEXPR uint8_t null_type = 'Z';
        JSONCONS_INLINE_CONSTEXPR uint8_t no_op_type = 'N';
        JSONCONS_INLINE_CONSTEXPR uint8_t true_type = 'T';
        JSONCONS_INLINE_CONSTEXPR uint8_t false_type = 'F';
        JSONCONS_INLINE_CONSTEXPR uint8_t int8_type = 'i';
        JSONCONS_INLINE_CONSTEXPR uint8_t uint8_type = 'U';
        JSONCONS_INLINE_CONSTEXPR uint8_t int16_type = 'I';
        JSONCONS_INLINE_CONSTEXPR uint8_t int32_type = 'l';
        JSONCONS_INLINE_CONSTEXPR uint8_t int64_type = 'L';
        JSONCONS_INLINE_CONSTEXPR uint8_t float32_type = 'd';
        JSONCONS_INLINE_CONSTEXPR uint8_t float64_type = 'D';
        JSONCONS_INLINE_CONSTEXPR uint8_t high_precision_number_type = 'H';
        JSONCONS_INLINE_CONSTEXPR uint8_t char_type = 'C';
        JSONCONS_INLINE_CONSTEXPR uint8_t string_type = 'S';
        JSONCONS_INLINE_CONSTEXPR uint8_t start_array_marker = '[';
        JSONCONS_INLINE_CONSTEXPR uint8_t end_array_marker = ']';
        JSONCONS_INLINE_CONSTEXPR uint8_t start_object_marker = '{';
        JSONCONS_INLINE_CONSTEXPR uint8_t end_object_marker = '}';
        JSONCONS_INLINE_CONSTEXPR uint8_t type_marker = '$';
        JSONCONS_INLINE_CONSTEXPR uint8_t count_marker = '#';
    }
 
} // namespace ubjson
} // namespace jsoncons

#endif // JSONCONS_EXT_UBJSON_UBJSON_TYPE_HPP
