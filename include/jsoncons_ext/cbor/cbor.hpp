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
#include <iterator>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/binary/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>

namespace jsoncons { namespace cbor {

// decode_cbor

template<class Json>
class Decode_cbor_
{
    const uint8_t* begin_input_;
    const uint8_t* end_input_;
    const uint8_t* input_ptr_;
public:
    typedef typename Json::char_type char_type;

    Decode_cbor_(const uint8_t* begin, const uint8_t* end)
        : begin_input_(begin), end_input_(end), input_ptr_(begin)
    {
    }

    Json decode()
    {
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
                    JSONCONS_THROW(cbor_decode_error(endp-begin_input_));
                }
                input_ptr_ = endp;
                return Json(val);
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
                    JSONCONS_THROW(cbor_decode_error(endp-begin_input_));
                }
                input_ptr_ = endp;
                return Json(val);
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
                    JSONCONS_THROW(cbor_decode_error(end_input_-pos));
                }
                else
                {
                    input_ptr_ = endp;
                }

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
                const uint8_t* endp;
                std::string s = detail::get_text_string(pos,end_input_,&endp);
                if (endp == pos)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-pos));
                }
                else
                {
                    input_ptr_ = endp;
                }
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
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint8_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_array(len);
            }

            // array (two-byte uint16_t for n follow)
        case 0x99:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_array(len);
            }

            // array (four-byte uint32_t for n follow)
        case 0x9a:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<int32_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_array(len);
            }

            // array (eight-byte uint64_t for n follow)
        case 0x9b:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<int64_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_array(len);
            }

            // array (indefinite length)
        case 0x9f:
            {
                Json result = typename Json::array();
                while (*input_ptr_ != 0xff)
                {
                    result.push_back(decode());
                    pos = input_ptr_;
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
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint8_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_map(len);
            }

            // map (two-byte uint16_t for n follow)
        case 0xb9:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_map(len);
            }

            // map (four-byte uint32_t for n follow)
        case 0xba:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_map(len);
            }

            // map (eight-byte uint64_t for n follow)
        case 0xbb:
            {
                const uint8_t* endp;
                const auto len = binary::from_big_endian<uint64_t>(input_ptr_,end_input_,&endp);
                if (endp == input_ptr_)
                {
                    JSONCONS_THROW(cbor_decode_error(end_input_-input_ptr_));
                }
                else
                {
                    input_ptr_ = endp;
                }
                return get_fixed_length_map(len);
            }

            // map (indefinite length)
        case 0xbf:
            {
                Json result = typename Json::object();
                while (*input_ptr_ != 0xff)
                {
                    auto j = decode();
                    result.set(j.as_string_view(),decode());
                    pos = input_ptr_;
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
                    JSONCONS_THROW(cbor_decode_error(endp-begin_input_));
                }
                input_ptr_ = endp;
                return Json(val);
            }

        default:
            {
                JSONCONS_THROW(cbor_decode_error(end_input_-pos));
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
void encode_cbor(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_cbor_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template<class Json>
void encode_cbor(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_cbor_serializer<char_type,jsoncons::detail::bytes_writer<uint8_t>> serializer(v);
    j.dump(serializer);
}

template<class Json>
Json decode_cbor(const cbor_view& v)
{
    Decode_cbor_<Json> decoder(v.buffer(),v.buffer()+v.buflen());
    return decoder.decode();
}

template<class Json>
Json decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    std::vector<uint8_t> v;
    is.seekg(0, std::ios::end);   
    v.resize(is.tellg());
    is.seekg(0, std::ios::beg);    
    is.read((char_type*)&v[0],v.size());

    Decode_cbor_<Json> decoder(v.data(),v.data()+v.size());
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
