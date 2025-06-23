/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_RESULT_HPP    
#define JSONCONS_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/conv_error.hpp>
#include <cassert>

namespace jsoncons {
    
struct in_place_t
{
    explicit in_place_t() = default; 
};

JSONCONS_INLINE_CONSTEXPR in_place_t in_place{};

struct unexpect_t
{
    explicit unexpect_t() = default; 
};

JSONCONS_INLINE_CONSTEXPR unexpect_t unexpect{};

template <typename T, typename E>
class result
{
public:
    using value_type = T;
    using error_type = E;
private:
    bool has_value_;
    union {
        E error_;
        T value_;
    };
public:

    result(const T& value) 
        : has_value_(true)
    {
        construct(value);
    }

     result(T&& value) noexcept
         : has_value_(true)
     {
         construct(std::move(value));
     }

     template <typename... Args>    
     result(in_place_t, Args&& ... args) noexcept
         : has_value_(true)
     {
         ::new (&value_) T(std::forward<Args>(args)...);
     }

     result(const E& err)
        : has_value_(false)
    {
        ::new (&error_) E(err);
    }

    result(E&& err) noexcept
        : has_value_(false)
    {
        ::new (&error_) E(std::move(err));
    }

    template <typename... Args>    
    result(unexpect_t, Args&& ... args) noexcept
        : has_value_(false)
    {
        ::new (&error_) E(std::forward<Args>(args)...);
    }
    
    // copy constructors
    result(const result<T,E>& other) 
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(other.value_);
        }
        else
        {
            ::new (&error_) E(other.error_);
        }
    }

    // move constructors
    result(result<T,E>&& other) noexcept
        : has_value_(other.has_value())
    {
        if (other)
        {
            construct(std::move(other.value_));
        }
        else
        {
            ::new (&error_) E(other.error_);
        }
    }

    ~result() noexcept
    {
        destroy();
    }

    result& operator=(const result& other)
    {
        if (other)
        {
            assign(*other);
        }
        else
        {
            destroy();
            ::new (&error_) E(other.error_);
        }
        return *this;
    }

    result& operator=(result&& other)
    {
        if (other)
        {
            assign(std::move(*other));
        }
        else
        {
            destroy();
            ::new (&error_) E(other.error_);
        }
        return *this;
    }

    // value assignment
    result& operator=(const T& v)
    {
        assign(v);
        return *this;
    }

    result& operator=(T&& v)
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
        JSONCONS_THROW(std::runtime_error("Bad result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T& value() const &
    {
        if (has_value_)
        {
            return this->value_;
        }
        JSONCONS_THROW(std::runtime_error("Bad result access"));
    }

    JSONCONS_CPP14_CONSTEXPR T&& value() &&
    {
        if (has_value_)
        {
            return std::move(this->value_);
        }
        JSONCONS_THROW(std::runtime_error("Bad result access"));
    }

    JSONCONS_CPP14_CONSTEXPR const T&& value() const &&
    {
        if (has_value_)
        {
            return std::move(this->value_);
        }
        JSONCONS_THROW(std::runtime_error("Bad result access"));
    }

    JSONCONS_CPP14_CONSTEXPR E& error() & noexcept
    {
        assert(!has_value_);
        return this->error_;
    }

    JSONCONS_CPP14_CONSTEXPR const E& error() const& noexcept
    {
        assert(!has_value_);
        return this->error_;
    }

    JSONCONS_CPP14_CONSTEXPR E&& error() && noexcept
    {
        assert(!has_value_);
        return std::move(this->error_);
    }

    JSONCONS_CPP14_CONSTEXPR const E&& error() const && noexcept
    {
        assert(!has_value_);
        return std::move(this->error_);
    }

    JSONCONS_CPP14_CONSTEXPR const T* operator->() const noexcept
    {
        return std::addressof(this->value_);
    }

    JSONCONS_CPP14_CONSTEXPR T* operator->() noexcept
    {
        return std::addressof(this->value_);
    }

    JSONCONS_CPP14_CONSTEXPR const T& operator*() const & noexcept
    {
        return this->value_;
    }

    JSONCONS_CPP14_CONSTEXPR T& operator*() & noexcept
    {
        return this->value_;
    }

    JSONCONS_CPP14_CONSTEXPR const T&& operator*() const && noexcept
    {
        return this->value_;
    }

    JSONCONS_CPP14_CONSTEXPR T&& operator*() && noexcept
    {
        return this->value_;
    }

    void swap(result& other) noexcept(std::is_nothrow_move_constructible<T>::value /*&&
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
            result& source = contains_a_value ? *this : other;
            result& target = contains_a_value ? other : *this;
            target = result<T,E>(*source);
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
            error_.~E();
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

template <typename T,typename E>
typename std::enable_if<std::is_nothrow_move_constructible<T>::value,void>::type
swap(result<T,E>& lhs, result<T,E>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // jsoncons

#endif // JSONCONS_RESULT_HPP
