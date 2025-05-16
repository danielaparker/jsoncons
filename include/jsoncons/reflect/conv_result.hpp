/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_REFLECT_CONV_RESULT_HPP    
#define JSONCONS_REFLECT_CONV_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {
namespace reflect {

template <typename T>
class conv_result;

template <typename T>
class conv_result
{
public:
    using value_type = T;
private:
    bool has_value_;
    union {
        std::error_code ec_;
        T value_;
    };
public:
    conv_result(std::error_code ec) noexcept
        : has_value_(false), ec_{ec}
    {
    }

    // copy constructors
    conv_result(const conv_result<T>& other)
        : has_value_(false), ec_{other.ec_}
    {
        if (other)
        {
            construct(*other);
        }
    }

    // move constructors
    conv_result(conv_result<T>&& other)
        : has_value_(false), ec_{other.ec_}
   {
        if (other)
        {
            construct(std::move(other.value_));
        }
   }

    // value constructors
    conv_result(T&& value) // (8)
        : has_value_(true), value_(std::move(value))
    {
    }

    ~conv_result() noexcept
    {
        destroy();
    }

    conv_result& operator=(const conv_result& other)
    {
        if (other)
        {
            assign(*other);
        }
        else
        {
            reset();
        }
        return *this;
    }

    conv_result& operator=(conv_result&& other )
    {
        if (other)
        {
            assign(std::move(*other));
        }
        else
        {
            reset();
        }
        return *this;
    }

    // value assignment
    conv_result<T>& operator=(T&& v)
    {
        assign(std::move(v));
        return *this;
    }

    constexpr operator bool() const noexcept
    {
        return has_value_;
    }
    
    constexpr bool has_value() const noexcept
    {
        return has_value_;
    }

    JSONCONS_CPP14_CONSTEXPR T& value() &
    {
        if (has_value_)
        {
            return get();
        }
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
    }

    std::error_code error() const
    {
        if (!has_value_)
        {
            return this->ec_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T& value() const &
    {
        if (has_value_)
        {
            return get();
        }
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
    }

    constexpr const T* operator->() const
    {
        return std::addressof(this->value_);
    }

    constexpr T* operator->()
    {
        return std::addressof(this->value_);
    }

    constexpr const T& operator*() const&
    {
        return value();
    }

    constexpr T& operator*() &
    {
        return value();
    }

    void reset() noexcept
    {
        destroy();
    }

    void swap(conv_result& other) noexcept(std::is_nothrow_move_constructible<T>::value /*&&
                                        std::is_nothrow_swappable<T>::value*/)
    {
        const bool contains_a_value = has_value();
        if (contains_a_value == other.has_value())
        {
            if (contains_a_value)
            {
                using std::swap;
                swap(**this, *other);
            }
        }
        else
        {
            conv_result& source = contains_a_value ? *this : other;
            conv_result& target = contains_a_value ? other : *this;
            target = conv_result<T>(*source);
            source.reset();
        }
    }
private:
    constexpr const T& get() const { return this->value_; }
    T& get() { return this->value_; }

    template <typename... Args>
    void construct(Args&&... args) 
    {
        ::new (static_cast<void*>(&this->value_)) T(std::forward<Args>(args)...);
        has_value_ = true;
    }

    void destroy() noexcept 
    {
        if (has_value_) 
        {
            value_.~T();
            has_value_ = false;
        }
    }

    void assign(T&& u) 
    {
        if (has_value_) 
        {
            value_ = std::move(u);
        } 
        else 
        {
            construct(std::move(u));
        }
    }
};

template <typename T>
typename std::enable_if<std::is_nothrow_move_constructible<T>::value,void>::type
swap(conv_result<T>& lhs, conv_result<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <class T1, typename T2>
constexpr bool operator==(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
}

template <class T1, typename T2>
constexpr bool operator!=(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);
}

template <class T1, typename T2>
constexpr bool operator<(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

template <class T1, typename T2>
constexpr bool operator>(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

template <class T1, typename T2>
constexpr bool operator<=(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

template <class T1, typename T2>
constexpr bool operator>=(const conv_result<T1>& lhs, const conv_result<T2>& rhs) noexcept 
{
    return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

template <class T1, typename T2>
constexpr bool operator==(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs == rhs : false;
}
template <class T1, typename T2>
constexpr bool operator==(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs == *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator!=(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs != rhs : true;
}
template <class T1, typename T2>
constexpr bool operator!=(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs != *rhs : true;
}

template <class T1, typename T2>
constexpr bool operator<(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs < rhs : true;
}
template <class T1, typename T2>
constexpr bool operator<(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs < *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator<=(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs <= rhs : true;
}
template <class T1, typename T2>
constexpr bool operator<=(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs <= *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator>(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs > rhs : false;
}

template <class T1, typename T2>
constexpr bool operator>(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs > *rhs : true;
}

template <class T1, typename T2>
constexpr bool operator>=(const conv_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs >= rhs : false;
}
template <class T1, typename T2>
constexpr bool operator>=(const T1& lhs, const conv_result<T2>& rhs) noexcept 
{
    return rhs ? lhs >= *rhs : true;
}

} // reflect
} // jsoncons

#endif // JSONCONS_REFLECT_CONV_RESULT_HPP
