// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 
// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_CURSOR_HPP
#define JSONCONS_CSV_CSV_CURSOR_HPP

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
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons_ext/csv/csv_cursor.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/source.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Src=jsoncons::stream_source<CharT>,class Allocator=std::allocator<char>>
class basic_csv_cursor : public basic_staj_cursor<CharT>, private virtual ser_context
{
public:
    using source_type = Src;
    using char_type = CharT;
    using allocator_type = Allocator;
private:
    static constexpr size_t default_max_buffer_length = 16384;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    source_type source_;
    basic_csv_parser<CharT,Allocator> parser_;
    basic_staj_visitor<CharT> cursor_visitor_;
    std::vector<CharT,char_allocator_type> buffer_;
    std::size_t buffer_length_;
    bool eof_;
    bool begin_;

    // Noncopyable and nonmoveable
    basic_csv_cursor(const basic_csv_cursor&) = delete;
    basic_csv_cursor& operator=(const basic_csv_cursor&) = delete;

public:
    using string_view_type = basic_string_view<CharT>;

    // Constructors that throw parse exceptions

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
                     std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : source_(source),
         parser_(options,err_handler,alloc),
         cursor_visitor_(accept_all),
         buffer_(alloc),
         buffer_length_(default_max_buffer_length),
         eof_(false),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
        if (!done())
        {
            next();
        }
    }

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
                     std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : parser_(options,err_handler,alloc),
         cursor_visitor_(accept_all),
         buffer_(alloc),
         buffer_length_(0),
         eof_(false),
         begin_(false)
    {
        basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            JSONCONS_THROW(ser_error(result.ec,parser_.line(),parser_.column()));
        }
        std::size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!done())
        {
            next();
        }
    }


    // Constructors that set parse error codes
    template <class Source>
    basic_csv_cursor(Source&& source, 
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           basic_csv_decode_options<CharT>(),
                           default_csv_parsing(),
                           ec)
    {
    }

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options,
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           options,
                           default_csv_parsing(),
                           ec)
    {
    }

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           options,
                           err_handler,
                           ec)
    {
    }

    template <class Source>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
                     Source&& source, 
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec,
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : source_(source),
         parser_(options,err_handler,alloc),
         cursor_visitor_(accept_all),
         eof_(false),
         buffer_(alloc),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
        if (!done())
        {
            next(ec);
        }
    }

    template <class Source>
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
                     Source&& source, 
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec,
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : parser_(options,err_handler,alloc),
         cursor_visitor_(accept_all),
         eof_(false),
         buffer_(alloc),
         buffer_length_(0),
         begin_(false)
    {
        basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            ec = result.ec;
            return;
        }
        std::size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!done())
        {
            next(ec);
        }
    }

    std::size_t buffer_length() const
    {
        return buffer_length_;
    }

    void buffer_length(std::size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
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
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read_to(basic_json_visitor<CharT>& visitor,
                std::error_code& ec) override
    {
        if (staj_to_saj_event(cursor_visitor_.event(), visitor, *this, ec))
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

    static bool accept_all(const basic_staj_event<CharT>&, const ser_context&) 
    {
        return true;
    }

    void read_buffer(std::error_code& ec)
    {
        buffer_.clear();
        buffer_.resize(buffer_length_);
        std::size_t count = source_.read(buffer_.data(), buffer_length_);
        buffer_.resize(static_cast<std::size_t>(count));
        if (buffer_.size() == 0)
        {
            eof_ = true;
        }
        else if (begin_)
        {
            auto result = unicons::skip_bom(buffer_.begin(), buffer_.end());
            if (result.ec != unicons::encoding_errc())
            {
                ec = result.ec;
                return;
            }
            std::size_t offset = result.it - buffer_.begin();
            parser_.update(buffer_.data()+offset,buffer_.size()-offset);
            begin_ = false;
        }
        else
        {
            parser_.update(buffer_.data(),buffer_.size());
        }
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    const ser_context& context() const override
    {
        return *this;
    }

    void check_done(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = csv_errc::source_error;
            return;
        }   
        if (eof_)
        {
            parser_.check_done(ec);
            if (ec) return;
        }
        else
        {
            while (!eof_)
            {
                if (parser_.source_exhausted())
                {
                    if (!source_.eof())
                    {
                        read_buffer(ec);     
                        if (ec) return;
                    }
                    else
                    {
                        eof_ = true;
                    }
                }
                if (!eof_)
                {
                    parser_.check_done(ec);
                    if (ec) return;
                }
            }
        }
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
    basic_staj_filter_view<CharT> operator|(basic_csv_cursor& cursor, 
                                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> pred)
    {
        return basic_staj_filter_view<CharT>(cursor, pred);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
                     std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : source_(source),
         parser_(options,err_handler,alloc),
         cursor_visitor_(filter),
         buffer_(alloc),
         buffer_length_(default_max_buffer_length),
         eof_(false),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
        if (!done())
        {
            next();
        }
    }
    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>(),
                     std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing(),
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : parser_(options,err_handler,alloc),
         cursor_visitor_(filter),
         buffer_(alloc),
         buffer_length_(0),
         eof_(false),
         begin_(false)
    {
        basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            JSONCONS_THROW(ser_error(result.ec,parser_.line(),parser_.column()));
        }
        std::size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!done())
        {
            next();
        }
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(Source&& source,
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           filter,
                           basic_csv_decode_options<CharT>(),
                           default_csv_parsing(),
                           ec)
    {
    }
    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           filter,
                           options,
                           default_csv_parsing(),
                           ec)
    {
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec)
        : basic_csv_cursor(std::allocator_arg, Allocator(),
                           std::forward<Source>(source),
                           filter,
                           options,
                           err_handler,
                           ec)
    {
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
                     Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec,
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : source_(source),
         parser_(options,err_handler,alloc),
         cursor_visitor_(filter),
         eof_(false),
         buffer_(alloc),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
        if (!done())
        {
            next(ec);
        }
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_csv_cursor(std::allocator_arg_t, const Allocator& alloc, 
                     Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec,
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : parser_(options,err_handler,alloc),
         cursor_visitor_(filter),
         eof_(false),
         buffer_(alloc),
         buffer_length_(0),
         begin_(false)
    {
        basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            ec = result.ec;
            return;
        }
        std::size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
        if (!done())
        {
            next(ec);
        }
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read_to(basic_json_visitor<CharT>&)")
    void read(basic_json_visitor<CharT>& visitor) 
    {
        read_to(visitor);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read_to(basic_json_visitor<CharT>&, std::error_code&)")
    void read(basic_json_visitor<CharT>& visitor,
                 std::error_code& ec) 
    {
        read_to(visitor, ec);
    }
#endif
private:

    void read_next(std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.finished())
        {
            if (parser_.source_exhausted())
            {
                if (!source_.eof())
                {
                    read_buffer(ec);
                    if (ec) return;
                }
                else
                {
                    eof_ = true;
                }
            }
            parser_.parse_some(cursor_visitor_, ec);
            if (ec) return;
        }
    }

    void read_next(basic_json_visitor<CharT>& visitor, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.finished())
        {
            if (parser_.source_exhausted())
            {
                if (!source_.eof())
                {
                    read_buffer(ec);
                    if (ec) return;
                }
                else
                {
                    eof_ = true;
                }
            }
            parser_.parse_some(visitor, ec);
            if (ec) return;
        }
    }
};

using csv_cursor = basic_csv_cursor<char>;
using wcsv_cursor = basic_csv_cursor<wchar_t>;

}}

#endif

