// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONTENT_HANDLER_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_HPP

#include <string>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/streaming_context.hpp>
#include <jsoncons/json_serializing_options.hpp>
#if !defined(JSONCONS_NO_DEPRECATED)
#include <jsoncons/json_type_traits.hpp> // for null_type
#endif

namespace jsoncons {

template <class CharT>
class basic_json_content_handler
{
public:
    typedef CharT char_type;
    typedef std::char_traits<char_type> char_traits_type;

    typedef basic_string_view<char_type,char_traits_type> string_view_type;

    virtual ~basic_json_content_handler() {}

    bool begin_document()
    {
        return do_begin_document();
    }

    bool end_document()
    {
        return do_end_document();
    }

    bool begin_object()
    {
        return do_begin_object(null_streaming_context());
    }

    bool begin_object(const streaming_context& context)
    {
        return do_begin_object(context);
    }

    bool begin_object(size_t length)
    {
        return do_begin_object(length, null_streaming_context());
    }

    bool begin_object(size_t length, const streaming_context& context)
    {
        return do_begin_object(length, context);
    }

    bool end_object()
    {
        return do_end_object(null_streaming_context());
    }

    bool end_object(const streaming_context& context)
    {
        return do_end_object(context);
    }

    bool begin_array()
    {
        return do_begin_array(null_streaming_context());
    }

    bool begin_array(size_t length)
    {
        return do_begin_array(length, null_streaming_context());
    }

    bool begin_array(const streaming_context& context)
    {
        return do_begin_array(context);
    }

    bool begin_array(size_t length, const streaming_context& context)
    {
        return do_begin_array(length, context);
    }

    bool end_array()
    {
        return do_end_array(null_streaming_context());
    }

    bool end_array(const streaming_context& context)
    {
        return do_end_array(context);
    }

    bool write_name(const string_view_type& name)
    {
        return do_name(name, null_streaming_context());
    }

    bool write_name(const string_view_type& name, const streaming_context& context)
    {
        return do_name(name, context);
    }

    bool string_value(const string_view_type& value) 
    {
        return do_string_value(value, null_streaming_context());
    }

    bool string_value(const string_view_type& value, const streaming_context& context) 
    {
        return do_string_value(value, context);
    }

    bool byte_string_value(const uint8_t* data, size_t length) 
    {
        return do_byte_string_value(data, length, null_streaming_context());
    }

    bool byte_string_value(const uint8_t* data, size_t length, const streaming_context& context) 
    {
        return do_byte_string_value(data, length, context);
    }

    bool byte_string_value(const std::vector<uint8_t>& v) 
    {
        return do_byte_string_value(v.data(), v.size(), null_streaming_context());
    }

    bool byte_string_value(const std::vector<uint8_t>& v, const streaming_context& context) 
    {
        return do_byte_string_value(v.data(), v.size(), context);
    }

    bool bignum_value(int signum, const uint8_t* data, size_t length) 
    {
        return do_bignum_value(signum, data, length, null_streaming_context());
    }

    bool bignum_value(int signum, const uint8_t* data, size_t length, const streaming_context& context) 
    {
        return do_bignum_value(signum, data, length, context);
    }

    bool bignum_value(const string_view_type& s) 
    {
        bignum n(s.data(),s.size());
        int signum;
        std::vector<uint8_t> v;
        n.dump(signum, v);

        return do_bignum_value(signum, v.data(), v.size(), null_streaming_context());
    }

    bool bignum_value(const string_view_type& s, const streaming_context& context) 
    {
        bignum n(s.data(),s.size());
        int signum;
        std::vector<uint8_t> v;
        n.dump(signum, v);

        return do_bignum_value(signum, v.data(), v.size(), context);
    }

    bool integer_value(int64_t value)
    {
        return do_integer_value(value,null_streaming_context());
    }

    bool integer_value(int64_t value, const streaming_context& context)
    {
        return do_integer_value(value,context);
    }

    bool uinteger_value(uint64_t value)
    {
        return do_uinteger_value(value,null_streaming_context());
    }

    bool uinteger_value(uint64_t value, const streaming_context& context)
    {
        return do_uinteger_value(value,context);
    }

    bool double_value(double value)
    {
        return do_double_value(value, floating_point_options(), null_streaming_context());
    }

    bool double_value(double value, uint8_t precision)
    {
        return do_double_value(value, floating_point_options(chars_format::general, precision), null_streaming_context());
    }

    bool double_value(double value, const floating_point_options& fmt)
    {
        return do_double_value(value, fmt, null_streaming_context());
    }

    bool double_value(double value, const streaming_context& context)
    {
        return do_double_value(value, floating_point_options(), context);
    }

    bool double_value(double value, uint8_t precision, const streaming_context& context)
    {
        return do_double_value(value, floating_point_options(chars_format::general, precision), context);
    }

    bool double_value(double value, const floating_point_options& fmt, const streaming_context& context)
    {
        return do_double_value(value, fmt, context);
    }

    bool bool_value(bool value) 
    {
        return do_bool_value(value,null_streaming_context());
    }

    bool bool_value(bool value, const streaming_context& context) 
    {
        return do_bool_value(value,context);
    }

    bool null_value() 
    {
        return do_null_value(null_streaming_context());
    }

    bool null_value(const streaming_context& context) 
    {
        return do_null_value(context);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    bool begin_json()
    {
        return do_begin_document();
    }

    bool end_json()
    {
        return do_end_document();
    }

    bool write_name(const CharT* p, size_t length, const streaming_context& context) 
    {
        return do_name(string_view_type(p, length), context);
    }

    bool value(const std::basic_string<CharT>& value, const streaming_context& context) 
    {
        return do_string_value(value, context);
    }

    bool value(const CharT* p, size_t length, const streaming_context& context) 
    {
        return do_string_value(string_view_type(p, length), context);
    }

    bool value(const CharT* p, const streaming_context& context)
    {
        return do_string_value(string_view_type(p), context);
    }

    bool value(int value, const streaming_context& context) 
    {
        return do_integer_value(value,context);
    }

    bool value(long value, const streaming_context& context) 
    {
        return do_integer_value(value,context);
    }

    bool value(long long value, const streaming_context& context) 
    {
        return do_integer_value(value,context);
    }

    bool value(unsigned int value, const streaming_context& context) 
    {
        return do_uinteger_value(value,context);
    }

    bool value(unsigned long value, const streaming_context& context) 
    {
        return do_uinteger_value(value,context);
    }

    bool value(unsigned long long value, const streaming_context& context) 
    {
        return do_uinteger_value(value,context);
    }

    bool value(float value, uint8_t precision, const streaming_context& context)
    {
        return do_double_value(value, floating_point_options(chars_format::general, precision), context);
    }

    bool value(double value, uint8_t precision, const streaming_context& context)
    {
        return do_double_value(value, floating_point_options(chars_format::general, precision), context);
    }

    bool value(bool value, const streaming_context& context) 
    {
        return do_bool_value(value,context);
    }

    bool value(null_type, const streaming_context& context)
    {
        return do_null_value(context);
    }
#endif

private:
    virtual bool do_begin_document() = 0;

    virtual bool do_end_document() = 0;

    virtual bool do_begin_object(const streaming_context& context) = 0;

    virtual bool do_begin_object(size_t, const streaming_context& context) 
    {
        return do_begin_object(context);
    }

    virtual bool do_end_object(const streaming_context& context) = 0;

    virtual bool do_begin_array(const streaming_context& context) = 0;

    virtual bool do_begin_array(size_t, const streaming_context& context) 
    {
        return do_begin_array(context);
    }

    virtual bool do_end_array(const streaming_context& context) = 0;

    virtual bool do_name(const string_view_type& name, const streaming_context& context) = 0;

    virtual bool do_null_value(const streaming_context& context) = 0;

    virtual bool do_string_value(const string_view_type& value, const streaming_context& context) = 0;

    virtual bool do_byte_string_value(const uint8_t* data, size_t length, const streaming_context& context) = 0;

    virtual bool do_bignum_value(int signum, const uint8_t* data, size_t length, const streaming_context& context) = 0;

    virtual bool do_double_value(double value, const floating_point_options& fmt, const streaming_context& context) = 0;

    virtual bool do_integer_value(int64_t value, const streaming_context& context) = 0;

    virtual bool do_uinteger_value(uint64_t value, const streaming_context& context) = 0;

    virtual bool do_bool_value(bool value, const streaming_context& context) = 0;
};

template <class CharT>
class basic_null_json_content_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type                                 ;
private:
    bool do_begin_document() override
    {
        return true;
    }

    bool do_end_document() override
    {
        return true;
    }

    bool do_begin_object(const streaming_context&) override
    {
        return true;
    }

    bool do_end_object(const streaming_context&) override
    {
        return true;
    }

    bool do_begin_array(const streaming_context&) override
    {
        return true;
    }

    bool do_end_array(const streaming_context&) override
    {
        return true;
    }

    bool do_name(const string_view_type&, const streaming_context&) override
    {
        return true;
    }

    bool do_null_value(const streaming_context&) override
    {
        return true;
    }

    bool do_string_value(const string_view_type&, const streaming_context&) override
    {
        return true;
    }

    bool do_byte_string_value(const uint8_t*, size_t, const streaming_context&) override
    {
        return true;
    }

    bool do_bignum_value(int, const uint8_t*, size_t, const streaming_context&) override
    {
        return true;
    }

    bool do_double_value(double, const floating_point_options&, const streaming_context&) override
    {
        return true;
    }

    bool do_integer_value(int64_t, const streaming_context&) override
    {
        return true;
    }

    bool do_uinteger_value(uint64_t, const streaming_context&) override
    {
        return true;
    }

    bool do_bool_value(bool, const streaming_context&) override
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
