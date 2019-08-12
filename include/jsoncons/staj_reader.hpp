// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STAJ_READER_HPP
#define JSONCONS_STAJ_READER_HPP

#include <memory> // std::allocator
#include <string>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <type_traits> // std::enable_if
#include <array> // std::array
#include <functional> // std::function
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/ser_context.hpp>
#include <jsoncons/result.hpp>
#include <jsoncons/detail/print_number.hpp>

namespace jsoncons {

enum class staj_event_type
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
    uint64_value,
    double_value
};

inline
std::ostream& operator<<(std::ostream& os, staj_event_type tag)
{
    switch (tag)
    {
        case staj_event_type::begin_array:
        {
            os << "begin_array";
            break;
        }
        case staj_event_type::end_array:
        {
            os << "end_array";
            break;
        }
        case staj_event_type::begin_object:
        {
            os << "begin_object";
            break;
        }
        case staj_event_type::end_object:
        {
            os << "end_object";
            break;
        }
        case staj_event_type::name:
        {
            os << "name";
            break;
        }
        case staj_event_type::string_value:
        {
            os << "string_value";
            break;
        }
        case staj_event_type::byte_string_value:
        {
            os << "byte_string_value";
            break;
        }
        case staj_event_type::null_value:
        {
            os << "null_value";
            break;
        }
        case staj_event_type::bool_value:
        {
            os << "bool_value";
            break;
        }
        case staj_event_type::int64_value:
        {
            os << "int64_value";
            break;
        }
        case staj_event_type::uint64_value:
        {
            os << "uint64_value";
            break;
        }
        case staj_event_type::double_value:
        {
            os << "double_value";
            break;
        }
    }
    return os;
}

JSONCONS_STRING_LITERAL(null,'n','u','l','l')
JSONCONS_STRING_LITERAL(true,'t','r','u','e')
JSONCONS_STRING_LITERAL(false,'f','a','l','s','e')

template<class CharT>
class basic_staj_event
{
    staj_event_type event_type_;
    semantic_tag tag_;
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
    typedef basic_string_view<CharT> string_view_type;

    basic_staj_event(staj_event_type event_type, semantic_tag tag = semantic_tag::none)
        : event_type_(event_type), tag_(tag), length_(0)
    {
    }

    basic_staj_event(null_type, semantic_tag tag)
        : event_type_(staj_event_type::null_value), tag_(tag), length_(0)
    {
    }

    basic_staj_event(bool value, semantic_tag tag)
        : event_type_(staj_event_type::bool_value), tag_(tag), length_(0)
    {
        value_.bool_value_ = value;
    }

    basic_staj_event(int64_t value, semantic_tag tag)
        : event_type_(staj_event_type::int64_value), tag_(tag), length_(0)
    {
        value_.int64_value_ = value;
    }

    basic_staj_event(uint64_t value, semantic_tag tag)
        : event_type_(staj_event_type::uint64_value), tag_(tag), length_(0)
    {
        value_.uint64_value_ = value;
    }

    basic_staj_event(double value, semantic_tag tag)
        : event_type_(staj_event_type::double_value), tag_(tag), length_(0)
    {
        value_.double_value_ = value;
    }

    basic_staj_event(const string_view_type& s,
        staj_event_type event_type,
        semantic_tag tag = semantic_tag::none)
        : event_type_(event_type), tag_(tag), length_(s.length())
    {
        value_.string_data_ = s.data();
    }

    basic_staj_event(const byte_string_view& s,
        staj_event_type event_type,
        semantic_tag tag = semantic_tag::none)
        : event_type_(event_type), tag_(tag), length_(s.length())
    {
        value_.byte_string_data_ = s.data();
    }

    template<class T, class CharT_ = CharT>
    typename std::enable_if<jsoncons::detail::is_string_like<T>::value && std::is_same<typename T::value_type, CharT_>::value, T>::type
        get() const
    {
        T s;
        switch (event_type_)
        {
        case staj_event_type::name:
        case staj_event_type::string_value:
            s = T(value_.string_data_, length_);
            break;
        case staj_event_type::int64_value:
        {
            jsoncons::string_result<T> result(s);
            jsoncons::detail::print_integer(value_.int64_value_, result);
            break;
        }
        case staj_event_type::uint64_value:
        {
            jsoncons::string_result<T> result(s);
            jsoncons::detail::print_uinteger(value_.uint64_value_, result);
            break;
        }
        case staj_event_type::double_value:
        {
            jsoncons::string_result<T> result(s);
            jsoncons::detail::print_double f{float_chars_format::general,0};
            f(value_.double_value_, result);
            break;
        }
        case staj_event_type::bool_value:
        {
            jsoncons::string_result<T> result(s);
            if (value_.bool_value_)
            {
                result.append(true_literal<CharT>().data(),true_literal<CharT>().size());
            }
            else
            {
                result.append(false_literal<CharT>().data(),false_literal<CharT>().size());
            }
            break;
        }
        case staj_event_type::null_value:
        {
            jsoncons::string_result<T> result(s);
            result.append(null_literal<CharT>().data(),null_literal<CharT>().size());
            break;
        }
        default:
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a string"));
        }
        return s;
    }

    template<class T, class CharT_ = CharT>
    typename std::enable_if<jsoncons::detail::is_string_view_like<T>::value && std::is_same<typename T::value_type, CharT_>::value, T>::type
        get() const
    {
        T s;
        switch (event_type_)
        {
        case staj_event_type::name:
        case staj_event_type::string_value:
            s = T(value_.string_data_, length_);
            break;
        default:
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a string"));
        }
        return s;
    }

    template<class T>
    typename std::enable_if<std::is_same<T, byte_string_view>::value, T>::type
        get() const
    {
        T s;
        switch (event_type_)
        {
        case staj_event_type::byte_string_value:
            s = T(value_.byte_string_data_, length_);
            break;
        default:
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a byte_string"));
        }
        return s;
    }

    template<class T>
    typename std::enable_if<jsoncons::detail::is_integer_like<T>::value, T>::type
        get() const
    {
        return static_cast<T>(as_int64());
    }

    template<class T>
    typename std::enable_if<jsoncons::detail::is_uinteger_like<T>::value, T>::type
        get() const
    {
        return static_cast<T>(as_uint64());
    }

    template<class T>
    typename std::enable_if<jsoncons::detail::is_floating_point_like<T>::value, T>::type
        get() const
    {
        return static_cast<T>(as_double());
    }

    template<class T, class UserAllocator = std::allocator<uint8_t>>
    typename std::enable_if<std::is_same<T, basic_bignum<UserAllocator>>::value, T>::type
        get() const
    {
        return as_bignum<UserAllocator>();
    }

    template<class T>
    typename std::enable_if<std::is_same<T, bool>::value, T>::type
        get() const
    {
        return as_bool();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    template<class T>
    JSONCONS_DEPRECATED_MSG("Instead, use get<T>()")
    T as() const
    {
        return get<T>();
    }
    semantic_tag get_semantic_tag() const noexcept { return tag_; }
#endif

    staj_event_type event_type() const noexcept { return event_type_; }

    semantic_tag tag() const noexcept { return tag_; }
private:

    int64_t as_int64() const
    {
        int64_t value = 0;
        switch (event_type_)
        {
            case staj_event_type::name:
            case staj_event_type::string_value:
            {
                auto result = jsoncons::detail::to_integer<int64_t>(value_.string_data_, length_);
                if (result.ec != jsoncons::detail::to_integer_errc())
                {
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>(make_error_code(result.ec).message()));
                }
                value = result.value;
                break;
            }
            case staj_event_type::double_value:
                value = static_cast<int64_t>(value_.double_value_);
                break;
            case staj_event_type::int64_value:
                value = value_.int64_value_;
                break;
            case staj_event_type::uint64_value:
                value = static_cast<int64_t>(value_.uint64_value_);
                break;
            case staj_event_type::bool_value:
                value = value_.bool_value_ ? 1 : 0;
                break;
            default:
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an integer"));
        }
        return value;
    }

    uint64_t as_uint64() const
    {
        uint64_t value = 0;
        switch (event_type_)
        {
            case staj_event_type::name:
            case staj_event_type::string_value:
            {
                auto result = jsoncons::detail::to_integer<uint64_t>(value_.string_data_, length_);
                if (result.ec != jsoncons::detail::to_integer_errc())
                {
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>(make_error_code(result.ec).message()));
                }
                value = result.value;
                break;
            }
            case staj_event_type::double_value:
                value = static_cast<uint64_t>(value_.double_value_);
                break;
            case staj_event_type::int64_value:
                value = static_cast<uint64_t>(value_.int64_value_);
                break;
            case staj_event_type::uint64_value:
                value = value_.uint64_value_;
                break;
            case staj_event_type::bool_value:
                value = value_.bool_value_ ? 1 : 0;
                break;
            default:
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an unsigned integer"));
        }
        return value;
    }

    double as_double() const
    {
        switch (event_type_)
        {
            case staj_event_type::name:
            case staj_event_type::string_value:
            {
                jsoncons::detail::string_to_double f;
                return f(value_.string_data_, length_);
            }
            case staj_event_type::double_value:
                return value_.double_value_;
            case staj_event_type::int64_value:
                return static_cast<double>(value_.int64_value_);
            case staj_event_type::uint64_value:
                return static_cast<double>(value_.uint64_value_);
            default:
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a double"));
        }
    }

    bool as_bool() const
    {
        switch (event_type_)
        {
            case staj_event_type::bool_value:
                return value_.bool_value_;
            case staj_event_type::double_value:
                return value_.double_value_ != 0.0;
            case staj_event_type::int64_value:
                return value_.int64_value_ != 0;
            case staj_event_type::uint64_value:
                return value_.uint64_value_ != 0;
            default:
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a bool"));
        }
    }

    template <class UserAllocator = std::allocator<uint8_t>>
    basic_bignum<UserAllocator> as_bignum() const
    {
        switch (event_type_)
        {
            case staj_event_type::string_value:
                if (!jsoncons::detail::is_integer(value_.string_data_, length_))
                {
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a bignum"));
                }
                return basic_bignum<UserAllocator>(value_.string_data_, length_);
            case staj_event_type::double_value:
                return basic_bignum<UserAllocator>(value_.double_value_);
            case staj_event_type::int64_value:
                return basic_bignum<UserAllocator>(value_.int64_value_);
            case staj_event_type::uint64_value:
                return basic_bignum<UserAllocator>(value_.uint64_value_);
            case staj_event_type::bool_value:
                return basic_bignum<UserAllocator>(value_.bool_value_ ? 1 : 0);
            default:
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not a bignum"));
        }
    }

};

// basic_staj_event_handler

template <class CharT>
class basic_staj_event_handler final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
private:
    std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter_;
    basic_staj_event<CharT> event_;
public:
    basic_staj_event_handler()
        : filter_(accept), event_(staj_event_type::null_value)
    {
    }

    basic_staj_event_handler(std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter)
        : filter_(filter), event_(staj_event_type::null_value)
    {
    }

    const basic_staj_event<CharT>& event() const
    {
        return event_;
    }
private:
    static bool accept(const basic_staj_event<CharT>&, const ser_context&) 
    {
        return true;
    }

    bool do_begin_object(semantic_tag tag, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::begin_object, tag);
        return !filter_(event_, context);
    }

    bool do_end_object(const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::end_object);
        return !filter_(event_, context);
    }

    bool do_begin_array(semantic_tag tag, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::begin_array, tag);
        return !filter_(event_, context);
    }

    bool do_end_array(const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::end_array);
        return !filter_(event_, context);
    }

    bool do_name(const string_view_type& name, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(name, staj_event_type::name);
        return !filter_(event_, context);
    }

    bool do_null_value(semantic_tag tag, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::null_value, tag);
        return !filter_(event_, context);
    }

    bool do_bool_value(bool value, semantic_tag tag, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_string_value(const string_view_type& s, semantic_tag tag, const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(s, staj_event_type::string_value, tag);
        return !filter_(event_, context);
    }

    bool do_byte_string_value(const byte_string_view& s, 
                              semantic_tag tag,
                              const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(s, staj_event_type::byte_string_value, tag);
        return !filter_(event_, context);
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag tag,
                        const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag tag, 
                         const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_double_value(double value, 
                         semantic_tag tag, 
                         const ser_context& context) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    void do_flush() override
    {
    }
};

template<class CharT>
bool staj_to_saj_event(const basic_staj_event<CharT>& ev,
                       basic_json_content_handler<CharT>& handler,
                       const ser_context& context)
{
    switch (ev.event_type())
    {
        case staj_event_type::begin_array:
            return handler.begin_array(ev.tag(), context);
        case staj_event_type::end_array:
            return handler.end_array(context);
        case staj_event_type::begin_object:
            return handler.begin_object(ev.tag(), context);
        case staj_event_type::end_object:
            return handler.end_object(context);
        case staj_event_type::name:
            return handler.name(ev.template get<jsoncons::basic_string_view<CharT>>(), context);
        case staj_event_type::string_value:
            return handler.string_value(ev.template get<basic_string_view<CharT>>(), ev.tag(), context);
        case staj_event_type::byte_string_value:
            return handler.byte_string_value(ev.template get<byte_string_view>(), ev.tag(), context);
        case staj_event_type::null_value:
            return handler.null_value(ev.tag(), context);
        case staj_event_type::bool_value:
            return handler.bool_value(ev.template get<bool>(), ev.tag(), context);
        case staj_event_type::int64_value:
            return handler.int64_value(ev.template get<int64_t>(), ev.tag(), context);
        case staj_event_type::uint64_value:
            return handler.uint64_value(ev.template get<uint64_t>(), ev.tag(), context);
        case staj_event_type::double_value:
            return handler.double_value(ev.template get<double>(), ev.tag(), context);
        default:
            return false;
    }
}

// basic_staj_reader

 template<class CharT>
class basic_staj_reader
{
public:
    virtual ~basic_staj_reader() = default;

    virtual bool done() const = 0;

    virtual const basic_staj_event<CharT>& current() const = 0;

    virtual void read_to(basic_json_content_handler<CharT>& handler) = 0;

    virtual void read_to(basic_json_content_handler<CharT>& handler,
                        std::error_code& ec) = 0;

    virtual void next() = 0;

    virtual void next(std::error_code& ec) = 0;

    virtual const ser_context& context() const = 0;
};

typedef basic_staj_event<char> staj_event;
typedef basic_staj_event<wchar_t> wstaj_event;

typedef basic_staj_reader<char> staj_reader;
typedef basic_staj_reader<wchar_t> wstaj_reader;

#if !defined(JSONCONS_NO_DEPRECATED)

JSONCONS_DEPRECATED_MSG("Instead, use staj_event_type") typedef staj_event_type stream_event_type;

template<class CharT>
using basic_stream_event = basic_staj_event<CharT>;

template<class CharT>
using basic_stream_reader = basic_staj_reader<CharT>;

JSONCONS_DEPRECATED_MSG("Instead, use staj_event") typedef basic_staj_event<char> stream_event;
JSONCONS_DEPRECATED_MSG("Instead, use wstaj_event") typedef basic_staj_event<wchar_t> wstream_event;

JSONCONS_DEPRECATED_MSG("Instead, use staj_reader") typedef basic_staj_reader<char> stream_reader;
JSONCONS_DEPRECATED_MSG("Instead, use wstaj_reader") typedef basic_staj_reader<wchar_t> wstream_reader;

#endif

}

#endif

