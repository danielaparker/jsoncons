// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_ARRAY_HPP
#define JSONCONS_JSON_ARRAY_HPP

#include <algorithm> // std::sort, std::stable_sort, std::lower_bound, std::unique
#include <cassert> // assert
#include <cstring>
#include <initializer_list>
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <type_traits> // std::enable_if
#include <utility>
#include <utility> // std::move
#include <vector>

#include <jsoncons/allocator_holder.hpp>
#include <jsoncons/json_type.hpp>

namespace jsoncons {

    // json_array

    template <typename Json,template <typename,typename> class SequenceContainer = std::vector>
    class json_array : public allocator_holder<typename Json::allocator_type>
    {
    public:
        using allocator_type = typename Json::allocator_type;
        using value_type = Json;
    private:
        using value_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type>;                   
        using value_container_type = SequenceContainer<value_type,value_allocator_type>;
        value_container_type data_;
    public:
        using iterator = typename value_container_type::iterator;
        using const_iterator = typename value_container_type::const_iterator;
        using reference = typename std::iterator_traits<iterator>::reference;
        using const_reference = typename std::iterator_traits<const_iterator>::reference;

        using allocator_holder<allocator_type>::get_allocator;

        json_array() = default;

        explicit json_array(const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              data_(value_allocator_type(alloc))
        {
        }

        explicit json_array(std::size_t n, 
                            const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              data_(n,Json(),value_allocator_type(alloc))
        {
        }

        explicit json_array(std::size_t n, 
                            const Json& value, 
                            const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              data_(n,value,value_allocator_type(alloc))
        {
        }

        template <typename InputIterator>
        json_array(InputIterator begin, InputIterator end, const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              data_(begin,end,value_allocator_type(alloc))
        {
        }

        json_array(const json_array& other)
            : allocator_holder<allocator_type>(other.get_allocator()),
              data_(other.data_)
        {
        }
        json_array(const json_array& other, const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              data_(other.data_,value_allocator_type(alloc))
        {
        }

        json_array(json_array&& other) noexcept
            : allocator_holder<allocator_type>(other.get_allocator()), 
              data_(std::move(other.data_))
        {
        }
        json_array(json_array&& other, const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              data_(std::move(other.data_),value_allocator_type(alloc))
        {
        }

        json_array(const std::initializer_list<Json>& init, 
                   const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              data_(init,value_allocator_type(alloc))
        {
        }
        ~json_array() noexcept
        {
            flatten_and_destroy();
        }

        reference back()
        {
            return data_.back();
        }

        const_reference back() const
        {
            return data_.back();
        }

        void pop_back()
        {
            data_.pop_back();
        }

        bool empty() const
        {
            return data_.empty();
        }

        void swap(json_array& other) noexcept
        {
            data_.swap(other.data_);
        }

        std::size_t size() const {return data_.size();}

        std::size_t capacity() const {return data_.capacity();}

        // Direct access to data
        value_type* data() noexcept { return data_.data(); }
        const value_type* data() const noexcept { return data_.data(); }

        void clear() {data_.clear();}

        void shrink_to_fit() 
        {
            for (std::size_t i = 0; i < data_.size(); ++i)
            {
                data_[i].shrink_to_fit();
            }
            data_.shrink_to_fit();
        }

        void reserve(std::size_t n) {data_.reserve(n);}

        void resize(std::size_t n) {data_.resize(n);}

        void resize(std::size_t n, const Json& val) {data_.resize(n,val);}

        iterator erase(const_iterator pos) 
        {
            return data_.erase(pos);
        }

        iterator erase(const_iterator first, const_iterator last) 
        {
            return data_.erase(first,last);
        }

        Json& operator[](std::size_t i) {return data_[i];}

        const Json& operator[](std::size_t i) const {return data_[i];}

        // push_back

        template <typename T,typename A=allocator_type>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,void>::type 
        push_back(T&& value)
        {
            data_.emplace_back(std::forward<T>(value));
        }

        template <typename T,typename A=allocator_type>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,void>::type 
        push_back(T&& value)
        {
            data_.emplace_back(std::forward<T>(value));
        }

        template <typename T,typename A=allocator_type>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert(const_iterator pos, T&& value)
        {
            return data_.emplace(pos, std::forward<T>(value));
        }
        template <typename T,typename A=allocator_type>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert(const_iterator pos, T&& value)
        {
            return data_.emplace(pos, std::forward<T>(value));
        }

        template <typename InputIt>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            return data_.insert(pos, first, last);
        }

        template <typename A=allocator_type,typename... Args>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        emplace(const_iterator pos, Args&&... args)
        {
            return data_.emplace(pos, std::forward<Args>(args)...);
        }

        template <typename... Args>
        Json& emplace_back(Args&&... args)
        {
            data_.emplace_back(std::forward<Args>(args)...);
            return data_.back();
        }

        iterator begin() {return data_.begin();}

        iterator end() {return data_.end();}

        const_iterator begin() const {return data_.begin();}

        const_iterator end() const {return data_.end();}

        bool operator==(const json_array& rhs) const noexcept
        {
            return data_ == rhs.data_;
        }

        bool operator<(const json_array& rhs) const noexcept
        {
            return data_ < rhs.data_;
        }

        json_array& operator=(const json_array& other)
        {
            data_ = other.data_;
            return *this;
        }
    private:

        void flatten_and_destroy() noexcept
        {
            while (!data_.empty())
            {
                value_type current = std::move(data_.back());
                data_.pop_back();
                switch (current.storage_kind())
                {
                    case json_storage_kind::array:
                    {
                        for (auto&& item : current.array_range())
                        {
                            if ((item.storage_kind() == json_storage_kind::array || item.storage_kind() == json_storage_kind::object)
                                && !item.empty()) // non-empty object or array
                            {
                                data_.push_back(std::move(item));
                            }
                        }
                        current.clear();                           
                        break;
                    }
                    case json_storage_kind::object:
                    {
                        for (auto&& kv : current.object_range())
                        {
                            if ((kv.value().storage_kind() == json_storage_kind::array || kv.value().storage_kind() == json_storage_kind::object)
                                && !kv.value().empty()) // non-empty object or array
                            {
                                data_.push_back(std::move(kv.value()));
                            }
                        }
                        current.clear();                           
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    };

} // namespace jsoncons

#endif // JSONCONS_JSON_ARRAY_HPP
