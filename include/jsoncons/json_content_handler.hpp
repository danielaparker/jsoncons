// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONTENT_HANDLER_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons/serializing_context.hpp>
#include <jsoncons/json_options.hpp>

namespace jsoncons {

// null_type

struct null_type
{
};

enum class semantic_tag_type : uint8_t 
{
    none = 0,
    undefined,
    date_time,
    timestamp,
    big_integer,
    big_decimal,
    big_float
};

template <class CharT>
class basic_json_content_handler
{
#if !defined(JSONCONS_NO_DEPRECATED)
    std::basic_string<CharT> buffer_;
#endif
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view<char_type,char_traits_type> string_view_type;

    virtual ~basic_json_content_handler() {}

    void flush()
    {
        do_flush();
    }

    bool begin_object(semantic_tag_type tag=semantic_tag_type::none,
                      const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_begin_object(tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool begin_object(semantic_tag_type tag,
                      const serializing_context& context,
                      std::error_code& ec)
    {
        return do_begin_object(tag, context, ec);
    }

    bool begin_object(size_t length, 
                      semantic_tag_type tag=semantic_tag_type::none, 
                      const serializing_context& context = null_serializing_context())
    {
        std::error_code ec;
        bool res = do_begin_object(length, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool begin_object(size_t length, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec)
    {
        return do_begin_object(length, tag, context, ec);
    }

    bool end_object(const serializing_context& context = null_serializing_context())
    {
        std::error_code ec;
        bool res = do_end_object(context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool end_object(const serializing_context& context, std::error_code& ec)
    {
        return do_end_object(context, ec);
    }

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_begin_array(tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool begin_array(semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
    {
        return do_begin_array(tag, context, ec);
    }

    bool begin_array(size_t length, 
                     semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_begin_array(length, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool begin_array(size_t length, semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
    {
        return do_begin_array(length, tag, context, ec);
    }

    bool end_array(const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_end_array(context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool end_array(const serializing_context& context, std::error_code& ec)
    {
        return do_end_array(context, ec);
    }

    bool name(const string_view_type& name, const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_name(name, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool name(const string_view_type& name, const serializing_context& context, std::error_code& ec)
    {
        return do_name(name, context, ec);
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(value, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec) 
    {
        return do_string_value(value, tag, context, ec);
    }

    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_byte_string_value(b, encoding_hint, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint,
                           semantic_tag_type tag, 
                           const serializing_context& context,
                           std::error_code& ec)
    {
        return do_byte_string_value(b, encoding_hint, tag, context, ec);
    }

    bool byte_string_value(const uint8_t* p, size_t size, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_byte_string_value(byte_string(p, size), encoding_hint, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool big_integer_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(s, semantic_tag_type::big_integer, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool big_integer_value(const string_view_type& s, const serializing_context& context, std::error_code& ec) 
    {
        return do_string_value(s, semantic_tag_type::big_integer, context, ec);
    }

    bool big_decimal_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(s, semantic_tag_type::big_decimal, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool big_decimal_value(const string_view_type& s, const serializing_context& context, std::error_code& ec) 
    {
        return do_string_value(s, semantic_tag_type::big_decimal, context, ec);
    }

    bool date_time_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(s, semantic_tag_type::date_time, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool date_time_value(const string_view_type& s, const serializing_context& context, std::error_code& ec) 
    {
        return do_string_value(s, semantic_tag_type::date_time, context, ec);
    }

    bool timestamp_value(int64_t val, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_int64_value(val, semantic_tag_type::timestamp, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool timestamp_value(int64_t val, const serializing_context& context, std::error_code& ec) 
    {
        return do_int64_value(val, semantic_tag_type::timestamp, context, ec);
    }

    bool int64_value(int64_t value, 
                     semantic_tag_type tag = semantic_tag_type::none, 
                     const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_int64_value(value, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool int64_value(int64_t value, 
                     semantic_tag_type tag, 
                     const serializing_context& context,
                     std::error_code& ec)
    {
        return do_int64_value(value, tag, context, ec);
    }

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_uint64_value(value, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec)
    {
        return do_uint64_value(value, tag, context, ec);
    }

    bool double_value(double value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_double_value(value, floating_point_options(), tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool double_value(double value, 
                      const floating_point_options& fmt, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        std::error_code ec;
        bool res = do_double_value(value, fmt, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool double_value(double value, 
                      const floating_point_options& fmt, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec)
    {
        return do_double_value(value, fmt, tag, context, ec);
    }

    bool bool_value(bool value, 
                    semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_bool_value(value, tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool bool_value(bool value, 
                    semantic_tag_type tag,
                    const serializing_context& context,
                    std::error_code& ec) 
    {
        return do_bool_value(value, tag, context, ec);
    }

    bool null_value(semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_null_value(tag, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool null_value(semantic_tag_type tag,
                    const serializing_context& context,
                    std::error_code& ec) 
    {
        return do_null_value(tag, context, ec);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    bool begin_document()
    {
        return true;
    }

    bool end_document()
    {
        flush();
        return true;
    }

    void begin_json()
    {
    }

    void end_json()
    {
        end_document();
    }

    void name(const CharT* p, size_t length, const serializing_context& context) 
    {
        name(string_view_type(p, length), context);
    }

    void integer_value(int64_t value)
    {
        int64_value(value);
    }

    void integer_value(int64_t value, const serializing_context& context)
    {
        int64_value(value,context);
    }

    void uinteger_value(uint64_t value)
    {
        uint64_value(value);
    }

    void uinteger_value(uint64_t value, const serializing_context& context)
    {
        uint64_value(value,context);
    }

    bool double_value(double value, uint8_t precision, const serializing_context& context = null_serializing_context())
    {
        std::error_code ec;
        bool res = do_double_value(value, 
                               floating_point_options(chars_format::general, precision), 
                               semantic_tag_type::none,
                               context);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool bignum_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(s, semantic_tag_type::big_integer, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool bignum_value(const string_view_type& s, const serializing_context& context, std::error_code& ec) 
    {
        return do_string_value(s, semantic_tag_type::big_integer, context, ec);
    }

    bool decimal_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_string_value(s, semantic_tag_type::big_decimal, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool decimal_value(const string_view_type& s, const serializing_context& context, std::error_code& ec) 
    {
        return do_string_value(s, semantic_tag_type::big_decimal, context, ec);
    }

    bool epoch_time_value(int64_t val, const serializing_context& context=null_serializing_context()) 
    {
        std::error_code ec;
        bool res = do_int64_value(val, semantic_tag_type::timestamp, context, ec);
        if (ec)
        {
            throw serialization_error(ec, context.line_number(), context.column_number());
        }
        return res;
    }

    bool epoch_time_value(int64_t val, const serializing_context& context, std::error_code& ec) 
    {
        return do_int64_value(val, semantic_tag_type::timestamp, context, ec);
    }

#endif

private:
    virtual void do_flush() = 0;

    virtual bool do_begin_object(semantic_tag_type, const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_begin_object(size_t, semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
    {
        return do_begin_object(tag, context, ec);
    }

    virtual bool do_end_object(const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_begin_array(semantic_tag_type, const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_begin_array(size_t, semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
    {
        return do_begin_array(tag, context, ec);
    }

    virtual bool do_end_array(const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_name(const string_view_type& name, const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_null_value(semantic_tag_type, const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_string_value(const string_view_type& value, semantic_tag_type tag, const serializing_context& context, std::error_code& ec) = 0;

    virtual bool do_byte_string_value(const byte_string_view& b, 
                                      byte_string_chars_format encoding_hint,
                                      semantic_tag_type tag, 
                                      const serializing_context& context, 
                                      std::error_code& ec) = 0;

    virtual bool do_double_value(double value, 
                                 const floating_point_options& fmt, 
                                 semantic_tag_type tag,
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag_type tag,
                                const serializing_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag_type tag, 
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;

    virtual bool do_bool_value(bool value, semantic_tag_type tag, const serializing_context& context, std::error_code& ec) = 0;
};

template <class CharT>
class basic_null_json_content_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
private:
    void do_flush() override
    {
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_end_object(const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_end_array(const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_name(const string_view_type&, const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_string_value(const string_view_type&, semantic_tag_type, const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_byte_string_value(const byte_string_view&,
                              byte_string_chars_format, 
                              semantic_tag_type, 
                              const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_int64_value(int64_t, 
                        semantic_tag_type, 
                        const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_uint64_value(uint64_t, 
                         semantic_tag_type, 
                         const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_double_value(double, 
                         const floating_point_options&, 
                         semantic_tag_type,
                         const serializing_context&, std::error_code&) override
    {
        return true;
    }

    bool do_bool_value(bool, semantic_tag_type, const serializing_context&, std::error_code&) override
    {
        return true;
    }
};

typedef basic_json_content_handler<char> json_content_handler;
typedef basic_json_content_handler<wchar_t> wjson_content_handler;

typedef basic_null_json_content_handler<char> null_json_content_handler;
typedef basic_null_json_content_handler<wchar_t> wnull_json_content_handler;

}

#endif
