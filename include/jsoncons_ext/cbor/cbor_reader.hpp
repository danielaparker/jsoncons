// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_READER_HPP
#define JSONCONS_CBOR_CBOR_READER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons_ext/cbor/cbor_view.hpp>
#endif  

namespace jsoncons { namespace cbor {

template <class Source>
class basic_cbor_reader : public serializing_context
{
    Source source_;
    json_content_handler& handler_;
    size_t nesting_depth_;
    std::string buffer_;
public:
    basic_cbor_reader(Source source, json_content_handler& handler)
       : source_(std::move(source)),
         handler_(handler), 
         nesting_depth_(0)
    {
    }

    void read(std::error_code& ec)
    {
        try
        {
            read_internal(ec);
        }
        catch (const serialization_error& e)
        {
            ec = e.code();
        }
    }

    size_t line_number() const override
    {
        return 0;
    }

    size_t column_number() const override
    {
        return source_.position();
    }
private:

    void read_internal(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = cbor_errc::source_error;
            return;
        }   

        bool has_cbor_tag = false;
        uint8_t cbor_tag = 0;

        cbor_major_type major_type;
        uint8_t info;
        int c = source_.peek();
        switch (c)
        {
            case Source::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                return;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }

        if (major_type == cbor_major_type::semantic_tag)
        {
            has_cbor_tag = true;
            cbor_tag = info;
            source_.ignore(1);
            c = source_.peek();
            switch (c)
            {
                case Source::traits_type::eof():
                    ec = cbor_errc::unexpected_eof;
                    return;
                default:
                    major_type = get_major_type((uint8_t)c);
                    info = get_additional_information_value((uint8_t)c);
                    break;
            }
        }

        switch (major_type)
        {
            case cbor_major_type::unsigned_integer:
            {
                uint64_t val = jsoncons::cbor::detail::get_uint64_value(source_, ec);
                if (ec)
                {
                    return;
                }

                if (has_cbor_tag && cbor_tag == 1)
                {
                    handler_.uint64_value(val, semantic_tag_type::timestamp, *this);
                }
                else
                {
                    handler_.uint64_value(val, semantic_tag_type::none, *this);
                }
                break;
            }
            case cbor_major_type::negative_integer:
            {
                int64_t val = jsoncons::cbor::detail::get_int64_value(source_, ec);
                if (ec)
                {
                    return;
                }
                if (has_cbor_tag && cbor_tag == 1)
                {
                    handler_.int64_value(val, semantic_tag_type::timestamp, *this);
                }
                else 
                {
                    handler_.int64_value(val, semantic_tag_type::none, *this);
                }
                break;
            }
            case cbor_major_type::byte_string:
            {
                std::vector<uint8_t> v = jsoncons::cbor::detail::get_byte_string(source_, ec);
                if (ec)
                {
                    return;
                }

                if (has_cbor_tag)
                {
                    switch (cbor_tag)
                    {
                        case 0x2:
                            {
                                bignum n(1, v.data(), v.size());
                                buffer_.clear();
                                n.dump(buffer_);
                                handler_.big_integer_value(buffer_, *this);
                                break;
                            }
                        case 0x3:
                            {
                                bignum n(-1, v.data(), v.size());
                                buffer_.clear();
                                n.dump(buffer_);
                                handler_.big_integer_value(buffer_, *this);
                                break;
                            }
                        case 0x15:
                            {
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag_type::base64url, *this);
                                break;
                            }
                        case 0x16:
                            {
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag_type::base64, *this);
                                break;
                            }
                        case 0x17:
                            {
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag_type::base16, *this);
                                break;
                            }
                        default:
                            handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag_type::none, *this);
                            break;
                    }
                }
                else
                {
                    handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag_type::none, *this);
                }
                break;
            }
            case cbor_major_type::text_string:
            {
                if (ec)
                {
                    return;
                }
                semantic_tag_type tag = semantic_tag_type::none;
                if (has_cbor_tag)
                {
                    switch (cbor_tag)
                    {
                        case 0:
                            tag = semantic_tag_type::date_time;
                            break;
                        case 32:
                            tag = semantic_tag_type::uri;
                            break;
                        case 33:
                            tag = semantic_tag_type::base64url;
                            break;
                        case 34:
                            tag = semantic_tag_type::base64;
                            break;
                        default:
                            break;
                    }
                }
                std::string s = jsoncons::cbor::detail::get_text_string(source_, ec);
                auto result = unicons::validate(s.begin(),s.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    return;
                }
                handler_.string_value(basic_string_view<char>(s.data(),s.length()), tag, *this);
                break;
            }
            case cbor_major_type::array:
            {
                semantic_tag_type tag = semantic_tag_type::none;
                if (has_cbor_tag)
                {
                    switch (cbor_tag)
                    {
                        case 0x04:
                            tag = semantic_tag_type::big_decimal;
                            break;
                        case 0x05:
                            tag = semantic_tag_type::big_float;
                            break;
                        default:
                            break;
                    }
                }
                if (tag == semantic_tag_type::big_decimal)
                {
                    std::string s = jsoncons::cbor::detail::get_array_as_decimal_string(source_, ec);
                    if (ec)
                    {
                        return;
                    }
                    handler_.string_value(s, semantic_tag_type::big_decimal);
                }
                else
                {
                    switch (info)
                    {
                        case additional_info::indefinite_length:
                        {
                            ++nesting_depth_;
                            handler_.begin_array(tag, *this);
                            source_.ignore(1);
                            bool done = false;
                            while (!done)
                            {
                                int test = source_.peek();
                                switch (test)
                                {
                                    case Source::traits_type::eof():
                                        ec = cbor_errc::unexpected_eof;
                                        return;
                                    case 0xff:
                                        done = true;
                                        break;
                                    default:
                                        read(ec);
                                        if (ec)
                                        {
                                            return;
                                        }
                                        break;
                                }
                            }
                            source_.ignore(1);
                            handler_.end_array(*this);
                            --nesting_depth_;
                            break;
                        }
                        default: // definite length
                        {
                            size_t len = jsoncons::cbor::detail::get_length(source_,ec);
                            if (ec)
                            {
                                return;
                            }
                            ++nesting_depth_;
                            handler_.begin_array(len, tag, *this);
                            for (size_t i = 0; i < len; ++i)
                            {
                                read(ec);
                                if (ec)
                                {
                                    return;
                                }
                            }
                            handler_.end_array(*this);
                            --nesting_depth_;
                            break;
                        }
                    }
                }
                break;
            }
            case cbor_major_type::map:
            {
                switch (info)
                {
                    case additional_info::indefinite_length: 
                    {
                        ++nesting_depth_;
                        handler_.begin_object(semantic_tag_type::none, *this);
                        source_.ignore(1);
                        bool done = false;
                        while (!done)
                        {
                            int test = source_.peek();
                            switch (test)
                            {
                                case Source::traits_type::eof():
                                    ec = cbor_errc::unexpected_eof;
                                    return;
                                case 0xff:
                                    done = true;
                                    break;
                                default:
                                    read_name(ec);
                                    if (ec)
                                    {
                                        return;
                                    }
                                    read(ec);
                                    if (ec)
                                    {
                                        return;
                                    }
                                    break;
                            }
                        }
                        source_.ignore(1);
                        handler_.end_object(*this);
                        --nesting_depth_;
                        break;
                    }
                    default: // definite_length
                    {
                        size_t len = jsoncons::cbor::detail::get_length(source_, ec);
                        if (ec)
                        {
                            return;
                        }
                        ++nesting_depth_;
                        handler_.begin_object(len, semantic_tag_type::none, *this);
                        for (size_t i = 0; i < len; ++i)
                        {
                            read_name(ec);
                            if (ec)
                            {
                                return;
                            }
                            read(ec);
                            if (ec)
                            {
                                return;
                            }
                        }
                        handler_.end_object(*this);
                        --nesting_depth_;
                        break;
                    }
                }
                break;
            }
            case cbor_major_type::semantic_tag:
            {
                break;
            }
            case cbor_major_type::simple:
            {
                switch (info)
                {
                    case 0x14:
                        handler_.bool_value(false, semantic_tag_type::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x15:
                        handler_.bool_value(true, semantic_tag_type::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x16:
                        handler_.null_value(semantic_tag_type::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x17:
                        handler_.null_value(semantic_tag_type::undefined, *this);
                        source_.ignore(1);
                        break;
                    case 0x19: // Half-Precision Float (two-byte IEEE 754)
                    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
                    case 0x1b: // Double-Precision Float (eight-byte IEEE 754)
                        double val = jsoncons::cbor::detail::get_double(source_, ec);
                        if (ec)
                        {
                            return;
                        }
                        if (has_cbor_tag && cbor_tag == 1)
                        {
                            handler_.double_value(val, semantic_tag_type::timestamp, *this);
                        }
                        else
                        {
                            handler_.double_value(val, semantic_tag_type::none, *this);
                        }
                        break;
                }
                break;
            }
        }
        if (nesting_depth_ == 0)
        {
            handler_.flush();
        }
    }

    void read_name(std::error_code& ec)
    {
        cbor_major_type major_type;
        int c = source_.peek();
        switch (c)
        {
            case Source::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                return;
            default:
                major_type = get_major_type((uint8_t)c);
                break;
        }
        switch (major_type)
        {
            case cbor_major_type::text_string:
            {
                std::string s = jsoncons::cbor::detail::get_text_string(source_,ec);
                if (ec)
                {
                    return;
                }
                auto result = unicons::validate(s.begin(),s.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    return;
                }
                handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
                break;
            }
            case cbor_major_type::byte_string:
            {
                std::vector<uint8_t> v = jsoncons::cbor::detail::get_byte_string(source_,ec);
                if (ec)
                {
                    return;
                }
                std::string s;
                encode_base64url(v.data(),v.size(),s);
                handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
                break;
            }
            default:
            {
                std::string s;
                json_string_serializer serializer(s);
                basic_cbor_reader<Source> reader(std::move(source_), serializer);
                reader.read(ec);
                source_ = std::move(reader.source_);
                auto result = unicons::validate(s.begin(),s.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    return;
                }
                handler_.name(basic_string_view<char>(s.data(),s.length()), *this);
            }
        }
    }
};

typedef basic_cbor_reader<jsoncons::binary_stream_source> cbor_reader;

typedef basic_cbor_reader<jsoncons::buffer_source> cbor_buffer_reader;

}}

#endif
