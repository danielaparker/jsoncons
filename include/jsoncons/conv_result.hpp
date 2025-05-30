/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_CONV_RESULT_HPP    
#define JSONCONS_CONV_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/conv_error.hpp>

namespace jsoncons {

// Ignore false positives 
#if defined(__GNUC__) 
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif

template <typename T>
class conv_result
{
public:
    using value_type = T;
private:
    bool has_value_;
    union {
        conv_error error_;
        T value_;
    };
public:

    conv_result(const T& value) 
        : has_value_(true)
    {
        construct(value);
    }

     conv_result(T&& value) noexcept
         : has_value_(true)
     {
         construct(std::move(value));
     }

     conv_result(const conv_error& err)
        : has_value_(false)
    {
        ::new (&error_) conv_error(err);
    }

    conv_result(conv_error&& err) noexcept
        : has_value_(false)
    {
        ::new (&error_) conv_error(std::move(err));
    }

    template <typename... Args>    
    conv_result(unexpect_t, Args&& ... args) noexcept
        : has_value_(false)
    {
        ::new (&error_) conv_error(std::forward<Args>(args)...);
    }
    
    // copy constructors
    conv_result(const conv_result<T>& other) 
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(other.value_);
        }
        else
        {
            ::new (&error_) conv_error(other.error_);
        }
    }

    // move constructors
    conv_result(conv_result<T>&& other) noexcept
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(std::move(other.value_));
        }
        else
        {
            ::new (&error_) conv_error(other.error_);
        }
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
            destroy();
            ::new (&error_) conv_error(other.error_);
        }
        return *this;
    }

    conv_result& operator=(conv_result&& other)
    {
        if (other)
        {
            assign(std::move(*other));
        }
        else
        {
            destroy();
            ::new (&error_) conv_error(other.error_);
        }
        return *this;
    }

    // value assignment
    conv_result& operator=(const T& v)
    {
        assign(v);
        return *this;
    }

    conv_result& operator=(T&& v)
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
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
    }

    conv_error error() &
    {
        if (!has_value_)
        {
            return this->error_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T& value() const &
    {
        if (has_value_)
        {
            return this->value_;
        }
        JSONCONS_THROW(std::runtime_error("Bad conv_result access"));
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
swap(conv_result<T>& lhs, conv_result<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif

} // jsoncons

#endif // JSONCONS_CONV_RESULT_HPP
