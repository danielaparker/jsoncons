// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONTENT_HANDLER_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/serializing_context.hpp>
#include <jsoncons/json_serializing_options.hpp>

namespace jsoncons {

// null_type

struct null_type
{
};

enum class semantic_tag_type : uint8_t 
{
    na = 0x00,
    bignum_tag = 0x01,
    date_time_tag = 0x02,
    epoch_time_tag = 0x03
};

template <class CharT>
class basic_json_content_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view<char_type,char_traits_type> string_view_type;

    virtual ~basic_json_content_handler() {}

    void flush()
    {
        do_flush();
    }

    bool begin_object()
    {
        return do_begin_object(null_serializing_context());
    }

    bool begin_object(const serializing_context& context)
    {
        return do_begin_object(context);
    }

    bool begin_object(size_t length)
    {
        return do_begin_object(length, null_serializing_context());
    }

    bool begin_object(size_t length, const serializing_context& context)
    {
        return do_begin_object(length, context);
    }

    bool end_object()
    {
        return do_end_object(null_serializing_context());
    }

    bool end_object(const serializing_context& context)
    {
        return do_end_object(context);
    }

    bool begin_array()
    {
        return do_begin_array(null_serializing_context());
    }

    bool begin_array(size_t length)
    {
        return do_begin_array(length, null_serializing_context());
    }

    bool begin_array(const serializing_context& context)
    {
        return do_begin_array(context);
    }

    bool begin_array(size_t length, const serializing_context& context)
    {
        return do_begin_array(length, context);
    }

    bool end_array()
    {
        return do_end_array(null_serializing_context());
    }

    bool end_array(const serializing_context& context)
    {
        return do_end_array(context);
    }

    bool name(const string_view_type& name)
    {
        return do_name(name, null_serializing_context());
    }

    bool name(const string_view_type& name, const serializing_context& context)
    {
        return do_name(name, context);
    }

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag = semantic_tag_type::na, 
                      const serializing_context& context=null_serializing_context()) 
    {
        return do_string_value(value, tag, context);
    }

    bool byte_string_value(const uint8_t* data, size_t length, 
                           semantic_tag_type tag=semantic_tag_type::na, 
                           const serializing_context& context=null_serializing_context()) 
    {
        return do_byte_string_value(data, length, tag, context);
    }

    bool byte_string_value(const std::vector<uint8_t>& v, 
                           semantic_tag_type tag=semantic_tag_type::na, 
                           const serializing_context& context=null_serializing_context())
    {
        return do_byte_string_value(v.data(), v.size(), tag, context);
    }

    bool bignum_value(int signum, const uint8_t* data, size_t length) 
    {
        bignum n(signum, data, length);
        std::basic_string<CharT> s;
        n.dump(s);
        return do_string_value(s, semantic_tag_type::bignum_tag, null_serializing_context());
    }

    bool bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) 
    {
        bignum n(signum, data, length);
        std::basic_string<CharT> s;
        n.dump(s);
        return do_string_value(s, semantic_tag_type::bignum_tag, context);
    }

    bool bignum_value(const string_view_type& s) 
    {
        return do_string_value(s, semantic_tag_type::bignum_tag, null_serializing_context());
    }

    bool bignum_value(const string_view_type& s, const serializing_context& context) 
    {
        return do_string_value(s, semantic_tag_type::bignum_tag, context);
    }

    bool int64_value(int64_t value, 
                     semantic_tag_type tag = semantic_tag_type::na, 
                     const serializing_context& context=null_serializing_context())
    {
        return do_int64_value(value, tag, context);
    }

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag = semantic_tag_type::na, 
                      const serializing_context& context=null_serializing_context())
    {
        return do_uint64_value(value, tag, context);
    }

    bool double_value(double value, uint8_t precision)
    {
        return do_double_value(value, 
                               floating_point_options(chars_format::general, precision), 
                               semantic_tag_type::na,
                               null_serializing_context());
    }

    bool double_value(double value, uint8_t precision, const serializing_context& context)
    {
        return do_double_value(value, 
                               floating_point_options(chars_format::general, precision), 
                               semantic_tag_type::na,
                               context);
    }

    bool double_value(double value, 
                      const floating_point_options& fmt = floating_point_options(), 
                      semantic_tag_type tag = semantic_tag_type::na, 
                      const serializing_context& context=null_serializing_context())
    {
        return do_double_value(value, fmt, tag, context);
    }

    bool bool_value(bool value, 
                    const serializing_context& context=null_serializing_context()) 
    {
        return do_bool(value,context);
    }

    bool null_value(const serializing_context& context=null_serializing_context()) 
    {
        return do_null_value(context);
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

    void value(const std::basic_string<CharT>& value, const serializing_context& context) 
    {
        string_value(value, context);
    }

    void value(const CharT* p, size_t length, const serializing_context& context) 
    {
        string_value(string_view_type(p, length), context);
    }

    void value(const CharT* p, const serializing_context& context)
    {
        string_value(string_view_type(p), context);
    }

    void value(int value, const serializing_context& context) 
    {
        int64_value(value,context);
    }

    void value(long value, const serializing_context& context) 
    {
        int64_value(value,context);
    }

    void value(long long value, const serializing_context& context) 
    {
        int64_value(value,context);
    }

    void value(unsigned int value, const serializing_context& context) 
    {
        uint64_value(value,context);
    }

    void value(unsigned long value, const serializing_context& context) 
    {
        uint64_value(value,context);
    }

    void value(unsigned long long value, const serializing_context& context) 
    {
        uint64_value(value,context);
    }

    void value(float value, uint8_t precision, const serializing_context& context)
    {
        double_value(value, precision, context);
    }

    void value(double value, uint8_t precision, const serializing_context& context)
    {
        double_value(value, precision, context);
    }

    void value(bool value, const serializing_context& context) 
    {
        bool_value(value,context);
    }

    bool value(null_type, const serializing_context& context)
    {
        return null_value(context);
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

#endif

private:
    virtual void do_flush() = 0;

    virtual bool do_begin_object(const serializing_context& context) = 0;

    virtual bool do_begin_object(size_t, const serializing_context& context) 
    {
        return do_begin_object(context);
    }

    virtual bool do_end_object(const serializing_context& context) = 0;

    virtual bool do_begin_array(const serializing_context& context) = 0;

    virtual bool do_begin_array(size_t, const serializing_context& context) 
    {
        return do_begin_array(context);
    }

    virtual bool do_end_array(const serializing_context& context) = 0;

    virtual bool do_name(const string_view_type& name, const serializing_context& context) = 0;

    virtual bool do_null_value(const serializing_context& context) = 0;

    virtual bool do_string_value(const string_view_type& value, semantic_tag_type tag, const serializing_context& context) = 0;

    virtual bool do_byte_string_value(const uint8_t* data, size_t length, semantic_tag_type tag, const serializing_context& context) = 0;

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

    virtual bool do_bool(bool value, const serializing_context& context) = 0;
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

    bool do_begin_object(const serializing_context&) override
    {
        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        return true;
    }

    bool do_begin_array(const serializing_context&) override
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

    bool do_null_value(const serializing_context&) override
    {
        return true;
    }

    bool do_string_value(const string_view_type&, semantic_tag_type, const serializing_context&) override
    {
        return true;
    }

    bool do_byte_string_value(const uint8_t*, size_t, semantic_tag_type, const serializing_context&) override
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

    bool do_bool(bool, const serializing_context&) override
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
