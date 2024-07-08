// Copyright 2013-2024 Daniel Parker
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
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/ubjson/ubjson_type.hpp>
#include <jsoncons_ext/ubjson/ubjson_error.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

namespace jsoncons { namespace ubjson {

template <typename Source,typename Allocator=std::allocator<char>>
class basic_ubjson_reader
{
    basic_ubjson_parser<Source,Allocator> parser_;
    json_visitor& visitor_;
public:
    template <typename Sourceable>
    basic_ubjson_reader(Sourceable&& source, 
                      json_visitor& visitor, 
                      const Allocator& alloc)
       : basic_ubjson_reader(std::forward<Sourceable>(source),
                           visitor,
                           ubjson_decode_options(),
                           alloc)
    {
    }

    template <typename Sourceable>
    basic_ubjson_reader(Sourceable&& source, 
                      json_visitor& visitor, 
                      const ubjson_decode_options& options = ubjson_decode_options(),
                      const Allocator& alloc=Allocator())
       : parser_(std::forward<Sourceable>(source), options, alloc),
         visitor_(visitor)
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
        parser_.parse(visitor_, ec);
        if (ec)
        {
            return;
        }
    }

    std::size_t line() const 
    {
        return parser_.line();
    }

    std::size_t column() const
    {
        return parser_.column();
    }
};

using ubjson_stream_reader = basic_ubjson_reader<jsoncons::binary_stream_source>;

using ubjson_bytes_reader = basic_ubjson_reader<jsoncons::bytes_source>;

}}

#endif
