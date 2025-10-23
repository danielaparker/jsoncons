// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 
// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CSV_CSV_CURSOR_HPP
#define JSONCONS_EXT_CSV_CSV_CURSOR_HPP

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
#include <jsoncons/source_adaptor.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

#include <jsoncons_ext/csv/csv_parser.hpp>

namespace jsoncons { 
namespace csv {

template <typename CharT,typename Source=jsoncons::stream_source<CharT>,typename Allocator=std::allocator<char>>
class basic_csv_cursor : public basic_staj_cursor<CharT>, private virtual ser_context
{
public:
    using source_type = Source;
    using char_type = CharT;
    using allocator_type = Allocator;
private:
    static constexpr size_t default_max_buffer_size = 16384;

    using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT>;

    text_source_adaptor<Source> source_;
    basic_csv_parser<CharT,Allocator> parser_;
    basic_staj_visitor<CharT> cursor_visitor_;

public:
    using string_view_type = jsoncons::basic_string_view<CharT>;

    // Noncopyable and nonmoveable
    basic_csv_cursor(const basic_csv_cursor&) = delete;
    basic_csv_cursor(basic_csv_cursor&&) = delete;

    // Constructors that throw parse exceptions

    template <typename Sourceable>
    basic_csv_cursor(Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
        std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
        const Allocator& alloc = Allocator(),
        typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(std::forward<Sourceable>(source)),
         parser_(options,err_handler,alloc)
    {
        parser_.cursor_mode(true);
        if (!read_done())
        {
            read_next();
        }
    }

    template <typename Sourceable>
    basic_csv_cursor(Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
        std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
        const Allocator& alloc = Allocator(),
        typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(),
         parser_(options,err_handler,alloc)
    {
        parser_.cursor_mode(true);
        jsoncons::basic_string_view<CharT> sv(std::forward<Sourceable>(source));
        initialize_with_string_view(sv);
    }


    // Constructors that set parse error codes
    template <typename Sourceable>
    basic_csv_cursor(Sourceable&& source, std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
              std::forward<Sourceable>(source),
              basic_csv_decode_options<CharT>(),
              default_csv_parsing(),
              ec)
    {
    }

    template <typename Sourceable>
    basic_csv_cursor(Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
              std::forward<Sourceable>(source),
              options,
              default_csv_parsing(),
              ec)
    {
    }

    template <typename Sourceable>
    basic_csv_cursor(Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::function<bool(csv_errc,const ser_context&)> err_handler,
        std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
              std::forward<Sourceable>(source),
              options,
              err_handler,
              ec)
    {
    }

    template <typename Sourceable>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::function<bool(csv_errc,const ser_context&)> err_handler,
        std::error_code& ec,
        typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(std::forward<Sourceable>(source)),
         parser_(options,err_handler,alloc)
    {
        parser_.cursor_mode(true);
        if (!read_done())
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::error_code& ec,
        typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(std::forward<Sourceable>(source)),
         parser_(options,alloc)
    {
        parser_.cursor_mode(true);
        if (!read_done())
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::error_code& ec,
        typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(),
         parser_(options,alloc)
    {
        parser_.cursor_mode(true);
        jsoncons::basic_string_view<CharT> sv(std::forward<Sourceable>(source));
        initialize_with_string_view(sv, ec);
    }

    template <typename Sourceable>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source, 
        const basic_csv_decode_options<CharT>& options,
        std::function<bool(csv_errc,const ser_context&)> err_handler,
        std::error_code& ec,
        typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(),
         parser_(options,err_handler,alloc)
    {
        parser_.cursor_mode(true);
        jsoncons::basic_string_view<CharT> sv(std::forward<Sourceable>(source));
        initialize_with_string_view(sv, ec);
    }
    
    ~basic_csv_cursor() = default;

    basic_csv_cursor& operator=(const basic_csv_cursor&) = delete;
    basic_csv_cursor& operator=(basic_csv_cursor&&) = delete;

    template <typename Sourceable>
    typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source)
    {
        source_ = std::forward<Sourceable>(source);
        parser_.reinitialize();
        cursor_visitor_.reset();
        if (!read_done())
        {
            read_next();
        }
    }

    template <typename Sourceable>
    typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source)
    {
        source_ = {};
        parser_.reinitialize();
        cursor_visitor_.reset();
        initialize_with_string_view(std::forward<Sourceable>(source));
    }

    template <typename Sourceable>
    typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source, std::error_code& ec)
    {
        source_ = std::forward<Sourceable>(source);
        parser_.reinitialize();
        cursor_visitor_.reset();
        if (!read_done())
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source, std::error_code& ec)
    {
        source_ = {};
        parser_.reinitialize();
        initialize_with_string_view(std::forward<Sourceable>(source), ec);
    }

    bool done() const override
    {
        return parser_.done();
    }

    const basic_staj_event<CharT>& current() const override
    {
        return cursor_visitor_.event();
    }

    void read_to(basic_json_visitor<CharT>& visitor) override
    {
        std::error_code ec;
        read_to(visitor, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read_to(basic_json_visitor<CharT>& visitor, std::error_code& ec) override
    {
        if (is_begin_container(current().event_type()))
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
        return parser_.source_exhausted() && source_.eof();
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
    basic_staj_filter_view<CharT> operator|(basic_csv_cursor& cursor, 
        std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> pred)
    {
        return basic_staj_filter_view<CharT>(cursor, pred);
    }

private:

    bool read_done() const
    {
        return parser_.done();
    }

    void initialize_with_string_view(string_view_type sv)
    {
        auto r = unicode_traits::detect_json_encoding(sv.data(), sv.size());
        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
        {
            JSONCONS_THROW(ser_error(json_errc::illegal_unicode_character,parser_.line(),parser_.column()));
        }
        std::size_t offset = (r.ptr - sv.data());
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!read_done())
        {
            read_next();
        }
    }

    void initialize_with_string_view(string_view_type sv, std::error_code& ec)
    {
        auto r = unicode_traits::detect_encoding_from_bom(sv.data(), sv.size());
        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
        {
            ec = json_errc::illegal_unicode_character;
            return;
        }
        std::size_t offset = (r.ptr - sv.data());
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!read_done())
        {
            read_next(ec);
        }
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
        read_next(cursor_visitor_, ec);
    }

    void read_next(basic_json_visitor<CharT>& visitor, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            if (parser_.source_exhausted())
            {
                auto s = source_.read_buffer(ec);
                if (JSONCONS_UNLIKELY(ec)) {return;}
                if (s.size() > 0)
                {
                    parser_.update(s.data(),s.size());
                }
            }
            parser_.parse_some(visitor, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
    }
};

using csv_stream_cursor = basic_csv_cursor<char,jsoncons::stream_source<char>>;
using csv_string_cursor = basic_csv_cursor<char,jsoncons::string_source<char>>;
using wcsv_stream_cursor = basic_csv_cursor<wchar_t,jsoncons::stream_source<wchar_t>>;
using wcsv_string_cursor = basic_csv_cursor<wchar_t,jsoncons::string_source<wchar_t>>;

} // namespace csv
} // namespace jsoncons

#endif // JSONCONS_EXT_CSV_CSV_CURSOR_HPP

