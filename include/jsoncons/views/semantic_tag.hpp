// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS2_SEMANTIC_TAG_HPP
#define JSONCONS2_SEMANTIC_TAG_HPP

#include <ostream>
#include <jsoncons/views/jsoncons_config.hpp>

namespace jsoncons2 {
 
enum class semantic_tag : uint8_t 
{
    none = 0,               // 00000000 
    noesc = 1,              // 00000001
    datetime = 2,           // 00000010
    epoch_seconds = 3,      // 00000011
    epoch_millis = 4,       // 00000100
    epoch_nanos = 5,        // 00000101
    base16 = 6,             // 00000110
    base64 = 7,             // 00000111
    base64url = 8,          // 00001000
    bigint = 12,            // 00001100
    bignum = 13,            // 00001101
    bigdec = 14,            // 00001110
    float128 = 15           // 00001111
};

inline constexpr bool is_number_tag(semantic_tag tag) noexcept
{
    constexpr uint8_t mask{ uint8_t(semantic_tag::bigint) & uint8_t(semantic_tag::bigdec) 
        & uint8_t(semantic_tag::bignum) & uint8_t(semantic_tag::float128) };
    return (uint8_t(tag) & mask) == mask;
}

JSONCONS2_FORCEINLINE
std::ostream& operator<<(std::ostream& os, semantic_tag tag)
{
    switch (tag)
    {
        case semantic_tag::none:
        {
            os << "none";
            break;
        }
        case semantic_tag::noesc:
        {
            os << "noesc";
            break;
        }
        case semantic_tag::bigint:
        {
            os << "bigint";
            break;
        }
        case semantic_tag::bignum:
        {
            os << "bignum";
            break;
        }
        default:
        {
            os << "undefined";
            break;
        }
    }
    return os;
}

} // namespace jsoncons2

#endif
