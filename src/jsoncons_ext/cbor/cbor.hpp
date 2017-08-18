// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BINARY_CBOR_HPP
#define JSONCONS_BINARY_CBOR_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/detail/binary_utilities.hpp>

namespace jsoncons { namespace cbor {
  
template<class Json>
class Encode_cbor_
{
    std::vector<uint8_t> v_;
public:
    typedef typename Json::char_type char_type;
    typedef typename Json::string_view_type string_view_type;

    std::vector<uint8_t> encode(const Json& jval)
    {
        v_.reserve(calculate_size(jval));
        encode_(jval);
        return std::move(v_);
    }

    static size_t calculate_size(const Json& jval)
    {
        size_t n = 0;
        switch (jval.type_id())
        {
            case value_type::null_t:
            {
                ++n;
                break;
            }

            case value_type::bool_t:
            {
                ++n;
                break;
            }

            case value_type::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val <= (std::numeric_limits<int8_t>::max)())
                    {
                        ++n;
                    }
                    else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        // uint 8 stores a 8-bit unsigned integer
                        n += (1 + sizeof(uint8_t));
                    }
                    else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        // uint 16 stores a 16-bit big-endian unsigned integer
                        n += (1 + sizeof(uint16_t));
                    }
                    else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        // uint 32 stores a 32-bit big-endian unsigned integer
                        n += (1 + sizeof(uint32_t));
                    }
                    else if (val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // uint 64 stores a 64-bit big-endian unsigned integer
                        n += (1 + sizeof(uint64_t));
                    }
                }
                else
                {
                    if (val >= -32)
                    {
                        // negative fixnum stores 5-bit negative integer
                        ++n;
                    }
                    else if (val >= (std::numeric_limits<int8_t>::min)() && val <= (std::numeric_limits<int8_t>::max)())
                    {
                        // int 8 stores a 8-bit signed integer
                        n += (1 + sizeof(uint8_t));
                    }
                    else if (val >= (std::numeric_limits<int16_t>::min)() && val <= (std::numeric_limits<int16_t>::max)())
                    {
                        // int 16 stores a 16-bit big-endian signed integer
                        n += (1 + sizeof(uint16_t));
                    }
                    else if (val >= (std::numeric_limits<int32_t>::min)() && val <= INT32_MAX)
                    {
                        // int 32 stores a 32-bit big-endian signed integer
                        n += (1 + sizeof(uint32_t));
                    }
                    else if (val >= (std::numeric_limits<int64_t>::min)() && val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // int 64 stores a 64-bit big-endian signed integer
                        n += (1 + sizeof(uint64_t));
                    }
                }
                break;
            }

        case value_type::uinteger_t:
            {
                uint64_t val = jval.as_uinteger();
                if (val <= (std::numeric_limits<int8_t>::max)())
                {
                    // positive fixnum stores 7-bit positive integer
                    ++n;
                }
                else if (val <= (std::numeric_limits<uint8_t>::max)())
                {
                    n += (1 + sizeof(uint8_t));
                }
                else if (val <= (std::numeric_limits<uint16_t>::max)())
                {
                    // uint 16 stores a 16-bit big-endian unsigned integer
                    n += (1 + sizeof(uint16_t));
                }
                else if (val <= (std::numeric_limits<uint32_t>::max)())
                {
                    // uint 32 stores a 32-bit big-endian unsigned integer
                    n += (1 + sizeof(uint32_t));
                }
                else if (val <= (std::numeric_limits<uint64_t>::max)())
                {
                    // uint 64 stores a 64-bit big-endian unsigned integer
                    n += (1 + sizeof(uint64_t));
                }
                break;
            }

            case value_type::double_t:
            {
                // float 64
                n += (1 + sizeof(double));
                break;
            }

            case value_type::small_string_t:
            case value_type::string_t:
            {
                n += calculate_string_size(jval.as_string_view());
                break;
            }

            case value_type::array_t:
            {
                const auto length = jval.array_value().size();
                if (length <= 15)
                {
                    // fixarray
                    n += sizeof(uint8_t);
                }
                else if (length <= (std::numeric_limits<uint16_t>::max)())
                {
                    // array 16
                    n += 1 + sizeof(uint16_t);
                }
                else if (length <= (std::numeric_limits<uint32_t>::max)())
                {
                    // array 32
                    n += 1 + sizeof(uint32_t);
                }

                // calculate size for each element
                for (const auto& el : jval.array_range())
                {
                    n += calculate_size(el);
                }
                break;
            }

            case value_type::object_t:
            {
                const auto length = jval.object_value().size();
                if (length <= 15)
                {
                    // fixmap
                    n += sizeof(uint8_t);
                }
                else if (length <= 65535)
                {
                    // map 16
                    n += 1 + sizeof(uint16_t);
                }
                else if (length <= 4294967295)
                {
                    // map 32
                    n += 1 + sizeof(uint32_t);
                }

                // calculate size for each member
                for (const auto& kv: jval.object_range())
                {
                    n += calculate_string_size(kv.key());
                    n += calculate_size(kv.value());
                }
                break;
            }

            default:
            {
                break;
            }
        }
        return n;
    }

    static size_t calculate_string_size(string_view_type sv)
    {
        size_t n = 0;

        const size_t length = unicons::u8_length(sv.begin(),sv.end());
        if (length <= 31)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            n += sizeof(uint8_t);
        }
        else if (length <= (std::numeric_limits<uint8_t>::max)())
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            n += 1 + sizeof(uint8_t);
        }
        else if (length <= (std::numeric_limits<uint16_t>::max)())
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            n += 1 + sizeof(uint16_t);
        }
        else if (length <= (std::numeric_limits<uint32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            n += 1 + sizeof(uint32_t);
        }

        n += length;

        return n;
    }

    void encode_(const Json& jval)
    {
        switch (jval.type_id())
        {
            case value_type::null_t:
            {
                v_.push_back(0xf6);
                break;
            }

            case value_type::bool_t:
            {
                v_.push_back(jval.as_bool() ? 0xf5 : 0xf4);
                break;
            }

            case value_type::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val <= 0x17)
                    {
                        detail::binary::to_big_endian(static_cast<uint8_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        v_.push_back(0x18);
                        detail::binary::to_big_endian(static_cast<uint8_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        v_.push_back(0x19);
                        detail::binary::to_big_endian(static_cast<uint16_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        v_.push_back(0x1a);
                        detail::binary::to_big_endian(static_cast<uint32_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<int64_t>::max)())
                    {
                        v_.push_back(0x1b);
                        detail::binary::to_big_endian(static_cast<int64_t>(val),v_);
                    }
                }
                else
                {
                    const auto posnum = -1 - val;
                    if (val >= -24)
                    {
                        v_.push_back(static_cast<uint8_t>(0x20 + posnum));
                    }
                    else if (posnum <= (std::numeric_limits<uint8_t>::max)())
                    {
                        v_.push_back(0x38);
                        detail::binary::to_big_endian(static_cast<uint8_t>(posnum),v_);
                    }
                    else if (posnum <= (std::numeric_limits<uint16_t>::max)())
                    {
                        v_.push_back(0x39);
                        detail::binary::to_big_endian(static_cast<uint16_t>(posnum),v_);
                    }
                    else if (posnum <= (std::numeric_limits<uint32_t>::max)())
                    {
                        v_.push_back(0x3a);
                        detail::binary::to_big_endian(static_cast<uint32_t>(posnum),v_);
                    }
                    else if (posnum <= (std::numeric_limits<int64_t>::max)())
                    {
                        v_.push_back(0x3b);
                        detail::binary::to_big_endian(static_cast<int64_t>(posnum),v_);
                    }
                }
                break;
            }

        case value_type::uinteger_t:
            {
                uint64_t val = jval.as_uinteger();
                if (val <= 0x17)
                {
                    detail::binary::to_big_endian(static_cast<uint8_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint8_t>::max)())
                {
                    v_.push_back(0x18);
                    detail::binary::to_big_endian(static_cast<uint8_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint16_t>::max)())
                {
                    v_.push_back(0x19);
                    detail::binary::to_big_endian(static_cast<uint16_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint32_t>::max)())
                {
                    v_.push_back(0x1a);
                    detail::binary::to_big_endian(static_cast<uint32_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint64_t>::max)())
                {
                    v_.push_back(0x1b);
                    detail::binary::to_big_endian(static_cast<uint64_t>(val),v_);
                }
                break;
            }

            case value_type::double_t:
            {
                v_.push_back(0xfb);
                detail::binary::to_big_endian(jval.as_double(),v_);
                break;
            }

            case value_type::small_string_t:
            case value_type::string_t:
            {
                encode_string(jval.as_string_view());
                break;
            }

            case value_type::array_t:
            {
                const auto length = jval.array_value().size();
                if (length <= 0x17)
                {
                    v_.push_back(static_cast<uint8_t>(0x80 + length));
                }
                else if (length <= 0xff)
                {
                    v_.push_back(0x98);
                    v_.push_back(static_cast<uint8_t>(length));
                }
                else if (length <= 0xffff)
                {
                    v_.push_back(0x99);
                    detail::binary::to_big_endian(static_cast<uint16_t>(length),v_);
                }
                else if (length <= 0xffffffff)
                {
                    v_.push_back(0x9a);
                    detail::binary::to_big_endian(static_cast<uint32_t>(length),v_);
                }
                else if (length <= 0xffffffffffffffff)
                {
                    v_.push_back(0x9b);
                    detail::binary::to_big_endian(static_cast<uint64_t>(length),v_);
                }

                // append each element
                for (const auto& el : jval.array_range())
                {
                    encode_(el);
                }
                break;
            }

            case value_type::object_t:
            {
                const auto length = jval.object_value().size();
                if (length <= 0x17)
                {
                    v_.push_back(static_cast<uint8_t>(0xa0 + length));
                }
                else if (length <= 0xff)
                {
                    v_.push_back(0xb8);
                    v_.push_back(static_cast<uint8_t>(length));
                }
                else if (length <= 0xffff)
                {
                    v_.push_back(0xb9);
                    detail::binary::to_big_endian(static_cast<uint16_t>(length),v_);
                }
                else if (length <= 0xffffffff)
                {
                    v_.push_back(0xba);
                    detail::binary::to_big_endian(static_cast<uint32_t>(length),v_);
                }
                else if (length <= 0xffffffffffffffff)
                {
                    v_.push_back(0xbb);
                    detail::binary::to_big_endian(static_cast<uint64_t>(length),v_);
                }

                // append each element
                for (const auto& kv: jval.object_range())
                {
                    encode_string(kv.key());
                    encode_(kv.value());
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void encode_string(string_view_type sv)
    {
        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.first != unicons::conv_errc::ok)
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
        }

        const size_t length = target.length();
        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            v_.push_back(static_cast<uint8_t>(0x60 + length));
        }
        else if (length <= 0xff)
        {
            v_.push_back(0x78);
            v_.push_back(static_cast<uint8_t>(length));
        }
        else if (length <= 0xffff)
        {
            v_.push_back(0x79);
            detail::binary::to_big_endian(static_cast<uint16_t>(length), v_);
        }
        else if (length <= 0xffffffff)
        {
            v_.push_back(0x7a);
            detail::binary::to_big_endian(static_cast<uint32_t>(length), v_);
        }
        else if (length <= 0xffffffffffffffff)
        {
            v_.push_back(0x7b);
            detail::binary::to_big_endian(static_cast<uint64_t>(length),v_);
        }

        for (size_t i = 0; i < length; ++i)
        {
            v_.push_back(target.data()[i]);
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
        // store && increment index
        const uint8_t* pos = it_++;
        switch (*pos)
        {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
                return Json(*pos);

            case 0x18: // Unsigned integer (one-byte uint8_t follows)
                {
                    auto x = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                    it_ += sizeof(uint8_t); 
                    return Json(x);
                }

            case 0x19: // Unsigned integer (two-byte uint16_t follows)
                {
                    auto x = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += sizeof(uint16_t); 
                    return Json(x);
                }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
                {
                    auto x = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += sizeof(uint32_t); 
                    return Json(x);
                }

            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
                {
                    auto x = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                    it_ += sizeof(uint64_t); 
                    return Json(x);
                }

            // Negative integer -1-0x00..-1-0x17 (-1..-24)
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
            case 0x29:
            case 0x2a:
            case 0x2b:
            case 0x2c:
            case 0x2d:
            case 0x2e:
            case 0x2f:
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            case 0x35:
            case 0x36:
            case 0x37:
                return Json(static_cast<int8_t>(0x20 - 1 - *pos));

            case 0x38: // Negative integer (one-byte uint8_t follows)
            {
                // must be uint8_t !
                auto x = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                it_ += sizeof(uint8_t); 
                return Json(static_cast<int64_t>(-1) - x);
            }

            case 0x39: // Negative integer -1-n (two-byte uint16_t follows)
            {
                auto x = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                it_ += sizeof(uint16_t); 
                return Json(static_cast<int64_t>(-1) - x);
            }

            case 0x3a: // Negative integer -1-n (four-byte uint32_t follows)
            {
                auto x = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                it_ += sizeof(uint32_t); 
                return Json(static_cast<int64_t>(-1) - x);
            }

            case 0x3b: // Negative integer -1-n (eight-byte uint64_t follows)
            {
                auto x = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                it_ += sizeof(uint64_t); 
                return Json(static_cast<int64_t>(-1) - static_cast<int64_t>(x));
            }

            // UTF-8 string (0x00..0x17 bytes follow)
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6a:
            case 0x6b:
            case 0x6c:
            case 0x6d:
            case 0x6e:
            case 0x6f:
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x75:
            case 0x76:
            case 0x77:
                {
                    std::string s = get_string(*pos & 0x1f);
                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return Json(target);
                }
            case 0x78: // UTF-8 string (one-byte uint8_t for n follows)
                {
                    const auto len = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                    it_ += sizeof(uint8_t); 
                    std::string s = get_string(len);               
                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return Json(target);
                }
            case 0x79: // UTF-8 string (two-byte uint16_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += sizeof(uint16_t); 
                    std::string s = get_string(len);               
                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return Json(target);
                }
            case 0x7a: // UTF-8 string (four-byte uint32_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += sizeof(uint32_t); 
                    std::string s = get_string(len);               
                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return Json(target);
                }
            case 0x7b: // UTF-8 string (eight-byte uint64_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                    it_ += sizeof(uint64_t); 
                    std::string s =  get_string(len);               
                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return Json(target);
                }
            case 0x7f: // UTF-8 string (indefinite length)
            {
                std::string s;
                while (*it_ != 0xff)
                {
                    if (it_ == end_)
                    {
                        JSONCONS_THROW_EXCEPTION(std::invalid_argument,"eof");
                    }
                    std::string ss = get_string();
                    s.append(std::move(ss));
                }
                std::basic_string<char_type> target;
                auto result = unicons::convert(
                    s.begin(),s.end(),std::back_inserter(target),unicons::conv_flags::strict);
                if (result.first != unicons::conv_errc::ok)
                {
                    JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                }
                return Json(target);
            }

            // array (0x00..0x17 data items follow)
            case 0x80:
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
            case 0x86:
            case 0x87:
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b:
            case 0x8c:
            case 0x8d:
            case 0x8e:
            case 0x8f:
            case 0x90:
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x96:
            case 0x97:
            {
                return get_cbor_array(*pos & 0x1f);
            }

            case 0x98: // array (one-byte uint8_t for n follows)
            {
                const auto len = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                it_ += sizeof(uint8_t); 
                return get_cbor_array(len);
            }

            case 0x99: // array (two-byte uint16_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                it_ += sizeof(uint16_t); 
                return get_cbor_array(len);
            }

            case 0x9a: // array (four-byte uint32_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<int32_t>()(it_,end_);
                it_ += sizeof(uint32_t); 
                return get_cbor_array(len);
            }

            case 0x9b: // array (eight-byte uint64_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<int64_t>()(it_,end_);
                it_ += sizeof(uint64_t); 
                return get_cbor_array(len);
            }

            case 0x9f: // array (indefinite length)
            {
                Json result = typename Json::array();
                while (*pos != 0xff)
                {
                    result.add(decode());
                    pos = it_;
                }
                return result;
            }

            // map (0x00..0x17 pairs of data items follow)
            case 0xa0:
            case 0xa1:
            case 0xa2:
            case 0xa3:
            case 0xa4:
            case 0xa5:
            case 0xa6:
            case 0xa7:
            case 0xa8:
            case 0xa9:
            case 0xaa:
            case 0xab:
            case 0xac:
            case 0xad:
            case 0xae:
            case 0xaf:
            case 0xb0:
            case 0xb1:
            case 0xb2:
            case 0xb3:
            case 0xb4:
            case 0xb5:
            case 0xb6:
            case 0xb7:
            {
                return get_cbor_object(*pos & 0x1f);
            }

            case 0xb8: // map (one-byte uint8_t for n follows)
            {
                const auto len = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                it_ += sizeof(uint8_t); 
                return get_cbor_object(len);
            }

            case 0xb9: // map (two-byte uint16_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                it_ += sizeof(uint16_t); 
                return get_cbor_object(len);
            }

            case 0xba: // map (four-byte uint32_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                it_ += sizeof(uint32_t); 
                return get_cbor_object(len);
            }

            case 0xbb: // map (eight-byte uint64_t for n follow)
            {
                const auto len = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                it_ += sizeof(uint64_t); 
                return get_cbor_object(len);
            }

            case 0xbf: // map (indefinite length)
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

            case 0xf4: // false
            {
                return Json(false);
            }

            case 0xf5: // true
            {
                return Json(true);
            }

            case 0xf6: // null
            {
                return Json::null();
            }
/*
            case 0xf9: // Half-Precision Float (two-byte IEEE 754)
            {

                const int byte1 = get();
                check_eof();
                const int byte2 = get();
                check_eof();

                // code from RFC 7049, Appendix D, Figure 3:
                // As half-precision floating-point numbers were only added
                // to IEEE 754 in 2008, today's programming platforms often
                // still only have limited support for them. It is very
                // easy to include at least decoding support for them even
                // without such support. An example of a small decoder for
                // half-precision floating-point numbers in the C language
                // is shown in Fig. 3.
                const int half = (byte1 << 8) + byte2;
                const int exp = (half >> 10) & 0x1f;
                const int mant = half & 0x3ff;
                double val;
                if (exp == 0)
                {
                    val = std::ldexp(mant, -24);
                }
                else if (exp != 31)
                {
                    val = std::ldexp(mant + 1024, exp - 25);
                }
                else
                {
                    val = (mant == 0) ? std::numeric_limits<double>::infinity()
                          : std::numeric_limits<double>::quiet_NaN();
                }
                return (half & 0x8000) != 0 ? -val : val;
            }
*/
            case 0xfa: // Single-Precision Float (four-byte IEEE 754)
            {
                const auto val = detail::binary::from_big_endian<float>()(it_,end_);
                it_ += sizeof(float); 
                return Json(val);
            }

            case 0xfb: // Double-Precision Float (eight-byte IEEE 754)
            {
                const auto val = detail::binary::from_big_endian<double>()(it_,end_);
                it_ += sizeof(double); 
                return Json(val);
            }

            default: // anything else (0xFF is handled inside the other types)
            {
                JSONCONS_THROW_EXCEPTION_1(std::invalid_argument,"Error decoding a cbor at position %s", std::to_string(end_-pos));
            }
        }
    }

    std::string get_string()
    {
        const uint8_t* pos = it_++;
        switch (*pos)
        {
            // UTF-8 string (0x00..0x17 bytes follow)
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6a:
            case 0x6b:
            case 0x6c:
            case 0x6d:
            case 0x6e:
            case 0x6f:
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x75:
            case 0x76:
            case 0x77:
                {
                    return get_string(*pos & 0x1f);
                }
            case 0x78: // UTF-8 string (one-byte uint8_t for n follows)
                {
                    const auto len = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                    it_ += sizeof(uint8_t); 
                    return get_string(len);               
                }
            case 0x79: // UTF-8 string (two-byte uint16_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += sizeof(uint16_t); 
                    return get_string(len);               
                }
            case 0x7a: // UTF-8 string (four-byte uint32_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += sizeof(uint32_t); 
                    return get_string(len);               
                }
            case 0x7b: // UTF-8 string (eight-byte uint64_t for n follow)
                {
                    const auto len = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                    it_ += sizeof(uint64_t); 
                    return get_string(len);               
                }
            default: // anything else (0xFF is handled inside the other types)
            {
                JSONCONS_THROW_EXCEPTION_1(std::invalid_argument,"Error decoding a cbor at position %s", std::to_string(end_-pos));
            }
        }
    }

    template<typename T>
    std::string get_string(const T len)
    {
        const uint8_t* first = it_;
        const uint8_t* last = it_ + len;
        it_ += len; 

        return std::string(first,last);
    }

    template<typename T>
    Json get_cbor_array(const T len)
    {
        Json result = typename Json::array();
        result.reserve(len);
        for (T i = 0; i < len; ++i)
        {
            result.add(decode());
        }
        return result;
    }

    template<typename T>
    Json get_cbor_object(const T len)
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
std::vector<uint8_t> encode_cbor(const Json& j)
{
    Encode_cbor_<Json> encoder;
    return encoder.encode(j);
}

template<class Json>
Json decode_cbor(const std::vector<uint8_t>& v)
{
    Decode_cbor_<Json> decoder(v.data(),v.data()+v.size());
    return decoder.decode();
}

}}

#endif
