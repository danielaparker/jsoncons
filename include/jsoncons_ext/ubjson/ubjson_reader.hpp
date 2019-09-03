// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_READER_HPP
#define JSONCONS_UBJSON_UBJSON_READER_HPP

#include <string>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_error.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

namespace jsoncons { namespace ubjson {

template <class Src>
class basic_ubjson_reader : public ser_context
{
    basic_ubjson_parser<Src> parser_;
    json_content_handler& handler_;
public:
    template <class Source>
    basic_ubjson_reader(Source&& source, json_content_handler& handler)
        : parser_(std::forward<Source>(source)), handler_(handler)
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

typedef basic_ubjson_reader<jsoncons::binary_stream_source> ubjson_stream_reader;

typedef basic_ubjson_reader<jsoncons::bytes_source> ubjson_bytes_reader;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use ubjson_stream_reader") typedef ubjson_stream_reader ubjson_reader;
JSONCONS_DEPRECATED_MSG("Instead, use ubjson_bytes_reader") typedef ubjson_bytes_reader ubjson_buffer_reader;
#endif

}}

#endif
