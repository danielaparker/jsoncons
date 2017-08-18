// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BINARY_MSGPACK_HPP
#define JSONCONS_BINARY_MSGPACK_HPP

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

namespace jsoncons { namespace msgpack {
  
namespace msgpack_format
{
    const uint8_t nil_cd = 0xc0;
    const uint8_t false_cd = 0xc2;
    const uint8_t true_cd = 0xc3;
    const uint8_t float32_cd = 0xca;
    const uint8_t float64_cd = 0xcb;
    const uint8_t uint8_cd = 0xcc;
    const uint8_t uint16_cd = 0xcd;
    const uint8_t uint32_cd = 0xce;
    const uint8_t uint64_cd = 0xcf;
    const uint8_t int8_cd = 0xd0;
    const uint8_t int16_cd = 0xd1;
    const uint8_t int32_cd = 0xd2;
    const uint8_t int64_cd = 0xd3;
    const uint8_t str8_cd = 0xd9;
    const uint8_t str16_cd = 0xda;
    const uint8_t str32_cd = 0xdb;
    const uint8_t array16_cd = 0xdc;
    const uint8_t array32_cd = 0xdd;
    const uint8_t map16_cd = 0xde;
    const uint8_t map32_cd = 0xdf;
}

template<class Json>
class Encode_message_pack_
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
                // nil
                v_.push_back(msgpack_format::nil_cd);
                break;
            }

            case value_type::bool_t:
            {
                // true and false
                v_.push_back(jval.as_bool() ? msgpack_format::true_cd : msgpack_format::false_cd );
                break;
            }

            case value_type::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val <= (std::numeric_limits<int8_t>::max)())
                    {
                        // positive fixnum stores 7-bit positive integer
                        detail::binary::to_big_endian<int8_t>()(static_cast<int8_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        // uint 8 stores a 8-bit unsigned integer
                        v_.push_back(msgpack_format::uint8_cd);
                        detail::binary::to_big_endian<uint8_t>()(static_cast<uint8_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        // uint 16 stores a 16-bit big-endian unsigned integer
                        v_.push_back(msgpack_format::uint16_cd);
                        detail::binary::to_big_endian<uint16_t>()(static_cast<uint16_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        // uint 32 stores a 32-bit big-endian unsigned integer
                        v_.push_back(msgpack_format::uint32_cd);
                        detail::binary::to_big_endian<uint32_t>()(static_cast<uint32_t>(val),v_);
                    }
                    else if (val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // int 64 stores a 64-bit big-endian signed integer
                        v_.push_back(msgpack_format::int64_cd);
                        detail::binary::to_big_endian<int64_t>()(static_cast<int64_t>(val),v_);
                    }
                }
                else
                {
                    if (val >= -32)
                    {
                        // negative fixnum stores 5-bit negative integer
                        v_.push_back(static_cast<int8_t>((val)));
                    }
                    else if (val >= (std::numeric_limits<int8_t>::min)() && val <= (std::numeric_limits<int8_t>::max)())
                    {
                        // int 8 stores a 8-bit signed integer
                        v_.push_back(msgpack_format::int8_cd);
                        detail::binary::to_big_endian<int8_t>()(static_cast<int8_t>(val),v_);
                    }
                    else if (val >= (std::numeric_limits<int16_t>::min)() && val <= (std::numeric_limits<int16_t>::max)())
                    {
                        // int 16 stores a 16-bit big-endian signed integer
                        v_.push_back(msgpack_format::int16_cd);
                        detail::binary::to_big_endian<int16_t>()(static_cast<int16_t>(val),v_);
                    }
                    else if (val >= (std::numeric_limits<int32_t>::min)() && val <= (std::numeric_limits<int32_t>::max)())
                    {
                        // int 32 stores a 32-bit big-endian signed integer
                        v_.push_back(msgpack_format::int32_cd);
                        detail::binary::to_big_endian<int32_t>()(static_cast<int32_t>(val),v_);
                    }
                    else if (val >= (std::numeric_limits<int64_t>::min)() && val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // int 64 stores a 64-bit big-endian signed integer
                        v_.push_back(msgpack_format::int64_cd);
                        detail::binary::to_big_endian<int64_t>()(static_cast<int64_t>(val),v_);
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
                    v_.push_back(static_cast<uint8_t>((val)));
                }
                else if (val <= (std::numeric_limits<uint8_t>::max)())
                {
                    // uint 8 stores a 8-bit unsigned integer
                    v_.push_back(msgpack_format::uint8_cd);
                    v_.push_back(static_cast<uint8_t>((val)));
                }
                else if (val <= (std::numeric_limits<uint16_t>::max)())
                {
                    // uint 16 stores a 16-bit big-endian unsigned integer
                    v_.push_back(msgpack_format::uint16_cd);
                    detail::binary::to_big_endian<uint16_t>()(static_cast<uint16_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint32_t>::max)())
                {
                    // uint 32 stores a 32-bit big-endian unsigned integer
                    v_.push_back(msgpack_format::uint32_cd);
                    detail::binary::to_big_endian<uint32_t>()(static_cast<uint32_t>(val),v_);
                }
                else if (val <= (std::numeric_limits<uint64_t>::max)())
                {
                    // uint 64 stores a 64-bit big-endian unsigned integer
                    v_.push_back(msgpack_format::uint64_cd);
                    detail::binary::to_big_endian<uint64_t>()(static_cast<uint64_t>(val),v_);
                }
                break;
            }

            case value_type::double_t:
            {
                // float 64
                v_.push_back(msgpack_format::float64_cd);
                detail::binary::to_big_endian<double>()(jval.as_double(),v_);
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
                if (length <= 15)
                {
                    // fixarray
                    v_.push_back(static_cast<uint8_t>(0x90 | length));
                }
                else if (length <= (std::numeric_limits<uint16_t>::max)())
                {
                    // array 16
                    v_.push_back(msgpack_format::array16_cd);
                    detail::binary::to_big_endian<uint16_t>()(static_cast<uint16_t>(length),v_);
                }
                else if (length <= (std::numeric_limits<uint32_t>::max)())
                {
                    // array 32
                    v_.push_back(msgpack_format::array32_cd);
                    detail::binary::to_big_endian<uint32_t>()(static_cast<uint32_t>(length),v_);
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
                if (length <= 15)
                {
                    // fixmap
                    v_.push_back(static_cast<uint8_t>(0x80 | (length & 0xf)));
                }
                else if (length <= 65535)
                {
                    // map 16
                    v_.push_back(msgpack_format::map16_cd );
                    detail::binary::to_big_endian<uint16_t>()(static_cast<uint16_t>(length),v_);
                }
                else if (length <= 4294967295)
                {
                    // map 32
                    v_.push_back(msgpack_format::map32_cd );
                    detail::binary::to_big_endian<uint32_t>()(static_cast<uint32_t>(length),v_);
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
        if (length <= 31)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            v_.push_back(static_cast<uint8_t>(0xa0 | length));
        }
        else if (length <= (std::numeric_limits<uint8_t>::max)())
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            v_.push_back(msgpack_format::str8_cd);
            v_.push_back(static_cast<uint8_t>(length));
        }
        else if (length <= (std::numeric_limits<uint16_t>::max)())
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            v_.push_back(msgpack_format::str16_cd);
            detail::binary::to_big_endian<uint16_t>()(static_cast<uint16_t>(length), v_);
        }
        else if (length <= (std::numeric_limits<uint32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            v_.push_back(msgpack_format::str32_cd);
            detail::binary::to_big_endian<uint32_t>()(static_cast<uint32_t>(length),v_);
        }

        for (size_t i = 0; i < length; ++i)
        {
            v_.push_back(target.data()[i]);
        }
    }
};

// decode_msgpack

template<class Json>
class Decode_message_pack_
{
    const uint8_t* begin_;
    const uint8_t* end_;
    const uint8_t* it_;
public:
    typedef typename Json::char_type char_type;

    Decode_message_pack_(const uint8_t* begin, const uint8_t* end)
        : begin_(begin), end_(end), it_(begin)
    {
    }

    Json decode()
    {
        // store && increment index
        const uint8_t* pos = it_++;

        if (*pos <= 0xbf)
        {
            if (*pos <= 0x7f) 
            {
                // positive fixint
                return Json(*pos);
            }
            else if (*pos <= 0x8f) 
            {
                // fixmap
                Json result;
                const size_t len = *pos & 0x0f;
                result.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    auto j = decode();
                    result.set(j.as_string_view(),decode());
                }
                return result;
            }
            else if (*pos <= 0x9f) 
            {
                // fixarray
                Json result = typename Json::array();
                const size_t len = *pos & 0x0f;
                result.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    result.add(decode());
                }
                return result;
            }
            else 
            {
                // fixstr
                const size_t len = *pos & 0x1f;
                const uint8_t* first = &(*it_);
                const uint8_t* last = first + len;
                it_ += len; 

                std::basic_string<char_type> target;
                auto result = unicons::convert(
                    first, last,std::back_inserter(target),unicons::conv_flags::strict);
                if (result.first != unicons::conv_errc::ok)
                {
                    JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                }
                return Json(target);
            }
        }
        else if (*pos >= 0xe0) 
        {
            // negative fixint
            return static_cast<int8_t>(*pos);
        }
        else
        {
            switch (*pos)
            {
                case msgpack_format::nil_cd: 
                {
                    return Json(null_type());
                }
                case msgpack_format::true_cd:
                {
                    return Json(true);
                }
                case msgpack_format::false_cd:
                {
                    return Json(false);
                }
                case msgpack_format::float32_cd: 
                {
                    float res = detail::binary::from_big_endian<float>()(it_,end_);
                    it_ += sizeof(float); 
                    return res;
                }

                case msgpack_format::float64_cd: 
                {
                    double res = detail::binary::from_big_endian<double>()(it_,end_);
                    it_ += sizeof(double); 
                    return res;
                }

                case msgpack_format::uint8_cd: 
                {
                    auto x = detail::binary::from_big_endian<uint8_t>()(it_,end_);
                    it_ += sizeof(uint8_t); 
                    return Json(x);
                }

                case msgpack_format::uint16_cd: 
                {
                    auto x = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += sizeof(uint16_t); 
                    return x;
                }

                case msgpack_format::uint32_cd: 
                {
                    auto x = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += sizeof(uint32_t); 
                    return x;
                }

                case msgpack_format::uint64_cd: 
                {
                    auto x = detail::binary::from_big_endian<uint64_t>()(it_,end_);
                    it_ += sizeof(uint64_t); 
                    return x;
                }

                case msgpack_format::int8_cd: 
                {
                    auto x = detail::binary::from_big_endian<int8_t>()(it_,end_);
                    it_ += sizeof(int8_t); 
                    return Json(x);
                }

                case msgpack_format::int16_cd: 
                {
                    auto x = detail::binary::from_big_endian<int16_t>()(it_,end_);
                    it_ += sizeof(int16_t); 
                    return x;
                }

                case msgpack_format::int32_cd: 
                {
                    auto x = detail::binary::from_big_endian<int32_t>()(it_,end_);
                    it_ += sizeof(int32_t); 
                    return x;
                }

                case msgpack_format::int64_cd: 
                {
                    auto x = detail::binary::from_big_endian<int64_t>()(it_,end_);
                    it_ += sizeof(int64_t); 
                    return x;
                }

                case msgpack_format::str8_cd: 
                {
                    const auto len = detail::binary::from_big_endian<int8_t>()(it_,end_);
                    const uint8_t* first = &(*(pos + 2));
                    const uint8_t* last = first + len;
                    it_ += len+1; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return target;
                }

                case msgpack_format::str16_cd: 
                {
                    const auto len = detail::binary::from_big_endian<int16_t>()(it_,end_);
                    const uint8_t* first = &(*(pos + 3));
                    const uint8_t* last = first + len;
                    it_ += len + 2; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return target;
                }

                case msgpack_format::str32_cd: 
                {
                    const auto len = detail::binary::from_big_endian<int32_t>()(it_,end_);
                    const uint8_t* first = &(*(pos + 5));
                    const uint8_t* last = first + len;
                    it_ += len + 4; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.first != unicons::conv_errc::ok)
                    {
                        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Illegal unicode");
                    }
                    return target;
                }

                case msgpack_format::array16_cd: 
                {
                    Json result = typename Json::array();
                    const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += 2; 
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case msgpack_format::array32_cd: 
                {
                    Json result = typename Json::array();
                    const auto len = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += 4; 
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case msgpack_format::map16_cd : 
                {
                    const auto len = detail::binary::from_big_endian<uint16_t>()(it_,end_);
                    it_ += 2; 
                    Json result = typename Json::object();
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto j = decode();
                        result.set(j.as_string_view(),decode());
                    }
                    return result;
                }

                case msgpack_format::map32_cd : 
                {
                    Json result = typename Json::object();
                    const auto len = detail::binary::from_big_endian<uint32_t>()(it_,end_);
                    it_ += 4; 
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto key = decode().as_string_view();
                        result.set(key,decode());
                    }
                    return result;
                }

                default:
                {
                    JSONCONS_THROW_EXCEPTION_1(std::invalid_argument,"Error decoding a message pack at position %s", std::to_string(end_-pos));
                }
            }
        }
    }
};

template<class Json>
std::vector<uint8_t> encode_msgpack(const Json& j)
{
    Encode_message_pack_<Json> encoder;
    return encoder.encode(j);
}

template<class Json>
Json decode_msgpack(const std::vector<uint8_t>& v)
{
    Decode_message_pack_<Json> decoder(v.data(),v.data()+v.size());
    return decoder.decode();
}

}}

#endif
