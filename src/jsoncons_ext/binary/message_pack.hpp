// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BINARY_MESSAGE_PACK_HPP
#define JSONCONS_BINARY_MESSAGE_PACK_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/binary/binary_util.hpp>

namespace jsoncons { namespace binary {
  
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
            case value_types::null_t:
            {
                ++n;
                break;
            }

            case value_types::bool_t:
            {
                ++n;
                break;
            }

            case value_types::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val < (std::numeric_limits<int8_t>::max)())
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

        case value_types::uinteger_t:
            {
                uint64_t val = jval.as_uinteger();
                if (val < (std::numeric_limits<int8_t>::max)())
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

            case value_types::double_t:
            {
                // float 64
                double val = jval.as_double();
                if (val >= -(std::numeric_limits<float>::max)() && val <= (std::numeric_limits<float>::max)())
                {
                    n += (1 + sizeof(float));
                }
                else
                {
                    n += (1 + sizeof(double));
                }
                break;
            }

            case value_types::small_string_t:
            case value_types::string_t:
            {
                n += calculate_string_size(jval.as_string_view());
                break;
            }

            case value_types::array_t:
            {
                const auto length = jval.array_value().size();
                if (length <= 15)
                {
                    // fixarray
                    n += sizeof(uint8_t);
                }
                else if (length <= 0xffff)
                {
                    // array 16
                    n += 1 + sizeof(uint16_t);
                }
                else if (length <= 0xffffffff)
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

            case value_types::object_t:
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
                for (const auto& kvp: jval.object_range())
                {
                    n += calculate_string_size(kvp.key());
                    n += calculate_size(kvp.value());
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

    static size_t calculate_string_size(typename Json::string_view_type sv)
    {
        size_t n = 0;

        const size_t length = sv.length();
        if (length <= 31)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            n += sizeof(uint8_t);
        }
        else if (length <= 255)
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            n += 1 + sizeof(uint8_t);
        }
        else if (length <= 65535)
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            n += 1 + sizeof(uint16_t);
        }
        else if (length <= 4294967295)
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
            case value_types::null_t:
            {
                // nil
                v_.push_back(msgpack_format::nil_cd);
                break;
            }

            case value_types::bool_t:
            {
                // true and false
                v_.push_back(jval.as_bool() ? msgpack_format::true_cd : msgpack_format::false_cd );
                break;
            }

            case value_types::integer_t:
            {
                int64_t val = jval.as_integer();
                if (val >= 0)
                {
                    if (val < (std::numeric_limits<int8_t>::max)())
                    {
                        // positive fixnum stores 7-bit positive integer
                        to_big_endian<int64_t, sizeof(int8_t)>()(val,v_);
                    }
                    else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        // uint 8 stores a 8-bit unsigned integer
                        v_.push_back(msgpack_format::uint8_cd);
                        to_big_endian<int64_t, sizeof(uint8_t)>()(val,v_);
                    }
                    else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        // uint 16 stores a 16-bit big-endian unsigned integer
                        v_.push_back(msgpack_format::uint16_cd);
                        to_big_endian<int64_t, sizeof(uint16_t)>()(val,v_);
                    }
                    else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        // uint 32 stores a 32-bit big-endian unsigned integer
                        v_.push_back(msgpack_format::uint32_cd);
                        to_big_endian<int64_t, sizeof(uint32_t)>()(val,v_);
                    }
                    else if (val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // uint 64 stores a 64-bit big-endian unsigned integer
                        v_.push_back(msgpack_format::uint64_cd);
                        to_big_endian<int64_t, sizeof(uint64_t)>()(val,v_);
                    }
                }
                else
                {
                    if (val >= -32)
                    {
                        // negative fixnum stores 5-bit negative integer
                        v_.push_back(static_cast<uint8_t>((val)));
                    }
                    else if (val >= (std::numeric_limits<int8_t>::min)() && val <= (std::numeric_limits<int8_t>::max)())
                    {
                        // int 8 stores a 8-bit signed integer
                        v_.push_back(msgpack_format::int8_cd);
                        to_big_endian<uint64_t, sizeof(uint8_t)>()(val,v_);
                    }
                    else if (val >= (std::numeric_limits<int16_t>::min)() && val <= (std::numeric_limits<int16_t>::max)())
                    {
                        // int 16 stores a 16-bit big-endian signed integer
                        v_.push_back(msgpack_format::int16_cd);
                        to_big_endian<int64_t, sizeof(uint16_t)>()(val,v_);
                    }
                    else if (val >= (std::numeric_limits<int32_t>::min)() && val <= INT32_MAX)
                    {
                        // int 32 stores a 32-bit big-endian signed integer
                        v_.push_back(msgpack_format::int32_cd);
                        to_big_endian<int64_t, sizeof(uint32_t)>()(val,v_);
                    }
                    else if (val >= (std::numeric_limits<int64_t>::min)() && val <= (std::numeric_limits<int64_t>::max)())
                    {
                        // int 64 stores a 64-bit big-endian signed integer
                        v_.push_back(msgpack_format::int64_cd);
                        to_big_endian<int64_t, sizeof(uint64_t)>()(val,v_);
                    }
                }
                break;
            }

        case value_types::uinteger_t:
            {
                uint64_t val = jval.as_uinteger();
                if (val < (std::numeric_limits<int8_t>::max)())
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
                    to_big_endian<uint64_t, sizeof(uint16_t)>()(val,v_);
                }
                else if (val <= (std::numeric_limits<uint32_t>::max)())
                {
                    // uint 32 stores a 32-bit big-endian unsigned integer
                    v_.push_back(msgpack_format::uint32_cd);
                    to_big_endian<uint64_t, sizeof(uint32_t)>()(val,v_);
                }
                else if (val <= (std::numeric_limits<uint64_t>::max)())
                {
                    // uint 64 stores a 64-bit big-endian unsigned integer
                    v_.push_back(msgpack_format::uint64_cd);
                    to_big_endian<uint64_t, sizeof(uint64_t)>()(val,v_);
                }
                break;
            }

            case value_types::double_t:
            {
                // float 64
                double val = jval.as_double();
                if (val >= -(std::numeric_limits<float>::max)() && val <= (std::numeric_limits<float>::max)())
                {
                    v_.push_back(msgpack_format::float32_cd );
                    to_big_endian<float,sizeof(float)>()((float)jval.as_double(),v_);
                }
                else
                {
                    v_.push_back(msgpack_format::float64_cd);
                    to_big_endian<double,sizeof(double)>()(jval.as_double(),v_);
                }
                break;
            }

            case value_types::small_string_t:
            case value_types::string_t:
            {
                encode_string(jval.as_string_view());
                break;
            }

            case value_types::array_t:
            {
                const auto length = jval.array_value().size();
                if (length <= 15)
                {
                    // fixarray
                    v_.push_back(static_cast<uint8_t>(0x90 | length));
                }
                else if (length <= 0xffff)
                {
                    // array 16
                    v_.push_back(msgpack_format::array16_cd);
                    to_big_endian<uint64_t, sizeof(uint16_t)>()(static_cast<uint16_t>(length),v_);
                }
                else if (length <= 0xffffffff)
                {
                    // array 32
                    v_.push_back(msgpack_format::array32_cd);
                    to_big_endian<uint64_t, sizeof(uint32_t)>()(length,v_);
                }

                // append each element
                for (const auto& el : jval.array_range())
                {
                    encode_(el);
                }
                break;
            }

            case value_types::object_t:
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
                    to_big_endian<uint64_t, sizeof(uint16_t)>()(static_cast<uint16_t>(length),v_);
                }
                else if (length <= 4294967295)
                {
                    // map 32
                    v_.push_back(msgpack_format::map32_cd );
                    to_big_endian<uint64_t, sizeof(uint32_t)>()(length,v_);
                }

                // append each element
                for (const auto& kvp: jval.object_range())
                {
                    encode_string(kvp.key());
                    encode_(kvp.value());
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void encode_string(typename Json::string_view_type sv)
    {
        const size_t length = sv.length();
        if (length <= 31)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            v_.push_back(static_cast<uint8_t>(0xa0 | length));
        }
        else if (length <= 255)
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            v_.push_back(msgpack_format::str8_cd);
            v_.push_back(static_cast<uint8_t>(length));
        }
        else if (length <= 65535)
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            v_.push_back(msgpack_format::str16_cd);
            to_big_endian<size_t, sizeof(uint16_t)>()(length, v_);
        }
        else if (length <= 4294967295)
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            v_.push_back(msgpack_format::str32_cd);
            to_big_endian<size_t, sizeof(uint32_t)>()(length,v_);
        }

        for (size_t i = 0; i < length; ++i)
        {
            v_.push_back(sv.data()[i]);
        }
    }
};

template<class Json>
std::vector<uint8_t> encode_message_pack(const Json& jval)
{
    Encode_message_pack_<Json> encoder;
    return encoder.encode(jval);
}

// decode_message_pack

template<class Json>
class Decode_message_pack_
{
    std::vector<uint8_t>::const_iterator begin_;
    std::vector<uint8_t>::const_iterator end_;
    std::vector<uint8_t>::const_iterator it_;
public:
    Decode_message_pack_(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
        : begin_(begin), end_(end), it_(begin)
    {
    }

    Json decode()
    {
        // store && increment index
        std::vector<uint8_t>::const_iterator pos = it_++;

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
                auto offset = &(*(pos + 1));
                it_ += len; 
                return Json(reinterpret_cast<const typename Json::char_type*>(offset), len);
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
                    // reverse byte order
                    float res;
                    for (size_t byte = 0; byte < sizeof(float); ++byte)
                    {
                        reinterpret_cast<uint8_t*>(&res)[sizeof(float) - byte - 1] = *(pos + 1 + byte);
                    }
                    it_ += sizeof(float); 
                    return res;
                }

                case msgpack_format::float64_cd: 
                {
                    // reverse byte order
                    double res;
                    for (size_t byte = 0; byte < sizeof(double); ++byte)
                    {
                        reinterpret_cast<uint8_t*>(&res)[sizeof(double) - byte - 1] = *(pos + 1 + byte);
                    }
                    it_ += sizeof(double); 
                    return res;
                }

                case msgpack_format::uint8_cd: 
                {
                    it_ += 1; 
                    return from_big_endian<uint8_t>(pos,end_);
                }

                case msgpack_format::uint16_cd: 
                {
                    it_ += 2; 
                    return from_big_endian<uint16_t>(pos,end_);
                }

                case msgpack_format::uint32_cd: 
                {
                    it_ += 4; 
                    return from_big_endian<uint32_t>(pos,end_);
                }

                case msgpack_format::uint64_cd: 
                {
                    it_ += 8; 
                    return from_big_endian<uint64_t>(pos,end_);
                }

                case msgpack_format::int8_cd: 
                {
                    it_ += 1; 
                    return from_big_endian<int8_t>(pos,end_);
                }

                case msgpack_format::int16_cd: 
                {
                    it_ += 2; 
                    return from_big_endian<int16_t>(pos,end_);
                }

                case msgpack_format::int32_cd: 
                {
                    it_ += 4; 
                    return from_big_endian<int32_t>(pos,end_);
                }

                case msgpack_format::int64_cd: 
                {
                    it_ += 8; 
                    return from_big_endian<int64_t>(pos,end_);
                }

                case msgpack_format::str8_cd: 
                {
                    const auto len = from_big_endian<uint8_t>(pos,end_);
                    auto offset = &(*(pos + 2));
                    it_ += len + 1; 
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case msgpack_format::str16_cd: 
                {
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    auto offset = &(*(pos + 3));
                    it_ += len + 2; 
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case msgpack_format::str32_cd: 
                {
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    auto offset = &(*(pos + 5));
                    it_ += len + 4; 
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case msgpack_format::array16_cd: 
                {
                    Json result = typename Json::array();
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    it_ += 2; 
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case msgpack_format::array32_cd: 
                {
                    Json result = typename Json::array();
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    it_ += 4; 
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case msgpack_format::map16_cd : 
                {
                    Json result = typename Json::object();
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    it_ += 2; 
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
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    it_ += 4; 
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
Json decode_message_pack(const std::vector<uint8_t>& v)
{
    Decode_message_pack_<Json> decoder(v.begin(),v.end());
    return decoder.decode();
}

}}

#endif
