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

// 0x00..0x17 (0..23)
#define JSONCONS_CBOR_0x00_0x17 \
        0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x09:case 0x0a:case 0x0b:case 0x0c:case 0x0d:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:case 0x17

namespace jsoncons { namespace cbor {

template <class Source>
class basic_cbor_reader : public ser_context
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
        catch (const ser_error& e)
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
        std::vector<uint64_t> tags; 

        if (source_.is_error())
        {
            ec = cbor_errc::source_error;
            return;
        }   

        jsoncons::cbor::detail::cbor_major_type major_type;
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

        while (major_type == jsoncons::cbor::detail::cbor_major_type::semantic_tag)
        {
            uint64_t val = get_uint64_value(source_, ec);
            if (ec)
            {
                return;
            }
            tags.push_back(val);
            //source_.ignore(1);
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
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(source_, ec);
                if (ec)
                {
                    return;
                }

                if (!tags.empty() && tags.back() == 1)
                {
                    handler_.uint64_value(val, semantic_tag::timestamp, *this);
                }
                else
                {
                    handler_.uint64_value(val, semantic_tag::none, *this);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(source_, ec);
                if (ec)
                {
                    return;
                }
                if (!tags.empty() && tags.back() == 1)
                {
                    handler_.int64_value(val, semantic_tag::timestamp, *this);
                }
                else 
                {
                    handler_.int64_value(val, semantic_tag::none, *this);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                std::vector<uint8_t> v = get_byte_string(source_, ec);
                if (ec)
                {
                    return;
                }

                if (!tags.empty())
                {
                    switch (tags.back())
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
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag::base64url, *this);
                                break;
                            }
                        case 0x16:
                            {
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag::base64, *this);
                                break;
                            }
                        case 0x17:
                            {
                                handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag::base16, *this);
                                break;
                            }
                        default:
                            handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag::none, *this);
                            break;
                    }
                }
                else
                {
                    handler_.byte_string_value(byte_string_view(v.data(), v.size()), semantic_tag::none, *this);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                if (ec)
                {
                    return;
                }
                semantic_tag tag = semantic_tag::none;
                if (!tags.empty())
                {
                    switch (tags.back())
                    {
                        case 0:
                            tag = semantic_tag::date_time;
                            break;
                        case 32:
                            tag = semantic_tag::uri;
                            break;
                        case 33:
                            tag = semantic_tag::base64url;
                            break;
                        case 34:
                            tag = semantic_tag::base64;
                            break;
                        default:
                            break;
                    }
                }
                std::string s = get_text_string(source_, ec);
                auto result = unicons::validate(s.begin(),s.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    return;
                }
                handler_.string_value(basic_string_view<char>(s.data(),s.length()), tag, *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::array:
            {
                semantic_tag tag = semantic_tag::none;
                if (!tags.empty())
                {
                    switch (tags.back())
                    {
                        case 0x04:
                            tag = semantic_tag::big_decimal;
                            break;
                        case 0x05:
                            tag = semantic_tag::big_float;
                            break;
                        default:
                            break;
                    }
                }
                if (tag == semantic_tag::big_decimal)
                {
                    std::string s = get_array_as_decimal_string(source_, ec);
                    if (ec)
                    {
                        return;
                    }
                    handler_.string_value(s, semantic_tag::big_decimal);
                }
                else
                {
                    switch (info)
                    {
                        case jsoncons::cbor::detail::additional_info::indefinite_length:
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
                            size_t len = get_definite_length(source_,ec);
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
            case jsoncons::cbor::detail::cbor_major_type::map:
            {
                switch (info)
                {
                    case jsoncons::cbor::detail::additional_info::indefinite_length: 
                    {
                        ++nesting_depth_;
                        handler_.begin_object(semantic_tag::none, *this);
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
                        size_t len = get_definite_length(source_, ec);
                        if (ec)
                        {
                            return;
                        }
                        ++nesting_depth_;
                        handler_.begin_object(len, semantic_tag::none, *this);
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
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::simple:
            {
                switch (info)
                {
                    case 0x14:
                        handler_.bool_value(false, semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x15:
                        handler_.bool_value(true, semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x16:
                        handler_.null_value(semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x17:
                        handler_.null_value(semantic_tag::undefined, *this);
                        source_.ignore(1);
                        break;
                    case 0x19: // Half-Precision Float (two-byte IEEE 754)
                    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
                    case 0x1b: // Double-Precision Float (eight-byte IEEE 754)
                        double val = get_double(source_, ec);
                        if (ec)
                        {
                            return;
                        }
                        if (!tags.empty() && tags.back() == 1)
                        {
                            handler_.double_value(val, semantic_tag::timestamp, *this);
                        }
                        else
                        {
                            handler_.double_value(val, semantic_tag::none, *this);
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
        jsoncons::cbor::detail::cbor_major_type major_type;
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
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                std::string s = get_text_string(source_,ec);
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
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                std::vector<uint8_t> v = get_byte_string(source_,ec);
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

    static std::string get_text_string(Source& source, std::error_code& ec)
    {
        std::string s;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source.peek();
        switch (c)
        {
            case Source::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                return s;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string);
        auto func = [&s](Source& source, size_t length, std::error_code& ec)
        {
            s.reserve(s.size()+length);
            source.read(std::back_inserter(s), length);
            if (source.eof())
            {
                ec = cbor_errc::unexpected_eof;
                return;
            }
        };
        iterate_string_chunks(source, func, ec);
        
        return s;
    }

    static size_t get_definite_length(Source& source, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(source.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            return 0;
        }
        switch (get_major_type((uint8_t)source.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            case jsoncons::cbor::detail::cbor_major_type::array:
            case jsoncons::cbor::detail::cbor_major_type::map:
                break;
            default:
                return 0;
        }

        uint64_t u = get_uint64_value(source, ec);
        size_t len = (size_t)u;
        if (len != u)
        {
            ec = cbor_errc::number_too_large;
        }
        return len;
    }

    static std::vector<uint8_t> get_byte_string(Source& source, std::error_code& ec)
    {
        std::vector<uint8_t> v;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source.peek();
        switch (c)
        {
            case Source::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                return v;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);
        auto func = [&v](Source& source, size_t length, std::error_code& ec)
        {
            v.reserve(v.size()+length);
            source.read(std::back_inserter(v), length);
            if (source.eof())
            {
                ec = cbor_errc::unexpected_eof;
                return;
            }
        };
        iterate_string_chunks(source, func, ec);
        return v;
    }

    template <class Function>
    static void iterate_string_chunks(Source& source,
                                      Function func, 
                                      std::error_code& ec)
    {
        int c = source.peek();
        if (c == Source::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            return;
        }

        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string || major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);
        uint8_t info = get_additional_information_value((uint8_t)c);

        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                source.ignore(1);
                bool done = false;
                while (!done)
                {
                    int test = source.peek();
                    switch (test)
                    {
                        case Source::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            return;
                        case 0xff:
                            done = true;
                            break;
                        default:
                            iterate_string_chunks(source, func, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                    }
                }
                source.ignore(1);
                break;
            }
            default: // definite length
            {
                size_t length = get_definite_length(source, ec);
                if (ec)
                {
                    return;
                }
                func(source, length, ec);
                if (ec)
                {
                    return;
                }
                break;
            }
        }
    }

    static uint64_t get_uint64_value(Source& source, std::error_code& ec)
    {
        uint64_t val = 0;
        if (JSONCONS_UNLIKELY(source.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
            case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            {
                val = info;
                break;
            }

            case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                uint8_t c{};
                source.get(c);
                if (source.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return val;
                }
                val = c;
                break;
            }

            case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                uint8_t buf[sizeof(uint16_t)];
                source.read(buf, sizeof(uint16_t));
                val = jsoncons::detail::from_big_endian<uint16_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                uint8_t buf[sizeof(uint32_t)];
                source.read(buf, sizeof(uint32_t));
                val = jsoncons::detail::from_big_endian<uint32_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                uint8_t buf[sizeof(uint64_t)];
                source.read(buf, sizeof(uint64_t));
                val = jsoncons::detail::from_big_endian<uint64_t>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        return val;
    }

    static int64_t get_int64_value(Source& source, std::error_code& ec)
    {
        int64_t val = 0;
        if (JSONCONS_UNLIKELY(source.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t info = get_additional_information_value((uint8_t)source.peek());
        switch (get_major_type((uint8_t)source.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
                source.ignore(1);
                switch (info)
                {
                    case JSONCONS_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
                    {
                        val = static_cast<int8_t>(- 1 - info);
                        break;
                    }
                    case 0x18: // Negative integer (one-byte uint8_t follows)
                        {
                            uint8_t c{};
                            source.get(c);
                            if (source.eof())
                            {
                                ec = cbor_errc::unexpected_eof;
                                return val;
                            }
                            val = static_cast<int64_t>(-1)- c;
                            break;
                        }

                    case 0x19: // Negative integer -1-n (two-byte uint16_t follows)
                        {
                            uint8_t buf[sizeof(uint16_t)];
                            if (source.read(buf, sizeof(uint16_t)) != sizeof(uint16_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::from_big_endian<uint16_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1a: // Negative integer -1-n (four-byte uint32_t follows)
                        {
                            uint8_t buf[sizeof(uint32_t)];
                            if (source.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::from_big_endian<uint32_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1b: // Negative integer -1-n (eight-byte uint64_t follows)
                        {
                            uint8_t buf[sizeof(uint64_t)];
                            if (source.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::from_big_endian<uint64_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                            break;
                        }
                }
                break;

                case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
                {
                    uint64_t x = get_uint64_value(source, ec);
                    if (ec)
                    {
                        return 0;
                    }
                    if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                    {
                        val = x;
                    }
                    else
                    {
                        // error;
                    }
                    
                    break;
                }
                break;
            default:
                break;
        }

        return val;
    }

    static double get_double(Source& source, std::error_code& ec)
    {
        double val = 0;
        if (JSONCONS_UNLIKELY(source.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
        case 0x19: // Half-Precision Float (two-byte IEEE 754)
            {
                uint8_t buf[sizeof(uint16_t)];
                source.read(buf, sizeof(uint16_t));
                if (source.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return 0;
                }
                uint16_t x = jsoncons::detail::from_big_endian<uint16_t>(buf,buf+sizeof(buf),&endp);
                val = jsoncons::detail::decode_half(x);
                break;
            }


        case 0x1a: // Single-Precision Float (four-byte IEEE 754)
            {
                uint8_t buf[sizeof(float)];
                source.read(buf, sizeof(float));
                if (source.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return 0;
                }
                val = jsoncons::detail::from_big_endian<float>(buf,buf+sizeof(buf),&endp);
                break;
            }

        case 0x1b: //  Double-Precision Float (eight-byte IEEE 754)
            {
                uint8_t buf[sizeof(double)];
                source.read(buf, sizeof(double));
                if (source.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return 0;
                }
                val = jsoncons::detail::from_big_endian<double>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        
        return val;
    }

    static std::string get_array_as_decimal_string(Source& source, std::error_code& ec)
    {
        std::string s;
        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;

        int c;
        if ((c=source.get()) == Source::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            return s;
        }
        major_type = get_major_type((uint8_t)c);
        info = get_additional_information_value((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        JSONCONS_ASSERT(info == 2);

        if ((c=source.peek()) == Source::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            return s;
        }
        int64_t exponent = 0;
        switch (get_major_type((uint8_t)c))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = get_uint64_value(source,ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = get_int64_value(source,ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_decimal;
                return s;
            }
        }

        switch (get_major_type((uint8_t)source.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(source,ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::string_result<std::string> writer(s);
                jsoncons::detail::print_uinteger(val, writer);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(source,ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::string_result<std::string> writer(s);
                jsoncons::detail::print_integer(val, writer);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                if ((c=source.get()) == Source::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return s;
                }
                uint8_t tag = get_additional_information_value((uint8_t)c);
                if ((c=source.peek()) == Source::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    return s;
                }

                if (get_major_type((uint8_t)c) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    std::vector<uint8_t> v = get_byte_string(source,ec);
                    if (ec)
                    {
                        return s;
                    }
                    if (tag == 2)
                    {
                        bignum n(1, v.data(), v.size());
                        n.dump(s);
                    }
                    else if (tag == 3)
                    {
                        bignum n(-1, v.data(), v.size());
                        n.dump(s);
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_decimal;
                return s;
            }
        }

        std::string result;
        if (s.size() > 0)
        {
            if (s[0] == '-')
            {
                result.push_back('-');
                jsoncons::detail::prettify_string(s.c_str()+1, s.size()-1, (int)exponent, -4, 17, result);
            }
            else
            {
                jsoncons::detail::prettify_string(s.c_str(), s.size(), (int)exponent, -4, 17, result);
            }
        }
        //std::cout << "s: " << s << ", exponent: " << std::dec << exponent << ", result: " << result << "\n";
        return result;
    }

    static jsoncons::cbor::detail::cbor_major_type get_major_type(uint8_t type)
    {
        static const uint8_t major_type_shift = 0x05;
        uint8_t value = type >> major_type_shift;
        return static_cast<jsoncons::cbor::detail::cbor_major_type>(value);
    }

    static uint8_t get_additional_information_value(uint8_t type)
    {
        static const uint8_t additional_information_mask = (1U << 5) - 1;
        uint8_t value = type & additional_information_mask;
        return value;
    }
};

typedef basic_cbor_reader<jsoncons::binary_stream_source> cbor_reader;

typedef basic_cbor_reader<jsoncons::buffer_source> cbor_buffer_reader;

}}

#endif
