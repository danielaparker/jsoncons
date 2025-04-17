// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_VISITOR_HPP
#define JSONCONS_JSON_VISITOR_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_context.hpp>
#include <jsoncons/utility/bigint.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons {

    template <typename CharT>
    class basic_json_visitor
    {
    public:
        using char_type = CharT;
        using char_traits_type = std::char_traits<char_type>;

        using string_view_type = jsoncons::basic_string_view<char_type,char_traits_type>;

        basic_json_visitor() = default;

        virtual ~basic_json_visitor() = default;

        void flush()
        {
            visit_flush();
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_object(semantic_tag tag=semantic_tag::none,
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_begin_object(tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_object(std::size_t length, 
            semantic_tag tag=semantic_tag::none, 
            const ser_context& context = ser_context())
        {
            std::error_code ec;
            visit_begin_object(length, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_object(const ser_context& context = ser_context())
        {
            std::error_code ec;
            visit_end_object(context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_array(semantic_tag tag=semantic_tag::none,
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_begin_array(tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_array(std::size_t length, 
            semantic_tag tag=semantic_tag::none,
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_begin_array(length, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_array(const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_end_array(context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE key(const string_view_type& name, const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_key(name, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE null_value(semantic_tag tag = semantic_tag::none,
            const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            visit_null(tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE bool_value(bool value, 
            semantic_tag tag = semantic_tag::none,
            const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            visit_bool(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE string_value(const string_view_type& value, 
            semantic_tag tag = semantic_tag::none, 
            const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            visit_string(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        template <typename ByteStringLike>
        JSONCONS_VISITOR_RETURN_TYPE byte_string_value(const ByteStringLike& b, 
            semantic_tag tag=semantic_tag::none, 
            const ser_context& context=ser_context(),
            typename std::enable_if<ext_traits::is_byte_sequence<ByteStringLike>::value,int>::type = 0)
        {
            std::error_code ec;
            visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        template <typename ByteStringLike>
        JSONCONS_VISITOR_RETURN_TYPE byte_string_value(const ByteStringLike& b, 
            uint64_t ext_tag, 
            const ser_context& context=ser_context(),
            typename std::enable_if<ext_traits::is_byte_sequence<ByteStringLike>::value,int>::type = 0)
        {
            std::error_code ec;
            visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), ext_tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE uint64_value(uint64_t value, 
            semantic_tag tag = semantic_tag::none, 
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_uint64(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE int64_value(int64_t value, 
            semantic_tag tag = semantic_tag::none, 
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_int64(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE half_value(uint16_t value, 
            semantic_tag tag = semantic_tag::none, 
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_half(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE double_value(double value, 
            semantic_tag tag = semantic_tag::none, 
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_double(value, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_object(semantic_tag tag,
            const ser_context& context,
            std::error_code& ec)
        {
            visit_begin_object(tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_object(std::size_t length, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec)
        {
            visit_begin_object(length, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_object(const ser_context& context, std::error_code& ec)
        {
            visit_end_object(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            visit_begin_array(tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            visit_begin_array(length, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_array(const ser_context& context, std::error_code& ec)
        {
            visit_end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE key(const string_view_type& name, const ser_context& context, std::error_code& ec)
        {
            visit_key(name, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE null_value(semantic_tag tag,
            const ser_context& context,
            std::error_code& ec) 
        {
            visit_null(tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE bool_value(bool value, 
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec) 
        {
            visit_bool(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE string_value(const string_view_type& value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec) 
        {
            visit_string(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        template <typename Source>
        JSONCONS_VISITOR_RETURN_TYPE byte_string_value(const Source& b, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec,
            typename std::enable_if<ext_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        template <typename Source>
        JSONCONS_VISITOR_RETURN_TYPE byte_string_value(const Source& b, 
            uint64_t ext_tag, 
            const ser_context& context,
            std::error_code& ec,
            typename std::enable_if<ext_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), ext_tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE uint64_value(uint64_t value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec)
        {
            visit_uint64(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE int64_value(int64_t value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec)
        {
            visit_int64(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE half_value(uint16_t value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec)
        {
            visit_half(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE double_value(double value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec)
        {
            visit_double(value, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        template <typename T>
        JSONCONS_VISITOR_RETURN_TYPE typed_array(const jsoncons::span<T>& data, 
            semantic_tag tag=semantic_tag::none,
            const ser_context& context=ser_context())
        {
            std::error_code ec;
            visit_typed_array(data, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        template <typename T>
        JSONCONS_VISITOR_RETURN_TYPE typed_array(const jsoncons::span<T>& data, 
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec)
        {
            visit_typed_array(data, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE typed_array(half_arg_t, const jsoncons::span<const uint16_t>& s,
            semantic_tag tag = semantic_tag::none,
            const ser_context& context = ser_context())
        {
            std::error_code ec;
            visit_typed_array(half_arg, s, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE typed_array(half_arg_t, const jsoncons::span<const uint16_t>& s,
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec)
        {
            visit_typed_array(half_arg, s, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_multi_dim(const jsoncons::span<const size_t>& shape,
            semantic_tag tag = semantic_tag::multi_dim_row_major,
            const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            visit_begin_multi_dim(shape, tag, context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE begin_multi_dim(const jsoncons::span<const size_t>& shape,
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec) 
        {
            visit_begin_multi_dim(shape, tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_multi_dim(const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            visit_end_multi_dim(context, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE end_multi_dim(const ser_context& context,
                           std::error_code& ec) 
        {
            visit_end_multi_dim(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

    private:

        virtual void visit_flush() = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(std::size_t /*length*/, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec)
        {
            visit_begin_object(tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(std::size_t /*length*/, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec)
        {
            visit_begin_array(tag, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type& name, 
            const ser_context& context, 
            std::error_code&) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code&) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& value, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value, 
            uint64_t /* ext_tag */, 
            const ser_context& context,
            std::error_code& ec) 
        {
            visit_byte_string(value, semantic_tag::none, context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value, 
            semantic_tag tag, 
            const ser_context& context,
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value, 
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_half(uint16_t value, 
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec)
        {
            visit_double(binary::decode_half(value),
                tag,
                context,
                ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_double(double value, 
            semantic_tag tag,
            const ser_context& context,
            std::error_code& ec) = 0;

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const uint8_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                uint64_value(*p, semantic_tag::none, context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const uint16_t>& s, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                uint64_value(*p, semantic_tag::none, context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const uint32_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec) 
        {
            begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                uint64_value(*p, semantic_tag::none, context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const uint64_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec) 
        {
            begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                uint64_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const int8_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                int64_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const int16_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                int64_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const int32_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                int64_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const int64_t>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                int64_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(half_arg_t, 
            const jsoncons::span<const uint16_t>& s, 
            semantic_tag tag, 
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                half_value(*p, semantic_tag::none, context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const float>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                double_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_typed_array(const jsoncons::span<const double>& s, 
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec)  
        {
            begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); p != s.end(); ++p)
            {
                double_value(*p,semantic_tag::none,context, ec);
            }
            end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_begin_multi_dim(const jsoncons::span<const size_t>& shape,
            semantic_tag tag,
            const ser_context& context, 
            std::error_code& ec) 
        {
            visit_begin_array(2, tag, context, ec);
            visit_begin_array(shape.size(), tag, context, ec);
            for (auto it = shape.begin(); it != shape.end(); ++it)
            {
                visit_uint64(*it, semantic_tag::none, context, ec);
            }
            visit_end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }

        virtual JSONCONS_VISITOR_RETURN_TYPE visit_end_multi_dim(const ser_context& context,
            std::error_code& ec) 
        {
            visit_end_array(context, ec);
            JSONCONS_VISITOR_RETURN;
        }
    };

    template <typename CharT>
    class basic_default_json_visitor : public basic_json_visitor<CharT>
    {
    public:
        using typename basic_json_visitor<CharT>::string_view_type;

        basic_default_json_visitor() = default;
    private:
        void visit_flush() override
        {
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type&, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type&, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view&, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_half(uint16_t, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_double(double, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool, semantic_tag, const ser_context&, std::error_code&) override
        {
            JSONCONS_VISITOR_RETURN;
        }
    };

    template <typename CharT>
    class basic_json_diagnostics_visitor : public basic_default_json_visitor<CharT>
    {
    public:
        using stream_type = std::basic_ostream<CharT>;
        using string_type = std::basic_string<CharT>;

    private:
        using supertype = basic_default_json_visitor<CharT>;
        using string_view_type = typename supertype::string_view_type;

        struct enabler {};

        static constexpr CharT visit_begin_array_name[] = {'v','i','s','i','t','_','b','e','g','i','n','_','a','r','r','a','y', 0};
        static constexpr CharT visit_end_array_name[] =  {'v','i','s','i','t','_','e','n','d','_','a','r','r','a','y', 0};
        static constexpr CharT visit_begin_object_name[] =  {'v','i','s','i','t','_','b','e','g','i','n','_','o','b','j','e','c','t', 0};
        static constexpr CharT visit_end_object_name[] =  {'v','i','s','i','t','_','e','n','d','_','o','b','j','e','c','t', 0};
        static constexpr CharT visit_key_name[] =  {'v','i','s','i','t','_','k','e','y', 0};
        static constexpr CharT visit_string_name[] =  {'v','i','s','i','t','_','s','t','r','i','n','g', 0};
        static constexpr CharT visit_byte_string_name[] =  {'v','i','s','i','t','_','b','y','t','e','_','s','t','r','i','n','g', 0};
        static constexpr CharT visit_null_name[] =  {'v','i','s','i','t','_','n','u','l','l', 0};
        static constexpr CharT visit_bool_name[] =  {'v','i','s','i','t','_','b','o','o','l', 0};
        static constexpr CharT visit_uint64_name[] =  {'v','i','s','i','t','_','u','i','n','t','6','4', 0};
        static constexpr CharT visit_int64_name[] =  {'v','i','s','i','t','_','i','n','t','6','4', 0};
        static constexpr CharT visit_half_name[] =  {'v','i','s','i','t','_','h','a','l','f', 0};
        static constexpr CharT visit_double_name[] =  {'v','i','s','i','t','_','d','o','u','b','l','e', 0};

        static constexpr CharT separator_ = ':';

        stream_type& output_;
        string_type indentation_;
        long level_{0};

    public:
        // If CharT is char, then enable the default constructor which binds to
        // std::cout.
        template <typename U = enabler>
        basic_json_diagnostics_visitor(
            typename std::enable_if<std::is_same<CharT, char>::value, U>::type = enabler{})
            : basic_json_diagnostics_visitor(std::cout)
        {
        }

        // If CharT is wchar_t, then enable the default constructor which binds
        // to std::wcout.
        template <typename U = enabler>
        basic_json_diagnostics_visitor(
            typename std::enable_if<std::is_same<CharT, wchar_t>::value, U>::type = enabler{})
            : basic_json_diagnostics_visitor(std::wcout)
        {
        }

        explicit basic_json_diagnostics_visitor(
            stream_type& output,
            string_type indentation = string_type())
            : output_(output),
              indentation_(std::move(indentation))
        {
        }

    private:
        void indent()
        {
            for (long i=0; i<level_; ++i)
                output_ << indentation_;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_begin_object_name << '\n';
            ++level_;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(std::size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_begin_object_name << separator_ << length << '\n';
            ++level_;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) override
        {
            --level_;
            indent();
            output_ << visit_end_object_name << '\n';
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_begin_array_name << '\n';
            ++level_;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(std::size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_begin_array_name << separator_  << length << '\n';
            ++level_;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) override
        {
            --level_;
            indent();
            output_ << visit_end_array_name << '\n';
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type& s, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_key_name << separator_  << s << '\n';
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& s, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_string_name << separator_  << s << '\n';
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_int64_name << separator_  << val << '\n';
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_uint64_name << separator_ << val << '\n';
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_bool_name << separator_ << val << '\n';
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            indent();
            output_ << visit_null_name << '\n';
            JSONCONS_VISITOR_RETURN;
        }
    };

#if __cplusplus >= 201703L
// not needed for C++17
#else
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_begin_array_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_end_array_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_begin_object_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_end_object_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_key_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_string_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_byte_string_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_null_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_bool_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_uint64_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_int64_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_half_name[];
    template <typename C> constexpr C basic_json_diagnostics_visitor<C>::visit_double_name[];
#endif // C++17 check

    using json_visitor = basic_json_visitor<char>;
    using wjson_visitor = basic_json_visitor<wchar_t>;

    using default_json_visitor = basic_default_json_visitor<char>;
    using wdefault_json_visitor = basic_default_json_visitor<wchar_t>;

    using json_diagnostics_visitor = basic_json_diagnostics_visitor<char>;
    using wjson_diagnostics_visitor = basic_json_diagnostics_visitor<wchar_t>;

} // namespace jsoncons

#endif // JSONCONS_JSON_VISITOR_HPP
