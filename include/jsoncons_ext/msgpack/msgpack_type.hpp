// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_MSGPACK_MSGPACK_TYPE_HPP
#define JSONCONS_EXT_MSGPACK_MSGPACK_TYPE_HPP

#include <cstdint>

namespace jsoncons { 
namespace msgpack {

    namespace msgpack_type
    {
        JSONCONS_INLINE_CONSTEXPR uint8_t positive_fixint_base_type = 0x00;
        JSONCONS_INLINE_CONSTEXPR uint8_t nil_type = 0xc0;
        JSONCONS_INLINE_CONSTEXPR uint8_t false_type = 0xc2;
        JSONCONS_INLINE_CONSTEXPR uint8_t true_type = 0xc3;
        JSONCONS_INLINE_CONSTEXPR uint8_t float32_type = 0xca;
        JSONCONS_INLINE_CONSTEXPR uint8_t float64_type = 0xcb;
        JSONCONS_INLINE_CONSTEXPR uint8_t uint8_type = 0xcc;
        JSONCONS_INLINE_CONSTEXPR uint8_t uint16_type = 0xcd;
        JSONCONS_INLINE_CONSTEXPR uint8_t uint32_type = 0xce;
        JSONCONS_INLINE_CONSTEXPR uint8_t uint64_type = 0xcf;
        JSONCONS_INLINE_CONSTEXPR uint8_t int8_type = 0xd0;
        JSONCONS_INLINE_CONSTEXPR uint8_t int16_type = 0xd1;
        JSONCONS_INLINE_CONSTEXPR uint8_t int32_type = 0xd2;
        JSONCONS_INLINE_CONSTEXPR uint8_t int64_type = 0xd3;

        JSONCONS_INLINE_CONSTEXPR uint8_t fixmap_base_type = 0x80;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixarray_base_type = 0x90;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixstr_base_type = 0xa0;
        JSONCONS_INLINE_CONSTEXPR uint8_t str8_type = 0xd9;
        JSONCONS_INLINE_CONSTEXPR uint8_t str16_type = 0xda;
        JSONCONS_INLINE_CONSTEXPR uint8_t str32_type = 0xdb;

        JSONCONS_INLINE_CONSTEXPR uint8_t bin8_type = 0xc4; //  0xC4
        JSONCONS_INLINE_CONSTEXPR uint8_t bin16_type = 0xc5;
        JSONCONS_INLINE_CONSTEXPR uint8_t bin32_type = 0xc6;

        JSONCONS_INLINE_CONSTEXPR uint8_t fixext1_type = 0xd4;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixext2_type = 0xd5;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixext4_type = 0xd6;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixext8_type = 0xd7;
        JSONCONS_INLINE_CONSTEXPR uint8_t fixext16_type = 0xd8;
        JSONCONS_INLINE_CONSTEXPR uint8_t ext8_type = 0xc7; //  0xC4
        JSONCONS_INLINE_CONSTEXPR uint8_t ext16_type = 0xc8;
        JSONCONS_INLINE_CONSTEXPR uint8_t ext32_type = 0xc9;

        JSONCONS_INLINE_CONSTEXPR uint8_t array16_type = 0xdc;
        JSONCONS_INLINE_CONSTEXPR uint8_t array32_type = 0xdd;
        JSONCONS_INLINE_CONSTEXPR uint8_t map16_type = 0xde;
        JSONCONS_INLINE_CONSTEXPR uint8_t map32_type = 0xdf;
        JSONCONS_INLINE_CONSTEXPR uint8_t negative_fixint_base_type = 0xe0;
    }
 
} // namespace msgpack
} // namespace jsoncons

#endif // JSONCONS_EXT_MSGPACK_MSGPACK_TYPE_HPP
