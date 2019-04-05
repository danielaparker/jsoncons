// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_HPP
#define JSONCONS_BSON_BSON_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/bson/bson_encoder.hpp>
#include <jsoncons_ext/bson/bson_reader.hpp>

namespace jsoncons { namespace bson {

// decode_bson

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,T>::type 
decode_bson(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    basic_bson_reader<jsoncons::bytes_source> reader(v, adaptor);
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
decode_bson(std::basic_istream<char>& is)
{
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
    bson_reader reader(is, adaptor);
    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        throw ser_error(ec,reader.line_number(),reader.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_bson(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    json_content_handler_adaptor<jsoncons::json_content_handler,jsoncons::json_decoder<Json>> adaptor(decoder);
    bson_reader parser(is, adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw ser_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

// encode_bson

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,void>::type 
encode_bson(const T& j, std::ostream& os)
{
    typedef typename T::char_type char_type;
    bson_encoder encoder(os);
    auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
    j.dump(adaptor);
}

template<class T>
typename std::enable_if<is_basic_json_class<T>::value,void>::type 
encode_bson(const T& j, std::vector<uint8_t>& v)
{
    typedef typename T::char_type char_type;
    bson_bytes_encoder encoder(v);
    auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
    j.dump(adaptor);
}
  
}}

#endif
