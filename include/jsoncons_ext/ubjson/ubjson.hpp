// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_HPP
#define JSONCONS_UBJSON_UBJSON_HPP

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
#include <jsoncons_ext/ubjson/ubjson_serializer.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

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
    basic_ubjson_serializer<char_type,jsoncons::detail::bytes_writer> serializer(v);
    j.dump(serializer);
}

// decode_ubjson

template<class Json>
Json decode_ubjson(const std::vector<uint8_t>& v)
{
    jsoncons::json_decoder<Json> decoder;
    ubjson_parser parser(decoder);
    parser.update(v.data(),v.size());
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
std::vector<uint8_t> encode_ubjson(const Json& j)
{
    std::vector<uint8_t> v;
    encode_ubjson(j, v);
    return v;
}
#endif

}}

#endif
