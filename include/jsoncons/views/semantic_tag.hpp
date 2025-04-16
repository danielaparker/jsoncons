// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SEMANTIC_TAG_HPP
#define JSONCONS_SEMANTIC_TAG_HPP

#include <ostream>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {
 
enum class semantic_tag : uint8_t 
{
    none = 0,               // 00000000 
    noesc = 1,              // 00000010
    bigint = 2,             // 00000001
    bignum = 3,             // 00000011
    bigdec = 10,            // 00001010
    float128 = 11,          // 00001011
    datetime = 4,           // 00000111
    epoch_seconds = 5,      // 00001000
    epoch_millis = 6,       // 00001001
    epoch_nanos = 7,        // 00001010 
    base16 = 8,             // 00000100
    base64 = 9,             // 00000101
    base64url = 10          // 00000110
};

inline constexpr bool is_number_tag(semantic_tag tag) noexcept
{
    constexpr uint8_t mask1{ uint8_t(semantic_tag::bigint) & uint8_t(semantic_tag::bigdec) 
        & uint8_t(semantic_tag::bignum) & uint8_t(semantic_tag::float128) };
    constexpr uint8_t mask2{ (uint8_t)~mask1};
    return (uint8_t(tag) & mask1) == mask1 && (uint8_t)(~(uint8_t)tag & mask2) == mask2;
}

JSONCONS_FORCE_INLINE
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

} // namespace jsoncons

#endif
