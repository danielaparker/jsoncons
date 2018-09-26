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
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/print_number.hpp>

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

template<class CharT>
class basic_stream_event
{
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

    template<class T, class CharT_ = CharT>
    typename std::enable_if<std::is_same<T,std::basic_string<CharT_>>::value,T>::type
    as() const
    {
        std::basic_string<CharT> s;
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
                s = std::basic_string<CharT>(value_.string_data_,length_);
                break;
            case stream_event_type::int64_value:
            {
                detail::string_writer<T> writer(s);
                detail::print_integer(value_.int64_value_, writer);
                break;
            }
            case stream_event_type::uint64_value:
            {
                detail::string_writer<T> writer(s);
                detail::print_uinteger(value_.uint64_value_, writer);
                break;
            }
            case stream_event_type::double_value:
            {
                detail::string_writer<T> writer(s);
                floating_point_options def;
                detail::print_double f(def);
                f(value_.double_value_, def, writer);
                break;
            }
            case stream_event_type::bool_value:
            {
                detail::string_writer<T> writer(s);
                if (value_.bool_value_)
                {
                    writer.write(detail::true_literal<CharT>().data(),
                                 detail::true_literal<CharT>().length());
                }
                else
                {
                    writer.write(detail::false_literal<CharT>().data(),
                                 detail::false_literal<CharT>().length());
                }
                break;
            }
            case stream_event_type::null_value:
            {
                detail::string_writer<T> writer(s);
                writer.write(detail::null_literal<CharT>().data(), 
                             detail::null_literal<CharT>().size());
                break;
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a string"));
        }
        return s;
    }

    template<class T>
    typename std::enable_if<detail::is_integer_like<T>::value,T>::type
    as() const
    {
        return static_cast<T>(as_integer());
    }

    template<class T>
    typename std::enable_if<detail::is_uinteger_like<T>::value,T>::type
    as() const
    {
        return static_cast<T>(as_uinteger());
    }

    template<class T>
    typename std::enable_if<detail::is_floating_point_like<T>::value,T>::type
    as() const
    {
        return static_cast<T>(as_double());
    }

    template<class T>
    typename std::enable_if<std::is_same<T,bool>::value,T>::type
    as() const
    {
        return static_cast<T>(as_bool());
    }

    stream_event_type event_type() const JSONCONS_NOEXCEPT {return event_type_;}
private:

    int64_t as_integer() const
    {
        int64_t value = 0;
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
            {
                if (!detail::is_integer(value_.string_data_,length_))
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
                }
                auto result = detail::to_integer(value_.string_data_,length_);
                if (result.overflow)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Integer overflow"));
                }
                value = result.value;
                break;
            }
        case stream_event_type::double_value:
            value = static_cast<int64_t>(value_.double_value_);
            break;
        case stream_event_type::int64_value:
            value = static_cast<int64_t>(value_.int64_value_);
            break;
        case stream_event_type::uint64_value:
            value = static_cast<int64_t>(value_.uint64_value_);
            break;
        case stream_event_type::bool_value:
            value = value_.bool_value_ ? 1 : 0;
            break;
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
        }
        return value;
    }

    uint64_t as_uinteger() const
    {
        uint64_t value = 0;
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
            {
                if (!detail::is_uinteger(value_.string_data_,length_))
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
                }
                auto result = detail::to_uinteger(value_.string_data_,length_);
                if (result.overflow)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Integer overflow"));
                }
                value = result.value;
                break;
            }
        case stream_event_type::double_value:
            value = static_cast<uint64_t>(value_.double_value_);
            break;
        case stream_event_type::int64_value:
            value = static_cast<uint64_t>(value_.int64_value_);
            break;
        case stream_event_type::uint64_value:
            value = static_cast<uint64_t>(value_.uint64_value_);
            break;
        case stream_event_type::bool_value:
            value = value_.bool_value_ ? 1 : 0;
            break;
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an unsigned integer"));
        }
        return value;
    }

    double as_double() const
    {
        double value = 0;
        switch (event_type_)
        {
            case stream_event_type::name:
            case stream_event_type::string_value:
            {
                std::string target;
                auto result = unicons::convert(
                    value_.string_data_,value_.string_data_+length_, std::back_inserter(target),unicons::conv_flags::strict);
                if (result.ec != unicons::conv_errc())
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
                }
                detail::string_to_double f;
                value = f(target.data(),target.length());
                break;
            }
        case stream_event_type::double_value:
            return value_.double_value_;
        case stream_event_type::int64_value:
            return static_cast<double>(value_.int64_value_);
        case stream_event_type::uint64_value:
            return static_cast<double>(value_.uint64_value_);
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
        }
    }

    bool as_bool() const 
    {
        switch (value_.data_type())
        {
        case stream_event_type::bool_value:
            return value_.bool_value_;
        case stream_event_type::double_value:
            return value_.double_value_ != 0.0;
        case stream_event_type::int64_value:
            return value_.int64_value_ != 0;
        case stream_event_type::uint64_value:
            return value_.uint64_value_ != 0;
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bool"));
        }
    }
};

template<class CharT>
class basic_stream_reader 
{
public:

    virtual ~basic_stream_reader() = default;

    virtual bool done() const = 0;

    virtual const basic_stream_event<CharT>& current() const = 0;

    virtual void next() = 0;

    virtual size_t line_number() const = 0;

    virtual size_t column_number() const = 0;
};

typedef basic_stream_reader<char> stream_reader;
typedef basic_stream_reader<wchar_t> wstream_reader;

typedef basic_stream_event<char> stream_event;
typedef basic_stream_event<wchar_t> wstream_event;

}

#endif

