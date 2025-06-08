/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_CONVERSION_RESULT_HPP    
#define JSONCONS_CONVERSION_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/conv_error.hpp>

namespace jsoncons {

class conversion_error
{
    std::error_code ec_;
    std::string message_;
public:
    conversion_error(std::error_code ec)
        : ec_(ec)
    {
    }
    conversion_error(std::error_code ec, const jsoncons::string_view& message)
        : ec_(ec), message_(message)
    {
    }

    conversion_error(const conversion_error& other) = default;

    conversion_error(conversion_error&& other) = default;

    conversion_error& operator=(const conversion_error& other) = default;

    conversion_error& operator=(conversion_error&& other) = default;
    
    std::error_code code() const
    {
        return ec_;
    }
    
    const std::string& message() const 
    {
        return message_;
    }
};

inline
std::string to_string(const conversion_error& err)
{
    std::string str{err.message()};
    if (!str.empty())
    {
        str.append(": ");
    }
    str.append(err.code().message());
    return str;
}

inline
std::ostream& operator<<(std::ostream& os, const conversion_error& err)
{
    os << to_string(err);
    return os;
}


template <typename T>
class conversion_result
{
public:
    using value_type = T;
private:
    bool has_value_;
    union {
        conversion_error error_;
        T value_;
    };
public:

    conversion_result(const T& value) 
        : has_value_(true)
    {
        construct(value);
    }

     conversion_result(T&& value) noexcept
         : has_value_(true)
     {
         construct(std::move(value));
     }

     template <typename... Args>    
     conversion_result(in_place_t, Args&& ... args) noexcept
         : has_value_(true)
     {
         ::new (&value_) T(std::forward<Args>(args)...);
     }

     conversion_result(const conversion_error& err)
        : has_value_(false)
    {
        ::new (&error_) conversion_error(err);
    }

    conversion_result(conversion_error&& err) noexcept
        : has_value_(false)
    {
        ::new (&error_) conversion_error(std::move(err));
    }

    template <typename... Args>    
    conversion_result(unexpect_t, Args&& ... args) noexcept
        : has_value_(false)
    {
        ::new (&error_) conversion_error(std::forward<Args>(args)...);
    }
    
    // copy constructors
    conversion_result(const conversion_result<T>& other) 
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(other.value_);
        }
        else
        {
            ::new (&error_) conversion_error(other.error_);
        }
    }

    // move constructors
    conversion_result(conversion_result<T>&& other) noexcept
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(std::move(other.value_));
        }
        else
        {
            ::new (&error_) conversion_error(other.error_);
        }
    }

    ~conversion_result() noexcept
    {
        destroy();
    }

    conversion_result& operator=(const conversion_result& other)
    {
        if (other)
        {
            assign(*other);
        }
        else
        {
            destroy();
            ::new (&error_) conversion_error(other.error_);
        }
        return *this;
    }

    conversion_result& operator=(conversion_result&& other)
    {
        if (other)
        {
            assign(std::move(*other));
        }
        else
        {
            destroy();
            ::new (&error_) conversion_error(other.error_);
        }
        return *this;
    }

    // value assignment
    conversion_result& operator=(const T& v)
    {
        assign(v);
        return *this;
    }

    conversion_result& operator=(T&& v)
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
            return this->value_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conversion_result access"));
    }

    conversion_error error() &
    {
        if (!has_value_)
        {
            return this->error_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conversion_result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T& value() const &
    {
        if (has_value_)
        {
            return this->value_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conversion_result access"));
    }

    const T* operator->() const noexcept
    {
        return std::addressof(this->value_);
    }

    T* operator->() noexcept
    {
        return std::addressof(this->value_);
    }

    const T& operator*() const & noexcept
    {
        return this->value_;
    }

    T& operator*() & noexcept
    {
        return this->value_;
    }

    const T&& operator*() const && noexcept
    {
        return this->value_;
    }

    T&& operator*() && noexcept
    {
        return this->value_;
    }

    void swap(conversion_result& other) noexcept(std::is_nothrow_move_constructible<T>::value /*&&
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
            conversion_result& source = contains_a_value ? *this : other;
            conversion_result& target = contains_a_value ? other : *this;
            target = conversion_result<T>(*source);
            source.destroy();
            source.error_ = target.error_;
        }
    }
private:
    void construct(const T& value) 
    {
        ::new (&value_) T(value);
        has_value_ = true;
    }

    void construct(T&& value) noexcept
    {
        ::new (&value_) T(std::move(value));
        has_value_ = true;
    }

    void destroy() noexcept 
    {
        if (has_value_) 
        {
            value_.~T();
            has_value_ = false;
        }
        else
        {
            error_.~conversion_error();
        }
    }

    void assign(const T& u) 
    {
        if (has_value_) 
        {
            value_ = u;
        } 
        else 
        {
            construct(u);
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
swap(conversion_result<T>& lhs, conversion_result<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // jsoncons

#endif // JSONCONS_CONVERSION_RESULT_HPP
