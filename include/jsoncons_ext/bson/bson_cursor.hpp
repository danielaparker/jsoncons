// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_CURSOR_HPP
#define JSONCONS_BSON_BSON_CURSOR_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <istream> // std::basic_istream
#include <jsoncons/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons_ext/bson/bson_parser.hpp>

namespace jsoncons { 
namespace bson {

template<class Src=jsoncons::binary_stream_source,class Allocator=std::allocator<char>>
class basic_bson_cursor : public basic_staj_reader<char>, private virtual ser_context
{
public:
    using source_type = Src;
    using char_type = char;
    using allocator_type = Allocator;
private:
    basic_bson_parser<Src,Allocator> parser_;
    basic_staj_visitor<char_type> event_handler_;
    bool eof_;

    // Noncopyable and nonmoveable
    basic_bson_cursor(const basic_bson_cursor&) = delete;
    basic_bson_cursor& operator=(const basic_bson_cursor&) = delete;

public:
    using string_view_type = string_view;

    template <class Source>
    basic_bson_cursor(Source&& source,
                      const Allocator& alloc = Allocator())
       : basic_bson_cursor(std::forward<Source>(source), 
                           accept_all,
                           bson_decode_options(),
                           alloc)
    {
    }

    template <class Source>
    basic_bson_cursor(Source&& source,
                      const bson_decode_options& options,
                      const Allocator& alloc = Allocator())
       : basic_bson_cursor(std::forward<Source>(source), 
                           accept_all,
                           options,
                           alloc)
    {
    }

    template <class Source>
    basic_bson_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      const bson_decode_options& options = bson_decode_options(),
                      const Allocator& alloc = Allocator())
       : parser_(std::forward<Source>(source), options, alloc), 
         event_handler_(filter), 
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    // Constructors that set parse error codes

    template <class Source>
    basic_bson_cursor(Source&& source,
                      std::error_code& ec)
        : basic_bson_cursor(std::allocator_arg, Allocator(),
                            std::forward<Source>(source), accept_all, ec)
    {
    }

    template <class Source>
    basic_bson_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      std::error_code& ec)
       : basic_bson_cursor(std::allocator_arg, Allocator(), 
                           std::forward<Source>(source), filter, ec)
    {
    }

    template <class Source>
    basic_bson_cursor(std::allocator_arg_t, const Allocator& alloc, 
                      Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter, 
                      std::error_code& ec)
       : parser_(std::forward<Source>(source),alloc), 
         event_handler_(filter),
         eof_(false)
    {
        if (!done())
        {
            next(ec);
        }
    }

    bool done() const override
    {
        return parser_.done();
    }

    const basic_staj_event<char_type>& current() const override
    {
        return event_handler_.event();
    }

    void read(basic_json_visitor<char_type>& visitor) override
    {
        std::error_code ec;
        read(visitor, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read(basic_json_visitor<char_type>& visitor,
                std::error_code& ec) override
    {
        if (!staj_to_saj_event(event_handler_.event(), visitor, *this, ec))
        {
            return;
        }
        read_next(visitor, ec);
    }

    void next() override
    {
        std::error_code ec;
        next(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void next(std::error_code& ec) override
    {
        read_next(ec);
    }

    void read_next(std::error_code& ec)
    {
        read_next(event_handler_, ec);
    }

    void read_next(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            parser_.parse(visitor, ec);
            if (ec) return;
        }
    }

    const ser_context& context() const override
    {
        return *this;
    }

    bool eof() const
    {
        return eof_;
    }

    std::size_t line() const override
    {
        return parser_.line();
    }

    std::size_t column() const override
    {
        return parser_.column();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use read(basic_json_visitor<char_type>&)")
    void read_to(basic_json_visitor<char_type>& visitor)
    {
        read(visitor);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read(basic_json_visitor<char_type>&, std::error_code&)")
    void read_to(basic_json_visitor<char_type>& visitor,
                 std::error_code& ec) 
    {
        read(visitor, ec);
    }
#endif
private:
    static bool accept_all(const basic_staj_event<char_type>&, const ser_context&) 
    {
        return true;
    }
};

using bson_stream_cursor = basic_bson_cursor<jsoncons::binary_stream_source>;
using bson_bytes_cursor = basic_bson_cursor<jsoncons::bytes_source>;

} // namespace bson
} // namespace jsoncons

#endif

