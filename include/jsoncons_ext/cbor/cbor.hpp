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
#include <jsoncons/json_filter.hpp>
#include <jsoncons/binary/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>

namespace jsoncons { namespace cbor {

// decode_cbor

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
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    cbor_parser parser(decoder);
    parser.set_source(v.buffer(),v.buflen());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    cbor_parser parser(adaptor);
    parser.set_source(v.buffer(),v.buflen());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    std::vector<uint8_t> v;
    is.seekg(0, std::ios::end);   
    v.resize(is.tellg());
    is.seekg(0, std::ios::beg);    
    is.read((char_type*)&v[0],v.size());

    jsoncons::json_decoder<Json> decoder;
    cbor_parser parser(decoder);
    parser.set_source(v.data(),v.size());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    std::vector<uint8_t> v;
    is.seekg(0, std::ios::end);   
    v.resize(is.tellg());
    is.seekg(0, std::ios::beg);    
    is.read((char_type*)&v[0],v.size());

    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    cbor_parser parser(adaptor);
    parser.set_source(v.data(),v.size());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
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
