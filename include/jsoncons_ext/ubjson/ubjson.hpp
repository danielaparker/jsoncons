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
#include <jsoncons_ext/ubjson/ubjson_serializer.hpp>
#include <jsoncons_ext/ubjson/ubjson_reader.hpp>

namespace jsoncons { namespace ubjson {

// encode_ubjson

template<class Json>
void encode_ubjson(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_ubjson_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template<class Json>
void encode_ubjson(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_ubjson_serializer<char_type,jsoncons::buffer_result> serializer(v);
    j.dump(serializer);
}

// decode_ubjson

template<class Json>
Json decode_ubjson(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<Json> decoder;
    ubjson_buffer_reader parser(v, decoder);
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
decode_ubjson(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    ubjson_reader parser(is, decoder);
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
decode_ubjson(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    ubjson_reader parser(is, adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

}}

#endif
