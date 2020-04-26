// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_VISITOR_HPP
#define JSONCONS_CBOR_CBOR_VISITOR_HPP

#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_encoder.hpp>

namespace jsoncons { 
namespace cbor {

    template <class CharT>
    class basic_cbor_visitor_adaptor;

    template <class CharT>
    class basic_cbor_visitor 
    {
        friend class basic_cbor_visitor_adaptor<CharT>;
    public:
        using char_type = CharT;
        using char_traits_type = std::char_traits<char_type>;

        using string_view_type = basic_string_view<char_type,char_traits_type>;

        basic_cbor_visitor(basic_cbor_visitor&&) = default;

        basic_cbor_visitor& operator=(basic_cbor_visitor&&) = default;

        basic_cbor_visitor() = default;

        virtual ~basic_cbor_visitor() noexcept = default;

        void flush()
        {
            visit_flush();
        }

        bool begin_object(semantic_tag tag=semantic_tag::none,
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_object(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_object(std::size_t length, 
                          semantic_tag tag=semantic_tag::none, 
                          const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_object(length, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_object(const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_end_object(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_array(semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_array(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_array(std::size_t length, 
                         semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_array(length, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_array(const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_end_array(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool key(const string_view_type& name, const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_string(name, semantic_tag::none, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool null_value(semantic_tag tag = semantic_tag::none,
                        const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_null(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool bool_value(bool value, 
                        semantic_tag tag = semantic_tag::none,
                        const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_bool(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool string_value(const string_view_type& value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_string(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool byte_string_value(const byte_string_view& b, 
                               semantic_tag tag=semantic_tag::none, 
                               const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_byte_string(b, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool byte_string_value(const uint8_t* p, std::size_t size, 
                               semantic_tag tag=semantic_tag::none, 
                               const ser_context& context=ser_context())
        {
            return byte_string_value(byte_string(p, size), tag, context);
        }

        bool uint64_value(uint64_t value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_uint64(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool int64_value(int64_t value, 
                         semantic_tag tag = semantic_tag::none, 
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_int64(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool half_value(uint16_t value, 
                        semantic_tag tag = semantic_tag::none, 
                        const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_half(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool double_value(double value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_double(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_object(semantic_tag tag,
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_begin_object(tag, context, ec);
        }

        bool begin_object(std::size_t length, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_begin_object(length, tag, context, ec);
        }

        bool end_object(const ser_context& context, std::error_code& ec)
        {
            return visit_end_object(context, ec);
        }

        bool begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            return visit_begin_array(tag, context, ec);
        }

        bool begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            return visit_begin_array(length, tag, context, ec);
        }

        bool end_array(const ser_context& context, std::error_code& ec)
        {
            return visit_end_array(context, ec);
        }

        bool key(const string_view_type& name, const ser_context& context, std::error_code& ec)
        {
            return visit_string(name, semantic_tag::none, context, ec);
        }

        bool null_value(semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) 
        {
            return visit_null(tag, context, ec);
        }

        bool bool_value(bool value, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) 
        {
            return visit_bool(value, tag, context, ec);
        }

        bool string_value(const string_view_type& value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec) 
        {
            return visit_string(value, tag, context, ec);
        }

        bool byte_string_value(const byte_string_view& b, 
                               semantic_tag tag, 
                               const ser_context& context,
                               std::error_code& ec)
        {
            return visit_byte_string(b, tag, context, ec);
        }

        bool byte_string_value(const uint8_t* p, std::size_t size, 
                               semantic_tag tag, 
                               const ser_context& context,
                               std::error_code& ec)
        {
            return byte_string_value(byte_string(p, size), tag, context, ec);
        }

        bool uint64_value(uint64_t value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_uint64(value, tag, context, ec);
        }

        bool int64_value(int64_t value, 
                         semantic_tag tag, 
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_int64(value, tag, context, ec);
        }

        bool half_value(uint16_t value, 
                        semantic_tag tag, 
                        const ser_context& context,
                        std::error_code& ec)
        {
            return visit_half(value, tag, context, ec);
        }

        bool double_value(double value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_double(value, tag, context, ec);
        }

        template <class T>
        bool typed_array(const span<T>& data, 
                         semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_typed_array(data, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        template <class T>
        bool typed_array(const span<T>& data, 
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_typed_array(data, tag, context, ec);
        }

        bool typed_array(half_arg_t, const span<const uint16_t>& s,
            semantic_tag tag = semantic_tag::none,
            const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_typed_array(half_arg, s, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool typed_array(half_arg_t, const span<const uint16_t>& s,
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_typed_array(half_arg, s, tag, context, ec);
        }

        bool begin_multi_dim(const span<const size_t>& shape,
                             semantic_tag tag = semantic_tag::multi_dim_row_major,
                             const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_begin_multi_dim(shape, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_multi_dim(const span<const size_t>& shape,
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec) 
        {
            return visit_begin_multi_dim(shape, tag, context, ec);
        }

        bool end_multi_dim(const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_end_multi_dim(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_multi_dim(const ser_context& context,
                           std::error_code& ec) 
        {
            return visit_end_multi_dim(context, ec);
        }

    #if !defined(JSONCONS_NO_DEPRECATED)

        JSONCONS_DEPRECATED_MSG("Instead, use key(const string_view_type&, const ser_context&=ser_context())") 
        bool name(const string_view_type& name, const ser_context& context=ser_context())
        {
            return key(name, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use key(const string_view_type&, const ser_context&, std::error_code&)") 
        bool name(const string_view_type& name, const ser_context& context, std::error_code& ec)
        {
            return key(name, context, ec);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use byte_string_value(const byte_string_view&, semantic_tag=semantic_tag::none, const ser_context&=ser_context()") 
        bool byte_string_value(const byte_string_view& b, 
                               byte_string_chars_format encoding_hint, 
                               semantic_tag tag=semantic_tag::none, 
                               const ser_context& context=ser_context())
        {
            switch (encoding_hint)
            {
                case byte_string_chars_format::base16:
                    tag = semantic_tag::base16;
                    break;
                case byte_string_chars_format::base64:
                    tag = semantic_tag::base64;
                    break;
                case byte_string_chars_format::base64url:
                    tag = semantic_tag::base64url;
                    break;
                default:
                    break;
            }
            return byte_string_value(b, tag, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use byte_string_value(const byte_string_view&, semantic_tag=semantic_tag::none, const ser_context&=ser_context()") 
        bool byte_string_value(const uint8_t* p, std::size_t size, 
                               byte_string_chars_format encoding_hint, 
                               semantic_tag tag=semantic_tag::none, 
                               const ser_context& context=ser_context())
        {
            switch (encoding_hint)
            {
                case byte_string_chars_format::base16:
                    tag = semantic_tag::base16;
                    break;
                case byte_string_chars_format::base64:
                    tag = semantic_tag::base64;
                    break;
                case byte_string_chars_format::base64url:
                    tag = semantic_tag::base64url;
                    break;
                default:
                    break;
            }
            return byte_string_value(byte_string(p, size), tag, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigint") 
        bool big_integer_value(const string_view_type& value, const ser_context& context=ser_context()) 
        {
            return string_value(value, semantic_tag::bigint, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigdec") 
        bool big_decimal_value(const string_view_type& value, const ser_context& context=ser_context()) 
        {
            return string_value(value, semantic_tag::bigdec, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::datetime") 
        bool date_time_value(const string_view_type& value, const ser_context& context=ser_context()) 
        {
            return string_value(value, semantic_tag::datetime, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use int64_value with semantic_tag::timestamp") 
        bool timestamp_value(int64_t val, const ser_context& context=ser_context()) 
        {
            return int64_value(val, semantic_tag::timestamp, context);
        }

        JSONCONS_DEPRECATED_MSG("Remove calls to this method, it doesn't do anything") 
        bool begin_document()
        {
            return true;
        }

        JSONCONS_DEPRECATED_MSG("Instead, use flush() when serializing") 
        bool end_document()
        {
            flush();
            return true;
        }

        JSONCONS_DEPRECATED_MSG("Remove calls to this method, it doesn't do anything") 
        void begin_json()
        {
        }

        JSONCONS_DEPRECATED_MSG("Instead, use flush() when serializing") 
        void end_json()
        {
            end_document();
        }

        JSONCONS_DEPRECATED_MSG("Instead, use key(const string_view_type&, const ser_context&=ser_context())") 
        void name(const char_type* p, std::size_t length, const ser_context& context) 
        {
            name(string_view_type(p, length), context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=ser_context())") 
        void integer_value(int64_t value)
        {
            int64_value(value);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use int64_value(int64_t, semantic_tag = semantic_tag::none, const ser_context&=ser_context())") 
        void integer_value(int64_t value, const ser_context& context)
        {
            int64_value(value,context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=ser_context())") 
        void uinteger_value(uint64_t value)
        {
            uint64_value(value);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=ser_context())") 
        void uinteger_value(uint64_t value, const ser_context& context)
        {
            uint64_value(value,context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigint") 
        bool bignum_value(const string_view_type& value, const ser_context& context=ser_context()) 
        {
            return string_value(value, semantic_tag::bigint, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigdec") 
        bool decimal_value(const string_view_type& value, const ser_context& context=ser_context()) 
        {
            return string_value(value, semantic_tag::bigdec, context);
        }

        JSONCONS_DEPRECATED_MSG("Instead, use int64_value with semantic_tag::timestamp") 
        bool epoch_time_value(int64_t val, const ser_context& context=ser_context()) 
        {
            return int64_value(val, semantic_tag::timestamp, context);
        }

    #endif
    private:

        virtual void visit_flush() = 0;

        virtual bool visit_begin_object(semantic_tag tag, 
                                     const ser_context& context, 
                                     std::error_code& ec) = 0;

        virtual bool visit_begin_object(std::size_t /*length*/, 
                                     semantic_tag tag, 
                                     const ser_context& context, 
                                     std::error_code& ec)
        {
            return visit_begin_object(tag, context, ec);
        }

        virtual bool visit_end_object(const ser_context& context, 
                                   std::error_code& ec) = 0;

        virtual bool visit_begin_array(semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec) = 0;

        virtual bool visit_begin_array(std::size_t /*length*/, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec)
        {
            return visit_begin_array(tag, context, ec);
        }

        virtual bool visit_end_array(const ser_context& context, 
                                  std::error_code& ec) = 0;

        virtual bool visit_null(semantic_tag tag, 
                             const ser_context& context, 
                             std::error_code& ec) = 0;

        virtual bool visit_bool(bool value, 
                             semantic_tag tag, 
                             const ser_context& context, 
                             std::error_code&) = 0;

        virtual bool visit_string(const string_view_type& value, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) = 0;

        virtual bool visit_byte_string(const byte_string_view& value, 
                                    semantic_tag tag, 
                                    const ser_context& context,
                                    std::error_code& ec) = 0;

        virtual bool visit_uint64(uint64_t value, 
                               semantic_tag tag, 
                               const ser_context& context,
                               std::error_code& ec) = 0;

        virtual bool visit_int64(int64_t value, 
                              semantic_tag tag,
                              const ser_context& context,
                              std::error_code& ec) = 0;

        virtual bool visit_half(uint16_t value, 
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec)
        {
            return visit_double(jsoncons::detail::decode_half(value),
                             tag,
                             context,
                             ec);
        }

        virtual bool visit_double(double value, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec) = 0;

        virtual bool visit_typed_array(const span<const uint8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const uint32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) 
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const uint64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) 
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const int8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const int16_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const int32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const int64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(half_arg_t, 
                                    const span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = half_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const float>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = double_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const span<const double>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = double_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_begin_multi_dim(const span<const size_t>& shape,
                                        semantic_tag tag,
                                        const ser_context& context, 
                                        std::error_code& ec) 
        {
            bool more = visit_begin_array(2, tag, context, ec);
            if (more)
            {
                more = visit_begin_array(shape.size(), tag, context, ec);
                for (auto it = shape.begin(); more && it != shape.end(); ++it)
                {
                    visit_uint64(*it, semantic_tag::none, context, ec);
                }
                if (more)
                {
                    more = visit_end_array(context, ec);
                }
            }
            return more;
        }

        virtual bool visit_end_multi_dim(const ser_context& context,
                                      std::error_code& ec) 
        {
            return visit_end_array(context, ec);
        }
    };

    template <class CharT>
    class basic_cbor_visitor_adaptor : public basic_cbor_visitor<CharT>
    {
        JSONCONS_STRING_LITERAL(null_literal,'n','u','l','l')
        JSONCONS_STRING_LITERAL(true_literal,'t','r','u','e')
        JSONCONS_STRING_LITERAL(false_literal,'f','a','l','s','e')

    public:
        using typename basic_cbor_visitor<CharT>::char_type;
        using typename basic_cbor_visitor<CharT>::string_view_type;
    private:

        using string_type = std::basic_string<CharT>;

        enum class level_state {value=1, key};

        struct level
        {
            level_state state;
            bool is_object;
            bool is_key;

            level(level_state state, bool is_object)
                : state(state), is_object(is_object), is_key(is_object)
            {
            }

            level(level&&) = default;
            level& operator=(level&&) = default;

            void toggle_is_odd()
            {
                if (is_object)
                {
                    is_key = !is_key;
                }
            }
        };

        basic_json_visitor<char_type>& destination_;
        string_type key_;
        string_type key_buffer_;
        std::vector<level> level_stack_;

        // noncopyable and nonmoveable
        basic_cbor_visitor_adaptor(const basic_cbor_visitor_adaptor&) = delete;
        basic_cbor_visitor_adaptor& operator=(const basic_cbor_visitor_adaptor&) = delete;
    public:
        basic_cbor_visitor_adaptor(basic_json_visitor<char_type>& visitor)
            : destination_(visitor)
        {
            level_stack_.emplace_back(level_state(),false); // root
        }

        basic_json_visitor<char_type>& destination()
        {
            return destination_;
        }

    private:
        void visit_flush() override
        {
            destination_.flush();
        }

        bool visit_begin_object(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::value, true);
                        return destination_.begin_object(tag, context, ec);
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        level_stack_.back() = level(level_state::value,true);
                        return destination_.begin_object(tag, context, ec);
                }
            }

        }

        bool visit_begin_object(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::value, true);
                        return destination_.begin_object(length, tag, context, ec);
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, true);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        level_stack_.back() = level(level_state::value,true);
                        return destination_.begin_object(length, tag, context, ec);
                }
            }

        }

        bool visit_end_object(const ser_context& context, std::error_code& ec) override
        {
            switch (level_stack_.back().state)
            {
                case level_state::value:
                    level_stack_.pop_back();
                    return destination_.end_object(context, ec);
                case level_state::key:
                {
                    key_buffer_.push_back('}');
                    level_stack_.pop_back();

                    bool ret = true;
                    if (level_stack_.back().state == level_state::value)
                    {
                        ret = destination_.key(key_buffer_,context, ec);
                    }
                    else
                    {
                        key_buffer_.push_back(':');
                    }
                    return ret;
                }
                default:
                    return destination_.end_object(context, ec);
            }
        }

        bool visit_begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::key, false);
                        key_buffer_.push_back('[');
                        return true;
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, false);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::value, false);
                        return destination_.begin_array(tag, context, ec);
                    case level_state::key:
                        level_stack_.emplace_back(level_state::value, false);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        level_stack_.back() = level(level_state::value, false);
                        return destination_.begin_array(tag, context, ec);
                }
            }
        }

        bool visit_begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::key, false);
                        key_buffer_.push_back('[');
                        return true;
                    case level_state::key:
                        level_stack_.emplace_back(level_state::key, false);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        level_stack_.emplace_back(level_state::value, false);
                        return destination_.begin_array(length, tag, context, ec);
                    case level_state::key:
                        level_stack_.emplace_back(level_state::value, false);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        level_stack_.back() = level(level_state::value, false);
                        return destination_.begin_array(length, tag, context, ec);
                }
            }
        }

        bool visit_end_array(const ser_context& context, std::error_code& ec) override
        {
            switch (level_stack_.back().state)
            {
                case level_state::value:
                    level_stack_.pop_back();
                    return destination_.end_array(context, ec);
                case level_state::key:
                {
                    key_buffer_.push_back(']');
                    level_stack_.pop_back();
                    bool ret = true;
                    if (level_stack_.back().state == level_state::value)
                    {
                        ret = destination_.key(key_buffer_,context, ec);
                    }
                    else
                    {
                        key_buffer_.push_back(':');
                    }
                    return ret;
                }
                default:
                    return destination_.end_array(context, ec);
            }
        }

        bool visit_string(const string_view_type& value,
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();
            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(value, context, ec);
                    case level_state::key:
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), value.begin(), value.end());
                        key_buffer_.push_back('\"');
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), value.begin(), value.end());
                        key_buffer_.push_back('\"');
                        return true;
                    default:
                        return destination_.string_value(value, tag, context, ec);
                }
            }
        }

        bool visit_byte_string(const byte_string_view& value, 
                                  semantic_tag tag,
                                  const ser_context& context,
                                  std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_.clear();
                switch (tag)
                {
                    case semantic_tag::base64:
                        encode_base64(value.begin(), value.end(), key_);
                        break;
                    case semantic_tag::base16:
                        encode_base16(value.begin(), value.end(),key_);
                        break;
                    default:
                        encode_base64url(value.begin(), value.end(),key_);
                        break;
                }
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        return true;
                    default:
                        return destination_.byte_string_value(value, tag, context, ec);
                }
            }
        }

        bool visit_uint64(uint64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_.clear();
                jsoncons::detail::write_integer(value,key_);
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.uint64_value(value, tag, context, ec);
                }
            }
        }

        bool visit_int64(int64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_.clear();
                jsoncons::detail::write_integer(value,key_);
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.int64_value(value, tag, context, ec);
                }
            }
        }

        bool visit_half(uint16_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_.clear();
                jsoncons::string_sink<string_type> sink(key_);
                jsoncons::detail::write_double f{float_chars_format::general,0};
                double x = jsoncons::detail::decode_half(value);
                f(x, sink);
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.half_value(value, tag, context, ec);
                }
            }
        }

        bool visit_double(double value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_.clear();
                string_sink<string_type> sink(key_);
                jsoncons::detail::write_double f{float_chars_format::general,0};
                f(value, sink);
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.double_value(value, tag, context, ec);
                }
            }
        }

        bool visit_bool(bool value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_ = value ? string_type(true_literal<CharT>()) : string_type(false_literal<CharT>());
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.bool_value(value, tag, context, ec);
                }
            }
        }

        bool visit_null(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                key_ = string_type(null_literal<CharT>());
            }

            if (is_key)
            {
                switch (level_stack_.back().state)
                {
                    case level_state::value:
                        return destination_.key(key_, context, ec);
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        return true;
                    default:
                        JSONCONS_UNREACHABLE();
                }
            }
            else
            {
                switch (level_stack_.back().state)
                {
                    case level_state::key:
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        return true;
                    default:
                        return destination_.null_value(tag, context, ec);
                }
            }
        }

        bool visit_typed_array(const span<const uint8_t>& s, 
                               semantic_tag tag,
                               const ser_context& context, 
                               std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const uint32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const uint64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const int8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const int16_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const int32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const int64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(half_arg_t, 
                               const span<const uint16_t>& s, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(half_arg,s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(half_arg, s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const float>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const span<const double>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key;
            level_stack_.back().toggle_is_odd();

            if (is_key || level_stack_.back().state == level_state::key)
            {
                return basic_cbor_visitor<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_.typed_array(s, tag, context, ec);
            }
        }
    };

    using cbor_visitor = basic_cbor_visitor<char>;
    using cbor_visitor_adaptor = basic_cbor_visitor_adaptor<char>;


} // namespace cbor 
} // namespace jsoncons

#endif
