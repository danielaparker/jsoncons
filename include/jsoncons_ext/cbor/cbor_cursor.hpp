// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_CURSOR_HPP
#define JSONCONS_EXT_CBOR_CBOR_CURSOR_HPP

#include <cstddef>
#include <functional>
#include <ios>
#include <memory> // std::allocator
#include <system_error>

#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { 
namespace cbor {

template <typename Source=jsoncons::binary_stream_source,typename Allocator=std::allocator<char>>
class basic_cbor_cursor : public basic_staj_cursor<char>, private virtual ser_context
{
public:
    using source_type = Source;
    using char_type = char;
    using allocator_type = Allocator;
private:
    basic_cbor_parser<Source,Allocator> parser_;
    basic_staj_visitor<char_type> cursor_visitor_;
    basic_item_event_visitor_to_json_visitor<char_type,Allocator> cursor_handler_adaptor_;
    bool eof_{false};

public:
    using string_view_type = string_view;

    // Noncopyable and nonmoveable
    basic_cbor_cursor(const basic_cbor_cursor&) = delete;
    basic_cbor_cursor(basic_cbor_cursor&&) = delete;

    template <typename Sourceable>
    basic_cbor_cursor(Sourceable&& source,
        const cbor_decode_options& options = cbor_decode_options(),
        const Allocator& alloc = Allocator())
        : parser_(std::forward<Sourceable>(source), options, alloc), 
          cursor_handler_adaptor_(cursor_visitor_, alloc)
    {
        parser_.cursor_mode(true);
        if (!read_done())
        {
            read_next();
        }
    }

    // Constructors that set parse error codes

    template <typename Sourceable>
    basic_cbor_cursor(Sourceable&& source, 
        std::error_code& ec)
        : basic_cbor_cursor(std::allocator_arg, Allocator(),
              std::forward<Sourceable>(source), 
              cbor_decode_options(), 
              ec)
    {
    }

    template <typename Sourceable>
    basic_cbor_cursor(Sourceable&& source, 
        const cbor_decode_options& options,
        std::error_code& ec)
        : basic_cbor_cursor(std::allocator_arg, Allocator(),
              std::forward<Sourceable>(source), 
              options, 
              ec)
    {
    }

    template <typename Sourceable>
    basic_cbor_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source,
        const cbor_decode_options& options,
        std::error_code& ec)
       : parser_(std::forward<Sourceable>(source), options, alloc), 
         cursor_handler_adaptor_(cursor_visitor_, alloc),
         eof_(false)
    {
        parser_.cursor_mode(true);
        if (!read_done())
        {
            read_next(ec);
        }
    }

    ~basic_cbor_cursor() = default;
    
    basic_cbor_cursor& operator=(const basic_cbor_cursor&) = delete;
    basic_cbor_cursor& operator=(basic_cbor_cursor&&) = delete;

    void reset()
    {
        parser_.reset();
        cursor_visitor_.reset();
        cursor_handler_adaptor_.reset();
        eof_ = false;
        if (!read_done())
        {
            read_next();
        }
    }

    template <typename Sourceable>
    void reset(Sourceable&& source)
    {
        parser_.reset(std::forward<Sourceable>(source));
        cursor_visitor_.reset();
        cursor_handler_adaptor_.reset();
        eof_ = false;
        if (!read_done())
        {
            read_next();
        }
    }

    void reset(std::error_code& ec)
    {
        parser_.reset();
        cursor_visitor_.reset();
        cursor_handler_adaptor_.reset();
        eof_ = false;
        if (!read_done())
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    void reset(Sourceable&& source, std::error_code& ec)
    {
        parser_.reset(std::forward<Sourceable>(source));
        cursor_visitor_.reset();
        cursor_handler_adaptor_.reset();
        eof_ = false;
        if (!read_done())
        {
            read_next(ec);
        }
    }

    bool done() const override
    {
        return parser_.done();
    }
    
    uint64_t raw_tag() const
    {
        return parser_.raw_tag();
    }

    bool is_typed_array() const
    {
        return cursor_visitor_.is_typed_array();
    }

    const staj_event& current() const override
    {
        return cursor_visitor_.event();
    }

    void read_to(basic_json_visitor<char_type>& visitor) override
    {
        std::error_code ec;
        read_to(visitor, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read_to(basic_json_visitor<char_type>& visitor,
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
            cursor_visitor_.event().send_json_event(visitor, *this, ec);
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
            cursor_visitor_.event().send_json_event(visitor, *this, ec);
        }
    }

    void next() override
    {
        read_next();
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
    staj_filter_view operator|(basic_cbor_cursor& cursor, 
        std::function<bool(const staj_event&, const ser_context&)> pred)
    {
        return staj_filter_view(cursor, pred);
    }

private:

    bool read_done() const
    {
        return parser_.done();
    }

    void read_next()
    {
        std::error_code ec;
        read_next(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
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
                parser_.parse(cursor_handler_adaptor_, ec);
                if (JSONCONS_UNLIKELY(ec)) {return;}
            }
        }
    }

    void read_next(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        {
            struct resource_wrapper
            {
                basic_item_event_visitor_to_json_visitor<char_type,Allocator>& adaptor;
                basic_json_visitor<char_type>& original;

                resource_wrapper(basic_item_event_visitor_to_json_visitor<char_type,Allocator>& adaptor,
                                 basic_json_visitor<char_type>& visitor)
                    : adaptor(adaptor), original(adaptor.destination())
                {
                    adaptor.destination(visitor);
                }

                ~resource_wrapper()
                {
                    adaptor.destination(original);
                }
            } wrapper(cursor_handler_adaptor_, visitor);

            parser_.restart();
            while (!parser_.stopped())
            {
                parser_.parse(cursor_handler_adaptor_, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
            }
        }
    }
};

using cbor_stream_cursor = basic_cbor_cursor<jsoncons::binary_stream_source>;
using cbor_bytes_cursor = basic_cbor_cursor<jsoncons::bytes_source>;

} // namespace cbor
} // namespace jsoncons

#endif

