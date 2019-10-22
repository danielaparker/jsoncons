// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONTENT_HANDLER_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons/ser_context.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/config/binary_config.hpp>

namespace jsoncons {

// null_type

struct null_type
{
};

struct half_arg_t
{
};

constexpr half_arg_t half_arg = half_arg_t();

enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined = 0x01,
    datetime = 0x02,
    timestamp = 0x03,
    bigint = 0x04,
    bigdec = 0x05,
    bigfloat = 0x06,
    base16 = 0x07,
    base64 = 0x08,
    base64url = 0x09,
    uri = 0x0a,
    clamped = 0x0b
#if !defined(JSONCONS_NO_DEPRECATED)
    , big_integer = bigint
    , big_decimal = bigdec
    , big_float = bigfloat
    , date_time = datetime
#endif
};

inline
std::ostream& operator<<(std::ostream& os, semantic_tag tag)
{
    switch (tag)
    {
        case semantic_tag::none:
        {
            os << "n/a";
            break;
        }
        case semantic_tag::undefined:
        {
            os << "undefined";
            break;
        }
        case semantic_tag::datetime:
        {
            os << "datetime";
            break;
        }
        case semantic_tag::timestamp:
        {
            os << "timestamp";
            break;
        }
        case semantic_tag::bigint:
        {
            os << "bigint";
            break;
        }
        case semantic_tag::bigdec:
        {
            os << "bigdec";
            break;
        }
        case semantic_tag::bigfloat:
        {
            os << "bigfloat";
            break;
        }
        case semantic_tag::base16:
        {
            os << "base16";
            break;
        }
        case semantic_tag::base64:
        {
            os << "base64";
            break;
        }
        case semantic_tag::base64url:
        {
            os << "base64url";
            break;
        }
        case semantic_tag::uri:
        {
            os << "uri";
            break;
        }
        case semantic_tag::clamped:
        {
            os << "clamped";
            break;
        }
    }
    return os;
}

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use semantic_tag") typedef semantic_tag semantic_tag_type;
#endif

template <class CharT>
class basic_json_content_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view<char_type,char_traits_type> string_view_type;

    basic_json_content_handler(basic_json_content_handler&&) = default;

    basic_json_content_handler& operator=(basic_json_content_handler&&) = default;

    basic_json_content_handler() = default;

    virtual ~basic_json_content_handler() {}

    bool begin_object(semantic_tag tag=semantic_tag::none,
                      const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_begin_object(tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool begin_object(size_t length, 
                      semantic_tag tag=semantic_tag::none, 
                      const ser_context& context = null_ser_context())
    {
        std::error_code ec;
        bool more = do_begin_object(length, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool end_object(const ser_context& context = null_ser_context())
    {
        std::error_code ec;
        bool more = do_end_object(context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool begin_array(semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_begin_array(tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool begin_array(size_t length, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_begin_array(length, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool end_array(const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_end_array(context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool name(const string_view_type& name, const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_name(name, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool null_value(semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()) 
    {
        std::error_code ec;
        bool more = do_null_value(tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool bool_value(bool value, 
                    semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()) 
    {
        std::error_code ec;
        bool more = do_bool_value(value, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()) 
    {
        std::error_code ec;
        bool more = do_string_value(value, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_byte_string_value(b, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool byte_string_value(const uint8_t* p, size_t size, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context())
    {
        return byte_string_value(byte_string(p, size), tag, context);
    }

    bool uint64_value(uint64_t value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_uint64_value(value, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool int64_value(int64_t value, 
                     semantic_tag tag = semantic_tag::none, 
                     const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_int64_value(value, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool half_value(uint16_t value, 
                    semantic_tag tag = semantic_tag::none, 
                    const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_half_value(value, tag, context, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
        }
        return more;
    }

    bool double_value(double value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context())
    {
        std::error_code ec;
        bool more = do_double_value(value, tag, context, ec);
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
        return do_begin_object(tag, context, ec);
    }

    bool begin_object(size_t length, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec)
    {
        return do_begin_object(length, tag, context, ec);
    }

    bool end_object(const ser_context& context, std::error_code& ec)
    {
        return do_end_object(context, ec);
    }

    bool begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec)
    {
        return do_begin_array(tag, context, ec);
    }

    bool begin_array(size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec)
    {
        return do_begin_array(length, tag, context, ec);
    }

    bool end_array(const ser_context& context, std::error_code& ec)
    {
        return do_end_array(context, ec);
    }

    bool name(const string_view_type& name, const ser_context& context, std::error_code& ec)
    {
        return do_name(name, context, ec);
    }

    bool null_value(semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec) 
    {
        return do_null_value(tag, context, ec);
    }

    bool bool_value(bool value, 
                    semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec) 
    {
        return do_bool_value(value, tag, context, ec);
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec) 
    {
        return do_string_value(value, tag, context, ec);
    }

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec)
    {
        return do_byte_string_value(b, tag, context, ec);
    }

    bool byte_string_value(const uint8_t* p, size_t size, 
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
        return do_uint64_value(value, tag, context, ec);
    }

    bool int64_value(int64_t value, 
                     semantic_tag tag, 
                     const ser_context& context,
                     std::error_code& ec)
    {
        return do_int64_value(value, tag, context, ec);
    }

    bool half_value(uint16_t value, 
                    semantic_tag tag, 
                    const ser_context& context,
                    std::error_code& ec)
    {
        return do_half_value(value, tag, context, ec);
    }

    bool double_value(double value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec)
    {
        return do_double_value(value, tag, context, ec);
    }

    void flush()
    {
        do_flush();
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    JSONCONS_DEPRECATED_MSG("Instead, use const byte_string_view&, semantic_tag=semantic_tag::none, const ser_context&=null_ser_context()") 
    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context())
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

    JSONCONS_DEPRECATED_MSG("Instead, use byte_string_value(const byte_string_view&, semantic_tag=semantic_tag::none, const ser_context&=null_ser_context()") 
    bool byte_string_value(const uint8_t* p, size_t size, 
                           byte_string_chars_format encoding_hint, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context())
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
    bool big_integer_value(const string_view_type& value, const ser_context& context=null_ser_context()) 
    {
        return string_value(value, semantic_tag::bigint, context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigdec") 
    bool big_decimal_value(const string_view_type& value, const ser_context& context=null_ser_context()) 
    {
        return string_value(value, semantic_tag::bigdec, context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::datetime") 
    bool date_time_value(const string_view_type& value, const ser_context& context=null_ser_context()) 
    {
        return string_value(value, semantic_tag::datetime, context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use int64_value with semantic_tag::timestamp") 
    bool timestamp_value(int64_t val, const ser_context& context=null_ser_context()) 
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

    JSONCONS_DEPRECATED_MSG("Instead, use name(const string_view_type&, const ser_context&=null_ser_context())") 
    void name(const char_type* p, size_t length, const ser_context& context) 
    {
        name(string_view_type(p, length), context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=null_ser_context())") 
    void integer_value(int64_t value)
    {
        int64_value(value);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use int64_value(int64_t, semantic_tag = semantic_tag::none, const ser_context&=null_ser_context())") 
    void integer_value(int64_t value, const ser_context& context)
    {
        int64_value(value,context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=null_ser_context())") 
    void uinteger_value(uint64_t value)
    {
        uint64_value(value);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use uint64_value(uint64_t, semantic_tag = semantic_tag::none, const ser_context&=null_ser_context())") 
    void uinteger_value(uint64_t value, const ser_context& context)
    {
        uint64_value(value,context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigint") 
    bool bignum_value(const string_view_type& value, const ser_context& context=null_ser_context()) 
    {
        return string_value(value, semantic_tag::bigint, context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use string_value with semantic_tag::bigdec") 
    bool decimal_value(const string_view_type& value, const ser_context& context=null_ser_context()) 
    {
        return string_value(value, semantic_tag::bigdec, context);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use int64_value with semantic_tag::timestamp") 
    bool epoch_time_value(int64_t val, const ser_context& context=null_ser_context()) 
    {
        return int64_value(val, semantic_tag::timestamp, context);
    }

#endif
private:
    virtual bool do_begin_object(semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0;

    virtual bool do_begin_object(size_t /*length*/, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec)
    {
        return do_begin_object(tag, context, ec);
    }

    virtual bool do_end_object(const ser_context& context, 
                               std::error_code& ec) = 0;

    virtual bool do_begin_array(semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_begin_array(size_t /*length*/, 
                                semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec)
    {
        return do_begin_array(tag, context, ec);
    }

    virtual bool do_end_array(const ser_context& context, 
                              std::error_code& ec) = 0;

    virtual bool do_name(const string_view_type& name, 
                         const ser_context& context, 
                         std::error_code&) = 0;

    virtual bool do_null_value(semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) = 0;

    virtual bool do_bool_value(bool value, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code&) = 0;

    virtual bool do_string_value(const string_view_type& value, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0;

    virtual bool do_byte_string_value(const byte_string_view& value, 
                                      semantic_tag tag, 
                                      const ser_context& context,
                                      std::error_code& ec) = 0;

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag tag, 
                                 const ser_context& context,
                                 std::error_code& ec) = 0;

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag tag,
                                const ser_context& context,
                                std::error_code& ec) = 0;

    virtual bool do_half_value(uint16_t value, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec)
    {
        return do_double_value(jsoncons::detail::decode_half(value),
                               tag,
                               context,
                               ec);
    }

    virtual bool do_double_value(double value, 
                                 semantic_tag tag,
                                 const ser_context& context,
                                 std::error_code& ec) = 0;

    virtual void do_flush() = 0;

};

template <class CharT>
class basic_default_json_content_handler : public basic_json_content_handler<CharT>
{
    bool parse_more_;
public:
    using typename basic_json_content_handler<CharT>::string_view_type;

    basic_default_json_content_handler(bool parse_more = true)
        : parse_more_(parse_more)
    {
    }
private:
    void do_flush() override
    {
    }

    bool do_begin_object(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_end_object(const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_begin_array(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_end_array(const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_name(const string_view_type&, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_null_value(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_string_value(const string_view_type&, semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_byte_string_value(const byte_string_view&,
                              semantic_tag, 
                              const ser_context&,
                              std::error_code&) override
    {
        return parse_more_;
    }

    bool do_int64_value(int64_t, 
                        semantic_tag, 
                        const ser_context&,
                        std::error_code&) override
    {
        return parse_more_;
    }

    bool do_uint64_value(uint64_t, 
                         semantic_tag, 
                         const ser_context&,
                         std::error_code&) override
    {
        return parse_more_;
    }

    bool do_double_value(double, 
                         semantic_tag,
                         const ser_context&,
                         std::error_code&) override
    {
        return parse_more_;
    }

    bool do_bool_value(bool, semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }
};

typedef basic_json_content_handler<char> json_content_handler;
typedef basic_json_content_handler<wchar_t> wjson_content_handler;

typedef basic_default_json_content_handler<char> default_json_content_handler;
typedef basic_default_json_content_handler<wchar_t> wdefault_json_content_handler;

}

#endif
