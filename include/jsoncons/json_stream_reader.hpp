// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSTREAMREADER_HPP
#define JSONCONS_JSONSTREAMREADER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <iterator>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/stream_reader.hpp>
#include <jsoncons/json_parser.hpp>

namespace jsoncons {

template <class CharT>
class basic_stream_event_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
private:
    basic_stream_event<CharT> event_;
public:
    basic_stream_event_handler()
        : event_(stream_event_type::null_value)
    {
    }

    basic_stream_event_handler(stream_event_type event_type)
        : event_(event_type)
    {
    }

    const basic_stream_event<CharT>& event() const
    {
        return event_;
    }
private:

    bool do_begin_object(const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(stream_event_type::begin_object);
        return false;
    }

    bool do_end_object(const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(stream_event_type::end_object);
        return false;
    }

    bool do_begin_array(const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(stream_event_type::begin_array);
        return false;
    }

    bool do_end_array(const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(stream_event_type::end_array);
        return false;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(name.data(), name.length(), stream_event_type::name);
        return false;
    }

    bool do_null_value(const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(stream_event_type::null_value);
        return false;
    }

    bool do_bool(bool value, const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(value);
        return false;
    }

    bool do_string_value(const string_view_type& s, semantic_tag_type tag, const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(s.data(), s.length(), stream_event_type::string_value, tag);
        return false;
    }

    bool do_byte_string_value(const uint8_t*, size_t, 
                              semantic_tag_type,
                              const serializing_context&) override
    {
        JSONCONS_UNREACHABLE();
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag_type tag,
                        const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(value, tag);
        return false;
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag_type tag, 
                         const serializing_context&) override
    {
        std::cout << "basic_stream_event_handler do_uint64_value\n";
        event_ = basic_stream_event<CharT>(value, tag);
        return false;
    }

    bool do_double_value(double value, 
                         const floating_point_options& fmt, 
                         semantic_tag_type tag, 
                         const serializing_context&) override
    {
        event_ = basic_stream_event<CharT>(value, fmt, tag);
        return false;
    }

    void do_flush() override
    {
    }
};

template<class CharT,class Allocator>
class basic_json_stream_reader : public basic_stream_reader<CharT>, private virtual serializing_context
{
    static const size_t default_max_buffer_length = 16384;

    basic_stream_event_handler<CharT> event_handler_;
    default_parse_error_handler default_err_handler_;

    default_basic_stream_filter<CharT> default_filter_;

    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    basic_json_parser<CharT,Allocator> parser_;
    std::basic_istream<CharT>& is_;
    basic_stream_filter<CharT>& filter_;
    bool eof_;
    std::vector<CharT,char_allocator_type> buffer_;
    size_t buffer_length_;
    bool begin_;

    // Noncopyable and nonmoveable
    basic_json_stream_reader(const basic_json_stream_reader&) = delete;
    basic_json_stream_reader& operator=(const basic_json_stream_reader&) = delete;

public:

    basic_json_stream_reader(std::basic_istream<CharT>& is)
        : basic_json_stream_reader(is,default_filter_,basic_json_serializing_options<CharT>(),default_err_handler_)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is,
                             basic_stream_filter<CharT>& filter)
        : basic_json_stream_reader(is,filter,basic_json_serializing_options<CharT>(),default_err_handler_)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is,
                             parse_error_handler& err_handler)
        : basic_json_stream_reader(is,default_filter_,basic_json_serializing_options<CharT>(),err_handler)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is,
                             basic_stream_filter<CharT>& filter,
                             parse_error_handler& err_handler)
        : basic_json_stream_reader(is,filter,basic_json_serializing_options<CharT>(),err_handler)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is, 
                             const basic_json_read_options<CharT>& options)
        : basic_json_stream_reader(is,default_filter_,options,default_err_handler_)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is,
                             basic_stream_filter<CharT>& filter, 
                             const basic_json_read_options<CharT>& options)
        : basic_json_stream_reader(is,filter,options,default_err_handler_)
    {
    }

    basic_json_stream_reader(std::basic_istream<CharT>& is, 
                             basic_stream_filter<CharT>& filter,
                             const basic_json_read_options<CharT>& options,
                             parse_error_handler& err_handler)
       : parser_(event_handler_,options,err_handler),
         is_(is),
         filter_(filter),
         eof_(false),
         buffer_length_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
        if (!done())
        {
            next();
        }
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

    bool done() const override
    {
        return parser_.done();
    }

    const basic_stream_event<CharT>& current() const override
    {
        return event_handler_.event();
    }

    void next() override
    {
        std::error_code ec;

        do
        {
            read_next(ec);
            if (ec)
            {
                throw parse_error(ec,parser_.line_number(),parser_.column_number());
            }
        } while (!done() && !filter_.accept(event_handler_.event(), *this));
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
        while (!parser_.stopped())
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
                    parser_.update(buffer_.data(),0);
                    eof_ = true;
                }
            }
            parser_.parse_some(ec);
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

    const serializing_context& context() const override
    {
        return *this;
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

    size_t line_number() const override
    {
        return parser_.line_number();
    }

    size_t column_number() const override
    {
        return parser_.column_number();
    }
private:
};

typedef basic_json_stream_reader<char,std::allocator<char>> json_stream_reader;
typedef basic_json_stream_reader<wchar_t, std::allocator<wchar_t>> wjson_stream_reader;

}

#endif

