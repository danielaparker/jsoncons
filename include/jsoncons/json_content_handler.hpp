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

    void begin_document()
    {
        do_begin_document();
    }

    void end_document()
    {
        do_end_document();
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

    bool write_name(const string_view_type& name)
    {
        return do_name(name, null_serializing_context());
    }

    bool write_name(const string_view_type& name, const serializing_context& context)
    {
        return do_name(name, context);
    }

    bool write_string(const string_view_type& value) 
    {
        return do_string(value, null_serializing_context());
    }

    bool write_string(const string_view_type& value, const serializing_context& context) 
    {
        return do_string(value, context);
    }

    bool write_byte_string(const uint8_t* data, size_t length) 
    {
        return do_byte_string(data, length, null_serializing_context());
    }

    bool write_byte_string(const uint8_t* data, size_t length, const serializing_context& context) 
    {
        return do_byte_string(data, length, context);
    }

    bool write_byte_string(const std::vector<uint8_t>& v) 
    {
        return do_byte_string(v.data(), v.size(), null_serializing_context());
    }

    bool write_byte_string(const std::vector<uint8_t>& v, const serializing_context& context) 
    {
        return do_byte_string(v.data(), v.size(), context);
    }

    bool write_bignum(int signum, const uint8_t* data, size_t length) 
    {
        return do_bignum(signum, data, length, null_serializing_context());
    }

    bool write_bignum(int signum, const uint8_t* data, size_t length, const serializing_context& context) 
    {
        return do_bignum(signum, data, length, context);
    }

    bool write_bignum(const string_view_type& s) 
    {
        bignum n(s.data(),s.size());
        int signum;
        std::vector<uint8_t> v;
        n.dump(signum, v);

        return do_bignum(signum, v.data(), v.size(), null_serializing_context());
    }

    bool write_bignum(const string_view_type& s, const serializing_context& context) 
    {
        bignum n(s.data(),s.size());
        int signum;
        std::vector<uint8_t> v;
        n.dump(signum, v);

        return do_bignum(signum, v.data(), v.size(), context);
    }

    bool write_integer(int64_t value)
    {
        return do_integer(value,null_serializing_context());
    }

    bool write_integer(int64_t value, const serializing_context& context)
    {
        return do_integer(value,context);
    }

    bool write_uinteger(uint64_t value)
    {
        return do_uinteger(value,null_serializing_context());
    }

    bool write_uinteger(uint64_t value, const serializing_context& context)
    {
        return do_uinteger(value,context);
    }

    bool write_double(double value)
    {
        return do_double(value, floating_point_options(), null_serializing_context());
    }

    bool write_double(double value, uint8_t precision)
    {
        return do_double(value, floating_point_options(chars_format::general, precision), null_serializing_context());
    }

    bool write_double(double value, const floating_point_options& fmt)
    {
        return do_double(value, fmt, null_serializing_context());
    }

    bool write_double(double value, const serializing_context& context)
    {
        return do_double(value, floating_point_options(), context);
    }

    bool write_double(double value, uint8_t precision, const serializing_context& context)
    {
        return do_double(value, floating_point_options(chars_format::general, precision), context);
    }

    bool write_double(double value, const floating_point_options& fmt, const serializing_context& context)
    {
        return do_double(value, fmt, context);
    }

    bool write_bool(bool value) 
    {
        return do_bool(value,null_serializing_context());
    }

    bool write_bool(bool value, const serializing_context& context) 
    {
        return do_bool(value,context);
    }

    bool write_null() 
    {
        return do_null(null_serializing_context());
    }

    bool write_null(const serializing_context& context) 
    {
        return do_null(context);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void begin_json()
    {
        begin_document();
    }

    void end_json()
    {
        end_document();
    }

    void name(const CharT* p, size_t length, const serializing_context& context) 
    {
        write_name(string_view_type(p, length), context);
    }

    void value(const std::basic_string<CharT>& value, const serializing_context& context) 
    {
        write_string(value, context);
    }

    void value(const CharT* p, size_t length, const serializing_context& context) 
    {
        write_string(string_view_type(p, length), context);
    }

    void value(const CharT* p, const serializing_context& context)
    {
        write_string(string_view_type(p), context);
    }

    void value(int value, const serializing_context& context) 
    {
        write_integer(value,context);
    }

    void value(long value, const serializing_context& context) 
    {
        write_integer(value,context);
    }

    void value(long long value, const serializing_context& context) 
    {
        write_integer(value,context);
    }

    void value(unsigned int value, const serializing_context& context) 
    {
        write_uinteger(value,context);
    }

    void value(unsigned long value, const serializing_context& context) 
    {
        write_uinteger(value,context);
    }

    void value(unsigned long long value, const serializing_context& context) 
    {
        write_uinteger(value,context);
    }

    void value(float value, uint8_t precision, const serializing_context& context)
    {
        write_double(value, precision, context);
    }

    void value(double value, uint8_t precision, const serializing_context& context)
    {
        write_double(value, precision, context);
    }

    void value(bool value, const serializing_context& context) 
    {
        write_bool(value,context);
    }

    bool value(null_type, const serializing_context& context)
    {
        return write_null(context);
    }

    void name(const string_view_type& name)
    {
        write_name(name);
    }

    void name(const string_view_type& name, const serializing_context& context)
    {
        write_name(name, context);
    }

    void string_value(const string_view_type& value) 
    {
        write_string(value);
    }

    void string_value(const string_view_type& value, const serializing_context& context) 
    {
        write_string(value, context);
    }

    void byte_string_value(const uint8_t* data, size_t length) 
    {
        write_byte_string(data, length);
    }

    void byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) 
    {
        write_byte_string(data, length, context);
    }

    void byte_string_value(const std::vector<uint8_t>& v) 
    {
        write_byte_string(v);
    }

    void byte_string_value(const std::vector<uint8_t>& v, const serializing_context& context) 
    {
        write_byte_string(v, context);
    }

    void bignum_value(int signum, const uint8_t* data, size_t length) 
    {
        write_bignum(signum, data, length);
    }

    void bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) 
    {
        write_bignum(signum, data, length, context);
    }

    void bignum_value(const string_view_type& s) 
    {
        write_bignum(s);
    }

    void bignum_value(const string_view_type& s, const serializing_context& context) 
    {
        write_bignum(s, context);
    }

    void integer_value(int64_t value)
    {
        write_integer(value);
    }

    void integer_value(int64_t value, const serializing_context& context)
    {
        write_integer(value,context);
    }

    void uinteger_value(uint64_t value)
    {
        write_uinteger(value);
    }

    void uinteger_value(uint64_t value, const serializing_context& context)
    {
        write_uinteger(value,context);
    }

    void double_value(double value)
    {
        write_double(value);
    }

    void double_value(double value, uint8_t precision)
    {
        write_double(value, precision);
    }

    void double_value(double value, const floating_point_options& fmt)
    {
        write_double(value, fmt);
    }

    void double_value(double value, const serializing_context& context)
    {
        write_double(value, context);
    }

    void double_value(double value, uint8_t precision, const serializing_context& context)
    {
        write_double(value, precision, context);
    }

    void double_value(double value, const floating_point_options& fmt, const serializing_context& context)
    {
        write_double(value, fmt, context);
    }

    void bool_value(bool value) 
    {
        write_bool(value);
    }

    void bool_value(bool value, const serializing_context& context) 
    {
        write_bool(value, context);
    }

    void null_value() 
    {
        write_null();
    }

    void null_value(const serializing_context& context) 
    {
        write_null(context);
    }

#endif

private:
    virtual void do_begin_document() = 0;

    virtual void do_end_document() = 0;

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

    virtual bool do_null(const serializing_context& context) = 0;

    virtual bool do_string(const string_view_type& value, const serializing_context& context) = 0;

    virtual bool do_byte_string(const uint8_t* data, size_t length, const serializing_context& context) = 0;

    virtual bool do_bignum(int signum, const uint8_t* data, size_t length, const serializing_context& context) = 0;

    virtual bool do_double(double value, const floating_point_options& fmt, const serializing_context& context) = 0;

    virtual bool do_integer(int64_t value, const serializing_context& context) = 0;

    virtual bool do_uinteger(uint64_t value, const serializing_context& context) = 0;

    virtual bool do_bool(bool value, const serializing_context& context) = 0;
};

template <class CharT>
class basic_null_json_content_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
private:
    void do_begin_document() override
    {
    }

    void do_end_document() override
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

    bool do_null(const serializing_context&) override
    {
        return true;
    }

    bool do_string(const string_view_type&, const serializing_context&) override
    {
        return true;
    }

    bool do_byte_string(const uint8_t*, size_t, const serializing_context&) override
    {
        return true;
    }

    bool do_bignum(int, const uint8_t*, size_t, const serializing_context&) override
    {
        return true;
    }

    bool do_double(double, const floating_point_options&, const serializing_context&) override
    {
        return true;
    }

    bool do_integer(int64_t, const serializing_context&) override
    {
        return true;
    }

    bool do_uinteger(uint64_t, const serializing_context&) override
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
