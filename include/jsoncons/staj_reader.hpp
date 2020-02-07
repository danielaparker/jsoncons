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
#include <jsoncons/sink.hpp>
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
    half_value,
    double_value
};

template <class CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, staj_event_type tag)
{
    JSONCONS_ARRAY_OF_CHAR(CharT,begin_array_name,'b','e','g','i','n','_','a','r','r','a','y')
    JSONCONS_ARRAY_OF_CHAR(CharT,end_array_name,'e','n','d','_','a','r','r','a','y')
    JSONCONS_ARRAY_OF_CHAR(CharT,begin_object_name,'b','e','g','i','n','_','o','b','j','e','c','t')
    JSONCONS_ARRAY_OF_CHAR(CharT,end_object_name,'e','n','d','_','o','b','j','e','c','t')
    JSONCONS_ARRAY_OF_CHAR(CharT,name_name,'n','a','m','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,string_value_name,'s','t','r','i','n','g','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,byte_string_value_name,'b','y','t','e','_','s','t','r','i','n','g','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,null_value_name,'n','u','l','l','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,bool_value_name,'b','o','o','l','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,uint64_value_name,'u','i','n','t','6','4','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,int64_value_name,'i','n','t','6','4','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,half_value_name,'h','a','l','f','_','v','a','l','u','e')
    JSONCONS_ARRAY_OF_CHAR(CharT,double_value_name,'d','o','u','b','l','e','_','v','a','l','u','e')

    switch (tag)
    {
        case staj_event_type::begin_array:
        {
            os << begin_array_name;
            break;
        }
        case staj_event_type::end_array:
        {
            os << end_array_name;
            break;
        }
        case staj_event_type::begin_object:
        {
            os << begin_object_name;
            break;
        }
        case staj_event_type::end_object:
        {
            os << end_object_name;
            break;
        }
        case staj_event_type::name:
        {
            os << name_name;
            break;
        }
        case staj_event_type::string_value:
        {
            os << string_value_name;
            break;
        }
        case staj_event_type::byte_string_value:
        {
            os << byte_string_value_name;
            break;
        }
        case staj_event_type::null_value:
        {
            os << null_value_name;
            break;
        }
        case staj_event_type::bool_value:
        {
            os << bool_value_name;
            break;
        }
        case staj_event_type::int64_value:
        {
            os << int64_value_name;
            break;
        }
        case staj_event_type::uint64_value:
        {
            os << uint64_value_name;
            break;
        }
        case staj_event_type::half_value:
        {
            os << half_value_name;
            break;
        }
        case staj_event_type::double_value:
        {
            os << double_value_name;
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
        uint16_t half_value_;
        double double_value_;
        const CharT* string_data_;
        const uint8_t* byte_string_data_;
    } value_;
    std::size_t length_;
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

    basic_staj_event(half_arg_t, uint16_t value, semantic_tag tag)
        : event_type_(staj_event_type::half_value), tag_(tag), length_(0)
    {
        value_.half_value_ = value;
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
        : event_type_(event_type), tag_(tag), length_(s.size())
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
            jsoncons::string_sink<T> sink(s);
            jsoncons::detail::print_integer(value_.int64_value_, sink);
            break;
        }
        case staj_event_type::uint64_value:
        {
            jsoncons::string_sink<T> sink(s);
            jsoncons::detail::print_uinteger(value_.uint64_value_, sink);
            break;
        }
        case staj_event_type::half_value:
        {
            jsoncons::string_sink<T> sink(s);
            jsoncons::detail::print_double f{float_chars_format::general,0};
            double x = jsoncons::detail::decode_half(value_.half_value_);
            f(x, sink);
            break;
        }
        case staj_event_type::double_value:
        {
            jsoncons::string_sink<T> sink(s);
            jsoncons::detail::print_double f{float_chars_format::general,0};
            f(value_.double_value_, sink);
            break;
        }
        case staj_event_type::bool_value:
        {
            jsoncons::string_sink<T> sink(s);
            if (value_.bool_value_)
            {
                sink.append(true_literal<CharT>().data(),true_literal<CharT>().size());
            }
            else
            {
                sink.append(false_literal<CharT>().data(),false_literal<CharT>().size());
            }
            break;
        }
        case staj_event_type::null_value:
        {
            jsoncons::string_sink<T> sink(s);
            sink.append(null_literal<CharT>().data(),null_literal<CharT>().size());
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
                auto sink = jsoncons::detail::integer_from_json<int64_t>(value_.string_data_, length_);
                if (!sink)
                {
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>(sink.error_code().message()));
                }
                value = sink.value();
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
                auto sink = jsoncons::detail::integer_from_json<uint64_t>(value_.string_data_, length_);
                if (!sink)
                {
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>(sink.error_code().message()));
                }
                value = sink.value();
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
            case staj_event_type::half_value:
            {
                double x = jsoncons::detail::decode_half(value_.half_value_);
                return static_cast<double>(x);
            }
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
                if (!jsoncons::detail::is_base10(value_.string_data_, length_))
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

enum class staj_reader_state
{
    typed_array = 1,
    multi_dim,
    shape
};

struct uint8_array_arg_t {explicit uint8_array_arg_t() = default; };
constexpr uint8_array_arg_t uint8_array_arg = uint8_array_arg_t();
struct uint16_array_arg_t {explicit uint16_array_arg_t() = default; };
struct uint32_array_arg_t {explicit uint32_array_arg_t() = default; };
constexpr uint32_array_arg_t uint32_array_arg = uint32_array_arg_t();
struct uint64_array_arg_t {explicit uint64_array_arg_t() = default; };
constexpr uint64_array_arg_t uint64_array_arg = uint64_array_arg_t();
struct int8_array_arg_t {explicit int8_array_arg_t() = default; };
constexpr int8_array_arg_t int8_array_arg = int8_array_arg_t();
struct int16_array_arg_t {explicit int16_array_arg_t() = default; };
constexpr int16_array_arg_t int16_array_arg = int16_array_arg_t();
struct int32_array_arg_t {explicit int32_array_arg_t() = default; };
constexpr int32_array_arg_t int32_array_arg = int32_array_arg_t();
struct int64_array_arg_t {explicit int64_array_arg_t() = default; };
constexpr int64_array_arg_t int64_array_arg = int64_array_arg_t();
constexpr uint16_array_arg_t uint16_array_arg = uint16_array_arg_t();
struct half_array_arg_t {explicit half_array_arg_t() = default; };
constexpr half_array_arg_t half_array_arg = half_array_arg_t();
struct float_array_arg_t {explicit float_array_arg_t() = default; };
constexpr float_array_arg_t float_array_arg = float_array_arg_t();
struct double_array_arg_t {explicit double_array_arg_t() = default; };
constexpr double_array_arg_t double_array_arg = double_array_arg_t();
struct float128_array_arg_t {explicit float128_array_arg_t() = default; };
constexpr float128_array_arg_t float128_array_arg = float128_array_arg_t();

enum typed_array_type {uint8_value=1,uint16_value,uint32_value,uint64_value,
                      int8_value,int16_value,int32_value,int64_value, 
                      half_value, float_value,double_value};

template <class Allocator=std::allocator<char>>
class typed_array
{
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint8_t> uint8_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint16_t> uint16_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint32_t> uint32_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint64_t> uint64_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int8_t> int8_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int16_t> int16_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int32_t> int32_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int64_t> int64_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<float> float_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<double> double_allocator_type;

    Allocator alloc_;
    typed_array_type type_;
    union 
    {
        uint8_t* uint8_data_;
        uint16_t* uint16_data_;
        uint32_t* uint32_data_;
        uint64_t* uint64_data_;
        int8_t* int8_data_;
        int16_t* int16_data_;
        int32_t* int32_data_;
        int64_t* int64_data_;
        float* float_data_;
        double* double_data_;
    } data_;
    std::size_t size_;
public:
    typed_array(const Allocator& alloc)
        : alloc_(alloc), type_(), data_(), size_(0)
    {
    }

    typed_array(const typed_array& other)
        : alloc_(other.alloc_), type_(other.type_), data_(), size_(other.size())
    {
        switch (other.type_)
        {
            case typed_array_type::uint8_value:
            {
                uint8_allocator_type alloc{ alloc_ };
                data_.uint8_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint16_value:
            {
                uint16_allocator_type alloc{ alloc_ };
                data_.uint16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint32_value:
            {
                uint32_allocator_type alloc{ alloc_ };
                data_.uint32_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint64_value:
            {
                uint64_allocator_type alloc{ alloc_ };
                data_.uint64_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int8_value:
            {
                int8_allocator_type alloc{ alloc_ };
                data_.int8_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int16_value:
            {
                int16_allocator_type alloc{ alloc_ };
                data_.int16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int32_value:
            {
                int32_allocator_type alloc{ alloc_ };
                data_.int32_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int64_value:
            {
                int64_allocator_type alloc{ alloc_ };
                data_.int64_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::half_value:
            {
                uint16_allocator_type alloc{ alloc_ };
                data_.uint16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::float_value:
            {
                float_allocator_type alloc{ alloc_ };
                data_.float_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::double_value:
            {
                double_allocator_type alloc{alloc_};
                data_.double_data_ = alloc.allocate(size_);
                break;
            }
            default:
                break;
        }
    }

    typed_array(typed_array&& other)
    {
        swap(*this,other);
    }
    typed_array(uint8_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::uint8_value), size_(size)
    {
        uint8_allocator_type alloc(alloc_);
        data_.uint8_data_ = alloc.allocate(size);
    }

    typed_array(uint16_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::uint16_value), size_(size)
    {
        uint16_allocator_type alloc(alloc_);
        data_.uint16_data_ = alloc.allocate(size);
    }

    typed_array(uint32_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::uint32_value), size_(size)
    {
        uint32_allocator_type alloc(alloc_);
        data_.uint32_data_ = alloc.allocate(size);
    }

    typed_array(uint64_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::uint64_value), size_(size)
    {
        uint64_allocator_type alloc(alloc_);
        data_.uint64_data_ = alloc.allocate(size);
    }

    typed_array(int8_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::int8_value), size_(size)
    {
        int8_allocator_type alloc(alloc_);
        data_.int8_data_ = alloc.allocate(size);
    }

    typed_array(int16_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::int16_value), size_(size)
    {
        int16_allocator_type alloc(alloc_);
        data_.int16_data_ = alloc.allocate(size);
    }

    typed_array(int32_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::int32_value), size_(size)
    {
        int32_allocator_type alloc(alloc_);
        data_.int32_data_ = alloc.allocate(size);
    }

    typed_array(int64_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::int64_value), size_(size)
    {
        int64_allocator_type alloc(alloc_);
        data_.int64_data_ = alloc.allocate(size);
    }

    typed_array(half_array_arg_t, std::size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::half_value), size_(size)
    {
        uint16_allocator_type alloc(alloc_);
        data_.uint16_data_ = alloc.allocate(size);
    }

    typed_array(float_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::float_value), size_(size)
    {
        float_allocator_type alloc(alloc_);
        data_.float_data_ = alloc.allocate(size);
    }

    typed_array(double_array_arg_t,size_t size, const Allocator& allocator)
        : alloc_(allocator), type_(typed_array_type::double_value), size_(size)
    {
        double_allocator_type alloc(alloc_);
        data_.double_data_ = alloc.allocate(size);
    }

    ~typed_array()
    {
        switch (type_)
        {
            case typed_array_type::uint8_value:
            {
                uint8_allocator_type alloc(alloc_);
                alloc.deallocate(data_.uint8_data_, size_);
                break;
            }
            case typed_array_type::uint16_value:
            {
                uint16_allocator_type alloc(alloc_);
                alloc.deallocate(data_.uint16_data_, size_);
                break;
            }
            case typed_array_type::uint32_value:
            {
                uint32_allocator_type alloc(alloc_);
                alloc.deallocate(data_.uint32_data_, size_);
                break;
            }
            case typed_array_type::uint64_value:
            {
                uint64_allocator_type alloc(alloc_);
                alloc.deallocate(data_.uint64_data_, size_);
                break;
            }
            case typed_array_type::int8_value:
            {
                int8_allocator_type alloc(alloc_);
                alloc.deallocate(data_.int8_data_, size_);
                break;
            }
            case typed_array_type::int16_value:
            {
                int16_allocator_type alloc(alloc_);
                alloc.deallocate(data_.int16_data_, size_);
                break;
            }
            case typed_array_type::int32_value:
            {
                int32_allocator_type alloc(alloc_);
                alloc.deallocate(data_.int32_data_, size_);
                break;
            }
            case typed_array_type::int64_value:
            {
                int64_allocator_type alloc(alloc_);
                alloc.deallocate(data_.int64_data_, size_);
                break;
            }
            case typed_array_type::float_value:
            {
                float_allocator_type alloc(alloc_);
                alloc.deallocate(data_.float_data_, size_);
                break;
            }
            case typed_array_type::double_value:
            {
                double_allocator_type alloc(alloc_);
                alloc.deallocate(data_.double_data_, size_);
                break;
            }
            default:
                break;
        }
        type_ = typed_array_type();
        size_ = 0;
    }

    typed_array& operator=(const typed_array& other)
    {
        if (this != &other)
        {
            typed_array temp(other);
            swap(*this,temp);
        }
        return *this;
    }

    typed_array& operator=(typed_array&& other)
    {
        if (this != &other)
        {
            swap(*this,other);
        }
        return *this;
    }

    typed_array_type type() const
    {
        return type_;
    }

    std::size_t size() const
    {
        return size_;
    }

    span<uint8_t> data(uint8_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<uint8_t>(data_.uint8_data_, size_);
    }

    span<const uint8_t> data(uint8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<const uint8_t>(data_.uint8_data_, size_);
    }
    span<uint16_t> data(uint16_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint16_t> data(uint16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<uint32_t> data(uint32_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<uint32_t>(data_.uint32_data_, size_);
    }

    span<const uint32_t> data(uint32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<const uint32_t>(data_.uint32_data_, size_);
    }

    span<uint64_t> data(uint64_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
        return span<uint64_t>(data_.uint64_data_, size_);
    }

    span<const uint64_t> data(uint64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
        return span<const uint64_t>(data_.uint64_data_, size_);
    }

    span<int8_t> data(int8_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<int8_t>(data_.int8_data_, size_);
    }

    span<const int8_t> data(int8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<const int8_t>(data_.int8_data_, size_);
    }

    span<int16_t> data(int16_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<int16_t>(data_.int16_data_, size_);
    }

    span<const int16_t> data(int16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<const int16_t>(data_.int16_data_, size_);
    }

    span<int32_t> data(int32_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<int32_t>(data_.int32_data_, size_);
    }

    span<const int32_t> data(int32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<const int32_t>(data_.int32_data_, size_);
    }

    span<int64_t> data(int64_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<int64_t>(data_.int64_data_, size_);
    }

    span<const int64_t> data(int64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<const int64_t>(data_.int64_data_, size_);
    }

    span<uint16_t> data(half_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint16_t> data(half_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<float> data(float_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<float>(data_.float_data_, size_);
    }

    span<const float> data(float_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<const float>(data_.float_data_, size_);
    }

    span<double> data(double_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<double>(data_.double_data_, size_);
    }

    span<const double> data(double_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<const double>(data_.double_data_, size_);
    }
    friend void swap(typed_array& a, typed_array& b) noexcept
    {
        std::swap(a.data_,b.data_);
        std::swap(a.alloc_,b.alloc_);
        std::swap(a.type_,b.type_);
        std::swap(a.size_,b.size_);
    }
   
};

class typed_array_view
{
    typed_array_type type_;
    union 
    {
        const uint8_t* uint8_data_;
        const uint16_t* uint16_data_;
        const uint32_t* uint32_data_;
        const uint64_t* uint64_data_;
        const int8_t* int8_data_;
        const int16_t* int16_data_;
        const int32_t* int32_data_;
        const int64_t* int64_data_;
        const float* float_data_;
        const double* double_data_;
    } data_;
    std::size_t size_;
public:

    typed_array_view()
        : type_(), data_(), size_(0)
    {
    }

    typed_array_view(const typed_array_view& other)
        : type_(other.type_), data_(other.data_), size_(other.size())
    {
    }

    typed_array_view(typed_array_view&& other)
    {
        swap(*this,other);
    }

    typed_array_view(const typed_array<>& other)
        : type_(other.type()), data_(), size_(other.size())
    {
        switch (other.type())
        {
            case typed_array_type::uint8_value:
            {
                data_.uint8_data_ = other.data(uint8_array_arg).data();
                break;
            }
            case typed_array_type::uint16_value:
            {
                data_.uint16_data_ = other.data(uint16_array_arg).data();
                break;
            }
            case typed_array_type::uint32_value:
            {
                data_.uint32_data_ = other.data(uint32_array_arg).data();
                break;
            }
            case typed_array_type::uint64_value:
            {
                data_.uint64_data_ = other.data(uint64_array_arg).data();
                break;
            }
            case typed_array_type::int8_value:
            {
                data_.int8_data_ = other.data(int8_array_arg).data();
                break;
            }
            case typed_array_type::int16_value:
            {
                data_.int16_data_ = other.data(int16_array_arg).data();
                break;
            }
            case typed_array_type::int32_value:
            {
                data_.int32_data_ = other.data(int32_array_arg).data();
                break;
            }
            case typed_array_type::int64_value:
            {
                data_.int64_data_ = other.data(int64_array_arg).data();
                break;
            }
            case typed_array_type::float_value:
            {
                data_.float_data_ = other.data(float_array_arg).data();
                break;
            }
            case typed_array_type::double_value:
            {
                data_.double_data_ = other.data(double_array_arg).data();
                break;
            }
            default:
                break;
        }
    }

    typed_array_view(const uint8_t* data,size_t size)
        : type_(typed_array_type::uint8_value), size_(size)
    {
        data_.uint8_data_ = data;
    }

    typed_array_view(const uint16_t* data,size_t size)
        : type_(typed_array_type::uint16_value), size_(size)
    {
        data_.uint16_data_ = data;
    }

    typed_array_view(const uint32_t* data,size_t size)
        : type_(typed_array_type::uint32_value), size_(size)
    {
        data_.uint32_data_ = data;
    }

    typed_array_view(const uint64_t* data,size_t size)
        : type_(typed_array_type::uint64_value), size_(size)
    {
        data_.uint64_data_ = data;
    }

    typed_array_view(const int8_t* data,size_t size)
        : type_(typed_array_type::int8_value), size_(size)
    {
        data_.int8_data_ = data;
    }

    typed_array_view(const int16_t* data,size_t size)
        : type_(typed_array_type::int16_value), size_(size)
    {
        data_.int16_data_ = data;
    }

    typed_array_view(const int32_t* data,size_t size)
        : type_(typed_array_type::int32_value), size_(size)
    {
        data_.int32_data_ = data;
    }

    typed_array_view(const int64_t* data,size_t size)
        : type_(typed_array_type::int64_value), size_(size)
    {
        data_.int64_data_ = data;
    }

    typed_array_view(half_array_arg_t, const uint16_t* data, std::size_t size)
        : type_(typed_array_type::half_value), size_(size)
    {
        data_.uint16_data_ = data;
    }

    typed_array_view(const float* data,size_t size)
        : type_(typed_array_type::float_value), size_(size)
    {
        data_.float_data_ = data;
    }

    typed_array_view(const double* data,size_t size)
        : type_(typed_array_type::double_value), size_(size)
    {
        data_.double_data_ = data;
    }

    typed_array_view& operator=(const typed_array_view& other)
    {
        typed_array_view temp(other);
        swap(*this,temp);
        return *this;
    }

    typed_array_type type() const {return type_;}

    std::size_t size() const
    {
        return size_;
    }

    span<const uint8_t> data(uint8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<const uint8_t>(data_.uint8_data_, size_);
    }

    span<const uint16_t> data(uint16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint32_t> data(uint32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<const uint32_t>(data_.uint32_data_, size_);
    }

    span<const uint64_t> data(uint64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
        return span<const uint64_t>(data_.uint64_data_, size_);
    }

    span<const int8_t> data(int8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<const int8_t>(data_.int8_data_, size_);
    }

    span<const int16_t> data(int16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<const int16_t>(data_.int16_data_, size_);
    }

    span<const int32_t> data(int32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<const int32_t>(data_.int32_data_, size_);
    }

    span<const int64_t> data(int64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<const int64_t>(data_.int64_data_, size_);
    }

    span<const uint16_t> data(half_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<const float> data(float_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<const float>(data_.float_data_, size_);
    }

    span<const double> data(double_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<const double>(data_.double_data_, size_);
    }

    friend void swap(typed_array_view& a, typed_array_view& b) noexcept
    {
        std::swap(a.data_,b.data_);
        std::swap(a.type_,b.type_);
        std::swap(a.size_,b.size_);
    }
};

template <class CharT>
class basic_staj_event_handler : public basic_json_content_handler<CharT>
{
    using super_type = basic_json_content_handler<CharT>;
public:
    using char_type = CharT;
    using typename super_type::string_view_type;
private:
    std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter_;
    basic_staj_event<CharT> event_;

    staj_reader_state state_;
    typed_array_view data_;
    span<const size_t> shape_;
    std::size_t index_;
public:
    basic_staj_event_handler()
        : filter_(accept), event_(staj_event_type::null_value),
          state_(), data_(), shape_(), index_(0)
    {
    }

    basic_staj_event_handler(std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter)
        : filter_(filter), event_(staj_event_type::null_value),
          state_(), data_(), shape_(), index_(0)
    {
    }

    const basic_staj_event<CharT>& event() const
    {
        return event_;
    }

    bool in_available() const
    {
        return state_ != staj_reader_state();
    }

    void send_available(std::error_code& ec)
    {
        switch (state_)
        {
            case staj_reader_state::typed_array:
                advance_typed_array(ec);
                break;
            case staj_reader_state::multi_dim:
            case staj_reader_state::shape:
                advance_multi_dim(ec);
                break;
            default:
                break;
        }
    }

    bool is_typed_array() const
    {
        return data_.type() != typed_array_type();
    }

    staj_reader_state state() const
    {
        return state_;
    }

    void advance_typed_array(std::error_code& ec)
    {
        if (data_.type() != typed_array_type())
        {
            if (index_ < data_.size())
            {
                switch (data_.type())
                {
                    case typed_array_type::uint8_value:
                    {
                        this->uint64_value(data_.data(uint8_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::uint16_value:
                    {
                        this->uint64_value(data_.data(uint16_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::uint32_value:
                    {
                        this->uint64_value(data_.data(uint32_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::uint64_value:
                    {
                        this->uint64_value(data_.data(uint64_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::int8_value:
                    {
                        this->int64_value(data_.data(int8_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::int16_value:
                    {
                        this->int64_value(data_.data(int16_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::int32_value:
                    {
                        this->int64_value(data_.data(int32_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::int64_value:
                    {
                        this->int64_value(data_.data(int64_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::half_value:
                    {
                        this->half_value(data_.data(half_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::float_value:
                    {
                        this->double_value(data_.data(float_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    case typed_array_type::double_value:
                    {
                        this->double_value(data_.data(double_array_arg)[index_], semantic_tag::none, null_ser_context(), ec);
                        break;
                    }
                    default:
                        break;
                }
                ++index_;
            }
            else
            {
                this->end_array();
                state_ = staj_reader_state();
                data_ = typed_array_view();
                index_ = 0;
            }
        }
    }

    void advance_multi_dim(std::error_code& ec)
    {
        if (shape_.size() != 0)
        {
            if (state_ == staj_reader_state::multi_dim)
            {
                this->begin_array(shape_.size(), semantic_tag::none, null_ser_context(), ec);
                state_ = staj_reader_state::shape;
            }
            else if (index_ < shape_.size())
            {
                this->uint64_value(shape_[index_], semantic_tag::none, null_ser_context(), ec);
                ++index_;
            }
            else
            {
                state_ = staj_reader_state();
                this->end_array(null_ser_context(), ec);
                shape_ = span<const size_t>();
                index_ = 0;
            }
        }
    }

    bool dump(basic_json_content_handler<CharT>& handler, const ser_context& context, std::error_code& ec)
    {
        bool more = true;
        if (data_.type() != typed_array_type())
        {
            if (index_ != 0)
            {
                more = staj_to_saj_event(event(), handler, context, ec);
                while (more && data_.type() != typed_array_type())
                {
                    if (index_ < data_.size())
                    {
                        switch (data_.type())
                        {
                            case typed_array_type::uint8_value:
                            {
                                more = handler.uint64_value(data_.data(uint8_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::uint16_value:
                            {
                                more = handler.uint64_value(data_.data(uint16_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::uint32_value:
                            {
                                more = handler.uint64_value(data_.data(uint32_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::uint64_value:
                            {
                                more = handler.uint64_value(data_.data(uint64_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::int8_value:
                            {
                                more = handler.int64_value(data_.data(int8_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::int16_value:
                            {
                                more = handler.int64_value(data_.data(int16_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::int32_value:
                            {
                                more = handler.int64_value(data_.data(int32_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::int64_value:
                            {
                                more = handler.int64_value(data_.data(int64_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::float_value:
                            {
                                more = handler.double_value(data_.data(float_array_arg)[index_]);
                                break;
                            }
                            case typed_array_type::double_value:
                            {
                                more = handler.double_value(data_.data(double_array_arg)[index_]);
                                break;
                            }
                            default:
                                break;
                        }
                        ++index_;
                    }
                    else
                    {
                        more = handler.end_array();
                        state_ = staj_reader_state();
                        data_ = typed_array_view();
                        index_ = 0;
                    }
                }
            }
            else
            {
                switch (data_.type())
                {
                    case typed_array_type::uint8_value:
                    {
                        more = handler.typed_array(data_.data(uint8_array_arg));
                        break;
                    }
                    case typed_array_type::uint16_value:
                    {
                        more = handler.typed_array(data_.data(uint16_array_arg));
                        break;
                    }
                    case typed_array_type::uint32_value:
                    {
                        more = handler.typed_array(data_.data(uint32_array_arg));
                        break;
                    }
                    case typed_array_type::uint64_value:
                    {
                        more = handler.typed_array(data_.data(uint64_array_arg));
                        break;
                    }
                    case typed_array_type::int8_value:
                    {
                        more = handler.typed_array(data_.data(int8_array_arg));
                        break;
                    }
                    case typed_array_type::int16_value:
                    {
                        more = handler.typed_array(data_.data(int16_array_arg));
                        break;
                    }
                    case typed_array_type::int32_value:
                    {
                        more = handler.typed_array(data_.data(int32_array_arg));
                        break;
                    }
                    case typed_array_type::int64_value:
                    {
                        more = handler.typed_array(data_.data(int64_array_arg));
                        break;
                    }
                    case typed_array_type::float_value:
                    {
                        more = handler.typed_array(data_.data(float_array_arg));
                        break;
                    }
                    case typed_array_type::double_value:
                    {
                        more = handler.typed_array(data_.data(double_array_arg));
                        break;
                    }
                    default:
                        break;
                }

                state_ = staj_reader_state();
                data_ = typed_array_view();
            }
        }
        else
        {
            more = staj_to_saj_event(event(), handler, context, ec);
        }
        return more;
    }

private:
    static constexpr bool accept(const basic_staj_event<CharT>&, const ser_context&) 
    {
        return true;
    }

    bool do_begin_object(semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::begin_object, tag);
        return !filter_(event_, context);
    }

    bool do_end_object(const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::end_object);
        return !filter_(event_, context);
    }

    bool do_begin_array(semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::begin_array, tag);
        return !filter_(event_, context);
    }

    bool do_end_array(const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::end_array);
        return !filter_(event_, context);
    }

    bool do_name(const string_view_type& name, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(name, staj_event_type::name);
        return !filter_(event_, context);
    }

    bool do_null_value(semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(staj_event_type::null_value, tag);
        return !filter_(event_, context);
    }

    bool do_bool_value(bool value, semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_string_value(const string_view_type& s, semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(s, staj_event_type::string_value, tag);
        return !filter_(event_, context);
    }

    bool do_byte_string_value(const byte_string_view& s, 
                              semantic_tag tag,
                              const ser_context& context,
                              std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(s, staj_event_type::byte_string_value, tag);
        return !filter_(event_, context);
    }

    bool do_int64_value(int64_t value, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_uint64_value(uint64_t value, 
                         semantic_tag tag, 
                         const ser_context& context,
                         std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_double_value(double value, 
                         semantic_tag tag, 
                         const ser_context& context,
                         std::error_code&) override
    {
        event_ = basic_staj_event<CharT>(value, tag);
        return !filter_(event_, context);
    }

    bool do_typed_array(const span<const uint8_t>& v, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(v.data(), v.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const uint16_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const uint32_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const uint64_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const int8_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const int16_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const int32_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const int64_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(half_arg_t, const span<const uint16_t>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const float>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }

    bool do_typed_array(const span<const double>& data, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) override
    {
        state_ = staj_reader_state::typed_array;
        data_ = typed_array_view(data.data(), data.size());
        index_ = 0;
        return this->begin_array(tag, context, ec);
    }
/*
    bool do_typed_array(const span<const float128_type>&, 
                        semantic_tag,
                        const ser_context&,
                        std::error_code&) override
    {
        return true;
    }
*/
    bool do_begin_multi_dim(const span<const size_t>& shape,
                            semantic_tag tag,
                            const ser_context& context, 
                            std::error_code& ec) override
    {
        state_ = staj_reader_state::multi_dim;
        shape_ = shape;
        return this->begin_array(2, tag, context, ec);
    }

    bool do_end_multi_dim(const ser_context& context,
                          std::error_code& ec) override
    {
        return this->end_array(context, ec);
    }

    void do_flush() override
    {
    }
};

template<class CharT>
bool staj_to_saj_event(const basic_staj_event<CharT>& ev,
                       basic_json_content_handler<CharT>& handler,
                       const ser_context& context,
                       std::error_code& ec)
{
    switch (ev.event_type())
    {
        case staj_event_type::begin_array:
            return handler.begin_array(ev.tag(), context);
        case staj_event_type::end_array:
            return handler.end_array(context);
        case staj_event_type::begin_object:
            return handler.begin_object(ev.tag(), context, ec);
        case staj_event_type::end_object:
            return handler.end_object(context, ec);
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

    virtual void read(basic_json_content_handler<CharT>& handler) = 0;

    virtual void read(basic_json_content_handler<CharT>& handler,
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

