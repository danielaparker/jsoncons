// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_HPP
#define JSONCONS_MSGPACK_MSGPACK_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons/config/binary_utilities.hpp>
#include <jsoncons_ext/msgpack/msgpack_serializer.hpp>

namespace jsoncons { namespace msgpack {
  
class msgpack_decode_error : public std::invalid_argument, public virtual json_exception
{
public:
    explicit msgpack_decode_error(size_t pos) noexcept
        : std::invalid_argument("")
    {
        buffer_.append("Error decoding a message pack at position ");
        buffer_.append(std::to_string(pos));
    }
    ~msgpack_decode_error() noexcept
    {
    }
    const char* what() const noexcept override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

struct Encode_msgpack_
{
    template <typename T>
    void operator()(T val, std::vector<uint8_t>& v)
    {
        binary::to_big_endian(val, std::back_inserter(v));
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

// decode_msgpack

template<class Json>
class Decode_msgpack_
{
    const uint8_t* begin_;
    const uint8_t* end_;
    const uint8_t* it_;
public:
    typedef typename Json::char_type char_type;

    Decode_msgpack_(const uint8_t* begin, const uint8_t* end)
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
                    result.insert_or_assign(j.as_string_view(),decode());
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
                    result.push_back(decode());
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
                if (result.ec != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
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
                    const uint8_t* endp;
                    float res = binary::from_big_endian<float>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return res;
                }

                case msgpack_format::float64_cd: 
                {
                    const uint8_t* endp;
                    double res = binary::from_big_endian<double>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return res;
                }

                case msgpack_format::uint8_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint8_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return Json(x);
                }

                case msgpack_format::uint16_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint16_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::uint32_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint32_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::uint64_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint64_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::int8_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int8_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return Json(x);
                }

                case msgpack_format::int16_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int16_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::int32_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int32_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::int64_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int64_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    return x;
                }

                case msgpack_format::str8_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int8_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }

                    const uint8_t* first = &(*(pos + 2));
                    const uint8_t* last = first + len;
                    it_ += len; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    return target;
                }

                case msgpack_format::str16_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int16_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }

                    const uint8_t* first = &(*(pos + 3));
                    const uint8_t* last = first + len;
                    it_ += len; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    return target;
                }

                case msgpack_format::str32_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int32_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }

                    const uint8_t* first = &(*(pos + 5));
                    const uint8_t* last = first + len;
                    it_ += len; 

                    std::basic_string<char_type> target;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(target),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    return target;
                }

                case msgpack_format::array16_cd: 
                {
                    Json result = typename Json::array();
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint16_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.push_back(decode());
                    }
                    return result;
                }

                case msgpack_format::array32_cd: 
                {
                    Json result = typename Json::array();
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint32_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        result.push_back(decode());
                    }
                    return result;
                }

                case msgpack_format::map16_cd : 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint16_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    Json result = typename Json::object();
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto j = decode();
                        result.insert_or_assign(j.as_string_view(),decode());
                    }
                    return result;
                }

                case msgpack_format::map32_cd : 
                {
                    Json result = typename Json::object();
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint32_t>(it_,end_,&endp);
                    if (endp == it_)
                    {
                        JSONCONS_THROW(msgpack_decode_error(end_-it_));
                    }
                    else
                    {
                        it_ = endp;
                    }
                    result.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto key = decode().as_string_view();
                        result.insert_or_assign(key,decode());
                    }
                    return result;
                }

                default:
                {
                    JSONCONS_THROW(msgpack_decode_error(end_-pos));
                }
            }
        }
    }
};

// encode_msgpack

template<class Json>
void encode_msgpack(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_msgpack_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template<class Json>
void encode_msgpack(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_msgpack_serializer<char_type,jsoncons::detail::bytes_writer> serializer(v);
    j.dump(serializer);
}

// decode_msgpack

template<class Json>
Json decode_msgpack(const std::vector<uint8_t>& v)
{
    Decode_msgpack_<Json> decoder(v.data(),v.data()+v.size());
    return decoder.decode();
}
  
#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
std::vector<uint8_t> encode_msgpack(const Json& j)
{
    std::vector<uint8_t> v;
    encode_msgpack(j, v);
    return v;
}
#endif

}}

#endif
