// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_FILTER_HPP
#define JSONCONS_JSON_FILTER_HPP

#include <string>

#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>

namespace jsoncons {

template <class CharT>
class basic_json_filter : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type                                 ;
private:
    basic_json_content_handler<CharT>& downstream_handler_;

    // noncopyable and nonmoveable
    basic_json_filter<CharT>(const basic_json_filter<CharT>&) = delete;
    basic_json_filter<CharT>& operator=(const basic_json_filter<CharT>&) = delete;
public:
    basic_json_filter(basic_json_content_handler<CharT>& handler)
        : downstream_handler_(handler)
    {
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_json_content_handler<CharT>& input_handler()
    {
        return downstream_handler_;
    }
#endif

    basic_json_content_handler<CharT>& downstream_handler()
    {
        return downstream_handler_;
    }

private:
    void do_begin_document() override
    {
        downstream_handler_.begin_document();
    }

    void do_end_document() override
    {
        downstream_handler_.end_document();
    }

    bool do_begin_object(const serializing_context& context) override
    {
        return downstream_handler_.begin_object(context);
    }

    bool do_begin_object(size_t length, const serializing_context& context) override
    {
        return downstream_handler_.begin_object(length, context);
    }

    bool do_end_object(const serializing_context& context) override
    {
        return downstream_handler_.end_object(context);
    }

    bool do_begin_array(const serializing_context& context) override
    {
        return downstream_handler_.begin_array(context);
    }

    bool do_begin_array(size_t length, const serializing_context& context) override
    {
        return downstream_handler_.begin_array(length, context);
    }

    bool do_end_array(const serializing_context& context) override
    {
        return downstream_handler_.end_array(context);
    }

    bool do_name(const string_view_type& name,
                 const serializing_context& context) override
    {
        return downstream_handler_.write_name(name,context);
    }

    bool do_string(const string_view_type& value,
                         const serializing_context& context) override
    {
        return downstream_handler_.write_string(value,context);
    }

    bool do_byte_string(const uint8_t* data, size_t length,
                              const serializing_context& context) override
    {
        return downstream_handler_.write_byte_string(data, length, context);
    }

    bool do_bignum(int signum, const uint8_t* data, size_t length,
                         const serializing_context& context) override
    {
        return downstream_handler_.write_bignum(signum, data, length, context);
    }

    bool do_double(double value, const floating_point_options& fmt,
                         const serializing_context& context) override
    {
        return downstream_handler_.write_double(value, fmt, context);
    }

    bool do_integer(int64_t value,
                          const serializing_context& context) override
    {
        return downstream_handler_.write_integer(value,context);
    }

    bool do_uinteger(uint64_t value,
                           const serializing_context& context) override
    {
        return downstream_handler_.write_uinteger(value,context);
    }

    bool do_bool(bool value,
                       const serializing_context& context) override
    {
        return downstream_handler_.write_bool(value,context);
    }

    bool do_null(const serializing_context& context) override
    {
        return downstream_handler_.write_null(context);
    }

};

// Filters out begin_document and end_document events
template <class CharT>
class basic_json_fragment_filter : public basic_json_filter<CharT>
{
public:
    using typename basic_json_filter<CharT>::string_view_type;

    basic_json_fragment_filter(basic_json_content_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler)
    {
    }
private:
    void do_begin_document() override
    {
    }

    void do_end_document() override
    {
    }
};

template <class CharT>
class basic_rename_object_member_filter : public basic_json_filter<CharT>
{
public:
    using typename basic_json_filter<CharT>::string_view_type;

private:
    std::basic_string<CharT> name_;
    std::basic_string<CharT> new_name_;
public:
    basic_rename_object_member_filter(const std::basic_string<CharT>& name,
                             const std::basic_string<CharT>& new_name,
                             basic_json_content_handler<CharT>& handler)
        : basic_json_filter<CharT>(handler), 
          name_(name), new_name_(new_name)
    {
    }

private:
    bool do_name(const string_view_type& name,
                 const serializing_context& context) override
    {
        if (name == name_)
        {
            return this->downstream_handler().write_name(new_name_,context);
        }
        else
        {
            return this->downstream_handler().write_name(name,context);
        }
    }
};

template <class CharT>
class basic_utf8_adaptor : public basic_json_content_handler<char>
{
public:
    using typename basic_json_content_handler<char>::string_view_type;
private:
    basic_json_content_handler<CharT>& downstream_handler_;

    // noncopyable and nonmoveable
    basic_utf8_adaptor<CharT>(const basic_utf8_adaptor<CharT>&) = delete;
    basic_utf8_adaptor<CharT>& operator=(const basic_utf8_adaptor<CharT>&) = delete;
public:
    basic_utf8_adaptor(basic_json_content_handler<CharT>& handler)
        : downstream_handler_(handler)
    {
    }

    basic_json_content_handler<CharT>& downstream_handler()
    {
        return downstream_handler_;
    }

private:
    void do_begin_document() override
    {
        downstream_handler_.begin_document();
    }

    void do_end_document() override
    {
        downstream_handler_.end_document();
    }

    bool do_begin_object(const serializing_context& context) override
    {
        return downstream_handler_.begin_object(context);
    }

    bool do_begin_object(size_t length, const serializing_context& context) override
    {
        return downstream_handler_.begin_object(length, context);
    }

    bool do_end_object(const serializing_context& context) override
    {
        return downstream_handler_.end_object(context);
    }

    bool do_begin_array(const serializing_context& context) override
    {
        return downstream_handler_.begin_array(context);
    }

    bool do_begin_array(size_t length, const serializing_context& context) override
    {
        return downstream_handler_.begin_array(length, context);
    }

    bool do_end_array(const serializing_context& context) override
    {
        return downstream_handler_.end_array(context);
    }

    bool do_name(const string_view_type& name,
                 const serializing_context& context) override
    {
        std::basic_string<CharT> target;
        auto result = unicons::convert(name.begin(),name.end(),std::back_inserter(target),unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }
        return downstream_handler().write_name(target,context);
    }

    bool do_string(const string_view_type& value,
                         const serializing_context& context) override
    {
        std::basic_string<CharT> target;
        auto result = unicons::convert(value.begin(),value.end(),std::back_inserter(target),unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }
        return downstream_handler().write_string(target,context);
    }

    bool do_byte_string(const uint8_t* data, size_t length,
                              const serializing_context& context) override
    {
        return downstream_handler_.write_byte_string(data, length, context);
    }

    bool do_bignum(int signum, const uint8_t* data, size_t length,
                         const serializing_context& context) override
    {
        return downstream_handler_.write_bignum(signum, data, length, context);
    }

    bool do_double(double value, const floating_point_options& fmt,
                         const serializing_context& context) override
    {
        return downstream_handler_.write_double(value, fmt, context);
    }

    bool do_integer(int64_t value,
                          const serializing_context& context) override
    {
        return downstream_handler_.write_integer(value,context);
    }

    bool do_uinteger(uint64_t value,
                           const serializing_context& context) override
    {
        return downstream_handler_.write_uinteger(value,context);
    }

    bool do_bool(bool value,
                       const serializing_context& context) override
    {
        return downstream_handler_.write_bool(value,context);
    }

    bool do_null(const serializing_context& context) override
    {
        return downstream_handler_.write_null(context);
    }

};

typedef basic_utf8_adaptor<char> json_filter;
typedef basic_utf8_adaptor<wchar_t> wjson_filter;
typedef basic_rename_object_member_filter<char> rename_object_member_filter;
typedef basic_rename_object_member_filter<wchar_t> wrename_object_member_filter;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_rename_object_member_filter<char> rename_name_filter;
typedef basic_rename_object_member_filter<wchar_t> wrename_name_filter;
#endif

}

#endif
