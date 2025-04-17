// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_BSON_BSON_TYPE_HPP
#define JSONCONS_EXT_BSON_BSON_TYPE_HPP

#include <cstdint>

namespace jsoncons { 
namespace bson {

    namespace bson_type
    {
        JSONCONS_INLINE_CONSTEXPR uint8_t double_type = 0x01;
        JSONCONS_INLINE_CONSTEXPR uint8_t string_type = 0x02; // UTF-8 string
        JSONCONS_INLINE_CONSTEXPR uint8_t document_type = 0x03;
        JSONCONS_INLINE_CONSTEXPR uint8_t array_type = 0x04;
        JSONCONS_INLINE_CONSTEXPR uint8_t binary_type = 0x05;
        JSONCONS_INLINE_CONSTEXPR uint8_t undefined_type = 0x06; // map to null
        JSONCONS_INLINE_CONSTEXPR uint8_t object_id_type = 0x07;
        JSONCONS_INLINE_CONSTEXPR uint8_t bool_type = 0x08;
        JSONCONS_INLINE_CONSTEXPR uint8_t datetime_type = 0x09;
        JSONCONS_INLINE_CONSTEXPR uint8_t null_type = 0x0a;
        JSONCONS_INLINE_CONSTEXPR uint8_t regex_type = 0x0b;
        JSONCONS_INLINE_CONSTEXPR uint8_t javascript_type = 0x0d;
        JSONCONS_INLINE_CONSTEXPR uint8_t symbol_type = 0x0e; // deprecated, mapped to string
        JSONCONS_INLINE_CONSTEXPR uint8_t javascript_with_scope_type = 0x0f; // unsupported
        JSONCONS_INLINE_CONSTEXPR uint8_t int32_type = 0x10;
        JSONCONS_INLINE_CONSTEXPR uint8_t timestamp_type = 0x11; // MongoDB internal Timestamp, uint64
        JSONCONS_INLINE_CONSTEXPR uint8_t int64_type = 0x12;
        JSONCONS_INLINE_CONSTEXPR uint8_t decimal128_type = 0x13;
        JSONCONS_INLINE_CONSTEXPR uint8_t min_key_type = 0xff;
        JSONCONS_INLINE_CONSTEXPR uint8_t max_key_type = 0x7f;
    }

    enum class bson_container_type {document, array};

} // namespace bson
} // namespace jsoncons

#endif // JSONCONS_EXT_BSON_BSON_TYPE_HPP
