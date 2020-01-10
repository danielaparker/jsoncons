// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_OPTIONAL_HPP
#define JSONCONS_DETAIL_OPTIONAL_HPP

#include <new> // placement new
#include <memory>
#include <utility> // std::swap
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
        typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type storage_;
        value_type* valuep_;
    public:
        constexpr optional() noexcept
            : valuep_(nullptr)
        {
        }

        optional(const optional& other) noexcept
            : valuep_(nullptr)
        {
            if (other)
            {
                valuep_ = ::new(&storage_)value_type(other.value());
            }
        }

        optional(optional&& other) noexcept
            : valuep_(nullptr)
        {
            std::swap(valuep_,other.valuep_);
            std::swap(storage_,other.storage_);
        }

        template <class U=T>
        optional(const optional<U>& other,
                 typename std::enable_if<std::is_copy_constructible<U>::value>::type* = 0)
            : valuep_(nullptr)
        {
            if (other)
            {
                valuep_ = ::new(&storage_)value_type(other.value());
            }
        }

        template <class U = T>
        optional(U&& value,
             typename std::enable_if<std::is_constructible<T, U&&>::value &&
                                    std::is_convertible<U&&,T>::value>::type * = 0) // (8)
            : valuep_(nullptr)
        {
            valuep_ = ::new(&storage_)value_type(std::forward<U>(value));
        }

        template <class U = T>
        explicit optional(U&& value,
                      typename std::enable_if<std::is_constructible<T, U&&>::value &&
                                              !std::is_convertible<U&&,T>::value>::type* = 0) // (8)
            : valuep_(nullptr)
        {
            valuep_ = ::new(&storage_)value_type(std::forward<U>(value));
        }
        ~optional()
        {
            if (valuep_)
            {
                valuep_->~T();
                valuep_ = nullptr;
            }
        }

        optional& operator=( const optional& other )
        {
            if (this != &other)
            {
                if (valuep_)
                {
                    valuep_->~T();
                    valuep_ = nullptr;
                }
                if (other)
                {
                    valuep_ = ::new(&storage_)value_type(other.value());
                }
            }
            return *this;
        }

        optional& operator=( optional&& other ) noexcept
        {
            if (this != &other)
            {
                std::swap(valuep_, other.valuep_);
            }
            return *this;
        }

        template< class U = T >
        optional& operator=(U&& value)
        {
            if (valuep_)
            {
                valuep_->~T();
                valuep_ = nullptr;
            }
            valuep_ = ::new(&storage_)value_type(value);
            return *this;
        }

        void swap( optional& other ) noexcept
        {
            std::swap(valuep_,other.valuep_);
        }

        constexpr explicit operator bool() const noexcept
        {
            return valuep_ != nullptr;
        }
        constexpr bool has_value() const noexcept
        {
            return valuep_ != nullptr;
        }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif // _MSC_VER

        T& value() &
        {
            if (valuep_ == nullptr)
            {
                JSONCONS_THROW(std::runtime_error("Bad optional access"));
            }
            return *valuep_;
        }

        constexpr const T& value() const &
        {
            if (valuep_ == nullptr)
            {
                JSONCONS_THROW(std::runtime_error("Bad optional access"));
            }
            return *valuep_;
        }

        template <typename U>
        constexpr T value_or(U&& default_value) const & 
        {
            static_assert(std::is_copy_constructible<T>::value,
                          "get_value_or: T must be copy constructible");
            static_assert(std::is_convertible<U&&, T>::value,
                          "get_value_or: U must be convertible to T");
            return bool(*this) ? **this : static_cast<T>(std::forward<U>(default_value));
        }

        template <typename U>
        T value_or(U&& default_value) && 
        {
            static_assert(std::is_move_constructible<T>::value,
                          "get_value_or: T must be move constructible");
            static_assert(std::is_convertible<U&&, T>::value,
                          "get_value_or: U must be convertible to T");
            return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
        }
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

        const T* operator->() const
        {
            return valuep_;
        }

        T* operator->()
        {
            return valuep_;
        }

        constexpr const T& operator*() const&
        {
            return value();
        }

        T& operator*() &
        {
            return value();
        }
    };
} // namespace detail
} // namespace jsoncons

#endif
