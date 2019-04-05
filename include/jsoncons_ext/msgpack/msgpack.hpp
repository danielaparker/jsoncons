// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_HPP
#define JSONCONS_MSGPACK_MSGPACK_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/msgpack/msgpack_encoder.hpp>
#include <jsoncons_ext/msgpack/msgpack_reader.hpp>

namespace jsoncons { namespace msgpack {

// encode_msgpack

template<class Json>
void encode_msgpack(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_msgpack_encoder<char_type> encoder(os);
    j.dump(encoder);
}

template<class Json>
void encode_msgpack(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_msgpack_encoder<char_type,jsoncons::bytes_result> encoder(v);
    j.dump(encoder);
}

// decode_msgpack

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,T>::type 
decode_msgpack(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    basic_msgpack_reader<jsoncons::bytes_source> reader(v, adaptor);
    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        throw ser_error(ec,reader.line_number(),reader.column_number());
    }
    return decoder.get_result();
}

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,T>::type 
decode_msgpack(std::basic_istream<char>& is)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    msgpack_reader reader(is, adaptor);
    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        throw ser_error(ec,reader.line_number(),reader.column_number());
    }
    return decoder.get_result();
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
