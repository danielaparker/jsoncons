// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_HPP
#define JSONCONS_CBOR_CBOR_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/binary/binary_utilities.hpp>

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
    const uint8_t* walk(const uint8_t* it, const uint8_t* end);

    inline 
    std::tuple<std::string,const uint8_t*> get_fixed_length_text_string(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {       
        case JSONCONS_CBOR_0x60_0x77: // UTF-8 string (0x00..0x17 bytes follow)
            {
                size_t len = *pos & 0x1f;
                const uint8_t *first = it;
                it += len;
                return std::make_tuple(std::string(first, it), it);
            }
        case 0x78: // UTF-8 string (one-byte uint8_t for n follows)
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                const uint8_t *first = it;
                it += len;
                return std::make_tuple(std::string(first, it), it);
            }
        case 0x79: // UTF-8 string (two-byte uint16_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::string(first,it),it);
            }
        case 0x7a: // UTF-8 string (four-byte uint32_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
                it += sizeof(uint32_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::string(first,it),it);
            }
        case 0x7b: // UTF-8 string (eight-byte uint64_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
                it += sizeof(uint64_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::string(first,it),it);
            }
        default: 
            {
                JSONCONS_THROW(cbor_decode_error(end-pos));
            }
        }
    }

    inline
    std::tuple<std::string,const uint8_t*> get_text_string(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {
            
        case 0x7f: // UTF-8 string (0x00..0x17 bytes follow)
            {
                std::string s;
                while (*it != 0xff)
                {
                    if (it == end)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    std::string ss;
                    std::tie(ss,it) = detail::get_fixed_length_text_string(it,end);
                    s.append(std::move(ss));
                }
                return std::make_tuple(s,it);
            }
        default:
            return detail::get_fixed_length_text_string(pos,end);
        }
    }

    inline 
    std::tuple<std::vector<uint8_t>,const uint8_t*> get_fixed_length_byte_string(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {
        
        case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
            {
                size_t len = *pos & 0x1f;
                const uint8_t *first = it;
                it += len;
                return std::make_tuple(std::vector<uint8_t>(first, it), it);
            }
        case 0x58: // byte string (one-byte uint8_t for n follows)
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                const uint8_t *first = it;
                it += len;
                return std::make_tuple(std::vector<uint8_t>(first, it), it);
            }
        case 0x59: // byte string (two-byte uint16_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::vector<uint8_t>(first,it),it);
            }
        case 0x5a: // byte string (four-byte uint32_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
                it += sizeof(uint32_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::vector<uint8_t>(first,it),it);
            }
        case 0x5b: // byte string (eight-byte uint64_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
                it += sizeof(uint64_t);
                const uint8_t* first = it;
                it += len;
                return std::make_tuple(std::vector<uint8_t>(first,it),it);
            }
        default: 
            {
                JSONCONS_THROW(cbor_decode_error(end-pos));
            }
        }
    }

    inline
    std::tuple<std::vector<uint8_t>,const uint8_t*> get_byte_string(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {
        
        case 0x5f: // byte string, byte strings follow, terminated by "break"
            {
                std::vector<uint8_t> v;
                while (*it != 0xff)
                {
                    if (it == end)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    std::vector<uint8_t> ss;
                    std::tie(ss,it) = detail::get_fixed_length_byte_string(it,end);
                    v.insert(v.end(),ss.begin(),ss.end());
                }
                return std::make_tuple(v,it);
            }
        default:
            return detail::get_fixed_length_byte_string(pos,end);
        }
    }

    inline uint64_t get_uinteger(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {            
            case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
                return *pos;

            
            case 0x18: // Unsigned integer (one-byte uint8_t follows) 
                {
                    auto x = binary::detail::from_big_endian<uint8_t>(it,end);
                    it += sizeof(uint8_t); 
                    return x;
                }

            
            case 0x19: // Unsigned integer (two-byte uint16_t follows)
                {
                    auto x = binary::detail::from_big_endian<uint16_t>(it,end);
                    it += sizeof(uint16_t); 
                    return x;
                }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
                {
                    auto x = binary::detail::from_big_endian<uint32_t>(it,end);
                    it += sizeof(uint32_t); 
                    return x;
                }

            
            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
                {
                    auto x = binary::detail::from_big_endian<uint64_t>(it,end);
                    it += sizeof(uint64_t); 
                    return x;
                }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an unsigned integer"));
        }
    }

    inline const uint8_t* walk_string(const uint8_t* it, size_t len)
    {
        return it + len;
    }

    inline const uint8_t* walk_array(const uint8_t* it, const uint8_t* end, size_t len)
    {
        for (size_t i = 0; i < len; ++i)
        {
            it = walk(it, end);
        }
        return it;
    }

    inline 
    const uint8_t* walk_object(const uint8_t* it, const uint8_t* end, size_t len)
    {
        for (size_t i = 0; i < len; ++i)
        {
            it = walk(it, end);
            it = walk(it, end);
        }
        return it;
    }

    inline
    bool is_string(uint8_t b)
    {
        return (b >= 0x60 && b <= 0x7b) || b == 0x7f;
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

    inline const uint8_t* walk(const uint8_t* it, const uint8_t* end)
    {
        if (it >= end)
        {
            return end;
        }

        const uint8_t *pos = it++;
        switch (*pos)
        {
            
        case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            return it;

            
        case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                it += sizeof(uint8_t);
                return it;
            }

            
        case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                it += sizeof(uint16_t);
                return it;
            }

            
        case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                it += sizeof(uint32_t);
                return it;
            }

            
        case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                it += sizeof(uint64_t);
                return it;
            }

            
        case JSONCONS_CBOR_0x20_0x37: // Negative integer -1-0x00..-1-0x17 (-1..-24)
            return it;

            
        case 0x38: // Negative integer (one-byte uint8_t follows)
            {
                it += sizeof(uint8_t);
                return it;
            }

            
        case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
            {
                it += sizeof(uint16_t);
                return it;
            }

            
        case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
            {
                it += sizeof(uint32_t);
                return it;
            }

            
        case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
            {
                it += sizeof(uint64_t);
                return it;
            }

            
        case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
            {
                size_t len = *pos & 0x1f;
                return it + len;
            }
            
        case 0x58: // byte string (one-byte uint8_t for n follows)
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                return it + len;
            }
            
        case 0x59: // byte string (two-byte uint16_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                return it + len;
            }
            
        case 0x5a: // byte string (four-byte uint32_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
                it += sizeof(uint32_t);
                return it + len;
            }
            
        case 0x5b: // byte string (eight-byte uint64_t for n follow)
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
                it += sizeof(uint64_t);
                return it + len;
            }
            
        case 0x5f: // byte string (indefinite length)
            {
                while (*it != 0xff)
                {
                    if (it == end)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    it = walk(it, end);
                }
                return it;
            }

        // UTF-8 string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x60_0x77:
            {
                size_t len = *pos & 0x1f;
                return it + len;
            }
            // UTF-8 string (one-byte uint8_t for n follows)
        case 0x78:
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                return it + len;
            }
            // UTF-8 string (two-byte uint16_t for n follow)
        case 0x79:
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                return it + len;
            }
            // UTF-8 string (four-byte uint32_t for n follow)
        case 0x7a:
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
                it += sizeof(uint32_t);
                return it + len;
            }
            // UTF-8 string (eight-byte uint64_t for n follow)
        case 0x7b:
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
                it += sizeof(uint64_t);
                return it + len;
            }
            // UTF-8 string (indefinite length)
        case 0x7f:
            {
                while (*it != 0xff)
                {
                    if (it == end)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    it = walk(it, end);
                }
                return it;
            }

            // array (0x00..0x17 data items follow)
        case JSONCONS_CBOR_0x80_0x97:
            {
                return walk_array(it, end, *pos & 0x1f);
            }

            // array (one-byte uint8_t for n follows)
        case 0x98:
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                return walk_array(it, end, len);
            }

            // array (two-byte uint16_t for n follow)
        case 0x99:
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                return walk_array(it, end, len);
            }

            // array (four-byte uint32_t for n follow)
        case 0x9a:
            {
                const auto len = binary::detail::from_big_endian<int32_t>(it,end);
                it += sizeof(uint32_t);
                return walk_array(it, end, len);
            }

            // array (eight-byte uint64_t for n follow)
        case 0x9b:
            {
                const auto len = binary::detail::from_big_endian<int64_t>(it,end);
                it += sizeof(uint64_t);
                return walk_array(it, end, len);
            }

            // array (indefinite length)
        case 0x9f:
            {
                while (*it != 0xff)
                {
                    it = walk(it, end);
                }
                return it;
            }

            // map (0x00..0x17 pairs of data items follow)
        case JSONCONS_CBOR_0xa0_0xb7:
            {
                return walk_object(it, end, *pos & 0x1f);
            }

            // map (one-byte uint8_t for n follows)
        case 0xb8:
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
                it += sizeof(uint8_t);
                return walk_object(it, end, len);
            }

            // map (two-byte uint16_t for n follow)
        case 0xb9:
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
                it += sizeof(uint16_t);
                return walk_object(it, end, len);
            }

            // map (four-byte uint32_t for n follow)
        case 0xba:
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
                it += sizeof(uint32_t);
                return walk_object(it, end, len);
            }

            // map (eight-byte uint64_t for n follow)
        case 0xbb:
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
                it += sizeof(uint64_t);
                return walk_object(it, end, len);
            }

            // map (indefinite length)
        case 0xbf:
            {
                while (*it != 0xff)
                {
                    it = walk(it, end);
                    it = walk(it, end);
                }
                return it;
            }

            // False
        case 0xf4:
            {
                return it;
            }

            // True
        case 0xf5:
            {
                return it;
            }

            // Null
        case 0xf6:
            {
                return it;
            }

            // Half-Precision Float (two-byte IEEE 754)
        case 0xf9:
            {
                it += sizeof(uint16_t);
                return it;
            }

            // Single-Precision Float (four-byte IEEE 754)
        case 0xfa:
            {
                it += sizeof(float);
                return it;
            }

            //  Double-Precision Float (eight-byte IEEE 754)
        case 0xfb:
            {
                it += sizeof(double);
                return it;
            }

        default:
            {
                JSONCONS_THROW(cbor_decode_error(end-pos));
            }
        }
    }

    inline 
    std::tuple<size_t,const uint8_t*> size(const uint8_t* it, const uint8_t* end)
    {
        const uint8_t* pos = it++;
        switch (*pos)
        {
        // array (0x00..0x17 data items follow)
        case JSONCONS_CBOR_0x80_0x97:
        {
            return std::make_tuple(*pos & 0x1f,it);
        }

        // array (one-byte uint8_t for n follows)
        case 0x98: 
        {
            const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
            it += sizeof(uint8_t); 
            return std::make_tuple(len,it);
        }

        // array (two-byte uint16_t for n follow)
        case 0x99: 
        {
            const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
            it += sizeof(uint16_t); 
            return std::make_tuple(len,it);
        }

        // array (four-byte uint32_t for n follow)
        case 0x9a: 
        {
            const auto len = binary::detail::from_big_endian<int32_t>(it,end);
            it += sizeof(uint32_t); 
            return std::make_tuple(len,it);
        }

        // array (eight-byte uint64_t for n follow)
        case 0x9b: 
        {
            const auto len = binary::detail::from_big_endian<int64_t>(it,end);
            it += sizeof(uint64_t); 
            return std::make_tuple(len,it);
        }

        // array (indefinite length)
        case 0x9f: 
        {
            size_t len = 0;
            while (*it != 0xff)
            {
                size_t sz;
                std::tie(sz,it) = size(it,end);
                len += sz;
                it = walk(it, end);
            }
            return std::make_tuple(len,it);
        }

        // map (0x00..0x17 pairs of data items follow)
        case JSONCONS_CBOR_0xa0_0xb7:
        {
            return std::make_tuple(*pos & 0x1f,it);
        }

        // map (one-byte uint8_t for n follows)
        case 0xb8: 
        {
            const auto len = binary::detail::from_big_endian<uint8_t>(it,end);
            it += sizeof(uint8_t); 
            return std::make_tuple(len,it);
        }

        // map (two-byte uint16_t for n follow)
        case 0xb9: 
        {
            const auto len = binary::detail::from_big_endian<uint16_t>(it,end);
            it += sizeof(uint16_t); 
            return std::make_tuple(len,it);
        }

        // map (four-byte uint32_t for n follow)
        case 0xba: 
        {
            const auto len = binary::detail::from_big_endian<uint32_t>(it,end);
            it += sizeof(uint32_t); 
            return std::make_tuple(len,it);
        }

        // map (eight-byte uint64_t for n follow)
        case 0xbb: 
        {
            const auto len = binary::detail::from_big_endian<uint64_t>(it,end);
            it += sizeof(uint64_t); 
            return std::make_tuple(len,it);
        }

        // map (indefinite length)
        case 0xbf: 
        {
            size_t len = 0;
            while (*it != 0xff)
            {
                it = walk(it, end);
                it = walk(it, end);
            }
            return std::make_tuple(len,it);
        }
        default:
            return std::make_tuple(0,end);
        }
    }
}

// cbor_view

class cbor_view 
{
    const uint8_t* buffer_;
    size_t buflen_; 
public:
    typedef cbor_view value_type;
    typedef cbor_view& reference;
    typedef const cbor_view& const_reference;
    typedef cbor_view* pointer;
    typedef const cbor_view* const_pointer;
    typedef std::string string_type;
    typedef char char_type;
    typedef std::char_traits<char_type> char_traits_type;
    typedef basic_string_view_ext<char_type> string_view_type;

    cbor_view()
        : buffer_(nullptr), buflen_(0)
    {
    }

    cbor_view(const uint8_t* buffer, size_t buflen)
        : buffer_(buffer), buflen_(buflen)
    {
    }

    cbor_view(const std::vector<uint8_t>& v)
        : buffer_(v.data()), buflen_(v.size())
    {
    }

    cbor_view(const cbor_view& other)
        : buffer_(other.buffer_), buflen_(other.buflen_)
    {
    }

    cbor_view& operator=(const cbor_view&) = default;

    const uint8_t* buffer() const
    {
        return buffer_;
    }

    const size_t buflen() const
    {
        return buflen_;
    }

    bool is_array() const
    {
        JSONCONS_ASSERT(buflen_ > 0);
        return detail::is_array(buffer_[0]);
    }

    bool is_object() const
    {
        JSONCONS_ASSERT(buflen_ > 0);
        return detail::is_object(buffer_[0]);
    }

    size_t size() const
    {
        size_t len;
        const uint8_t* it;
        std::tie(len, it) = detail::size(buffer_,buffer_+buflen_);
        return len;
    }

    cbor_view at(size_t index) const
    {
        JSONCONS_ASSERT(is_array());
        size_t len;
        const uint8_t* it = buffer_;
        const uint8_t* end = buffer_ + buflen_;

        std::tie(len, it) = detail::size(it, end);

        for (size_t i = 0; i < index; ++i)
        {
            it = detail::walk(it, end);
        }

        const uint8_t* last = detail::walk(it,end);

        return cbor_view(it,last-it);
    }

    cbor_view at(const string_view_type& key) const
    {
        JSONCONS_ASSERT(is_object());
        size_t len;
        const uint8_t* it = buffer_;
        const uint8_t* end = buffer_ + buflen_;

        std::tie(len, it) = detail::size(buffer_, end);

        for (size_t i = 0; i < len; ++i)
        {
            string_type a_key;
            std::tie(a_key,it) = detail::get_fixed_length_text_string(it, end);
            if (a_key == key)
            {
                const uint8_t* last = detail::walk(it, end);
                JSONCONS_ASSERT(last >= it);
                return cbor_view(it,last-it);
            }
            const uint8_t* last = detail::walk(it, end);
            it = last;
        }
        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Key not found"));
    }

    bool has_key(const string_view_type& key) const
    {
        if (!is_object())
        {
            return false;
        }
        size_t len;
        const uint8_t* it = buffer_;
        const uint8_t* end = buffer_ + buflen_;

        std::tie(len, it) = detail::size(it, end);

        for (size_t i = 0; i < len; ++i)
        {
            string_type a_key;
            std::tie(a_key,it) = detail::get_fixed_length_text_string(it, end);
            if (a_key == key)
            {
                return true;
            }
            it = detail::walk(it, end);
        }
        return false;
    }
};

struct Encode_cbor_
{
    template <typename T>
    void operator()(T val, std::vector<uint8_t>& v)
    {
        binary::detail::to_big_endian(val,v);
    }
};

struct Calculate_size_
{
    template <typename T>
    void operator()(T, size_t& size)
    {
        size += sizeof(T);
    }
};
  
template<class Json>
struct cbor_Encoder_
{
    typedef typename Json::string_view_type string_view_type;

    static size_t calculate_size(const Json& j)
    {
        size_t n = 0;
        cbor_Encoder_<Json>::encode(j,Calculate_size_(),n);
        return n;
    }

    template <class Action, class Result>
    static void encode(const Json& jval, Action action, Result& v)
    {
        switch (jval.type_id())
        {
        case json_type_tag::null_t:
            {
                action(static_cast<uint8_t>(0xf6), v);
                break;
            }

        case json_type_tag::bool_t:
            {
                action(static_cast<uint8_t>(jval.as_bool() ? 0xf5 : 0xf4), v);
                break;
            }

        case json_type_tag::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val <= 0x17)
                    {
                        action(static_cast<uint8_t>(val), v);
                    } else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x18), v);
                        action(static_cast<uint8_t>(val), v);
                    } else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x19), v);
                        action(static_cast<uint16_t>(val), v);
                    } else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x1a), v);
                        action(static_cast<uint32_t>(val), v);
                    } else if (val <= (std::numeric_limits<int64_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x1b), v);
                        action(static_cast<int64_t>(val), v);
                    }
                } else
                {
                    const auto posnum = -1 - val;
                    if (val >= -24)
                    {
                        action(static_cast<uint8_t>(0x20 + posnum), v);
                    } else if (posnum <= (std::numeric_limits<uint8_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x38), v);
                        action(static_cast<uint8_t>(posnum), v);
                    } else if (posnum <= (std::numeric_limits<uint16_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x39), v);
                        action(static_cast<uint16_t>(posnum), v);
                    } else if (posnum <= (std::numeric_limits<uint32_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x3a), v);
                        action(static_cast<uint32_t>(posnum), v);
                    } else if (posnum <= (std::numeric_limits<int64_t>::max)())
                    {
                        action(static_cast<uint8_t>(0x3b), v);
                        action(static_cast<int64_t>(posnum), v);
                    }
                }
                break;
            }

        case json_type_tag::uinteger_t:
            {
                uint64_t val = jval.as_uinteger();
                if (val <= 0x17)
                {
                    action(static_cast<uint8_t>(val),v);
                } else if (val <=(std::numeric_limits<uint8_t>::max)())
                {
                    action(static_cast<uint8_t>(0x18), v);
                    action(static_cast<uint8_t>(val),v);
                } else if (val <=(std::numeric_limits<uint16_t>::max)())
                {
                    action(static_cast<uint8_t>(0x19), v);
                    action(static_cast<uint16_t>(val),v);
                } else if (val <=(std::numeric_limits<uint32_t>::max)())
                {
                    action(static_cast<uint8_t>(0x1a), v);
                    action(static_cast<uint32_t>(val),v);
                } else if (val <=(std::numeric_limits<uint64_t>::max)())
                {
                    action(static_cast<uint8_t>(0x1b), v);
                    action(static_cast<uint64_t>(val),v);
                }
                break;
            }

        case json_type_tag::double_t:
            {
                action(static_cast<uint8_t>(0xfb), v);
                action(jval.as_double(),v);
                break;
            }

        case json_type_tag::byte_string_t:
            {
                encode_byte_string(jval. template as<std::vector<uint8_t>>(), action, v);
                break;
            }

        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            {
                encode_string(jval.as_string_view(), action, v);
                break;
            }

        case json_type_tag::array_t:
            {
                const auto length = jval.array_value().size();
                if (length <= 0x17)
                {
                    action(static_cast<uint8_t>(static_cast<uint8_t>(0x80 + length)), v);
                } else if (length <= 0xff)
                {
                    action(static_cast<uint8_t>(0x98), v);
                    action(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
                } else if (length <= 0xffff)
                {
                    action(static_cast<uint8_t>(0x99), v);
                    action(static_cast<uint16_t>(length),v);
                } else if (length <= 0xffffffff)
                {
                    action(static_cast<uint8_t>(0x9a), v);
                    action(static_cast<uint32_t>(length),v);
                } else if (length <= 0xffffffffffffffff)
                {
                    action(static_cast<uint8_t>(0x9b), v);
                    action(static_cast<uint64_t>(length),v);
                }

                // append each element
                for (const auto& el : jval.array_range())
                {
                    encode(el,action,v);
                }
                break;
            }

        case json_type_tag::object_t:
            {
                const auto length = jval.object_value().size();
                if (length <= 0x17)
                {
                    action(static_cast<uint8_t>(static_cast<uint8_t>(0xa0 + length)), v);
                } else if (length <= 0xff)
                {
                    action(static_cast<uint8_t>(0xb8), v);
                    action(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
                } else if (length <= 0xffff)
                {
                    action(static_cast<uint8_t>(0xb9), v);
                    action(static_cast<uint16_t>(length),v);
                } else if (length <= 0xffffffff)
                {
                    action(static_cast<uint8_t>(0xba), v);
                    action(static_cast<uint32_t>(length),v);
                } else if (length <= 0xffffffffffffffff)
                {
                    action(static_cast<uint8_t>(0xbb), v);
                    action(static_cast<uint64_t>(length),v);
                }

                // append each element
                for (const auto& kv: jval.object_range())
                {
                    encode_string(kv.key(), action, v);
                    encode(kv.value(), action, v);
                }
                break;
            }

        default:
            {
                break;
            }
        }
    }

    template <class Action,class Result>
    static void encode_string(const string_view_type& sv, Action action, Result& v)
    {
        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }

        const size_t length = target.length();
        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            action(static_cast<uint8_t>(static_cast<uint8_t>(0x60 + length)), v);
        }
        else if (length <= 0xff)
        {
            action(static_cast<uint8_t>(0x78), v);
            action(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
        }
        else if (length <= 0xffff)
        {
            action(static_cast<uint8_t>(0x79), v);
            action(static_cast<uint16_t>(length), v);
        }
        else if (length <= 0xffffffff)
        {
            action(static_cast<uint8_t>(0x7a), v);
            action(static_cast<uint32_t>(length), v);
        }
        else if (length <= 0xffffffffffffffff)
        {
            action(static_cast<uint8_t>(0x7b), v);
            action(static_cast<uint64_t>(length),v);
        }

        for (size_t i = 0; i < length; ++i)
        {
            action(static_cast<uint8_t>(target.data()[i]), v);
        }
    }

    template <class Action,class Result>
    static void encode_byte_string(const std::vector<uint8_t>& target, Action action, Result& v)
    {
        const size_t length = target.size();
        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            action(static_cast<uint8_t>(static_cast<uint8_t>(0x40 + length)), v);
        }
        else if (length <= 0xff)
        {
            action(static_cast<uint8_t>(0x58), v);
            action(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
        }
        else if (length <= 0xffff)
        {
            action(static_cast<uint8_t>(0x59), v);
            action(static_cast<uint16_t>(length), v);
        }
        else if (length <= 0xffffffff)
        {
            action(static_cast<uint8_t>(0x5a), v);
            action(static_cast<uint32_t>(length), v);
        }
        else if (length <= 0xffffffffffffffff)
        {
            action(static_cast<uint8_t>(0x5b), v);
            action(static_cast<uint64_t>(length),v);
        }

        for (size_t i = 0; i < length; ++i)
        {
            action(static_cast<uint8_t>(target.data()[i]), v);
        }
    }
};

// decode_cbor

template<class Json>
class Decode_cbor_
{
    const uint8_t* begin_;
    const uint8_t* end_;
    const uint8_t* it_;
public:
    typedef typename Json::char_type char_type;

    Decode_cbor_(const uint8_t* begin, const uint8_t* end)
        : begin_(begin), end_(end), it_(begin)
    {
    }

    Json decode()
    {
        const uint8_t* pos = it_++;
        switch (*pos)
        {
            // Integer 0x00..0x17 (0..23)
        case JSONCONS_CBOR_0x00_0x17:
            return Json(*pos);

            // Unsigned integer (one-byte uint8_t follows)
        case 0x18:
            {
                auto x = binary::detail::from_big_endian<uint8_t>(it_,end_);
                it_ += sizeof(uint8_t);
                return Json(x);
            }

            // Unsigned integer (two-byte uint16_t follows)
        case 0x19:
            {
                auto x = binary::detail::from_big_endian<uint16_t>(it_,end_);
                it_ += sizeof(uint16_t);
                return Json(x);
            }

            // Unsigned integer (four-byte uint32_t follows)
        case 0x1a:
            {
                auto x = binary::detail::from_big_endian<uint32_t>(it_,end_);
                it_ += sizeof(uint32_t);
                return Json(x);
            }

            // Unsigned integer (eight-byte uint64_t follows)
        case 0x1b:
            {
                auto x = binary::detail::from_big_endian<uint64_t>(it_,end_);
                it_ += sizeof(uint64_t);
                return Json(x);
            }

            // Negative integer -1-0x00..-1-0x17 (-1..-24)
        case JSONCONS_CBOR_0x20_0x37:
            return Json(static_cast<int8_t>(0x20 - 1 - *pos));

            // Negative integer (one-byte uint8_t follows)
        case 0x38:
            {
                auto x = binary::detail::from_big_endian<uint8_t>(it_,end_);
                it_ += sizeof(uint8_t);
                return Json(static_cast<int64_t>(-1)- x);
            }

            // Negative integer -1-n (two-byte uint16_t follows)
        case 0x39:
            {
                auto x = binary::detail::from_big_endian<uint16_t>(it_,end_);
                it_ += sizeof(uint16_t);
                return Json(static_cast<int64_t>(-1)- x);
            }

            // Negative integer -1-n (four-byte uint32_t follows)
        case 0x3a:
            {
                auto x = binary::detail::from_big_endian<uint32_t>(it_,end_);
                it_ += sizeof(uint32_t);
                return Json(static_cast<int64_t>(-1)- x);
            }

            // Negative integer -1-n (eight-byte uint64_t follows)
        case 0x3b:
            {
                auto x = binary::detail::from_big_endian<uint64_t>(it_,end_);
                it_ += sizeof(uint64_t);
                return Json(static_cast<int64_t>(-1)- static_cast<int64_t>(x));
            }

            // byte string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x40_0x57:
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5f:
            {
                std::vector<uint8_t> v;
                std::tie(v,it_) = detail::get_byte_string(pos,end_);
                return Json(v.data(),v.size());
            }

            // UTF-8 string (0x00..0x17 bytes follow)
        case JSONCONS_CBOR_0x60_0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7f:
            {
                std::string s;
                std::tie(s,it_) = detail::get_text_string(pos,end_);
                std::basic_string<char_type> target;
                auto result = unicons::convert(s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                if (result.ec != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                }
                return Json(target);
            }

            // array (0x00..0x17 data items follow)
        case JSONCONS_CBOR_0x80_0x97:
            {
                return get_fixed_length_array(*pos & 0x1f);
            }

            // array (one-byte uint8_t for n follows)
        case 0x98:
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it_,end_);
                it_ += sizeof(uint8_t);
                return get_fixed_length_array(len);
            }

            // array (two-byte uint16_t for n follow)
        case 0x99:
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it_,end_);
                it_ += sizeof(uint16_t);
                return get_fixed_length_array(len);
            }

            // array (four-byte uint32_t for n follow)
        case 0x9a:
            {
                const auto len = binary::detail::from_big_endian<int32_t>(it_,end_);
                it_ += sizeof(uint32_t);
                return get_fixed_length_array(len);
            }

            // array (eight-byte uint64_t for n follow)
        case 0x9b:
            {
                const auto len = binary::detail::from_big_endian<int64_t>(it_,end_);
                it_ += sizeof(uint64_t);
                return get_fixed_length_array(len);
            }

            // array (indefinite length)
        case 0x9f:
            {
                Json result = typename Json::array();
                while (*pos != 0xff)
                {
                    result.push_back(decode());
                    pos = it_;
                }
                return result;
            }

            // map (0x00..0x17 pairs of data items follow)
        case JSONCONS_CBOR_0xa0_0xb7:
            {
                return get_fixed_length_map(*pos & 0x1f);
            }

            // map (one-byte uint8_t for n follows)
        case 0xb8:
            {
                const auto len = binary::detail::from_big_endian<uint8_t>(it_,end_);
                it_ += sizeof(uint8_t);
                return get_fixed_length_map(len);
            }

            // map (two-byte uint16_t for n follow)
        case 0xb9:
            {
                const auto len = binary::detail::from_big_endian<uint16_t>(it_,end_);
                it_ += sizeof(uint16_t);
                return get_fixed_length_map(len);
            }

            // map (four-byte uint32_t for n follow)
        case 0xba:
            {
                const auto len = binary::detail::from_big_endian<uint32_t>(it_,end_);
                it_ += sizeof(uint32_t);
                return get_fixed_length_map(len);
            }

            // map (eight-byte uint64_t for n follow)
        case 0xbb:
            {
                const auto len = binary::detail::from_big_endian<uint64_t>(it_,end_);
                it_ += sizeof(uint64_t);
                return get_fixed_length_map(len);
            }

            // map (indefinite length)
        case 0xbf:
            {
                Json result = typename Json::object();
                while (*pos != 0xff)
                {
                    auto j = decode();
                    result.set(j.as_string_view(),decode());
                    pos = it_;
                }
                return result;
            }

            // False
        case 0xf4:
            {
                return Json(false);
            }

            // True
        case 0xf5:
            {
                return Json(true);
            }

            // Null
        case 0xf6:
            {
                return Json::null();
            }

            // Half-Precision Float (two-byte IEEE 754)
        case 0xf9:
            {
                uint16_t x = binary::detail::from_big_endian<uint16_t>(it_,end_);
                it_ += sizeof(uint16_t);

                double val = binary::detail::decode_half(x);

                return Json(val);
            }

            // Single-Precision Float (four-byte IEEE 754)
        case 0xfa:
            {
                const auto val = binary::detail::from_big_endian<float>(it_,end_);
                it_ += sizeof(float);
                return Json(val);
            }

            //  Double-Precision Float (eight-byte IEEE 754)
        case 0xfb:
            {
                const auto val = binary::detail::from_big_endian<double>(it_,end_);
                it_ += sizeof(double);
                return Json(val);
            }

        default:
            {
                JSONCONS_THROW(cbor_decode_error(end_-pos));
            }
        }
    }

    template<typename T>
    Json get_fixed_length_array(const T len)
    {
        Json result = typename Json::array();
        result.reserve(len);
        for (T i = 0; i < len; ++i)
        {
            result.push_back(decode());
        }
        return result;
    }

    template<typename T>
    Json get_fixed_length_map(const T len)
    {
        Json result = typename Json::object();
        result.reserve(len);
        for (T i = 0; i < len; ++i)
        {
            auto j = decode();
            result.set(j.as_string_view(),decode());
        }
        return result;
    }
};

template<class Json>
void encode_cbor(const Json& j, std::vector<uint8_t>& v)
{
    size_t n = 0;
    cbor_Encoder_<Json>::encode(j,Calculate_size_(),n);

    v.reserve(n);
    cbor_Encoder_<Json>::encode(j,Encode_cbor_(),v);
}

template<class Json>
Json decode_cbor(const cbor_view& v)
{
    Decode_cbor_<Json> decoder(v.buffer(),v.buffer()+v.buflen());
    return decoder.decode();
}
  
#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
std::vector<uint8_t> encode_cbor(const Json& j)
{
    std::vector<uint8_t> v;
    encode_cbor(j, v);
    return v;
}
#endif

}}

#endif
