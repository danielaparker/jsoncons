/// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DESERIALIZE_RESULT_HPP
#define JSONCONS_DESERIALIZE_RESULT_HPP

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {
    template <typename T>
    class deserialize_result;

    template <typename T1, typename T2>
    struct is_constructible_or_convertible_from_optional
        : std::integral_constant<
              bool, std::is_constructible<T1, deserialize_result<T2>&>::value ||
                    std::is_constructible<T1, deserialize_result<T2>&&>::value ||
                    std::is_constructible<T1, const deserialize_result<T2>&>::value ||
                    std::is_constructible<T1, const deserialize_result<T2>&&>::value ||
                    std::is_convertible<deserialize_result<T2>&, T1>::value ||
                    std::is_convertible<deserialize_result<T2>&&, T1>::value ||
                    std::is_convertible<const deserialize_result<T2>&, T1>::value ||
                    std::is_convertible<const deserialize_result<T2>&&, T1>::value> {};

    template <typename T1, typename T2>
    struct is_constructible_convertible_or_assignable_from_optional
        : std::integral_constant<
              bool, is_constructible_or_convertible_from_optional<T1, T2>::value ||
                    std::is_assignable<T1&, deserialize_result<T2>&>::value ||
                    std::is_assignable<T1&, deserialize_result<T2>&&>::value ||
                    std::is_assignable<T1&, const deserialize_result<T2>&>::value ||
                    std::is_assignable<T1&, const deserialize_result<T2>&&>::value> {};

    template <typename T>
    class deserialize_result
    {
    public:
        using value_type = T;
    private:
        bool has_value_;
        union {
            std::error_code error_;
            T value_;
        };
    public:
        deserialize_result(std::error_code ec) noexcept
            : has_value_(false), error_{ec}
        {
        }
        
        // copy constructors
        deserialize_result(const deserialize_result<T>& other)
            : has_value_(false), error_{other.error_}
        {
            if (other)
            {
                construct(*other);
            }
        }

        // converting
        template <class U,
                  typename std::enable_if<!std::is_same<T,U>::value &&
                                          std::is_constructible<T, const U&>::value &&
                                          std::is_convertible<const U&,T>::value &&
                                          !is_constructible_or_convertible_from_optional<T,U>::value &&
                                          std::is_copy_constructible<typename std::decay<U>::type>::value,int>::type = 0>
        deserialize_result(const deserialize_result<U>& other)
            : has_value_(false), error_{other.error_}
        {
            if (other)
            {
                construct(*other);
            }
        }

        template <class U,
                  typename std::enable_if<!std::is_same<T,U>::value &&
                                          std::is_constructible<T, const U&>::value &&
                                          !std::is_convertible<const U&,T>::value &&
                                          !is_constructible_or_convertible_from_optional<T,U>::value &&
                                          std::is_copy_constructible<typename std::decay<U>::type>::value,int>::type = 0>
        explicit deserialize_result(const deserialize_result<U>& other)
            : has_value_(false), error_{other.error_}
        {
            if (other)
            {
                construct(*other);
            }
        }

        // move constructors
        template <class T2 = T>
        deserialize_result(deserialize_result<T>&& other,
                 typename std::enable_if<std::is_move_constructible<typename std::decay<T2>::type>::value>::type* = 0)
            : has_value_(false), error_{other.error_}
       {
            if (other)
            {
                construct(std::move(other.value_));
            }
       }

        // converting 
        template <class U>
        deserialize_result(deserialize_result<U>&& value,
             typename std::enable_if<!std::is_same<T,U>::value &&
                                     std::is_constructible<T, U&&>::value &&
                                     !is_constructible_or_convertible_from_optional<T,U>::value &&
                                     std::is_convertible<U&&,T>::value,int>::type = 0) // (8)
            : has_value_(true), value_(std::forward<U>(value))
        {
        }

        template <class U>
        explicit deserialize_result(deserialize_result<U>&& value,
                         typename std::enable_if<!std::is_same<T,U>::value &&
                                                 std::is_constructible<T, U&&>::value &&
                                                 !is_constructible_or_convertible_from_optional<T,U>::value &&
                                                 !std::is_convertible<U&&,T>::value,int>::type = 0) // (8)
            : has_value_(true), value_(std::forward<U>(value))
        {
        }


        // value constructors
        template <class T2>
        deserialize_result(T2&& value,
             typename std::enable_if<!std::is_same<deserialize_result<T>, typename std::decay<T2>::type>::value &&
                                     std::is_constructible<T, T2>::value &&
                                     std::is_convertible<T2,T>::value,int>::type = 0) // (8)
            : has_value_(true), value_(std::forward<T2>(value))
        {
        }

        template <class T2>
        explicit deserialize_result(T2&& value,
                         typename std::enable_if<!std::is_same<deserialize_result<T>, typename std::decay<T2>::type>::value &&
                                                 std::is_constructible<T, T2>::value &&
                                                 !std::is_convertible<T2,T>::value,int>::type = 0) // (8)
            : has_value_(true), value_(std::forward<T2>(value))
        {
        }

        ~deserialize_result() noexcept
        {
            destroy();
        }

        deserialize_result& operator=(const deserialize_result& other)
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

        deserialize_result& operator=(deserialize_result&& other )
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

        template <typename U>
        typename std::enable_if<!std::is_same<deserialize_result<T>, U>::value &&
                                std::is_constructible<T, const U&>::value &&
                               !is_constructible_convertible_or_assignable_from_optional<T,U>::value &&
                                std::is_assignable<T&, const U&>::value,
            deserialize_result&>::type
        operator=(const deserialize_result<U>& other)
        {
            if (other) 
            {
                assign(*other);
            } 
            else 
            {
                destroy();
            }
            return *this;
        }

        template <typename U>
        typename std::enable_if<!std::is_same<deserialize_result<T>, U>::value &&
                                std::is_constructible<T, U>::value &&
                                !is_constructible_convertible_or_assignable_from_optional<T,U>::value &&
                                std::is_assignable<T&, U>::value,
            deserialize_result&>::type
        operator=(deserialize_result<U>&& other)
        {
            if (other) 
            {
                assign(std::move(*other));
            } 
            else 
            {
                destroy();
            }
            return *this;
        }

        // value assignment
        template <typename T2>
        typename std::enable_if<!std::is_same<deserialize_result<T>, typename std::decay<T2>::type>::value &&
                                std::is_constructible<T, T2>::value &&
                                std::is_assignable<T&, T2>::value &&
                                !(std::is_scalar<T>::value && std::is_same<T, typename std::decay<T2>::type>::value),
            deserialize_result&>::type
        operator=(T2&& v)
        {
            assign(std::forward<T2>(v));
            return *this;
        }

        constexpr explicit operator bool() const noexcept
        {
            return has_value_;
        }
        
        constexpr bool has_value() const noexcept
        {
            return has_value_;
        }

        constexpr T& value() &
        {
            if (has_value_)
            {
                return get();
            }
            JSONCONS_THROW(std::runtime_error("Bad deserialize_result access"));
        }

        std::error_code error() &
        {
            if (!has_value_)
            {
                return this->error_;
            }
            JSONCONS_THROW(std::runtime_error("Bad deserialize_result access"));
        }

        constexpr const T& value() const &
        {
            if (has_value_)
            {
                return get();
            }
            JSONCONS_THROW(std::runtime_error("Bad deserialize_result access"));
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

        void swap(deserialize_result& other) noexcept(std::is_nothrow_move_constructible<T>::value /*&&
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
                deserialize_result& source = contains_a_value ? *this : other;
                deserialize_result& target = contains_a_value ? other : *this;
                target = deserialize_result<T>(*source);
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
    swap(deserialize_result<T>& lhs, deserialize_result<T>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator==(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator!=(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator<(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator>(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator<=(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator>=(const deserialize_result<T1>& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
    }

    template <class T1, typename T2>
    constexpr bool operator==(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs == rhs : false;
    }
    template <class T1, typename T2>
    constexpr bool operator==(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs == *rhs : false;
    }

    template <class T1, typename T2>
    constexpr bool operator!=(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs != rhs : true;
    }
    template <class T1, typename T2>
    constexpr bool operator!=(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs != *rhs : true;
    }

    template <class T1, typename T2>
    constexpr bool operator<(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs < rhs : true;
    }
    template <class T1, typename T2>
    constexpr bool operator<(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs < *rhs : false;
    }

    template <class T1, typename T2>
    constexpr bool operator<=(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs <= rhs : true;
    }
    template <class T1, typename T2>
    constexpr bool operator<=(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs <= *rhs : false;
    }

    template <class T1, typename T2>
    constexpr bool operator>(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs > rhs : false;
    }

    template <class T1, typename T2>
    constexpr bool operator>(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs > *rhs : true;
    }

    template <class T1, typename T2>
    constexpr bool operator>=(const deserialize_result<T1>& lhs, const T2& rhs) noexcept 
    {
        return lhs ? *lhs >= rhs : false;
    }
    template <class T1, typename T2>
    constexpr bool operator>=(const T1& lhs, const deserialize_result<T2>& rhs) noexcept 
    {
        return rhs ? lhs >= *rhs : true;
    }

}

#endif
