// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_READER_HPP
#define JSONCONS_BSON_BSON_READER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/bson/bson_detail.hpp>
#include <jsoncons_ext/bson/bson_error.hpp>
#include <jsoncons_ext/bson/bson_parser.hpp>

namespace jsoncons { namespace bson {

template <class Src>
class basic_bson_reader : public ser_context
{
    basic_bson_parser<Src> parser_;
    json_content_handler& handler_;
public:
    template <class Source>
    basic_bson_reader(Source&& source, json_content_handler& handler)
       : parser_(std::forward<Source>(source)),
         handler_(handler)
    {
    }

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            throw ser_error(ec,line(),column());
        }
    }

    void read(std::error_code& ec)
    {
        try
        {
            parser_.reset();
            parser_.parse(handler_, ec);
            if (ec)
            {
                return;
            }
        }
        catch (const ser_error& e)
        {
            ec = e.code();
        }
    }

    size_t line() const override
    {
        return parser_.line();
    }

    size_t column() const override
    {
        return parser_.column();
    }
};

typedef basic_bson_reader<jsoncons::binary_stream_source> bson_stream_reader;
typedef basic_bson_reader<jsoncons::bytes_source> bson_bytes_reader;

#if !defined(JSONCONS_NO_DEPRECATED) 
JSONCONS_DEPRECATED_MSG("Instead, use bson_stream_reader") typedef bson_stream_reader bson_reader;
JSONCONS_DEPRECATED_MSG("Instead, use bson_bytes_reader") typedef bson_bytes_reader bson_buffer_reader;
#endif

}}

#endif
