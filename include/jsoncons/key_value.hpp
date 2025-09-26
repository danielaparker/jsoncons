// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_KEY_VALUE_HPP
#define JSONCONS_KEY_VALUE_HPP

#include <algorithm> // std::sort, std::stable_sort, std::lower_bound, std::unique
#include <cassert> // assert
#include <cstring>
#include <initializer_list>
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <string>
#include <tuple>
#include <type_traits> // std::enable_if
#include <unordered_set>
#include <utility>
#include <utility> // std::move
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/allocator_holder.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {

    // key_value

    template <typename KeyT,typename ValueT>
    class key_value
    {
    public:
        using key_type = KeyT;
        using value_type = ValueT;
        using string_view_type = typename value_type::string_view_type;
        using allocator_type = typename ValueT::allocator_type;
    private:

        key_type key_;
        value_type value_;
    public:

        template <typename... Args>
        key_value(key_type&& name,  Args&& ... args) 
            : key_(std::move(name)), value_(std::forward<Args>(args)...)
        {
        }
        key_value(const key_value& member)
            : key_(member.key_), value_(member.value_)
        {
        }

        key_value(const key_value& member, const allocator_type& alloc)
            : key_(member.key_, alloc), value_(member.value_, alloc)
        {
        }

        key_value(key_value&& member) noexcept
            : key_(std::move(member.key_)), value_(std::move(member.value_))
        {
        }

        key_value(key_value&& member, const allocator_type& alloc) noexcept
            : key_(std::move(member.key_), alloc), value_(std::move(member.value_), alloc)
        {
        }

        const key_type& key() const
        {
            return key_;
        }

        value_type& value()
        {
            return value_;
        }

        const value_type& value() const
        {
            return value_;
        }

        template <typename T>
        void value(T&& newValue)
        {
            value_ = std::forward<T>(newValue);
        }

        void swap(key_value& member) noexcept
        {
            key_.swap(member.key_);
            value_.swap(member.value_);
        }

        key_value& operator=(const key_value& member)
        {
            if (this != & member)
            {
                key_ = member.key_;
                value_ = member.value_;
            }
            return *this;
        }

        key_value& operator=(key_value&& member) noexcept
        {
            if (this != &member)
            {
                key_.swap(member.key_);
                value_.swap(member.value_);
            }
            return *this;
        }

        void shrink_to_fit() 
        {
            key_.shrink_to_fit();
            value_.shrink_to_fit();
        }

        friend bool operator==(const key_value& lhs, const key_value& rhs) noexcept
        {
            return lhs.key_ == rhs.key_ && lhs.value_ == rhs.value_;
        }

        friend bool operator!=(const key_value& lhs, const key_value& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        friend bool operator<(const key_value& lhs, const key_value& rhs) noexcept
        {
            if (lhs.key_ < rhs.key_)
            {
                return true;
            }
            if (lhs.key_ == rhs.key_ && lhs.value_ < rhs.value_)
            {
                return true;
            }
            return false;
        }

        friend bool operator<=(const key_value& lhs, const key_value& rhs) noexcept
        {
            return !(rhs < lhs);
        }

        friend bool operator>(const key_value& lhs, const key_value& rhs) noexcept
        {
            return !(lhs <= rhs);
        }

        friend bool operator>=(const key_value& lhs, const key_value& rhs) noexcept
        {
            return !(lhs < rhs);
        }

        friend void swap(key_value& a, key_value& b) noexcept(
            noexcept(std::declval<key_value&>().swap(std::declval<key_value&>()))) 
        {
            a.swap(b);
        }
    };

} // namespace jsoncons

#endif // JSONCONS_KEY_VALUE_HPP
