// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_READER_HPP
#define JSONCONS_CBOR_CBOR_READER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { namespace cbor {

template <class Src,class Float128T = std::nullptr_t,class WorkAllocator=std::allocator<char>>
class basic_cbor_reader : public ser_context
{
    basic_cbor_parser<Src,Float128T,WorkAllocator> parser_;
    cbor_to_json_content_handler_adaptor<Float128T>& handler_;
public:
    template <class Source>
    basic_cbor_reader(Source&& source, 
                      cbor_to_json_content_handler_adaptor<Float128T>& handler,
                      const WorkAllocator allocator=WorkAllocator())
       : parser_(std::forward<Source>(source),allocator),
         handler_(handler)
    {
    }

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,line(),column()));
        }
    }

    void read(std::error_code& ec)
    {
        parser_.reset();
        parser_.parse(handler_, ec);
        if (ec)
        {
            return;
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

typedef basic_cbor_reader<jsoncons::binary_stream_source> cbor_stream_reader;

typedef basic_cbor_reader<jsoncons::bytes_source> cbor_bytes_reader;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use cbor_stream_reader") typedef cbor_stream_reader cbor_reader;
JSONCONS_DEPRECATED_MSG("Instead, use cbor_bytes_reader") typedef cbor_bytes_reader cbor_buffer_reader;
#endif

}}

#endif
