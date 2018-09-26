// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STREAMREADER_HPP
#define JSONCONS_STREAMREADER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <iterator>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json.hpp>

namespace jsoncons {

enum class stream_event_type
{
    begin_array,
    end_array,
    begin_object,
    end_object,
    name,
    string_value,
    byte_string_value,
    null_value,
    bool_value,
    int64_value,
    uint64_value ,
    bignum_value,
    double_value
};

template<class CharT,class Allocator=std::allocator<char>>
class basic_stream_event
{
    typedef basic_json<CharT,sorted_policy,Allocator> json_type;
    stream_event_type event_type_;
    union
    {
        bool bool_value_;
        int64_t int64_value_;
        uint64_t uint64_value_;
        double double_value_;
        const CharT* string_data_;
        const uint8_t* byte_string_data_;
    } value_;
    size_t length_;
public:
    basic_stream_event(stream_event_type event_type)
        : event_type_(event_type), length_(0)
    {
    }

    basic_stream_event(null_type value)
        : event_type_(stream_event_type::null_value), length_(0)
    {
    }

    basic_stream_event(bool value)
        : event_type_(stream_event_type::bool_value), length_(0)
    {
        value_.bool_value_ = value;
    }

    basic_stream_event(int64_t value)
        : event_type_(stream_event_type::int64_value), length_(0)
    {
        value_.int64_value_ = value;
    }

    basic_stream_event(uint64_t value)
        : event_type_(stream_event_type::uint64_value), length_(0)
    {
        value_.uint64_value_ = value;
    }

    basic_stream_event(double value)
        : event_type_(stream_event_type::double_value), length_(0)
    {
        value_.double_value_ = value;
    }

    basic_stream_event(const CharT* data, size_t length, stream_event_type type = stream_event_type::string_value)
        : event_type_(type), length_(length)
    {
        value_.string_data_ = data;
    }

    template<class T, class Traits, class OtherAllocator, class CharT_ = CharT>
    typename std::enable_if<std::is_same<T,std::basic_string<CharT_,Traits,OtherAllocator>>::value,T>::type
    is() const JSONCONS_NOEXCEPT
    {
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
                return true;
            default:
                return false;
        }
    }

    template<class T, class... Args>
    bool is(Args&&... args) const JSONCONS_NOEXCEPT
    {
        return as_json().template is<T>(std::forward<Args>(args)...);
    }

    template<class T, class Traits, class OtherAllocator, class CharT_ = CharT>
    typename std::enable_if<std::is_same<T,std::basic_string<CharT_,Traits,OtherAllocator>>::value,T>::type
    as() const
    {
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
                return std::basic_string<CharT>(value_.string_data_,length_);
            default:
                return as_json().template as<T>();
        }
    }

    template<class T, class... Args>
    T as(Args&&... args) const
    {
        return as_json().template as<T>(std::forward<Args>(args)...);
    }

    stream_event_type event_type() const JSONCONS_NOEXCEPT {return event_type_;}
private:
    json_type as_json() const
    {
        switch (event_type_)
        {
            case stream_event_type::name:
                return json_type(value_.string_data_,length_);
            case stream_event_type::string_value:
                return json_type(value_.string_data_,length_);
            case stream_event_type::bignum_value:
                return json_type(basic_bignum<Allocator>(value_.string_data_,length_));
            case stream_event_type::int64_value:
                return json_type(value_.int64_value_);
            case stream_event_type::uint64_value:
                return json_type(value_.uint64_value_);
            case stream_event_type::double_value:
                return json_type(value_.double_value_);
            case stream_event_type::bool_value:
                return json_type(value_.bool_value_);
            case stream_event_type::null_value:
                return json_type(jsoncons::null_type());
            default:
                JSONCONS_UNREACHABLE();
        }
    }
};

template<class CharT, class Allocator>
class basic_stream_reader 
{
public:

    virtual bool done() const = 0;

    virtual const basic_stream_event<CharT,Allocator>& current() const = 0;

    virtual void next() = 0;

    virtual size_t line_number() const = 0;

    virtual size_t column_number() const = 0;
};

typedef basic_stream_reader<char,std::allocator<char>> stream_reader;
typedef basic_stream_reader<wchar_t,std::allocator<wchar_t>> wstream_reader;

typedef basic_stream_event<char,std::allocator<char>> stream_event;
typedef basic_stream_event<wchar_t,std::allocator<wchar_t>> wstream_event;

}

#endif

