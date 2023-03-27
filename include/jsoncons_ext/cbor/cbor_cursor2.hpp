// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_CURSOR2_HPP
#define JSONCONS_CBOR_CBOR_CURSOR2_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <istream> // std::basic_istream
#include <jsoncons/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/item_event_visitor.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj2_cursor.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { 
namespace cbor {

    template<class Source=jsoncons::binary_stream_source,class Allocator=std::allocator<char>>
    class basic_cbor_cursor2 : public basic_staj2_cursor<char>, private virtual ser_context
    {
    public:
        using source_type = Source;
        using char_type = char;
        using allocator_type = Allocator;
    private:
        basic_cbor_parser<Source,Allocator> parser_;
        basic_staj2_visitor<char_type> cursor_visitor_;
        bool eof_;

        // Noncopyable and nonmoveable
        basic_cbor_cursor2(const basic_cbor_cursor2&) = delete;
        basic_cbor_cursor2& operator=(const basic_cbor_cursor2&) = delete;

    public:
        using string_view_type = string_view;

        template <class Sourceable>
        basic_cbor_cursor2(Sourceable&& source,
                          const cbor_decode_options& options = cbor_decode_options(),
                          const Allocator& alloc = Allocator())
            : parser_(std::forward<Sourceable>(source), options, alloc), 
              cursor_visitor_(accept_all), 
              eof_(false)
        {
            if (!done())
            {
                next();
            }
        }

        // Constructors that set parse error codes

        template <class Sourceable>
        basic_cbor_cursor2(Sourceable&& source, 
                          std::error_code& ec)
            : basic_cbor_cursor2(std::allocator_arg, Allocator(),
                                std::forward<Sourceable>(source), 
                                cbor_decode_options(), 
                                ec)
        {
        }

        template <class Sourceable>
        basic_cbor_cursor2(Sourceable&& source, 
                          const cbor_decode_options& options,
                          std::error_code& ec)
            : basic_cbor_cursor2(std::allocator_arg, Allocator(),
                                std::forward<Sourceable>(source), 
                                options, 
                                ec)
        {
        }

        template <class Sourceable>
        basic_cbor_cursor2(std::allocator_arg_t, const Allocator& alloc, 
                          Sourceable&& source,
                          const cbor_decode_options& options,
                          std::error_code& ec)
           : parser_(std::forward<Sourceable>(source), options, alloc), 
             cursor_visitor_(accept_all),
             eof_(false)
        {
            if (!done())
            {
                next(ec);
            }
        }

        void reset()
        {
            parser_.reset();
            cursor_visitor_.reset();
            eof_ = false;
            if (!done())
            {
                next();
            }
        }

        template <class Sourceable>
        void reset(Sourceable&& source)
        {
            parser_.reset(std::forward<Sourceable>(source));
            cursor_visitor_.reset();
            eof_ = false;
            if (!done())
            {
                next();
            }
        }

        void reset(std::error_code& ec)
        {
            parser_.reset();
            cursor_visitor_.reset();
            eof_ = false;
            if (!done())
            {
                next(ec);
            }
        }

        template <class Sourceable>
        void reset(Sourceable&& source, std::error_code& ec)
        {
            parser_.reset(std::forward<Sourceable>(source));
            cursor_visitor_.reset();
            eof_ = false;
            if (!done())
            {
                next(ec);
            }
        }

        bool done() const override
        {
            return parser_.done();
        }

        bool is_typed_array() const
        {
            return cursor_visitor_.is_typed_array();
        }

        const staj2_event& current() const override
        {
            return cursor_visitor_.event();
        }

        void read_to(basic_item_event_visitor<char_type>& visitor) override
        {
            std::error_code ec;
            read_to(visitor, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
            }
        }

        void read_to(basic_item_event_visitor<char_type>& visitor,
                     std::error_code& ec) override
        {
            if (cursor_visitor_.dump(visitor, *this, ec))
            {
                read_next(visitor, ec);
            }
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

        friend
        staj2_filter_view operator|(basic_cbor_cursor2& cursor, 
                                   std::function<bool(const staj2_event&, const ser_context&)> pred)
        {
            return staj2_filter_view(cursor, pred);
        }

    private:
        static bool accept_all(const staj2_event&, const ser_context&) 
        {
            return true;
        }

        void read_next(std::error_code& ec)
        {
            if (cursor_visitor_.in_available())
            {
                cursor_visitor_.send_available(ec);
            }
            else
            {
                parser_.restart();
                while (!parser_.stopped())
                {
                    parser_.parse(cursor_visitor_, ec);
                    if (ec) return;
                }
            }
        }

        void read_next(basic_item_event_visitor<char_type>& visitor, std::error_code& ec)
        {
            parser_.restart();
            while (!parser_.stopped())
            {
                parser_.parse(visitor, ec);
                if (ec)
                {
                    return;
                }
            }
        }
    };

    using cbor_stream_cursor2 = basic_cbor_cursor2<jsoncons::binary_stream_source>;
    using cbor_bytes_cursor2 = basic_cbor_cursor2<jsoncons::bytes_source>;

} // namespace cbor
} // namespace jsoncons

#endif

