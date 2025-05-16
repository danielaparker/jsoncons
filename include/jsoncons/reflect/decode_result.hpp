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

    decode_result(const T& value) 
        : has_value_(true)
    {
        construct(value);
    }

     decode_result(T&& value) noexcept
         : has_value_(true)
     {
         construct(std::move(value));
     }

     decode_result(const decode_error& err)
        : has_value_(false), error_{err}
    {
    }

    decode_result(decode_error&& err) noexcept
        : has_value_(false), error_{std::move(err)}
    {
    }
    
    // copy constructors
    decode_result(const decode_result<T>& other) 
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(other.value_);
        }
        else
        {
            error_ = other.error_;
        }
    }

    // move constructors
    decode_result(decode_result<T>&& other) noexcept
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(std::move(other.value_));
        }
        else
        {
            error_ = other.error_;
        }
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
            destroy();
            error_ = other.error_;
        }
        return *this;
    }

    decode_result& operator=(decode_result&& other)
    {
        if (other)
        {
            assign(std::move(*other));
        }
        else
        {
            destroy();
            error_ = other.error_;
        }
        return *this;
    }

    // value assignment
    decode_result& operator=(const T& v)
    {
        assign(v);
        return *this;
    }

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
            source.destroy();
            source.error_ = target.error_;
        }
    }
private:
    constexpr const T& get() const { return this->value_; }
    T& get() { return this->value_; }


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
swap(decode_result<T>& lhs, decode_result<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // reflect
} // jsoncons

#endif // JSONCONS_REFLECT_DECODE_RESULT_HPP
