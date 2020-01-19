// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_OPTIONAL_HPP
#define JSONCONS_DETAIL_OPTIONAL_HPP

#include <new> // placement new
#include <memory>
#include <utility> // std::swap
#include <type_traits>
#include <jsoncons/config/compiler_support.hpp>

namespace jsoncons 
{ 
namespace detail 
{
    template <typename T>
    class optional
    {
    public:
        typedef T value_type;
    private:
        bool has_value_;
        union {
            char dummy_;
            T value_;
        };
    public:
        constexpr optional() noexcept
            : has_value_(false), dummy_{}
        {
        }

        optional(const optional& other) = default;

        optional(optional&& other,
                 typename std::enable_if<std::is_move_constructible<T>::value>::type* = 0)
       {
            if (other)
            {
                construct(std::move(other.value_));
            }
            else
            {
                has_value_ = false;
            }
       }

        template <class U>
        optional(const optional<U>& other,
                 typename std::enable_if<!std::is_same<T,U>::value &&
                                         std::is_constructible<T, const U&>::value &&
                                         std::is_convertible<const U&,T>::value &&
                                         std::is_copy_constructible<U>::value>::type* = 0)
        {
            if (other)
            {
                construct(*other);
            }
        }

        template <class U>
        explicit optional(const optional<U>& other,
                          typename std::enable_if<!std::is_same<T,U>::value &&
                                                  std::is_constructible<T, const U&>::value &&
                                                  !std::is_convertible<const U&,T>::value &&
                                                  std::is_copy_constructible<U>::value>::type* = 0)
        {
            if (other)
            {
                construct(*other);
            }
        }

        template <class U = T>
        optional(U&& value,
             typename std::enable_if<!std::is_same<optional<T>, typename std::decay<U>::type>::value &&
                                    std::is_constructible<T, U&&>::value &&
                                    std::is_convertible<U&&,T>::value>::type * = 0) // (8)
            : has_value_(true), value_(std::forward<U>(value))
        {
        }

        template <class U = T>
        explicit optional(U&& value,
                      typename std::enable_if<!std::is_same<optional<T>, typename std::decay<U>::type>::value &&
                                              std::is_constructible<T, U&&>::value &&
                                              !std::is_convertible<U&&,T>::value>::type* = 0) // (8)
            : has_value_(true), value_(std::forward<U>(value))
        {
        }
        ~optional()
        {
            destroy();
        }

        void destroy() noexcept 
        {
            if (has_value_) 
            {
                value_.~T();
                has_value_ = false;
            }
        }

        optional& operator=( const optional& other ) = default;

        optional& operator=(optional&& other ) = default;

        template <typename U=T>
        typename std::enable_if<!std::is_same<optional<T>, typename std::decay<U>::type>::value &&
                                std::is_constructible<T, U>::value &&
                                std::is_assignable<T&, U>::value &&
                                !(std::is_scalar<T>::value && std::is_same<T, typename std::decay<U>::type>::value),
            optional&>::type
        operator=(U&& v)
        {
            assign(std::forward<U>(v));
            return *this;
        }

        template <typename U>
        typename std::enable_if<!std::is_same<optional<T>, U>::value &&
                                std::is_constructible<T, const U&>::value &&
                                std::is_assignable<T&, const U&>::value,
            optional&>::type
        operator=(const optional<U>& other)
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
        typename std::enable_if<!std::is_same<optional<T>, U>::value &&
                                std::is_constructible<T, U>::value &&
                                std::is_assignable<T&, U>::value,
            optional&>::type
        operator=(optional<U>&& other)
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

        constexpr explicit operator bool() const noexcept
        {
            return has_value_;
        }
        constexpr bool has_value() const noexcept
        {
            return has_value_;
        }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif // _MSC_VER

        T& value() &
        {
            return static_cast<bool>(*this)
                       ? get()
                       : JSONCONS_THROW(std::runtime_error("Bad optional access")), get();
        }

        constexpr const T& value() const &
        {
            return static_cast<bool>(*this)
                       ? get()
                       : JSONCONS_THROW(std::runtime_error("Bad optional access")), get();
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
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

        const T* operator->() const
        {
            return std::addressof(this->value_);
        }

        T* operator->()
        {
            return std::addressof(this->value_);
        }

        constexpr const T& operator*() const&
        {
            return value();
        }

        T& operator*() &
        {
            return value();
        }
    private:
        constexpr const T& get() const { return this->value_; }
        T& get() { return this->value_; }

        template <typename... Args>
        void construct(Args&&... args) 
        {
            ::new (static_cast<void*>(&this->dummy_)) T(std::forward<Args>(args)...);
            has_value_ = true;
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
} // namespace detail
} // namespace jsoncons

#endif
