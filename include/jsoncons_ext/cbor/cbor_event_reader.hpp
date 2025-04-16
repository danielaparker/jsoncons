// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_EVENT_READER_HPP
#define JSONCONS_EXT_CBOR_EVENT_READER_HPP

#include <cstddef>
#include <functional>
#include <ios>
#include <memory> // std::allocator
#include <system_error>

#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/item_event_visitor.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/ser_context.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/staj_event_reader.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { 
namespace cbor {

    template <typename Source=jsoncons::binary_stream_source,typename Allocator=std::allocator<char>>
    class cbor_event_reader : public basic_staj_event_reader<char>, private virtual ser_context
    {
    public:
        using source_type = Source;
        using char_type = char;
        using allocator_type = Allocator;
    private:
        basic_cbor_parser<Source,Allocator> parser_;
        basic_item_event_receiver<char_type> cursor_visitor_;
        bool eof_{false};

    public:
        using string_view_type = string_view;

        // Noncopyable and nonmoveable
        cbor_event_reader(const cbor_event_reader&) = delete;
        cbor_event_reader(cbor_event_reader&&) = delete;

        template <typename Sourceable>
        cbor_event_reader(Sourceable&& source,
            const cbor_decode_options& options = cbor_decode_options(),
            const Allocator& alloc = Allocator())
            : parser_(std::forward<Sourceable>(source), options, alloc)
        {
            parser_.cursor_mode(true);
            if (!done())
            {
                next();
            }
        }

        // Constructors that set parse error codes

        template <typename Sourceable>
        cbor_event_reader(Sourceable&& source, std::error_code& ec)
            : cbor_event_reader(std::allocator_arg, Allocator(),
                  std::forward<Sourceable>(source), 
                  cbor_decode_options(), 
                  ec)
        {
        }

        template <typename Sourceable>
        cbor_event_reader(Sourceable&& source, 
            const cbor_decode_options& options,
            std::error_code& ec)
            : cbor_event_reader(std::allocator_arg, Allocator(),
                  std::forward<Sourceable>(source), 
                  options, 
                  ec)
        {
        }

        template <typename Sourceable>
        cbor_event_reader(std::allocator_arg_t, const Allocator& alloc, 
            Sourceable&& source,
            const cbor_decode_options& options,
            std::error_code& ec)
           : parser_(std::forward<Sourceable>(source), options, alloc),
             eof_(false)
        {
            parser_.cursor_mode(true);
            if (!done())
            {
                next(ec);
            }
        }
        
        ~cbor_event_reader() = default;

        cbor_event_reader& operator=(const cbor_event_reader&) = delete;
        cbor_event_reader& operator=(cbor_event_reader&&) = delete;

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

        template <typename Sourceable>
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

        template <typename Sourceable>
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

        const basic_staj_event<char_type>& current() const override
        {
            return cursor_visitor_.event();
        }

        void read_to(basic_item_event_visitor<char_type>& visitor) override
        {
            std::error_code ec;
            read_to(visitor, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
            }
        }

        void read_to(basic_item_event_visitor<char_type>& visitor, 
            std::error_code& ec) override
        {
            if (is_typed_array())
            {
                cursor_visitor_.dump(visitor, *this, ec);
            }
            else if (is_begin_container(current().event_type()))
            {
                parser_.cursor_mode(false);
                parser_.mark_level(parser_.level());
                cursor_visitor_.dump(visitor, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                read_next(visitor, ec);
                parser_.cursor_mode(true);
                parser_.mark_level(0);
                if (current().event_type() == staj_event_type::begin_object)
                {
                    cursor_visitor_.end_object(*this);
                }
                else
                {
                    cursor_visitor_.end_array(*this);
                }
            }
            else
            {
                cursor_visitor_.dump(visitor, *this, ec);
            }
        }

        void next() override
        {
            std::error_code ec;
            next(ec);
            if (JSONCONS_UNLIKELY(ec))
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
        staj2_filter_view operator|(cbor_event_reader& cursor, 
            std::function<bool(const item_event&, const ser_context&)> pred)
        {
            return staj2_filter_view(cursor, pred);
        }

    private:
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
                    if (JSONCONS_UNLIKELY(ec)) {return;}
                }
            }
        }

        void read_next(basic_item_event_visitor<char_type>& visitor, std::error_code& ec)
        {
            parser_.restart();
            while (!parser_.stopped())
            {
                parser_.parse(visitor, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
            }
        }
    };

} // namespace cbor
} // namespace jsoncons

#endif

