/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_REFLECT_DECODE_RESULT_HPP    
#define JSONCONS_REFLECT_DECODE_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {
namespace reflect {

class decode_error
{
    std::error_code ec_{};
    std::size_t line_{};
    std::size_t column_{};
    
public:
    decode_error(std::error_code ec, std::size_t line, std::size_t column)
        : ec_{ec}, line_{line}, column_{column}
    {
    }
    
    decode_error(const decode_error& other) = default;

    decode_error(decode_error&& other) = default;

    decode_error& operator=(const decode_error& other) = default;

    decode_error& operator=(decode_error&& other) = default;
    
    const std::error_code& ec() const
    {
        return ec_;
    }
    std::size_t line() const
    {
        return line_;
    }
    std::size_t column() const
    {
        return column_;
    }
};

template <typename T>
class decode_result
{
public:
    using value_type = T;
private:
    bool has_value_;
    union {
        decode_error error_;
        T value_;
    };
public:
    decode_result(const decode_error& err) noexcept
        : has_value_(false), error_{err}
    {
    }

    decode_result(decode_error&& err) noexcept
        : has_value_(false), error_{err}
    {
    }
    
    // copy constructors
    decode_result(const decode_result<T>& other)
        : has_value_(false), error_{other.error_}
    {
        if (other)
        {
            construct(*other);
        }
    }

    // move constructors
    decode_result(decode_result<T>&& other)
        : has_value_(false), error_{other.error_}
   {
        if (other)
        {
            construct(std::move(other.value_));
        }
   }

   decode_result(const T& value) // (8)
       : has_value_(true), value_(value)
   {
   }

    decode_result(T&& value) // (8)
        : has_value_(true), value_(std::move(value))
    {
    }

    ~decode_result() noexcept
    {
        destroy();
    }

    decode_result& operator=(const decode_result& other)
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

    decode_result& operator=(decode_result&& other )
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
    decode_result& operator=(T&& v)
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
        JSONCONS_THROW(std::runtime_error("Bad decode_result access"));
    }

    decode_error error() &
    {
        if (!has_value_)
        {
            return this->error_;
        }
        JSONCONS_THROW(std::runtime_error("Bad decode_result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T& value() const &
    {
        if (has_value_)
        {
            return get();
        }
        JSONCONS_THROW(std::runtime_error("Bad decode_result access"));
    }

    template <typename U>
    constexpr T value_or(U&& default_value) const & 
    {
        static_assert(std::is_copy_constructible<T>::value,
                      "get_value_or: T must be copy constructible");
        static_assert(std::is_convertible<U&&, T>::value,
                      "get_value_or: U must be convertible to T");
        return static_cast<bool>(*this)
                   ? **this
                   : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    T value_or(U&& default_value) && 
    {
        static_assert(std::is_move_constructible<T>::value,
                      "get_value_or: T must be move constructible");
        static_assert(std::is_convertible<U&&, T>::value,
                      "get_value_or: U must be convertible to T");
        return static_cast<bool>(*this) ? std::move(**this)
                                        : static_cast<T>(std::forward<U>(default_value));
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

    void swap(decode_result& other) noexcept(std::is_nothrow_move_constructible<T>::value /*&&
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
            decode_result& source = contains_a_value ? *this : other;
            decode_result& target = contains_a_value ? other : *this;
            target = decode_result<T>(*source);
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

    template <typename U>
    void assign(U&& u) 
    {
        if (has_value_) 
        {
            value_ = std::forward<U>(u);
        } 
        else 
        {
            construct(std::forward<U>(u));
        }
    }
};

template <typename T>
typename std::enable_if<std::is_nothrow_move_constructible<T>::value,void>::type
swap(decode_result<T>& lhs, decode_result<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <class T1, typename T2>
constexpr bool operator==(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
}

template <class T1, typename T2>
constexpr bool operator!=(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);
}

template <class T1, typename T2>
constexpr bool operator<(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

template <class T1, typename T2>
constexpr bool operator>(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

template <class T1, typename T2>
constexpr bool operator<=(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

template <class T1, typename T2>
constexpr bool operator>=(const decode_result<T1>& lhs, const decode_result<T2>& rhs) noexcept 
{
    return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

template <class T1, typename T2>
constexpr bool operator==(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs == rhs : false;
}
template <class T1, typename T2>
constexpr bool operator==(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs == *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator!=(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs != rhs : true;
}
template <class T1, typename T2>
constexpr bool operator!=(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs != *rhs : true;
}

template <class T1, typename T2>
constexpr bool operator<(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs < rhs : true;
}
template <class T1, typename T2>
constexpr bool operator<(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs < *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator<=(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs <= rhs : true;
}
template <class T1, typename T2>
constexpr bool operator<=(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs <= *rhs : false;
}

template <class T1, typename T2>
constexpr bool operator>(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs > rhs : false;
}

template <class T1, typename T2>
constexpr bool operator>(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs > *rhs : true;
}

template <class T1, typename T2>
constexpr bool operator>=(const decode_result<T1>& lhs, const T2& rhs) noexcept 
{
    return lhs ? *lhs >= rhs : false;
}
template <class T1, typename T2>
constexpr bool operator>=(const T1& lhs, const decode_result<T2>& rhs) noexcept 
{
    return rhs ? lhs >= *rhs : true;
}

} // reflect
} // jsoncons

#endif // JSONCONS_REFLECT_DECODE_RESULT_HPP
