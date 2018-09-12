// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONEVENTREADER_HPP
#define JSONCONS_JSONEVENTREADER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json.hpp>

namespace jsoncons {

enum class json_event_type
{
    begin_document = 0,
    end_document = 1,
    begin_object = 2,
    end_object = 3,
    begin_array = 4,
    end_array = 5,
    name = 6,
    string = 7,
    byte_string = 8,
    bignum = 9,
    integer = 10,
    uinteger = 11,
    floating_point = 12,
    boolean = 13,
    null = 14
};

template<class CharT,class Allocator=std::allocator<char>>
class basic_json_event
{
    json_event_type event_type_;
    basic_json<CharT,sorted_policy,Allocator> value_;
public:
    basic_json_event(json_event_type event_type)
        : event_type_(event_type)
    {
    }

    basic_json_event(json_event_type event_type,
                     basic_json<CharT,sorted_policy,Allocator>&& value)
        : event_type_(event_type), value_(std::move(value))
    {
    }

    basic_json_event(json_event_type event_type,
                     const basic_json<CharT,sorted_policy,Allocator>& value)
        : event_type_(event_type), value_(value)
    {
    }

    template<class T, class... Args>
    bool is(Args&&... args) const
    {
        return value_.is<T>(std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    T as(Args&&... args) const
    {
        return value_.as<T>(std::forward<Args>(args)...);
    }

    json_event_type event_type() const {return event_type_;}
};


template <class CharT, class Allocator>
class basic_json_event_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
private:
    basic_json_event<CharT,Allocator> event_;
public:
    basic_json_event_handler()
        : event_(json_event_type::begin_document)
    {
    }

    basic_json_event_handler(json_event_type event_type)
        : event_(event_type)
    {
    }

    const basic_json_event<CharT,Allocator>& event() const
    {
        return event_;
    }
private:

    void do_begin_document() override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::begin_document);
    }

    void do_end_document() override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::end_document);
    }

    bool do_begin_object(const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::begin_object);
        return false;
    }

    bool do_end_object(const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::end_object);
        return false;
    }

    bool do_begin_array(const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::begin_array);
        return false;
    }

    bool do_end_array(const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::end_array);
        return false;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::name, basic_json<CharT,sorted_policy,Allocator>(name.data(),name.length()));
        return false;
    }

    bool do_null(const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::null, basic_json<CharT,sorted_policy,Allocator>(jsoncons::null_type()));
        return false;
    }

    bool do_bool(bool value, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::boolean, basic_json<CharT,sorted_policy,Allocator>(value));
        return false;
    }

    bool do_string(const string_view_type& s, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::string, basic_json<CharT,sorted_policy,Allocator>(s.data(), s.length()));
        return false;
    }

    bool do_byte_string(const uint8_t*, size_t, const serializing_context&) override
    {
        // noop
        return false;
    }

    bool do_bignum(int, const uint8_t*, size_t, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::bignum);
        return false;
    }

    bool do_integer(int64_t value, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::integer, basic_json<CharT,sorted_policy,Allocator>(value));
        return false;
    }

    bool do_uinteger(uint64_t value, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::uinteger, basic_json<CharT,sorted_policy,Allocator>(value));
        return false;
    }

    bool do_double(double value, const floating_point_options&, const serializing_context&) override
    {
        event_ = basic_json_event<CharT,Allocator>(json_event_type::floating_point, basic_json<CharT,sorted_policy,Allocator>(value));
        return false;
    }
};

template<class CharT,class Allocator=std::allocator<char>>
class basic_json_event_reader 
{
    static const size_t default_max_buffer_length = 16384;

    basic_json_event_handler<CharT, Allocator> event_handler_;
    default_parse_error_handler default_err_handler_;

    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    basic_json_parser<CharT,Allocator> parser_;
    std::basic_istream<CharT>& is_;
    bool eof_;
    std::vector<CharT,char_allocator_type> buffer_;
    size_t buffer_length_;
    bool begin_;

    // Noncopyable and nonmoveable
    basic_json_event_reader(const basic_json_event_reader&) = delete;
    basic_json_event_reader& operator=(const basic_json_event_reader&) = delete;

public:

    basic_json_event_reader(std::basic_istream<CharT>& is)
        : basic_json_event_reader(is,basic_json_serializing_options<CharT>(),default_err_handler_)
    {
    }

    basic_json_event_reader(std::basic_istream<CharT>& is,
                      parse_error_handler& err_handler)
        : basic_json_event_reader(is,basic_json_serializing_options<CharT>(),err_handler)
    {
    }

    basic_json_event_reader(std::basic_istream<CharT>& is, 
                      const basic_json_read_options<CharT>& options)
        : basic_json_event_reader(is,options,default_err_handler_)
    {
    }

    basic_json_event_reader(std::basic_istream<CharT>& is, 
                            const basic_json_read_options<CharT>& options,
                            parse_error_handler& err_handler)
       : parser_(event_handler_,options,err_handler),
         is_(is),
         eof_(false),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    size_t buffer_length() const
    {
        return buffer_length_;
    }

    void buffer_length(size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
    }

    bool done() const
    {
        return parser_.done();
    }

    basic_json_event<CharT,Allocator> event() const
    {
        return event_handler_.event();
    }

    void read_next()
    {
        std::error_code ec;
        read_next(ec);
        if (ec)
        {
            throw parse_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    void read_buffer(std::error_code& ec)
    {
        buffer_.clear();
        buffer_.resize(buffer_length_);
        is_.read(buffer_.data(), buffer_length_);
        buffer_.resize(static_cast<size_t>(is_.gcount()));
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
            size_t offset = result.it - buffer_.begin();
            parser_.update(buffer_.data()+offset,buffer_.size()-offset);
            begin_ = false;
        }
        else
        {
            parser_.update(buffer_.data(),buffer_.size());
        }
    }

    void read_next(std::error_code& ec)
    {
        parser_.restart();
        while (!eof_ && !parser_.stopped())
        {
            if (parser_.source_exhausted())
            {
                if (!is_.eof())
                {
                    if (is_.fail())
                    {
                        ec = json_parse_errc::source_error;
                        return;
                    }        
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
                parser_.parse_some(ec);
                if (ec) return;
            }
        }
        if (eof_)
        {
            parser_.end_parse(ec);
            if (ec) return;
        }
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            throw parse_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    size_t line_number() const
    {
        return parser_.line_number();
    }

    size_t column_number() const
    {
        return parser_.column_number();
    }

    void check_done(std::error_code& ec)
    {
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
                    if (!is_.eof())
                    {
                        if (is_.fail())
                        {
                            ec = json_parse_errc::source_error;
                            return;
                        }   
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

    void read()
    {
        read_next();
        check_done();
    }

    void read(std::error_code& ec)
    {
        read_next(ec);
        if (!ec)
        {
            check_done(ec);
        }
    }

private:
};

typedef basic_json_event_reader<char> json_event_reader;
typedef basic_json_event_reader<wchar_t> wjson_event_reader;

typedef basic_json_event<char> json_event;
typedef basic_json_event<wchar_t> wjson_event;

}

#endif

