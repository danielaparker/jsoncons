// Copyright 2013-2026 Daniel Parker
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
#include <jsoncons/json_array.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {

    template <typename Json>
    struct index_key_value
    {
        using key_type = typename Json::key_type;
        using allocator_type = typename Json::allocator_type;

        key_type name;
        int64_t index;
        Json value;

        template <typename... Args>
        index_key_value(key_type&& Name, int64_t Index, Args&& ... args) 
            : name(std::move(Name)), index(Index), value(std::forward<Args>(args)...)
        {
        }

        index_key_value() = default;
        index_key_value(const index_key_value&) = default;
        index_key_value(index_key_value&&) = default;
        index_key_value(const index_key_value& other, const allocator_type& alloc) 
            : name(other.name, alloc), index(0), value(other.value, alloc) 
        {
        }
        index_key_value(index_key_value&& other, const allocator_type& alloc)
            : name(std::move(other.name), alloc), index(0), value(std::move(other.value), alloc) 
        {

        }
        index_key_value& operator=(const index_key_value&) = default;
        index_key_value& operator=(index_key_value&&) = default;
    };

    struct sorted_unique_range_tag
    {
        explicit sorted_unique_range_tag() = default; 
    };

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

        string_view_type name() const
        {
            return string_view_type{key_.data(), key_.size()};
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

    // Structured Bindings Support
    // See https://blog.tartanllama.xyz/structured-bindings/
    template<std::size_t N,typename Key,typename Value,typename std::enable_if<N == 0, int>::type = 0>
    auto get(const key_value<Key,Value>& i) -> decltype(i.key())
    {
        return i.key();
    }
    // Structured Bindings Support
    // See https://blog.tartanllama.xyz/structured-bindings/
    template<std::size_t N,typename Key,typename Value,typename std::enable_if<N == 1, int>::type = 0>
    auto get(const key_value<Key,Value>& i) -> decltype(i.value())
    {
        return i.value();
    }

} // namespace jsoncons

namespace std
{
#if defined(__clang__)
    // Fix: https://github.com/nlohmann/json/issues/1401
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

    template <typename Key,typename Value>
    struct tuple_size<jsoncons::key_value<Key,Value>>
        : public std::integral_constant<std::size_t, 2> {};

    template <typename Key,typename Value> struct tuple_element<0, jsoncons::key_value<Key,Value>> { using type = Key; };
    template <typename Key,typename Value> struct tuple_element<1, jsoncons::key_value<Key,Value>> { using type = Value; };

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

}  // namespace std

namespace jsoncons {

    template <typename KeyT,typename ValueT>
    struct make_key_value
    {
        using key_value_type = key_value<KeyT,ValueT>;

        template <typename T1,typename T2>
        key_value_type operator()(const std::pair<T1,T2>& p) 
        {
            return key_value_type(KeyT(p.first),p.second);
        }

        template <typename T1,typename T2, typename Alloc>
        key_value_type operator()(const std::pair<T1,T2>& p, const Alloc& alloc) 
        {
            return key_value_type(jsoncons::make_obj_using_allocator<KeyT>(alloc, p.first), 
                p.second, alloc);
        }

        template <typename T1,typename T2>
        key_value_type operator()(std::pair<T1,T2>&& p)
        {
            return key_value_type(std::forward<T1>(p.first),std::forward<T2>(p.second));
        }

        template <typename T1,typename T2, typename Alloc>
        key_value_type operator()(std::pair<T1,T2>&& p, const Alloc& alloc)
        {
            return key_value_type(jsoncons::make_obj_using_allocator<KeyT>(alloc, std::forward<T1>(p.first)), 
                std::forward<T2>(p.second), alloc);
        }

        template <typename T1,typename T2>
        const key_value_type& operator()(const key_value<T1,T2>& p)
        {
            return p;
        }
        template <typename T1,typename T2>
        key_value_type operator()(key_value<T1,T2>&& p)
        {
            return std::move(p);
        }
    };

} // namespace jsoncons

#endif // JSONCONS_KEY_VALUE_HPP
