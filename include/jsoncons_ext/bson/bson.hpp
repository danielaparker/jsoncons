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
#include <jsoncons_ext/bson/bson_serializer.hpp>
#include <jsoncons_ext/bson/bson_reader.hpp>

namespace jsoncons { namespace bson {

// decode_bson

template<class Json>
Json decode_bson(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_bson_reader<jsoncons::buffer_source> parser{ v, decoder };
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_bson(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    bson_reader parser(is, decoder);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_bson(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    bson_reader parser(is, adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

// encode_bson

template<class Json>
void encode_bson(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_bson_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template<class Json>
void encode_bson(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_bson_serializer<char_type,jsoncons::buffer_result> serializer(v);
    j.dump(serializer);
}
  
}}

#endif
