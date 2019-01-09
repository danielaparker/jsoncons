// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_HPP
#define JSONCONS_CBOR_CBOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>

#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons_ext/cbor/cbor_view.hpp>
#endif

namespace jsoncons { namespace cbor {

// encode_cbor

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
    basic_cbor_serializer<char_type,jsoncons::buffer_result> serializer(v);
    j.dump(serializer);
}

// decode_cbor

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_cbor_reader<jsoncons::buffer_source> parser(jsoncons::buffer_source(v.data(),v.size()), decoder);
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
decode_cbor(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    basic_cbor_reader<jsoncons::buffer_source> parser(jsoncons::buffer_source(v.data(),v.size()), adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_cbor_reader<jsoncons::buffer_source> parser(jsoncons::buffer_source(v.buffer(),v.buflen()), decoder);
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
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    basic_cbor_reader<jsoncons::buffer_source> parser(jsoncons::buffer_source(v.buffer(),v.buflen()), adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}
#endif

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    cbor_reader parser(is, decoder);
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
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    //typedef typename Json::char_type char_type;

    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    cbor_reader parser(is, adaptor);
    std::error_code ec;
    parser.read(ec);
    if (ec)
    {
        throw serialization_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
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
