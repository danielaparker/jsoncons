// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MESSAGE_PACK_MESSAGE_PACK_HPP
#define JSONCONS_MESSAGE_PACK_MESSAGE_PACK_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/json_binary/json_binary_util.hpp>

namespace jsoncons { namespace json_binary {
  
// encode_message_pack
template<class StringView>
void encode_string(StringView sv,
                   std::vector<uint8_t>& v)
{
    const size_t length = sv.size();
    if (length <= 31)
    {
        // fixstr stores a byte array whose length is upto 31 bytes
        v.push_back(static_cast<uint8_t>(0xa0 | length));
    }
    else if (length <= 255)
    {
        // str 8 stores a byte array whose length is upto (2^8)-1 bytes
        v.push_back(0xd9);
        v.push_back(static_cast<uint8_t>(length));
    }
    else if (length <= 65535)
    {
        // str 16 stores a byte array whose length is upto (2^16)-1 bytes
        v.push_back(0xda);
        to_big_endian<size_t, sizeof(uint16_t)>()(length, v);
    }
    else if (length <= 4294967295)
    {
        // str 32 stores a byte array whose length is upto (2^32)-1 bytes
        v.push_back(0xdb);
        to_big_endian<size_t, sizeof(uint32_t)>()(length,v);
    }

    for (size_t i = 0; i < length; ++i)
    {
        v.push_back(sv.data()[i]);
    }
}

template<class Json>
void Encode_message_pack_(const Json& jval, std::vector<uint8_t>& v)
{
    switch (jval.type_id())
    {
        case value_types::null_t:
        {
            // nil
            v.push_back(0xc0);
            break;
        }

        case value_types::bool_t:
        {
            // true and false
            v.push_back(jval.as_bool() ? 0xc3 : 0xc2);
            break;
        }

        case value_types::integer_t:
        {
            int64_t ival = jval.as_integer();
            if (ival >= 0)
            {
                if (ival < 128)
                {
                    // positive fixnum stores 7-bit positive integer
                    to_big_endian<int64_t, sizeof(int8_t)>()(ival,v);
                }
                else if (ival <= (std::numeric_limits<int8_t>::max)())
                {
                    // uint 8 stores a 8-bit unsigned integer
                    v.push_back(0xcc);
                    to_big_endian<int64_t, sizeof(uint8_t)>()(ival,v);
                }
                else if (ival <= (std::numeric_limits<uint16_t>::max)())
                {
                    // uint 16 stores a 16-bit big-endian unsigned integer
                    v.push_back(0xcd);
                    to_big_endian<int64_t, sizeof(uint16_t)>()(ival,v);
                }
                else if (ival <= (std::numeric_limits<uint32_t>::max)())
                {
                    // uint 32 stores a 32-bit big-endian unsigned integer
                    v.push_back(0xce);
                    to_big_endian<int64_t, sizeof(uint32_t)>()(ival,v);
                }
                else if (ival <= (std::numeric_limits<int64_t>::max)())
                {
                    // uint 64 stores a 64-bit big-endian unsigned integer
                    v.push_back(0xcf);
                    to_big_endian<int64_t, sizeof(uint64_t)>()(ival,v);
                }
            }
            else
            {
                if (ival >= -32)
                {
                    // negative fixnum stores 5-bit negative integer
                    v.push_back(static_cast<uint8_t>((ival)));
                }
                else if (ival >= (std::numeric_limits<int8_t>::min)() && ival <= (std::numeric_limits<int8_t>::max)())
                {
                    // int 8 stores a 8-bit signed integer
                    v.push_back(0xd0);
                    to_big_endian<uint64_t, sizeof(uint8_t)>()(ival,v);
                }
                else if (ival >= (std::numeric_limits<int16_t>::min)() && ival <= (std::numeric_limits<int16_t>::max)())
                {
                    // int 16 stores a 16-bit big-endian signed integer
                    v.push_back(0xd1);
                    to_big_endian<int64_t, sizeof(uint16_t)>()(ival,v);
                }
                else if (ival >= (std::numeric_limits<int32_t>::min)() && ival <= INT32_MAX)
                {
                    // int 32 stores a 32-bit big-endian signed integer
                    v.push_back(0xd2);
                    to_big_endian<int64_t, sizeof(uint32_t)>()(ival,v);
                }
                else if (ival >= (std::numeric_limits<int64_t>::min)() && ival <= (std::numeric_limits<int64_t>::max)())
                {
                    // int 64 stores a 64-bit big-endian signed integer
                    v.push_back(0xd3);
                    to_big_endian<int64_t, sizeof(uint64_t)>()(ival,v);
                }
            }
            break;
        }

    case value_types::uinteger_t:
        {
            uint64_t ival = jval.as_uinteger();
            if (ival < (std::numeric_limits<int8_t>::max)())
            {
                // positive fixnum stores 7-bit positive integer
                v.push_back(static_cast<uint8_t>((ival)));
            }
            else if (ival <= (std::numeric_limits<uint8_t>::max)())
            {
                // uint 8 stores a 8-bit unsigned integer
                v.push_back(0xcc);
                v.push_back(static_cast<uint8_t>((ival)));
            }
            else if (ival <= (std::numeric_limits<uint16_t>::max)())
            {
                // uint 16 stores a 16-bit big-endian unsigned integer
                v.push_back(0xcd);
                to_big_endian<uint64_t, sizeof(uint16_t)>()(ival,v);
            }
            else if (ival <= (std::numeric_limits<uint32_t>::max)())
            {
                // uint 32 stores a 32-bit big-endian unsigned integer
                v.push_back(0xce);
                to_big_endian<uint64_t, sizeof(uint32_t)>()(ival,v);
            }
            else if (ival <= (std::numeric_limits<uint64_t>::max)())
            {
                // uint 64 stores a 64-bit big-endian unsigned integer
                v.push_back(0xcf);
                to_big_endian<uint64_t, sizeof(uint64_t)>()(ival,v);
            }
            break;
        }

        case value_types::double_t:
        {
            // float 64
            v.push_back(0xcb);
            to_big_endian<double,sizeof(double)>()(jval.as_double(),v);
            break;
        }

        case value_types::small_string_t:
        case value_types::string_t:
        {
            encode_string(jval.as_string_view(),v);
            break;
        }

        case value_types::array_t:
        {
            const auto length = jval.array_value().size();
            if (length <= 15)
            {
                // fixarray
                v.push_back(static_cast<uint8_t>(0x90 | length));
            }
            else if (length <= 0xffff)
            {
                // array 16
                v.push_back(0xdc);
                to_big_endian<uint64_t, sizeof(uint16_t)>()(static_cast<uint16_t>(length),v);
            }
            else if (length <= 0xffffffff)
            {
                // array 32
                v.push_back(0xdd);
                to_big_endian<uint64_t, sizeof(uint16_t)>()(length,v);
            }

            // append each element
            for (const auto& el : jval.array_range())
            {
                Encode_message_pack_(el, v);
            }
            break;
        }

        case value_types::object_t:
        {
            const auto length = jval.object_value().size();
            if (length <= 15)
            {
                // fixmap
                v.push_back(static_cast<uint8_t>(0x80 | (length & 0xf)));
            }
            else if (length <= 65535)
            {
                // map 16
                v.push_back(0xde);
                to_big_endian<uint64_t, sizeof(uint16_t)>()(static_cast<uint16_t>(length),v);
            }
            else if (length <= 4294967295)
            {
                // map 32
                v.push_back(0xdf);
                to_big_endian<uint64_t, sizeof(uint16_t)>()(length,v);
            }

            // append each element
            for (const auto& kvp: jval.object_range())
            {
                encode_string(kvp.key(),v);
                Encode_message_pack_(kvp.value(), v);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

template<class Json>
std::vector<uint8_t> encode_message_pack(const Json& jval)
{
    std::vector<uint8_t> v;
    Encode_message_pack_(jval,v);
    return v;
}

// decode_message_pack

template<class Json>
struct Decode_message_pack_
{
    std::vector<uint8_t>::const_iterator begin_;
    std::vector<uint8_t>::const_iterator end_;
    std::vector<uint8_t>::const_iterator it_;

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
                it_ += len; // skip content bytes
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
                case 0xc0: 
                {
                    // nil
                    return Json::null();
                }

                case 0xc2: 
                {
                    // false
                    return Json(false);
                }

                case 0xc3: 
                {
                    // true
                    return Json(true);
                }

                case 0xca: 
                {
                    // float 32
                    // copy bytes in reverse order into the double variable
                    float res;
                    for (size_t byte = 0; byte < sizeof(float); ++byte)
                    {
                        reinterpret_cast<uint8_t*>(&res)[sizeof(float) - byte - 1] = *(pos + 1 + byte);
                    }
                    it_ += sizeof(float); // skip content bytes
                    return res;
                }

                case 0xcb: 
                {
                    // float 64
                    // copy bytes in reverse order into the double variable
                    double res;
                    for (size_t byte = 0; byte < sizeof(double); ++byte)
                    {
                        reinterpret_cast<uint8_t*>(&res)[sizeof(double) - byte - 1] = *(pos + 1 + byte);
                    }
                    it_ += sizeof(double); // skip content bytes
                    return res;
                }

                case 0xcc: 
                {
                    // uint 8
                    it_ += 1; // skip content byte
                    return from_big_endian<uint8_t>(pos,end_);
                }

                case 0xcd: 
                {
                    // uint 16
                    it_ += 2; // skip 2 content bytes
                    return from_big_endian<uint16_t>(pos,end_);
                }

                case 0xce: 
                {
                    // uint 32
                    it_ += 4; // skip 4 content bytes
                    return from_big_endian<uint32_t>(pos,end_);
                }

                case 0xcf: 
                {
                    // uint 64
                    it_ += 8; // skip 8 content bytes
                    return from_big_endian<uint64_t>(pos,end_);
                }

                case 0xd0: 
                {
                    // int 8
                    it_ += 1; // skip content byte
                    return from_big_endian<int8_t>(pos,end_);
                }

                case 0xd1: 
                {
                    // int 16
                    it_ += 2; // skip 2 content bytes
                    return from_big_endian<int16_t>(pos,end_);
                }

                case 0xd2: 
                {
                    // int 32
                    it_ += 4; // skip 4 content bytes
                    return from_big_endian<int32_t>(pos,end_);
                }

                case 0xd3: 
                {
                    // int 64
                    it_ += 8; // skip 8 content bytes
                    return from_big_endian<int64_t>(pos,end_);
                }

                case 0xd9: 
                {
                    // str 8
                    const auto len = from_big_endian<uint8_t>(pos,end_);
                    auto offset = &(*(pos + 2));
                    it_ += len + 1; // skip size byte + content bytes
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case 0xda: 
                {
                    // str 16
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    auto offset = &(*(pos + 3));
                    it_ += len + 2; // skip 2 size bytes + content bytes
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case 0xdb: 
                {
                    // str 32
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    auto offset = &(*(pos + 5));
                    it_ += len + 4; // skip 4 size bytes + content bytes
                    return std::string(reinterpret_cast<const char*>(offset), len);
                }

                case 0xdc: 
                {
                    // array 16
                    Json result = typename Json::array();
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    it_ += 2; // skip 2 size bytes
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case 0xdd: 
                {
                    // array 32
                    Json result = typename Json::array();
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    it_ += 4; // skip 4 size bytes
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.add(decode());
                    }
                    return result;
                }

                case 0xde: 
                {
                    // map 16
                    Json result = typename Json::object();
                    const auto len = from_big_endian<uint16_t>(pos,end_);
                    it_ += 2; // skip 2 size bytes
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto j = decode();
                        result.set(j.as_string_view(),decode());
                    }
                    return result;
                }

                case 0xdf: 
                {
                    // map 32
                    Json result = typename Json::object();
                    const auto len = from_big_endian<uint32_t>(pos,end_);
                    it_ += 4; // skip 4 size bytes
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto key = decode().as_string_view();
                        result.set(key,decode());
                    }
                    return result;
                }

                default:
                {
                    throw std::invalid_argument("Error decoding a message pack at position " + std::to_string(end_-pos));
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
