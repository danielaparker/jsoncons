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
#include <jsoncons/json.hpp>

namespace jsoncons { namespace message_pack {

template<class Json>
Json decode_message_pack(std::vector<uint8_t>)
{
}

template<class Json>
void encode_message_pack(const Json& val, std::vector<uint8_t>& v)
{
    switch (val.type_id())
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
            v.push_back(val.as_bool() ? 0xc3 : 0xc2);
            break;
        }

        case value_types::integer_t:
        {
            if (val.m_value.number_integer >= 0)
            {
                if (val.as_integer() < 128)
                {
                    // positive fixnum
                    add_to_vector(v, 1, val.as_integer());
                }
                else if (val.as_integer() <= (std::numeric_limits<uint8_t>::max)())
                {
                    // uint 8
                    v.push_back(0xcc);
                    add_to_vector(v, 1, val.as_integer());
                }
                else if (val.as_integer() <= (std::numeric_limits<uint16_t>::max)())
                {
                    // uint 16
                    v.push_back(0xcd);
                    add_to_vector(v, 2, val.as_integer());
                }
                else if (val.as_integer() <= (std::numeric_limits<uint32_t>::max)())
                {
                    // uint 32
                    v.push_back(0xce);
                    add_to_vector(v, 4, val.as_integer());
                }
                else if (val.as_integer() <= (std::numeric_limits<uint64_t>::max)())
                {
                    // uint 64
                    v.push_back(0xcf);
                    add_to_vector(v, 8, val.as_integer());
                }
            }
            else
            {
                if (val.as_integer() >= -32)
                {
                    // negative fixnum
                    add_to_vector(v, 1, val.as_integer());
                }
                else if (val.as_integer() >= (std::numeric_limits<int8_t>::min)() and val.as_integer() <= (std::numeric_limits<int8_t>::max)())
                {
                    // int 8
                    v.push_back(0xd0);
                    add_to_vector(v, 1, val.as_integer());
                }
                else if (val.as_integer() >= (std::numeric_limits<int16_t>::min)() and val.as_integer() <= (std::numeric_limits<int16_t>::max)())
                {
                    // int 16
                    v.push_back(0xd1);
                    add_to_vector(v, 2, val.as_integer());
                }
                else if (val.as_integer() >= (std::numeric_limits<int32_t>::min)() and val.as_integer() <= INT32_MAX)
                {
                    // int 32
                    v.push_back(0xd2);
                    add_to_vector(v, 4, val.as_integer());
                }
                else if (val.as_integer() >= (std::numeric_limits<int64_t>::min)() and val.as_integer() <= (std::numeric_limits<int64_t>::max)())
                {
                    // int 64
                    v.push_back(0xd3);
                    add_to_vector(v, 8, val.as_integer());
                }
            }
            break;
        }

    case value_types::uinteger_t:
        {
            if (val.as_uinteger() < 128)
            {
                // positive fixnum
                add_to_vector(v, 1, val.as_uinteger());
            }
            else if (val.as_uinteger() <= (std::numeric_limits<uint8_t>::max)())
            {
                // uint 8
                v.push_back(0xcc);
                add_to_vector(v, 1, val.as_uinteger());
            }
            else if (val.as_uinteger() <= (std::numeric_limits<uint16_t>::max)())
            {
                // uint 16
                v.push_back(0xcd);
                add_to_vector(v, 2, val.as_uinteger());
            }
            else if (val.as_uinteger() <= (std::numeric_limits<uint32_t>::max)())
            {
                // uint 32
                v.push_back(0xce);
                add_to_vector(v, 4, val.as_uinteger());
            }
            else if (val.as_uinteger() <= (std::numeric_limits<uint64_t>::max)())
            {
                // uint 64
                v.push_back(0xcf);
                add_to_vector(v, 8, val.as_uinteger());
            }
            break;
        }

        case value_types::double_t:
        {
            // float 64
            v.push_back(0xcb);
            const uint8_t* helper = reinterpret_cast<const uint8_t*>(&(val.m_value.number_float));
            for (size_t i = 0; i < 8; ++i)
            {
                v.push_back(helper[7 - i]);
            }
            break;
        }

        case value_types::string_t:
        {
            const auto N = val.m_value.string->size();
            if (N <= 31)
            {
                // fixstr
                v.push_back(static_cast<uint8_t>(0xa0 | N));
            }
            else if (N <= 255)
            {
                // str 8
                v.push_back(0xd9);
                add_to_vector(v, 1, N);
            }
            else if (N <= 65535)
            {
                // str 16
                v.push_back(0xda);
                add_to_vector(v, 2, N);
            }
            else if (N <= 4294967295)
            {
                // str 32
                v.push_back(0xdb);
                add_to_vector(v, 4, N);
            }

            // append string
            std::copy(val.m_value.string->begin(), val.m_value.string->end(),
                      std::back_inserter(v));
            break;
        }

        case value_types::array_t:
        {
            const auto N = val.m_value.array->size();
            if (N <= 15)
            {
                // fixarray
                v.push_back(static_cast<uint8_t>(0x90 | N));
            }
            else if (N <= 0xffff)
            {
                // array 16
                v.push_back(0xdc);
                add_to_vector(v, 2, N);
            }
            else if (N <= 0xffffffff)
            {
                // array 32
                v.push_back(0xdd);
                add_to_vector(v, 4, N);
            }

            // append each element
            for (const auto& el : *val.m_value.array)
            {
                to_msgpack_internal(el, v);
            }
            break;
        }

        case value_types::object_t:
        {
            const auto N = val.m_value.object->size();
            if (N <= 15)
            {
                // fixmap
                v.push_back(static_cast<uint8_t>(0x80 | (N & 0xf)));
            }
            else if (N <= 65535)
            {
                // map 16
                v.push_back(0xde);
                add_to_vector(v, 2, N);
            }
            else if (N <= 4294967295)
            {
                // map 32
                v.push_back(0xdf);
                add_to_vector(v, 4, N);
            }

            // append each element
            for (const auto& el : *val.m_value.object)
            {
                to_msgpack_internal(el.first, v);
                to_msgpack_internal(el.second, v);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

}}

#endif
