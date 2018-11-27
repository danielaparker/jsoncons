// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_PARSER_HPP
#define JSONCONS_BSON_BSON_PARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <iterator>
#include <jsoncons/json.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_utilities.hpp>
#include <jsoncons_ext/bson/bson_detail.hpp>
#include <jsoncons_ext/bson/bson_error_category.hpp>

namespace jsoncons { namespace bson {

class bson_parser : public serializing_context
{
    const uint8_t* begin_input_;
    const uint8_t* end_input_;
    const uint8_t* input_ptr_;
    json_content_handler& handler_;
    size_t column_;
    size_t nesting_depth_;
    std::string buffer_;
public:
    bson_parser(json_content_handler& handler)
       : handler_(handler), 
         column_(1),
         nesting_depth_(0)
    {
    }

    void update(const uint8_t* input, size_t length)
    {
        begin_input_ = input;
        end_input_ = input + length;
        input_ptr_ = begin_input_;
    }

    void reset()
    {
        column_ = 1;
        nesting_depth_ = 0;
    }

    void parse_some(std::error_code& ec)
    {
        const uint8_t* pos = input_ptr_++;
        if (*pos <= 0xbf)
        {
            if (*pos <= 0x7f) 
            {
                // positive fixint
                handler_.uint64_value(*pos, semantic_tag_type::none, *this);
            }
            else if (*pos <= 0x8f) 
            {
                // fixmap
                const size_t len = *pos & 0x0f;
                handler_.begin_object(len, semantic_tag_type::none, *this);
                ++nesting_depth_;
                for (size_t i = 0; i < len; ++i)
                {
                    parse_name(ec);
                    if (ec)
                    {
                        return;
                    }
                    parse_some(ec);
                    if (ec)
                    {
                        return;
                    }
                }
                handler_.end_object(*this);
                --nesting_depth_;
            }
            else if (*pos <= 0x9f) 
            {
                // fixarray
                const size_t len = *pos & 0x0f;
                handler_.begin_array(len, semantic_tag_type::none, *this);
                ++nesting_depth_;
                for (size_t i = 0; i < len; ++i)
                {
                    parse_some(ec);
                    if (ec)
                    {
                        return;
                    }
                }
                handler_.end_array(*this);
                --nesting_depth_;
            }
            else 
            {
                // fixstr
                const size_t len = *pos & 0x1f;
                const uint8_t* first = input_ptr_;
                const uint8_t* last = first + len;
                input_ptr_ += len; 

                std::basic_string<char> s;
                auto result = unicons::convert(
                    first, last, std::back_inserter(s), unicons::conv_flags::strict);
                if (result.ec != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                }
                handler_.string_value(basic_string_view<char>(s.data(),s.length()), semantic_tag_type::none, *this);
            }
        }
        else if (*pos >= 0xe0) 
        {
            // negative fixint
            handler_.int64_value(static_cast<int8_t>(*pos), semantic_tag_type::none, *this);
        }
        else
        {
            switch (*pos)
            {
                case bson_format::nil_cd: 
                {
                    handler_.null_value(semantic_tag_type::none, *this);
                    break;
                }
                case bson_format::true_cd:
                {
                    handler_.bool_value(true, semantic_tag_type::none, *this);
                    break;
                }
                case bson_format::false_cd:
                {
                    handler_.bool_value(false, semantic_tag_type::none, *this);
                    break;
                }
                case bson_format::float32_cd: 
                {
                    const uint8_t* endp;
                    float res = binary::from_big_endian<float>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.double_value(res, floating_point_options(), semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::float64_cd: 
                {
                    const uint8_t* endp;
                    double res = binary::from_big_endian<double>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.double_value(res, floating_point_options(), semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::uint8_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint8_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.uint64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::uint16_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.uint64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::uint32_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.uint64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::uint64_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<uint64_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.uint64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::int8_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int8_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.int64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::int16_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.int64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::int32_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.int64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::int64_cd: 
                {
                    const uint8_t* endp;
                    auto x = binary::from_big_endian<int64_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.int64_value(x, semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::str8_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int8_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.string_value(basic_string_view<char>(s.data(),s.length()), semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::str16_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.string_value(basic_string_view<char>(s.data(),s.length()), semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::str32_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.string_value(basic_string_view<char>(s.data(),s.length()), semantic_tag_type::none, *this);
                    break;
                }

                case bson_format::bin8_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int8_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;

                    std::vector<uint8_t> v(first, last);
                    input_ptr_ += len; 

                    handler_.byte_string_value(byte_string_view(v.data(),v.size()), 
                                               byte_string_chars_format::none, 
                                               semantic_tag_type::none, 
                                               *this);
                    break;
                }

                case bson_format::bin16_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }


                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;

                    std::vector<uint8_t> v(first, last);
                    input_ptr_ += len; 

                    handler_.byte_string_value(byte_string_view(v.data(),v.size()), 
                                               byte_string_chars_format::none, 
                                               semantic_tag_type::none, 
                                               *this);
                    break;
                }

                case bson_format::bin32_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }


                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;

                    std::vector<uint8_t> v(first, last);
                    input_ptr_ += len; 

                    handler_.byte_string_value(byte_string_view(v.data(),v.size()), 
                                               byte_string_chars_format::none, 
                                               semantic_tag_type::none, 
                                               *this);
                    break;
                }

                case bson_format::array16_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.begin_array(len, semantic_tag_type::none, *this);
                    ++nesting_depth_;
                    for (size_t i = 0; i < len; ++i)
                    {
                        parse_some(ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    handler_.end_array(*this);
                    --nesting_depth_;
                    break;
                }

                case bson_format::array32_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.begin_array(len, semantic_tag_type::none, *this);
                    ++nesting_depth_;
                    for (size_t i = 0; i < len; ++i)
                    {
                        parse_some(ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    handler_.end_array(*this);
                    --nesting_depth_;
                    break;
                }

                case bson_format::map16_cd : 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.begin_object(len, semantic_tag_type::none, *this);
                    ++nesting_depth_;
                    for (size_t i = 0; i < len; ++i)
                    {
                        parse_name(ec);
                        if (ec)
                        {
                            return;
                        }
                        parse_some(ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    handler_.end_object(*this);
                    --nesting_depth_;
                    break;
                }

                case bson_format::map32_cd : 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<uint32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }
                    handler_.begin_object(len, semantic_tag_type::none, *this);
                    ++nesting_depth_;
                    for (size_t i = 0; i < len; ++i)
                    {
                        parse_name(ec);
                        if (ec)
                        {
                            return;
                        }
                        parse_some(ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    handler_.end_object(*this);
                    --nesting_depth_;
                    break;
                }

                default:
                {
                    JSONCONS_THROW(bson_decode_error(end_input_-pos));
                }
            }
        }
    }

    size_t line_number() const override
    {
        return 1;
    }

    size_t column_number() const override
    {
        return column_;
    }
private:
    void parse_name(std::error_code&)
    {
        const uint8_t* pos = input_ptr_++;
        if (*pos >= 0xa0 && *pos <= 0xbf)
        {
                    // fixstr
                const size_t len = *pos & 0x1f;
                const uint8_t* first = input_ptr_;
                const uint8_t* last = first + len;
                input_ptr_ += len; 

                std::basic_string<char> s;
                auto result = unicons::convert(
                    first, last, std::back_inserter(s), unicons::conv_flags::strict);
                if (result.ec != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                }
                handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
        }
        else
        {
            switch (*pos)
            {
                case bson_format::str8_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int8_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
                    break;
                }

                case bson_format::str16_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int16_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
                    break;
                }

                case bson_format::str32_cd: 
                {
                    const uint8_t* endp;
                    const auto len = binary::from_big_endian<int32_t>(input_ptr_,end_input_,&endp);
                    if (endp == input_ptr_)
                    {
                        JSONCONS_THROW(bson_decode_error(end_input_-input_ptr_));
                    }
                    else
                    {
                        input_ptr_ = endp;
                    }

                    const uint8_t* first = endp;
                    const uint8_t* last = first + len;
                    input_ptr_ += len; 

                    std::basic_string<char> s;
                    auto result = unicons::convert(
                        first, last,std::back_inserter(s),unicons::conv_flags::strict);
                    if (result.ec != unicons::conv_errc())
                    {
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
                    }
                    handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
                    break;
                }
            }

        }
    }
};

}}

#endif
