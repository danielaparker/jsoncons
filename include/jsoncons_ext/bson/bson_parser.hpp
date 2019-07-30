// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_PARSER_HPP
#define JSONCONS_BSON_BSON_PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/bson/bson_detail.hpp>
#include <jsoncons_ext/bson/bson_error.hpp>

namespace jsoncons { namespace bson {

enum class parse_mode {root,before_done,document,array,value};

struct parse_state 
{
    parse_mode mode; 
    size_t length;
    uint8_t type;
    size_t index;

    parse_state(parse_mode mode, size_t length, uint8_t type = 0)
        : mode(mode), length(length), type(type), index(0)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
};

template <class Src>
class basic_bson_parser : public ser_context
{
    Src source_;
    bool continue_;
    bool done_;
    std::string text_buffer_;
    std::vector<parse_state> state_stack_;
public:
    template <class Source>
    basic_bson_parser(Source&& source)
       : source_(std::forward<Source>(source)), 
         continue_(true), done_(false)
    {
        state_stack_.emplace_back(parse_mode::root,0);
    }

    void restart()
    {
        continue_ = true;
    }

    void reset()
    {
        state_stack_.clear();
        state_stack_.emplace_back(parse_mode::root,0);
        continue_ = true;
        done_ = false;
    }

    bool done() const
    {
        return done_;
    }

    bool stopped() const
    {
        return !continue_;
    }

    size_t line() const override
    {
        return 0;
    }

    size_t column() const override
    {
        return source_.position();
    }

    void parse(json_content_handler& handler, std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = bson_errc::source_error;
            return;
        }
        
        while (!done_ && continue_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::root:
                    state_stack_.back().mode = parse_mode::before_done;
                    begin_document(handler, ec);
                    break;
                case parse_mode::document:
                {
                    uint8_t t{};
                    if (source_.get(t) == 0)
                    {
                        ec = bson_errc::unexpected_eof;
                        return;
                    }
                    if (t != 0x00)
                    {
                        read_e_name(handler,jsoncons::bson::detail::bson_container_type::document,ec);
                        state_stack_.back().mode = parse_mode::value;
                        state_stack_.back().type = t;
                    }
                    else
                    {
                        end_document(handler,ec);
                    }
                    break;
                }
                case parse_mode::array:
                {
                    uint8_t t{};
                    if (source_.get(t) == 0)
                    {
                        ec = bson_errc::unexpected_eof;
                        return;
                    }
                    if (t != 0x00)
                    {
                        read_e_name(handler,jsoncons::bson::detail::bson_container_type::array,ec);
                        read_value(handler,t,ec);
                    }
                    else
                    {
                        end_array(handler,ec);
                    }
                    break;
                }
                case parse_mode::value:
                    state_stack_.back().mode = parse_mode::document;
                    read_value(handler,state_stack_.back().type,ec);
                    break;
                case parse_mode::before_done:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.clear();
                    continue_ = false;
                    done_ = true;
                    handler.flush();
                    break;
                }
            }
        }
    }

private:

    void begin_document(json_content_handler& handler, std::error_code& ec)
    {
        uint8_t buf[sizeof(int32_t)]; 
        if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
        {
            ec = bson_errc::unexpected_eof;
            return;
        }
        const uint8_t* endp;
        auto length = jsoncons::detail::from_little_endian<int32_t>(buf, buf+sizeof(int32_t),&endp);

        continue_ = handler.begin_object(semantic_tag::none, *this);
        state_stack_.emplace_back(parse_mode::document,length);
    }

    void end_document(json_content_handler& handler, std::error_code&)
    {
        continue_ = handler.end_object(*this);
        state_stack_.pop_back();
    }

    void begin_array(json_content_handler& handler, std::error_code& ec)
    {
        uint8_t buf[sizeof(int32_t)]; 
        if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
        {
            ec = bson_errc::unexpected_eof;
            return;
        }
        const uint8_t* endp;
        /* auto len = */ jsoncons::detail::from_little_endian<int32_t>(buf, buf+sizeof(int32_t),&endp);

        continue_ = handler.begin_array(semantic_tag::none, *this);
        state_stack_.emplace_back(parse_mode::array,0);
    }

    void end_array(json_content_handler& handler, std::error_code&)
    {
        continue_ = handler.end_array(*this);
        state_stack_.pop_back();
    }

    void read_e_name(json_content_handler& handler, jsoncons::bson::detail::bson_container_type type, std::error_code& ec)
    {
        text_buffer_.clear();
        uint8_t c{};
        while (source_.get(c) > 0 && c != 0)
        {
            text_buffer_.push_back(c);
        }
        if (type == jsoncons::bson::detail::bson_container_type::document)
        {
            auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
            if (result.ec != unicons::conv_errc())
            {
                ec = bson_errc::invalid_utf8_text_string;
                return;
            }
            continue_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
        }
    }

    void read_value(json_content_handler& handler, uint8_t type, std::error_code& ec)
    {
        switch (type)
        {
            case jsoncons::bson::detail::bson_format::double_cd:
            {
                uint8_t buf[sizeof(double)]; 
                if (source_.read(buf, sizeof(double)) != sizeof(double))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                double res = jsoncons::detail::from_little_endian<double>(buf,buf+sizeof(buf),&endp);
                continue_ = handler.double_value(res, semantic_tag::none, *this);
                break;
            }
            case jsoncons::bson::detail::bson_format::string_cd:
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                auto len = jsoncons::detail::from_little_endian<int32_t>(buf, buf+sizeof(buf),&endp);

                std::basic_string<char> s;
                s.reserve(len - 1);
                if ((int32_t)source_.read(std::back_inserter(s), len-1) != len-1)
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                uint8_t c{};
                source_.get(c); // discard 0
                auto result = unicons::validate(s.begin(),s.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = bson_errc::invalid_utf8_text_string;
                    return;
                }
                continue_ = handler.string_value(basic_string_view<char>(s.data(),s.length()), semantic_tag::none, *this);
                break;
            }
            case jsoncons::bson::detail::bson_format::document_cd: 
            {
                begin_document(handler,ec);
                break;
            }

            case jsoncons::bson::detail::bson_format::array_cd: 
            {
                begin_array(handler,ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::null_cd: 
            {
                continue_ = handler.null_value(semantic_tag::none, *this);
                break;
            }
            case jsoncons::bson::detail::bson_format::bool_cd:
            {
                uint8_t val{};
                if (source_.get(val) == 0)
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                continue_ = handler.bool_value(val != 0, semantic_tag::none, *this);
                break;
            }
            case jsoncons::bson::detail::bson_format::int32_cd: 
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                auto val = jsoncons::detail::from_little_endian<int32_t>(buf, buf+sizeof(int32_t),&endp);
                continue_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }

            case jsoncons::bson::detail::bson_format::timestamp_cd: 
            {
                uint8_t buf[sizeof(uint64_t)]; 
                if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                auto val = jsoncons::detail::from_little_endian<uint64_t>(buf, buf+sizeof(uint64_t),&endp);
                continue_ = handler.uint64_value(val, semantic_tag::timestamp, *this);
                break;
            }

            case jsoncons::bson::detail::bson_format::int64_cd: 
            {
                uint8_t buf[sizeof(int64_t)]; 
                if (source_.read(buf, sizeof(int64_t)) != sizeof(int64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                auto val = jsoncons::detail::from_little_endian<int64_t>(buf, buf+sizeof(int64_t),&endp);
                continue_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }

            case jsoncons::bson::detail::bson_format::datetime_cd: 
            {
                uint8_t buf[sizeof(int64_t)]; 
                if (source_.read(buf, sizeof(int64_t)) != sizeof(int64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                auto val = jsoncons::detail::from_little_endian<int64_t>(buf, buf+sizeof(int64_t),&endp);
                continue_ = handler.int64_value(val, semantic_tag::timestamp, *this);
                break;
            }
            case jsoncons::bson::detail::bson_format::binary_cd: 
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                const auto len = jsoncons::detail::from_little_endian<int32_t>(buf, buf+sizeof(int32_t),&endp);

                std::vector<uint8_t> v(len, 0);
                if (source_.read(v.data(), v.size()) != v.size())
                {
                    ec = bson_errc::unexpected_eof;
                    return;
                }

                continue_ = handler.byte_string_value(byte_string_view(v.data(),v.size()), 
                                           semantic_tag::none, 
                                           *this);
                break;
            }
        }

    }
};

}}

#endif
