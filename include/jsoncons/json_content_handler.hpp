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
        return do_begin_object(tag, context);
    }

    bool begin_object(size_t length, 
                      semantic_tag_type tag=semantic_tag_type::none, 
                      const serializing_context& context = null_serializing_context())
    {
        return do_begin_object(length, tag, context);
    }

    bool end_object(const serializing_context& context = null_serializing_context())
    {
        return do_end_object(context);
    }

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    {
        return do_begin_array(tag, context);
    }

    bool begin_array(size_t length, 
                     semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    {
        return do_begin_array(length, tag, context);
    }

    bool end_array(const serializing_context& context=null_serializing_context())
    {
        return do_end_array(context);
    }

    bool name(const string_view_type& name, const serializing_context& context=null_serializing_context())
    {
        return do_name(name, context);
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(value, tag, context);
    }

    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
    {
        return do_byte_string_value(b, encoding_hint, tag, context);
    }

    bool byte_string_value(const uint8_t* p, size_t size, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
    {
        return do_byte_string_value(byte_string(p, size), encoding_hint, tag, context);
    }

    bool big_integer_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(s, semantic_tag_type::big_integer, context);
    }

    bool big_decimal_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(s, semantic_tag_type::big_decimal, context);
    }

    bool date_time_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(s, semantic_tag_type::date_time, context);
    }

    bool timestamp_value(int64_t val, const serializing_context& context=null_serializing_context()) 
    {
        return do_int64_value(val, semantic_tag_type::timestamp, context);
    }

    bool int64_value(int64_t value, 
                     semantic_tag_type tag = semantic_tag_type::none, 
                     const serializing_context& context=null_serializing_context())
    {
        return do_int64_value(value, tag, context);
    }

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        return do_uint64_value(value, tag, context);
    }

    bool double_value(double value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        return do_double_value(value, floating_point_options(), tag, context);
    }

    bool double_value(double value, 
                      const floating_point_options& fmt, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    {
        return do_double_value(value, fmt, tag, context);
    }

    bool bool_value(bool value, 
                    semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    {
        return do_bool_value(value, tag, context);
    }

    bool null_value(semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    {
        return do_null_value(tag, context);
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
        return do_double_value(value, 
                               floating_point_options(chars_format::general, precision), 
                               semantic_tag_type::none,
                               context);
    }

    bool bignum_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(s, semantic_tag_type::big_integer, context);
    }

    bool decimal_value(const string_view_type& s, const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(s, semantic_tag_type::big_decimal, context);
    }

    bool epoch_time_value(int64_t val, const serializing_context& context=null_serializing_context()) 
    {
        return do_int64_value(val, semantic_tag_type::timestamp, context);
    }

#endif

private:
    virtual void do_flush() = 0;

    virtual bool do_begin_object(semantic_tag_type, const serializing_context& context) = 0;

    virtual bool do_begin_object(size_t, semantic_tag_type tag, const serializing_context& context)
    {
        return do_begin_object(tag, context);
    }

    virtual bool do_end_object(const serializing_context& context) = 0;

    virtual bool do_begin_array(semantic_tag_type, const serializing_context& context) = 0;

    virtual bool do_begin_array(size_t, semantic_tag_type tag, const serializing_context& context)
    {
        return do_begin_array(tag, context);
    }

    virtual bool do_end_array(const serializing_context& context) = 0;

    virtual bool do_name(const string_view_type& name, const serializing_context& context) = 0;

    virtual bool do_null_value(semantic_tag_type, const serializing_context& context) = 0;

    virtual bool do_string_value(const string_view_type& value, semantic_tag_type tag, const serializing_context& context) = 0;

    virtual bool do_byte_string_value(const byte_string_view& b, 
                                      byte_string_chars_format encoding_hint,
                                      semantic_tag_type tag, 
                                      const serializing_context& context) = 0;

    virtual bool do_double_value(double value, 
                                 const floating_point_options& fmt, 
                                 semantic_tag_type tag,
                                 const serializing_context& context) = 0;

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag_type tag,
                                const serializing_context& context) = 0;

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag_type tag, 
                                 const serializing_context& context) = 0;

    virtual bool do_bool_value(bool value, semantic_tag_type tag, const serializing_context& context) = 0;
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

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        return true;
    }

    bool do_name(const string_view_type&, const serializing_context&) override
    {
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        return true;
    }

    bool do_string_value(const string_view_type&, semantic_tag_type, const serializing_context&) override
    {
        return true;
    }

    bool do_byte_string_value(const byte_string_view&,
                              byte_string_chars_format, 
                              semantic_tag_type, 
                              const serializing_context&) override
    {
        return true;
    }

    bool do_int64_value(int64_t, 
                        semantic_tag_type, 
                        const serializing_context&) override
    {
        return true;
    }

    bool do_uint64_value(uint64_t, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        return true;
    }

    bool do_double_value(double, 
                         const floating_point_options&, 
                         semantic_tag_type,
                         const serializing_context&) override
    {
        return true;
    }

    bool do_bool_value(bool, semantic_tag_type, const serializing_context&) override
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
