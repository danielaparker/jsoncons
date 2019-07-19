// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_CURSOR_HPP
#define JSONCONS_UBJSON_UBJSON_CURSOR_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <istream> // std::basic_istream
#include <jsoncons/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

namespace jsoncons { 
namespace ubjson {

template<class Src=jsoncons::binary_stream_source,class Allocator=std::allocator<char>>
class basic_ubjson_cursor : public basic_staj_reader<char>, private virtual ser_context
{
public:
    typedef Allocator allocator_type;
private:
    basic_staj_event_handler<char> event_handler_;

    basic_ubjson_parser<Src> parser_;
    bool eof_;

    // Noncopyable and nonmoveable
    basic_ubjson_cursor(const basic_ubjson_cursor&) = delete;
    basic_ubjson_cursor& operator=(const basic_ubjson_cursor&) = delete;

public:
    typedef string_view string_view_type;

    template <class Source>
    basic_ubjson_cursor(Source&& source)
       : parser_(std::forward<Source>(source)),
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    template <class Source>
    basic_ubjson_cursor(Source&& source,
                        std::function<bool(const staj_event&, const ser_context&)> filter)
       : parser_(std::forward<Source>(source)), event_handler_(filter),
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    // Constructors that set parse error codes

    template <class Source>
    basic_ubjson_cursor(Source&& source, 
                      std::error_code& ec)
       : parser_(std::forward<Source>(source)),
         eof_(false)
    {
        if (!done())
        {
            next(ec);
        }
    }

    template <class Source>
    basic_ubjson_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter, 
                      std::error_code& ec)
       : parser_(std::forward<Source>(source)), event_handler_(filter),
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

    const basic_staj_event<char>& current() const override
    {
        return event_handler_.event();
    }

    void read_to(basic_json_content_handler<char>& handler) override
    {
        std::error_code ec;
        read_to(handler, ec);
        if (ec)
        {
            throw ser_error(ec,parser_.line(),parser_.column());
        }
    }

    void read_to(basic_json_content_handler<char>& handler,
                std::error_code& ec) override
    {
        if (!staj_to_saj_event(event_handler_.event(), handler, *this))
        {
            return;
        }
        read_next(handler, ec);
    }

    void next() override
    {
        std::error_code ec;
        next(ec);
        if (ec)
        {
            throw ser_error(ec,parser_.line(),parser_.column());
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

    void read_next(basic_json_content_handler<char>& handler, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            parser_.parse(handler, ec);
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

    size_t line() const override
    {
        return parser_.line();
    }

    size_t column() const override
    {
        return parser_.column();
    }
private:
};

typedef basic_ubjson_cursor<jsoncons::binary_stream_source> ubjson_stream_cursor;
typedef basic_ubjson_cursor<jsoncons::bytes_source> ubjson_bytes_cursor;

} // namespace ubjson
} // namespace jsoncons

#endif

