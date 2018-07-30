// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORPARSER_HPP
#define JSONCONS_CBOR_CBORPARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <iterator>
#include <jsoncons/json.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/binary/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_error_category.hpp>

enum class cbor_major_type : uint8_t
{
    unsigned_integer = 0x00,
    negative_integer = 0x01,
    byte_string = 0x02,
    text_string = 0x03,
    array = 0x04,
    map = 0x05,   
    semantic_tag = 0x06,
    simple = 0x7
};

const uint8_t major_type_shift = 0x05;
const unsigned major_type_mask = (~0U << major_type_shift);
const uint8_t indefinite_length = 0x1f;
const uint8_t additional_information_mask = (1U << 5) - 1;

namespace additional_information
{
    const uint8_t indefinite_length = 0x1f;
}

inline
cbor_major_type get_major_type(uint8_t type)
{
    uint8_t value = type >> major_type_shift;
    return static_cast<cbor_major_type>(value);
}

inline
uint8_t get_additional_information_value(uint8_t type)
{
    uint8_t value = type & additional_information_mask;
    return value;
}

// Positive integer 0x00..0x17 (0..23)
#define JSONCONS_CBOR_0x00_0x17 \
        0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x09:case 0x0a:case 0x0b:case 0x0c:case 0x0d:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:case 0x17

// Negative integer -1-0x00..-1-0x17 (-1..-24)
#define JSONCONS_CBOR_0x20_0x37 \
        0x20:case 0x21:case 0x22:case 0x23:case 0x24:case 0x25:case 0x26:case 0x27:case 0x28:case 0x29:case 0x2a:case 0x2b:case 0x2c:case 0x2d:case 0x2e:case 0x2f:case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:case 0x37

// byte string (0x00..0x17 bytes follow)
#define JSONCONS_CBOR_0x40_0x57 \
        0x40:case 0x41:case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:case 0x4c:case 0x4d:case 0x4e:case 0x4f:case 0x50:case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57

// UTF-8 string (0x00..0x17 bytes follow)
#define JSONCONS_CBOR_0x60_0x77 \
        0x60:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:case 0x70:case 0x71:case 0x72:case 0x73:case 0x74:case 0x75:case 0x76:case 0x77

// array (0x00..0x17 data items follow)
#define JSONCONS_CBOR_0x80_0x97 \
        0x80:case 0x81:case 0x82:case 0x83:case 0x84:case 0x85:case 0x86:case 0x87:case 0x88:case 0x89:case 0x8a:case 0x8b:case 0x8c:case 0x8d:case 0x8e:case 0x8f:case 0x90:case 0x91:case 0x92:case 0x93:case 0x94:case 0x95:case 0x96:case 0x97

// map (0x00..0x17 pairs of data items follow)
#define JSONCONS_CBOR_0xa0_0xb7 \
        0xa0:case 0xa1:case 0xa2:case 0xa3:case 0xa4:case 0xa5:case 0xa6:case 0xa7:case 0xa8:case 0xa9:case 0xaa:case 0xab:case 0xac:case 0xad:case 0xae:case 0xaf:case 0xb0:case 0xb1:case 0xb2:case 0xb3:case 0xb4:case 0xb5:case 0xb6:case 0xb7

namespace jsoncons { namespace cbor {

class cbor_decode_error : public std::invalid_argument, public virtual json_exception
{
public:
    explicit cbor_decode_error(size_t pos) JSONCONS_NOEXCEPT
        : std::invalid_argument("")
    {
        buffer_.append("Error decoding a cbor at position ");
        buffer_.append(std::to_string(pos));
    }
    ~cbor_decode_error() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

namespace detail {

void walk(const uint8_t* first, const uint8_t* last, const uint8_t** endp);

inline 
size_t get_byte_string_length(const uint8_t* first, const uint8_t* last, 
                              const uint8_t** endp)
{
    size_t length = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
            {
                length = *first & 0x1f;
                *endp = p;
                break;
            }
        case 0x58: // byte string (one-byte uint8_t for n follows)
            {
                length = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x59: // byte string (two-byte uint16_t for n follow)
            {
                length = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x5a: // byte string (four-byte uint32_t for n follow)
            {
                length = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x5b: // byte string (eight-byte uint64_t for n follow)
            {
                length = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x5f: // byte string, byte strings follow, terminated by "break"
            {
                length = 0;
                while (*p != 0xff)
                {
                    size_t len = detail::get_byte_string_length(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    length += len;
                }
                break;
            }
        default: 
            {
                *endp = first;
            }
        }
    }
    return length;
}

inline 
std::vector<uint8_t> get_byte_string(const uint8_t* first, const uint8_t* last, 
                                     const uint8_t** endp)
{
    std::vector<uint8_t> v;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
            {
                size_t length = *first & 0x1f;
                *endp = p + length;
                v = std::vector<uint8_t>(p, *endp);
            }
            break;
        case 0x58: // byte string (one-byte uint8_t for n follows)
            {
                const auto length = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    v = std::vector<uint8_t>(p, *endp);
                }
            }
            break;
        case 0x59: // byte string (two-byte uint16_t for n follow)
            {
                const auto length = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    v = std::vector<uint8_t>(p, *endp);
                }
            }
            break;
        case 0x5a: // byte string (four-byte uint32_t for n follow)
            {
                const auto length = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    v = std::vector<uint8_t>(p, *endp);
                }
            }
            break;
        case 0x5b: // byte string (eight-byte uint64_t for n follow)
            {
                const auto length = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    v = std::vector<uint8_t>(p, *endp);
                }
            }
            break;
        case 0x5f: // byte string, byte strings follow, terminated by "break"
            {
                while (*p != 0xff)
                {
                    std::vector<uint8_t> ss = detail::get_byte_string(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    else
                    {
                        p = *endp;
                        v.insert(v.end(),ss.begin(),ss.end());
                    }
                }
            }
            break;
        default: 
            {
                *endp = first;
            }
        }
    }
    return v;
}

inline 
size_t get_text_string_length(const uint8_t* first, const uint8_t* last, 
                              const uint8_t** endp)
{
    size_t length = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x60_0x77: // UTF-8 string (0x00..0x17 bytes follow)
            {
                length = *first & 0x1f;
                *endp = p;
                break;
            }
        case 0x78: // UTF-8 string (one-byte uint8_t for n follows)
            {
                length = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x79: // UTF-8 string (two-byte uint16_t for n follow)
            {
                length = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x7a: // UTF-8 string (four-byte uint32_t for n follow)
            {
                length = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x7b: // UTF-8 string (eight-byte uint64_t for n follow)
            {
                length = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        case 0x7f: // UTF-8 string, text strings follow, terminated by "break"
            {
                length = 0;
                while (*p != 0xff)
                {
                    size_t len = detail::get_text_string_length(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    length += len;
                }
                break;
            }
        default: 
            *endp = first;
            break;
        }
    }
    return length;
}

inline 
std::string get_text_string(const uint8_t* first, const uint8_t* last, 
                            const uint8_t** endp)
{
    std::string s;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x60_0x77: // UTF-8 string (0x00..0x17 bytes follow)
            {
                size_t length = *first & 0x1f;
                *endp = p + length;
                s = std::string(p, *endp);
                break;
            }
        case 0x78: // UTF-8 string (one-byte uint8_t for n follows)
            {
                const auto length = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    s = std::string(p, *endp);
                }
                break;
            }
        case 0x79: // UTF-8 string (two-byte uint16_t for n follow)
            {
                const auto length = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    s = std::string(p, *endp);
                }
                break;
            }
        case 0x7a: // UTF-8 string (four-byte uint32_t for n follow)
            {
                const auto length = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    s = std::string(p, *endp);
                }
                break;
            }
        case 0x7b: // UTF-8 string (eight-byte uint64_t for n follow)
            {
                const auto length = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    p = *endp;
                    *endp = p + length;
                    s = std::string(p, *endp);
                }
                break;
            }
        case 0x7f: // UTF-8 string, text strings follow, terminated by "break"
            {
                while (*p != 0xff)
                {
                    std::string ss = detail::get_text_string(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    else
                    {
                        p = *endp;
                        s.append(std::move(ss));
                    }
                }
                break;
            }
        default: 
            *endp = first;
            break;
        }
    }
    return s;
}

inline
void walk_object(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    size_t size = 0;

    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0xa0_0xb7: // map (0x00..0x17 pairs of data items follow)
            {
                size = *first & 0x1f;
                *endp = p;
                for (size_t i = 0; i < size; ++i)
                {
                    walk(*endp, last, endp);
                    walk(*endp, last, endp);
                }
                break;
            }

        case 0xb8: // map (one-byte uint8_t for n follows)
            {
                size = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

        case 0xb9: // map (two-byte uint16_t for n follow)
            {
                size = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

        case 0xba: // map (four-byte uint32_t for n follow)
            {
                size = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

        case 0xbb: // map (eight-byte uint64_t for n follow)
            {
                size = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                        walk(*endp, last, endp);
                    }
                }
                break;
            }
        case 0xbf:
            {
                while (*p != 0xff)
                {
                    walk(p, last, endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    else
                    {
                        p = *endp;
                    }
                    walk(p, last, endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    else
                    {
                        p = *endp;
                    }
                }
                *endp = p;
                break;
            }
        default:
            {
                *endp = first; 
            }
        }
    }
}

inline
void walk_array(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    size_t size = 0;

    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {
            case JSONCONS_CBOR_0x80_0x97: // array (0x00..0x17 data items follow)
            {
                size = *first & 0x1f;
                *endp = p;
                for (size_t i = 0; i < size; ++i)
                {
                    walk(*endp, last, endp);
                }
                break;
            }

            case 0x98: // array (one-byte uint8_t for n follows)
            {
                size = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

            case 0x99: // array (two-byte uint16_t for n follow)
            {
                size = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

            case 0x9a: // array (four-byte uint32_t for n follow)
            {
                size = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

            case 0x9b: // array (eight-byte uint64_t for n follow)
            {
                size = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        walk(*endp, last, endp);
                    }
                }
                break;
            }

            case 0x9f: // array (indefinite length)
            {
                while (*p != 0xff)
                {
                    walk(p, last, endp);
                    if (*endp == p)
                    {
                        *endp = first;
                        break;
                    }
                    else
                    {
                        p = *endp;
                    }
                }
                *endp = p;
                break;
            }
            default:
            {
                *endp = first; 
            }
        }
    }
}

inline
uint64_t get_uinteger(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    uint64_t val = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const size_t length = last - first;
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            val = *first;
            *endp = p;
            break;

        case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                val = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }

        case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                val = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }

        case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                val = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }

        case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                val = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                break;
            }
        default:
            {
                *endp = first; 
            }
        }
    }
    return val;
}

inline
int64_t get_integer(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    int64_t val = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const size_t length = last - first;
        const uint8_t* p = first+1;
        switch (*first)
        {
        case JSONCONS_CBOR_0x20_0x37: // Negative integer -1-0x00..-1-0x17 (-1..-24)
            val = static_cast<int8_t>(0x20 - 1 - *first);
            *endp = p;
            break;

        case 0x38: // Negative integer (one-byte uint8_t follows)
            {
                auto x = binary::from_big_endian<uint8_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    val = static_cast<int64_t>(-1)- x;
                }
                break;
            }

        case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
            {
                auto x = binary::from_big_endian<uint16_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    val = static_cast<int64_t>(-1)- x;
                }
                break;
            }

        case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
            {
                auto x = binary::from_big_endian<uint32_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {                       
                    val = static_cast<int64_t>(-1)- x;
                }
                break;
            }

        case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
            {
                auto x = binary::from_big_endian<uint64_t>(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                }
                else
                {
                    val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                }
                break;
            }
        case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            // FALLTHRU
        case 0x18: // Unsigned integer (one-byte uint8_t follows)
            // FALLTHRU
        case 0x19: // Unsigned integer (two-byte uint16_t follows)
            // FALLTHRU
        case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            // FALLTHRU
        case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                uint64_t x = detail::get_uinteger(first,last,endp);
                if (*endp != first)
                {
                    if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                    {
                        val = x;
                    }
                    else
                    {
                        *endp = first;
                    }
                }
                break;
            }
        default:
            {
                *endp = first; 
            }
        }
    }
    return val;
}

inline
double get_double(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    double val = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
    }
    else
    {
        const size_t length = last - first;
        const uint8_t* p = first+1;
        switch (*first)
        {
        case 0xf9: // Half-Precision Float (two-byte IEEE 754)
            {
                if (JSONCONS_UNLIKELY(1+sizeof(uint16_t) > length))
                {
                    *endp = first; 
                }
                else
                {
                    uint16_t x = binary::from_big_endian<uint16_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        val = binary::decode_half(x);
                    }
                }
            }
            break;


        case 0xfa: // Single-Precision Float (four-byte IEEE 754)
            {
                if (JSONCONS_UNLIKELY(1+sizeof(float) > length))
                {
                    *endp = first; 
                }
                else
                {
                    val = binary::from_big_endian<float>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                }
            }
            break;

        case 0xfb: //  Double-Precision Float (eight-byte IEEE 754)
            {
                if (JSONCONS_UNLIKELY(1+sizeof(double) > length))
                {
                    *endp = first; 
                }
                else
                {
                    val = binary::from_big_endian<double>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                }
            }
            break;
        default:
            {
                *endp = first; 
            }
        }
    }
    return val;
}

inline 
bool is_array(uint8_t b) 
{
    return (b >= 0x80 && b <= 0x9b) || b == 0x9f; 
}

inline 
bool is_object(uint8_t b) 
{
    return (b >= 0xa0 && b <= 0xbb) || b == 0xb8;
}

inline
bool is_string(uint8_t b)
{
    return (b >= 0x60 && b <= 0x7b) || b == 0x7f;
}

inline
bool is_bool(uint8_t b)
{
    return b == 0xf5 || b == 0xf4;
}

inline
bool is_double(uint8_t b)
{
    return b == 0xf9 || b == 0xfa || b == 0xfb;
}

inline
bool is_integer(const uint8_t* first, const uint8_t* last)
{
    bool result;

    switch (*first)
    {
    case JSONCONS_CBOR_0x20_0x37: // Negative integer -1-0x00..-1-0x17 (-1..-24)
    case 0x38: // Negative integer (one-byte uint8_t follows)
    case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
    case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
    case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
    case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
    case 0x18: // Unsigned integer (one-byte uint8_t follows)
    case 0x19: // Unsigned integer (two-byte uint16_t follows)
    case 0x1a: // Unsigned integer (four-byte uint32_t follows)
        result = true;
        break;
    case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
        {
        const uint8_t* endp;
            uint64_t x = detail::get_uinteger(first,last,&endp);
            if (endp != first)
            {
                if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                {
                    result = true;
                }
                else
                {
                    result = false;
                }
            }
            else
            {
                result = false;
            }
            break;
        }
    default:
        result = false; 
        break;
    }
    return result;
}

inline
bool is_uinteger(uint8_t b)
{
    bool result;

    switch (b)
    {
    case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
        // FALLTHRU
    case 0x18: // Unsigned integer (one-byte uint8_t follows)
        // FALLTHRU
    case 0x19: // Unsigned integer (two-byte uint16_t follows)
        // FALLTHRU
    case 0x1a: // Unsigned integer (four-byte uint32_t follows)
        // FALLTHRU
    case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
        result = true;
        break;
    default:
        result = false; 
        break;
    }
    return result;
}

inline 
void walk(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first >= last)
    {
        *endp = first;
    }
    else
    {
        const uint8_t* p = first+1;
        switch (*first)
        {               
            case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            {
                *endp = p;
                break;
            }
            case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                p += sizeof(uint8_t);
                *endp = p;
                break;
            }
            case 0x19: // Unsigned integer (two-byte uint16_t follows)
                {
                    p += sizeof(uint16_t);
                    *endp = p;
                    break;
                }
            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
                {
                    p += sizeof(uint32_t);
                    *endp = p;
                    break;
                }
            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
                {
                    p += sizeof(uint64_t);
                    *endp = p;
                    break;
                }
            case JSONCONS_CBOR_0x20_0x37: // Negative integer -1-0x00..-1-0x17 (-1..-24)
                {
                    *endp = p;
                    break;
                }
            case 0x38: // Negative integer (one-byte uint8_t follows)
                {
                    p += sizeof(uint8_t);
                    *endp = p;
                    break;
                }
            case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
                {
                    p += sizeof(uint16_t);
                    *endp = p;
                    break;
                }
            case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
                {
                    p += sizeof(uint32_t);
                    *endp = p;
                    break;
                }
            case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
                {
                    p += sizeof(uint64_t);
                    *endp = p;
                    break;
                }
            case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
                {
                    size_t len = *first & 0x1f;
                    *endp = p + len;
                    break;
                }
            case 0x58: // byte string (one-byte uint8_t for n follows)
                {
                    const auto len = binary::from_big_endian<uint8_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
            case 0x59: // byte string (two-byte uint16_t for n follow)
                {
                    const auto len = binary::from_big_endian<uint16_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
            case 0x5a: // byte string (four-byte uint32_t for n follow)
                {
                    const auto len = binary::from_big_endian<uint32_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
            case 0x5b: // byte string (eight-byte uint64_t for n follow)
                {
                    const auto len = binary::from_big_endian<uint64_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
            case 0x5f: // byte string (indefinite length)
                {
                    while (*p != 0xff)
                    {
                        if (p == last)
                        {
                            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                        }
                        walk(p, last, &p);
                    }
                    *endp = p;
                    break;
                }
            // UTF-8 string (0x00..0x17 bytes follow)
            case JSONCONS_CBOR_0x60_0x77:
                {
                    size_t len = *first & 0x1f;
                    *endp = p + len;
                    break;
                }
                // UTF-8 string (one-byte uint8_t for n follows)
            case 0x78:
                {
                    const auto len = binary::from_big_endian<uint8_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
                // UTF-8 string (two-byte uint16_t for n follow)
            case 0x79:
                {
                    const auto len = binary::from_big_endian<uint16_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
                // UTF-8 string (four-byte uint32_t for n follow)
            case 0x7a:
                {
                    const auto len = binary::from_big_endian<uint32_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
                // UTF-8 string (eight-byte uint64_t for n follow)
            case 0x7b:
                {
                    const auto len = binary::from_big_endian<uint64_t>(p,last,endp);
                    if (*endp == p)
                    {
                        *endp = first;
                    }
                    else
                    {
                        p = *endp;
                    }
                    *endp = p + len;
                    break;
                }
                // UTF-8 string (indefinite length)
            case 0x7f:
                {
                    while (*p != 0xff)
                    {
                        if (p == last)
                        {
                            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                        }
                        walk(p, last, &p);
                    }
                    *endp = p;
                    break;
                }
            case JSONCONS_CBOR_0x80_0x97: // array (0x00..0x17 data items follow)
                // FALLTHRU
            case 0x98: // array (one-byte uint8_t for n follows)
                // FALLTHRU
            case 0x99: // array (two-byte uint16_t for n follow)
                // FALLTHRU
            case 0x9a: // array (four-byte uint32_t for n follow)
                // FALLTHRU
            case 0x9b: // array (eight-byte uint64_t for n follow)
                // FALLTHRU
            case 0x9f: // array (indefinite length)
                {
                    walk_array(first,last,endp);
                    break;
                }
            case JSONCONS_CBOR_0xa0_0xb7: // map (0x00..0x17 pairs of data items follow)
                // FALLTHRU
            case 0xb8: // map (one-byte uint8_t for n follows)
                // FALLTHRU
            case 0xb9: // map (two-byte uint16_t for n follow)
                // FALLTHRU
            case 0xba: // map (four-byte uint32_t for n follow)
                // FALLTHRU
            case 0xbb: // map (eight-byte uint64_t for n follow)
                // FALLTHRU
            case 0xbf:
                {
                    walk_object(first,last,endp);
                    break;
                }
            case 0xC2:
            case 0xC3:
                walk(p, last, endp);
                break;

            // False
        case 0xf4:
            {
                *endp = p;
                break;
            }

            // True
        case 0xf5:
            {
                *endp = p;
                break;
            }

            // Null
        case 0xf6:
            {
                *endp = p;
                break;
            }

            // Half-Precision Float (two-byte IEEE 754)
        case 0xf9:
            {
                p += sizeof(uint16_t);
                *endp = p;
                break;
            }

            // Single-Precision Float (four-byte IEEE 754)
        case 0xfa:
            {
                p += sizeof(float);
                *endp = p;
                break;
            }

            //  Double-Precision Float (eight-byte IEEE 754)
        case 0xfb:
            {
                p += sizeof(double);
                *endp = p;
                break;
            }

        default:
            {
                *endp = first;
                break;
            }
        }
    }
}

inline 
size_t get_size(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    const uint8_t* p = first + 1;
    switch (*first)
    {
    // array (0x00..0x17 data items follow)
    case JSONCONS_CBOR_0x80_0x97:
    {
        *endp = p;
        return *first & 0x1f;
    }

    // array (one-byte uint8_t for n follows)
    case 0x98: 
    {
        const auto len = binary::from_big_endian<uint8_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // array (two-byte uint16_t for n follow)
    case 0x99: 
    {
        const auto len = binary::from_big_endian<uint16_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;

        return len;
    }

    // array (four-byte uint32_t for n follow)
    case 0x9a: 
    {
        const auto len = binary::from_big_endian<int32_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // array (eight-byte uint64_t for n follow)
    case 0x9b: 
    {
        const auto len = binary::from_big_endian<int64_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // array (indefinite length)
    case 0x9f: 
    {
        size_t len = 0;
        while (*p != 0xff)
        {
            walk(p, last, &p);
            ++len;
        }
        *endp = first + 1;
        return len;
    }

    // map (0x00..0x17 pairs of data items follow)
    case JSONCONS_CBOR_0xa0_0xb7:
    {
        *endp = p;
        return *first & 0x1f;
    }

    // map (one-byte uint8_t for n follows)
    case 0xb8: 
    {
        const auto len = binary::from_big_endian<uint8_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // map (two-byte uint16_t for n follow)
    case 0xb9: 
    {
        const auto len = binary::from_big_endian<uint16_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // map (four-byte uint32_t for n follow)
    case 0xba: 
    {
        const auto len = binary::from_big_endian<uint32_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // map (eight-byte uint64_t for n follow)
    case 0xbb: 
    {
        const auto len = binary::from_big_endian<uint64_t>(p,last,endp);
        if (*endp == p)
        {
            JSONCONS_THROW(cbor_decode_error(last-p));
        }
        else
        {
            p = *endp;
        }
        *endp = p;
        return len;
    }

    // map (indefinite length)
    case 0xbf: 
    {
        size_t len = 0;
        while (*p != 0xff)
        {
            walk(p, last, &p);
            walk(p, last, &p);
            ++len;
        }
        *endp = first + 1;
        return len;
    }
    default:
        *endp = last;
        return 0;
    }
}

template <class T>
class const_array_iterator
{
    const uint8_t* p_;
    const uint8_t* last_;
    const uint8_t* base_relative_;
    T current_;
public:
    typedef typename T::difference_type difference_type;
    typedef typename T::value_type value_type;
    typedef typename T::const_reference reference;
    typedef typename T::const_pointer pointer;
    typedef std::forward_iterator_tag iterator_catagory;

    const_array_iterator()
        : p_(nullptr), last_(nullptr), base_relative_(nullptr)
    {
    }

    const_array_iterator(const uint8_t* p, const uint8_t* last, const uint8_t* base_relative)
        : p_(p), last_(last), base_relative_(base_relative)
    {
    }

    const_array_iterator(const const_array_iterator& other) = default;

    friend bool operator==(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    friend bool operator!=(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ != rhs.p_; 
    }

    friend bool operator<(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    const_array_iterator& operator++()
    {
        detail::walk(p_, last_, &p_);
        return *this;
    }

    reference operator*() const
    {
        const uint8_t* endp;
        detail::walk(p_, last_, &endp);
        const_cast<T*>(&current_)->first_ = p_;
        const_cast<T*>(&current_)->last_ = endp;
        const_cast<T*>(&current_)->base_relative_ = base_relative_;
        return current_;
    }

    pointer operator->() const
    {
        const uint8_t* endp;
        detail::walk(p_, last_, &endp);
        const_cast<T*>(&current_)->first_ = p_;
        const_cast<T*>(&current_)->last_ = endp;
        const_cast<T*>(&current_)->base_relative_ = base_relative_;
        return &current_;
    }
};

template <class T>
class const_object_iterator;

template <class T>
class key_value_pair_view
{
    const uint8_t* key_begin_;
    const uint8_t* key_end_;
    const uint8_t* val_begin_;
    const uint8_t* val_end_;
    const uint8_t* base_relative_;

public:
    friend class const_object_iterator<T>;

    key_value_pair_view()
        : key_begin_(nullptr), key_end_(nullptr), val_begin_(nullptr), val_end_(nullptr)
    {
    }
    key_value_pair_view(const uint8_t* key_begin, const uint8_t* key_end, 
                        const uint8_t* val_begin, const uint8_t* val_end, 
                        const uint8_t* base_relative)
        : key_begin_(key_begin), key_end_(key_end), val_begin_(val_begin), val_end_(val_end), 
          base_relative_(base_relative)
    {
    }
    key_value_pair_view(const key_value_pair_view& other) = default;

    std::string key() const
    {
        const uint8_t* endp;
        return get_text_string(key_begin_, key_end_, &endp);
    }

    T value() const
    {
        return T(val_begin_, val_end_ - val_begin_, base_relative_);
    }
};

template <class T>
class const_object_iterator
{
    const uint8_t* p_;
    const uint8_t* last_;
    const uint8_t* base_relative_;
    key_value_pair_view<T> kvpair_;
public:
    typedef typename T::difference_type difference_type;
    typedef key_value_pair_view<T> value_type;
    typedef const key_value_pair_view<T>& reference;
    typedef const key_value_pair_view<T>* pointer;
    typedef std::forward_iterator_tag iterator_catagory;

    const_object_iterator()
        : p_(nullptr), last_(nullptr), base_relative_(nullptr)
    {
    }

    const_object_iterator(const uint8_t* p, const uint8_t* last, const uint8_t* base_relative)
        : p_(p), last_(last), base_relative_(base_relative)
    {
    }

    const_object_iterator(const const_object_iterator& other) = default;

    friend bool operator==(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    friend bool operator!=(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ != rhs.p_; 
    }

    friend bool operator<(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    const_object_iterator& operator++()
    {
        detail::walk(p_, last_, &p_);
        detail::walk(p_, last_, &p_);
        return *this;
    }

    reference operator*() const
    {
        const uint8_t* endp;

        const_cast<key_value_pair_view<T>*>(&kvpair_)->key_begin_ = p_;
        detail::walk(kvpair_.key_begin_, last_, &endp);
        const_cast<key_value_pair_view<T>*>(&kvpair_)->key_end_ = endp;
        const_cast<key_value_pair_view<T>*>(&kvpair_)->val_begin_ = kvpair_.key_end_;
        detail::walk(kvpair_.val_begin_, last_, &endp);
        const_cast<key_value_pair_view<T>*>(&kvpair_)->val_end_ = endp;
        const_cast<key_value_pair_view<T>*>(&kvpair_)->base_relative_ = base_relative_;

        return kvpair_;
    }

    pointer operator->() const
    {
        const uint8_t* endp;

        const_cast<key_value_pair_view<T>*>(&kvpair_)->key_begin_ = p_;
        detail::walk(kvpair_.key_begin_, last_, &endp);
        const_cast<key_value_pair_view<T>*>(&kvpair_)->key_end_ = endp;
        const_cast<key_value_pair_view<T>*>(&kvpair_)->val_begin_ = kvpair_.key_end_;
        detail::walk(kvpair_.val_begin_, last_, &endp);
        const_cast<key_value_pair_view<T>*>(&kvpair_)->val_end_ = endp;
        const_cast<key_value_pair_view<T>*>(&kvpair_)->base_relative_ = base_relative_;

        return &kvpair_;
    }
};

} // namespace detail

class cbor_parser : public serializing_context
{
    const uint8_t* begin_input_;
    const uint8_t* end_input_;
    const uint8_t* input_ptr_;
    json_content_handler& handler_;
    size_t column_;
    size_t nesting_depth_;
public:
    cbor_parser(json_content_handler& handler)
       : handler_(handler), 
         column_(1),
         nesting_depth_(0)
    {
    }

    void update(const uint8_t* input, size_t length)
    {
        begin_input_ = input;
        end_input_ = input + length;
        input_ptr_ = begin_input_;
    }

    void reset()
    {
        column_ = 1;
        nesting_depth_ = 0;
    }

    void parse_some(std::error_code& ec)
    {
        if (nesting_depth_ == 0)
        {
            handler_.begin_document();
        }

        const uint8_t* pos = input_ptr_++;
        switch (*pos)
        {

        case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            // FALLTHRU
        case 0x18: // Unsigned integer (one-byte uint8_t follows)
            // FALLTHRU
        case 0x19: // Unsigned integer (two-byte uint16_t follows)
            // FALLTHRU
        case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            // FALLTHRU
        case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                const uint8_t* endp;
                uint64_t val = detail::get_uinteger(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                handler_.uinteger_value(val, *this);
            }
            break;
        case JSONCONS_CBOR_0x20_0x37: // Negative integer -1-0x00..-1-0x17 (-1..-24)
            // FALLTHRU
        case 0x38: // Negative integer (one-byte uint8_t follows)
            // FALLTHRU
        case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
            // FALLTHRU
        case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
            // FALLTHRU
        case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
            {
                const uint8_t* endp;
                int64_t val = detail::get_integer(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                handler_.integer_value(val, *this);
            }
            break;

        case 0xC2:
            {
                const uint8_t* endp;
                std::vector<uint8_t> v = detail::get_byte_string(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;

                handler_.bignum_value(1, v.data(), v.size(), *this);
            }
            break;

        case 0xC3:
            {
                const uint8_t* endp;
                std::vector<uint8_t> v = detail::get_byte_string(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;

                handler_.bignum_value(-1, v.data(), v.size(), *this);
            }
            break;

            // False
        case 0xf4:
            handler_.bool_value(false, *this);
            break;

            // True
        case 0xf5:
            handler_.bool_value(true, *this);
            break;

            // Null
        case 0xf6:
            handler_.null_value(*this);
            break;

        case 0xf9: // Half-Precision Float (two-byte IEEE 754)
            // FALLTHRU
        case 0xfa: // Single-Precision Float (four-byte IEEE 754)
            // FALLTHRU
        case 0xfb: // Double-Precision Float (eight-byte IEEE 754)
            {
                const uint8_t* endp;
                double val = detail::get_double(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                handler_.double_value(val, *this);
            }
            break;
            // byte string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x40_0x57:
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5f:
            {
                const uint8_t* endp;
                std::vector<uint8_t> v = detail::get_byte_string(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;

                handler_.byte_string_value(v.data(), v.size(), *this);
            }
            break;

            // UTF-8 string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x60_0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7f:
            {
                const uint8_t* endp;
                std::string s = detail::get_text_string(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                handler_.string_value(basic_string_view<char>(s.data(),s.length()), *this);
            }
            break;

            // array (0x00..0x17 data items follow)
        case JSONCONS_CBOR_0x80_0x97:
            {
                size_t len = (*pos & 0x1f);
                parse_array(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // array (one-byte uint8_t for n follows)
        case 0x98:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint8_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_array(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // array (two-byte uint16_t for n follow)
        case 0x99:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_array(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // array (four-byte uint32_t for n follow)
        case 0x9a:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_array(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // array (eight-byte uint64_t for n follow)
        case 0x9b:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint64_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_array(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // array (indefinite length)
        case 0x9f:
            {
                ++nesting_depth_;
                handler_.begin_array(*this);
                while (*input_ptr_ != 0xff)
                {
                    parse_some(ec);
                    if (ec)
                    {
                        return;
                    }
                    pos = input_ptr_;
                }
                handler_.end_array(*this);
                --nesting_depth_;
            }
            break;

            // map (0x00..0x17 pairs of data items follow)
        case JSONCONS_CBOR_0xa0_0xb7:
            {
                size_t len = (*pos & 0x1f);
                parse_object(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // map (one-byte uint8_t for n follows)
        case 0xb8:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint8_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_object(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // map (two-byte uint16_t for n follow)
        case 0xb9:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_object(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // map (four-byte uint32_t for n follow)
        case 0xba:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_object(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // map (eight-byte uint64_t for n follow)
        case 0xbb:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint64_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                parse_object(len, ec);
                if (ec)
                {
                    return;
                }
            }
            break;

            // map (indefinite length)
        case 0xbf:
            {
                ++nesting_depth_;
                handler_.begin_object(*this);
                while (*input_ptr_ != 0xff)
                {
                    parse_name(ec);
                    if (ec)
                    {
                        return;
                    }
                    parse_some(ec);
                    if (ec)
                    {
                        return;
                    }
                    pos = input_ptr_;
                }
                handler_.end_object(*this);
                --nesting_depth_;
            }
            break;

        default:
            {
                ec = cbor_parse_errc::source_error;
                return;
            }
        }
        if (nesting_depth_ == 0)
        {
            handler_.end_document();
        }
    }
private:
    void parse_name(std::error_code& ec)
    {
        const uint8_t* pos = input_ptr_++;
        switch (*pos)
        {
            // UTF-8 string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x60_0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7f:
            {
                const uint8_t* endp;
                std::string s = detail::get_text_string(pos,end_input_,&endp);
                if (endp == pos)
                {
                    ec = cbor_parse_errc::unexpected_eof;
                    return;
                }
                input_ptr_ = endp;
                handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
            }
            break;
        default:
            ec = cbor_parse_errc::source_error;
            return;
        }
    }

    void parse_array(size_t len, std::error_code& ec)
    {
        ++nesting_depth_;
        handler_.begin_array(len, *this);
        for (size_t i = 0; i < len; ++i)
        {
            parse_some(ec);
            if (ec)
            {
                return;
            }
        }
        handler_.end_array(*this);
        --nesting_depth_;
    }

    void parse_object(size_t len, std::error_code& ec)
    {
        ++nesting_depth_;
        handler_.begin_object(len, *this);
        for (size_t i = 0; i < len; ++i)
        {
            parse_name(ec);
            if (ec)
            {
                return;
            }
            parse_some(ec);
            if (ec)
            {
                return;
            }
        }
        handler_.end_object(*this);
        --nesting_depth_;
    }

    size_t do_line_number() const override
    {
        return 1;
    }

    size_t do_column_number() const override
    {
        return column_;
    }
};


}}

#endif
