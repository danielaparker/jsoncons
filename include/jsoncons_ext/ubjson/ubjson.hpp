// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_HPP
#define JSONCONS_UBJSON_UBJSON_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_encoder.hpp>
#include <jsoncons_ext/ubjson/ubjson_reader.hpp>

namespace jsoncons { namespace ubjson {

// encode_ubjson

template<class Json>
void encode_ubjson(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_ubjson_encoder<char_type> encoder(os);
    j.dump(encoder);
}

template<class Json>
void encode_ubjson(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_ubjson_encoder<char_type,jsoncons::bytes_result> encoder(v);
    j.dump(encoder);
}

// decode_ubjson

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,T>::type 
decode_ubjson(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    basic_ubjson_reader<jsoncons::bytes_source> reader(v, adaptor);
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
decode_ubjson(std::basic_istream<char>& is)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    ubjson_reader reader(is, adaptor);
    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        throw ser_error(ec,reader.line_number(),reader.column_number());
    }
    return decoder.get_result();
}

}}

#endif
