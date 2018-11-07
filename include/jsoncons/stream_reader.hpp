// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STREAM_READER_HPP
#define JSONCONS_STREAM_READER_HPP

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
#include <jsoncons/bignum.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/serializing_context.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/print_number.hpp>
#include <jsoncons/key_value.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_type_traits.hpp>

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
    uint64_value,
    double_value
};

template<class CharT>
class basic_stream_event
{
    stream_event_type event_type_;
    semantic_tag_type semantic_tag_;
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
    floating_point_options fmt_;
public:
    basic_stream_event(stream_event_type event_type, semantic_tag_type semantic_tag = semantic_tag_type::none)
        : event_type_(event_type), semantic_tag_(semantic_tag), length_(0)
    {
    }

    basic_stream_event(null_type)
        : event_type_(stream_event_type::null_value), semantic_tag_(semantic_tag_type::none), length_(0)
    {
    }

    basic_stream_event(bool value)
        : event_type_(stream_event_type::bool_value), semantic_tag_(semantic_tag_type::none), length_(0)
    {
        value_.bool_value_ = value;
    }

    basic_stream_event(int64_t value, semantic_tag_type semantic_tag)
        : event_type_(stream_event_type::int64_value), semantic_tag_(semantic_tag), length_(0)
    {
        value_.int64_value_ = value;
    }

    basic_stream_event(uint64_t value, semantic_tag_type semantic_tag)
        : event_type_(stream_event_type::uint64_value), semantic_tag_(semantic_tag), length_(0)
    {
        value_.uint64_value_ = value;
    }

    basic_stream_event(double value, const floating_point_options& fmt, semantic_tag_type semantic_tag)
        : event_type_(stream_event_type::double_value), semantic_tag_(semantic_tag), length_(0), fmt_(fmt)
    {
        value_.double_value_ = value;
    }

    basic_stream_event(const CharT* data, size_t length,
        stream_event_type event_type,
        semantic_tag_type semantic_tag = semantic_tag_type::none)
        : event_type_(event_type), semantic_tag_(semantic_tag), length_(length)
    {
        value_.string_data_ = data;
    }

    template<class T, class CharT_ = CharT>
    typename std::enable_if<detail::is_string_like<T>::value && std::is_same<typename T::value_type, CharT_>::value, T>::type
        as() const
    {
        T s;
        switch (event_type_)
        {
        case stream_event_type::name:
        case stream_event_type::string_value:
            s = T(value_.string_data_, length_);
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
            detail::print_double f(fmt_);
            f(value_.double_value_, fmt_, writer);
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

    template<class T, class CharT_ = CharT>
    typename std::enable_if<detail::is_string_view_like<T>::value && std::is_same<typename T::value_type, CharT_>::value, T>::type
        as() const
    {
        T s;
        switch (event_type_)
        {
        case stream_event_type::name:
        case stream_event_type::string_value:
            s = T(value_.string_data_, length_);
            break;
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a string"));
        }
        return s;
    }

    template<class T>
    typename std::enable_if<detail::is_integer_like<T>::value, T>::type
        as() const
    {
        return static_cast<T>(as_int64());
    }

    template<class T>
    typename std::enable_if<detail::is_uinteger_like<T>::value, T>::type
        as() const
    {
        return static_cast<T>(as_uint64());
    }

    template<class T>
    typename std::enable_if<detail::is_floating_point_like<T>::value, T>::type
        as() const
    {
        return static_cast<T>(as_double());
    }

    template<class T, class UserAllocator = std::allocator<uint8_t>>
    typename std::enable_if<std::is_same<T, basic_bignum<UserAllocator>>::value, T>::type
        as() const
    {
        return as_bignum<UserAllocator>();
    }

    template<class T>
    typename std::enable_if<std::is_same<T, bool>::value, T>::type
        as() const
    {
        return as_bool();
    }

    stream_event_type event_type() const noexcept { return event_type_; }

    semantic_tag_type semantic_tag() const noexcept { return semantic_tag_; }
private:

    int64_t as_int64() const
    {
        int64_t value = 0;
        switch (event_type_)
        {
        case stream_event_type::name:
        case stream_event_type::string_value:
        {
            if (!detail::is_integer(value_.string_data_, length_))
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
            }
            auto result = detail::to_integer<int64_t>(value_.string_data_, length_);
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
            value = value_.int64_value_;
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

    uint64_t as_uint64() const
    {
        uint64_t value = 0;
        switch (event_type_)
        {
        case stream_event_type::name:
        case stream_event_type::string_value:
        {
            if (!detail::is_uinteger(value_.string_data_, length_))
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
            }
            auto result = detail::to_integer<uint64_t>(value_.string_data_, length_);
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
            value = value_.uint64_value_;
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
        switch (event_type_)
        {
        case stream_event_type::name:
        case stream_event_type::string_value:
        {
            std::string target;
            auto result = unicons::convert(
                value_.string_data_, value_.string_data_ + length_, std::back_inserter(target), unicons::conv_flags::strict);
            if (result.ec != unicons::conv_errc())
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
            }
            detail::string_to_double f;
            return f(target.data(), target.length());
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
        switch (event_type_)
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

    template <class UserAllocator = std::allocator<uint8_t>>
    basic_bignum<UserAllocator> as_bignum() const
    {
        switch (event_type_)
        {
        case stream_event_type::string_value:
            if (!detail::is_integer(value_.string_data_, length_))
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bignum"));
            }
            return basic_bignum<UserAllocator>(value_.string_data_, length_);
        case stream_event_type::double_value:
            return basic_bignum<UserAllocator>(value_.double_value_);
        case stream_event_type::int64_value:
            return basic_bignum<UserAllocator>(value_.int64_value_);
        case stream_event_type::uint64_value:
            return basic_bignum<UserAllocator>(value_.uint64_value_);
        case stream_event_type::bool_value:
            return basic_bignum<UserAllocator>(value_.bool_value_ ? 1 : 0);
        default:
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bignum"));
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

    virtual void accept(basic_json_content_handler<CharT>& handler) = 0;

    virtual void next() = 0;

    virtual const serializing_context& context() const = 0;
};

template<class CharT>
class basic_stream_filter
{
public:

    virtual ~basic_stream_filter() = default;

    virtual bool accept(const basic_stream_event<CharT>& event, const serializing_context& context) = 0;
};

template<class CharT>
class default_basic_stream_filter : public basic_stream_filter<CharT>
{
public:
    bool accept(const basic_stream_event<CharT>&, const serializing_context&) override
    {
        return true;
    }
};

template<class Json, class T = Json>
class array_iterator
{
    typedef Json json_type;
    typedef typename Json::char_type char_type;

    basic_stream_reader<char_type>* reader_;
    T value_;
public:
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    array_iterator()
        : reader_(nullptr)
    {
    }

    array_iterator(basic_stream_reader<char_type>& reader)
        : reader_(std::addressof(reader))
    {
        if (reader_->current().event_type() != stream_event_type::begin_array)
        {
            throw std::invalid_argument("Not an array");
        }
        next();
    }

    const T& operator*() const
    {
        return value_;
    }

    const T* operator->() const
    {
        return &value_;
    }

    array_iterator& operator++()
    {
        next();
        return *this;
    }

    array_iterator operator++(int) // postfix increment
    {
        array_iterator temp(*this);
        next();
        return temp;
    }

    friend bool operator==(const array_iterator<Json, T>& a, const array_iterator<Json, T>& b)
    {
        return !a.reader_ && !b.reader_
            || (!a.reader_ && b.done())
            || (!b.reader_ && a.done());
    }

    friend bool operator!=(const array_iterator<Json, T>& a, const array_iterator<Json, T>& b)
    {
        return !(a == b);
    }

private:

    bool done() const
    {
        return reader_->done() || reader_->current().event_type() == stream_event_type::end_array;
    }

    void next()
    {
        if (!done())
        {
            reader_->next();
            if (!done())
            {
                json_decoder<json_type> decoder;
                reader_->accept(decoder);
                value_ = decoder.get_result().template as<T>();
            }
        }
    }
};

template <class Json, class T>
array_iterator<Json, T> begin(array_iterator<Json, T> iter) noexcept
{
    return iter;
}

template <class Json, class T>
array_iterator<Json, T> end(const array_iterator<Json, T>&) noexcept
{
    return array_iterator<Json, T>();
}

template<class Json, class T = Json>
class object_iterator
{
    typedef Json json_type;
    typedef typename Json::char_type char_type;
    typedef std::basic_string<char_type> string_type;
    typedef std::pair<string_type,T> key_value_type;

    basic_stream_reader<char_type>* reader_;
    key_value_type kv_;
public:
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    object_iterator()
        : reader_(nullptr)
    {
    }

    object_iterator(basic_stream_reader<char_type>& reader)
        : reader_(std::addressof(reader))
    {
        std::cout << "event_type: " << (int)reader_->current().event_type() << "\n";
        if (reader_->current().event_type() != stream_event_type::begin_object)
        {
            throw std::invalid_argument("Not an object");
        }
        next();
    }

    const key_value_type& operator*() const
    {
        return kv_;
    }

    const key_value_type* operator->() const
    {
        return &kv_;
    }

    object_iterator& operator++()
    {
        next();
        return *this;
    }

    object_iterator operator++(int) // postfix increment
    {
        object_iterator temp(*this);
        next();
        return temp;
    }

    friend bool operator==(const object_iterator<Json,T>& a, const object_iterator<Json,T>& b)
    {
        return !a.reader_ && !b.reader_
               || (!a.reader_ && b.done())
               || (!b.reader_ && a.done());
    }

    friend bool operator!=(const object_iterator<Json,T>& a, const object_iterator<Json,T>& b)
    {
        return !(a == b);
    }

private:

    bool done() const
    {
        return reader_->done() || reader_->current().event_type() == stream_event_type::end_object;
    }

    void next()
    {
        reader_->next();
        if (!done())
        {
            JSONCONS_ASSERT(reader_->current().event_type() == stream_event_type::name);
            kv_.first =reader_->current(). template as<string_type>();
            reader_->next();
            if (!done())
            {
                json_decoder<json_type> decoder;
                reader_->accept(decoder);
                kv_.second = decoder.get_result().template as<T>();
            }
        }
    }
};

template <class Json,class T>
object_iterator<Json,T> begin(object_iterator<Json,T> iter) noexcept
{
    return iter;
}

template <class Json,class T>
object_iterator<Json,T> end(const object_iterator<Json,T>&) noexcept
{
    return object_iterator<Json,T>();
}

    typedef basic_stream_reader<char> stream_reader;
    typedef basic_stream_reader<wchar_t> wstream_reader;

    typedef basic_stream_filter<char> stream_filter;
    typedef basic_stream_filter<wchar_t> wstream_filter;

    typedef basic_stream_event<char> stream_event;
    typedef basic_stream_event<wchar_t> wstream_event;
}

#endif

