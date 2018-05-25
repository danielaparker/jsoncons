// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

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

namespace jsoncons {

// utf8_other_json_input_adapter

template <class CharT>
class json_utf8_other_content_handler_adapter : public json_content_handler
{
public:
    using json_content_handler::string_view_type;
private:
    basic_null_json_content_handler<CharT> default_content_handler_;
    basic_json_content_handler<CharT>& other_handler_;
    //parse_error_handler& err_handler_;

    // noncopyable and nonmoveable
    json_utf8_other_content_handler_adapter<CharT>(const json_utf8_other_content_handler_adapter<CharT>&) = delete;
    json_utf8_other_content_handler_adapter<CharT>& operator=(const json_utf8_other_content_handler_adapter<CharT>&) = delete;

public:
    json_utf8_other_content_handler_adapter()
        : other_handler_(default_content_handler_)
    {
    }

    json_utf8_other_content_handler_adapter(basic_json_content_handler<CharT>& other_handler/*,
                                          parse_error_handler& err_handler*/)
        : other_handler_(other_handler)/*,
          err_handler_(err_handler)*/
    {
    }

private:

    void do_begin_json() override
    {
        other_handler_.begin_json();
    }

    void do_end_json() override
    {
        other_handler_.end_json();
    }

    void do_begin_object(const serializing_context& context) override
    {
        other_handler_.begin_object(context);
    }

    void do_end_object(const serializing_context& context) override
    {
        other_handler_.end_object(context);
    }

    void do_begin_array(const serializing_context& context) override
    {
        other_handler_.begin_array(context);
    }

    void do_end_array(const serializing_context& context) override
    {
        other_handler_.end_array(context);
    }

    void do_name(const string_view_type& name, const serializing_context& context) override
    {
        std::basic_string<CharT> target;
        auto result = unicons::convert(
            name.begin(), name.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            throw parse_error(result.ec,context.line_number(),context.column_number());
        }
        other_handler_.name(target, context);
    }

    void do_string_value(const string_view_type& value, const serializing_context& context) override
    {
        std::basic_string<CharT> target;
        auto result = unicons::convert(
            value.begin(), value.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            throw parse_error(result.ec,context.line_number(),context.column_number());
        }
        other_handler_.string_value(target, context);
    }

    void do_integer_value(int64_t value, const serializing_context& context) override
    {
        other_handler_.integer_value(value, context);
    }

    void do_uinteger_value(uint64_t value, const serializing_context& context) override
    {
        other_handler_.uinteger_value(value, context);
    }

    void do_double_value(double value, const number_format& fmt, const serializing_context& context) override
    {
        other_handler_.double_value(value, fmt, context);
    }

    void do_bool_value(bool value, const serializing_context& context) override
    {
        other_handler_.bool_value(value, context);
    }

    void do_null_value(const serializing_context& context) override
    {
        other_handler_.null_value(context);
    }
};

template<class CharT,class Allocator=std::allocator<char>>
class basic_json_reader 
{
    static const size_t default_max_buffer_length = 16384;

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
    basic_json_reader(const basic_json_reader&) = delete;
    basic_json_reader& operator=(const basic_json_reader&) = delete;

public:

    basic_json_reader(std::basic_istream<CharT>& is)
        : parser_(),
          is_(is),
          eof_(false),
          buffer_length_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      parse_error_handler& err_handler)
       : parser_(err_handler),
         is_(is),
         eof_(false),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is, 
                      basic_json_content_handler<CharT>& handler)
        : parser_(handler),
          is_(is),
          eof_(false),
          buffer_length_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      basic_json_content_handler<CharT>& handler,
                      parse_error_handler& err_handler)
       : parser_(handler,err_handler),
         is_(is),
         eof_(false),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is, 
                      const basic_json_serializing_options<CharT>& options)
        : parser_(options),
          is_(is),
          eof_(false),
          buffer_length_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is, 
                      const basic_json_serializing_options<CharT>& options,
                      parse_error_handler& err_handler)
       : parser_(options,err_handler),
         is_(is),
         eof_(false),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is, 
                      const basic_json_serializing_options<CharT>& options, 
                      basic_json_content_handler<CharT>& handler)
        : parser_(handler,options),
          is_(is),
          eof_(false),
          buffer_length_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      basic_json_content_handler<CharT>& handler, 
                      const basic_json_serializing_options<CharT>& options,
                      parse_error_handler& err_handler)
       : parser_(handler,options,err_handler),
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
#if !defined(JSONCONS_NO_DEPRECATED)
    size_t max_nesting_depth() const
    {
        return parser_.max_nesting_depth();
    }

    void max_nesting_depth(size_t depth)
    {
        parser_.max_nesting_depth(depth);
    }
#endif
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
            parser_.set_source(buffer_.data()+offset,buffer_.size()-offset);
            begin_ = false;
        }
        else
        {
            parser_.set_source(buffer_.data(),buffer_.size());
        }
    }

    void read_next(std::error_code& ec)
    {
        parser_.reset();
        while (!eof_ && !parser_.done())
        {
            if (parser_.source_exhausted())
            {
                if (!is_.eof())
                {
                    if (is_.fail())
                    {
                        ec = json_parser_errc::source_error;
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
                            ec = json_parser_errc::source_error;
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

#if !defined(JSONCONS_NO_DEPRECATED)

    size_t buffer_capacity() const
    {
        return buffer_length_;
    }

    void buffer_capacity(size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
    }
    size_t max_depth() const
    {
        return parser_.max_nesting_depth();
    }

    void max_depth(size_t depth)
    {
        parser_.max_nesting_depth(depth);
    }
#endif

private:
};

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif

