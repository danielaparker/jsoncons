// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_ORDERED_JSON_OBJECT_HPP
#define JSONCONS_ORDERED_JSON_OBJECT_HPP

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
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/allocator_holder.hpp>
#include <jsoncons/json_array.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/key_value.hpp>

namespace jsoncons {

    // Preserve order
    template <typename KeyT,typename Json,template <typename,typename> class SequenceContainer = std::vector>
    class ordered_json_object : public allocator_holder<typename Json::allocator_type>
    {
    public:
        using allocator_type = typename Json::allocator_type;
        using char_type = typename Json::char_type;
        using key_type = KeyT;
        using string_view_type = typename Json::string_view_type;
        using key_value_type = key_value<KeyT,Json>;
    private:

        using key_value_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_type>;
        using key_value_container_type = SequenceContainer<key_value_type,key_value_allocator_type>;

        key_value_container_type data_;

        struct Comp
        {
            const key_value_container_type& data_;

            Comp(const key_value_container_type& data_)
                : data_(data_)
            {
            }

            bool operator() (std::size_t i, string_view_type k) const { return data_.at(i).key() < k; }
            bool operator() (string_view_type k, std::size_t i) const { return k < data_.at(i).key(); }
        };
    public:
        using size_type = typename key_value_container_type::size_type;
        using iterator = typename key_value_container_type::iterator;
        using const_iterator = typename key_value_container_type::const_iterator;

        using allocator_holder<allocator_type>::get_allocator;

        ordered_json_object() = default;

        ordered_json_object(const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              data_(key_value_allocator_type(alloc))
        {
        }

        ordered_json_object(const ordered_json_object& val)
            : allocator_holder<allocator_type>(val.get_allocator()), 
              data_(val.data_)
        {
        }

        ordered_json_object(ordered_json_object&& val,const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              data_(std::move(val.data_),key_value_allocator_type(alloc))
        {
        }

        ordered_json_object(ordered_json_object&& val) noexcept
            : allocator_holder<allocator_type>(val.get_allocator()), 
              data_(std::move(val.data_))
        {
        }

        ordered_json_object(const ordered_json_object& val, const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              data_(val.data_,key_value_allocator_type(alloc))
        {
        }

        template <typename InputIt>
        ordered_json_object(InputIt first, InputIt last)
        {
            std::size_t count = std::distance(first,last);
            if (count > 0)
            {
                data_.reserve(count);
                if (count <= bloom_bytes)
                {
                    std::uint8_t bloom[bloom_bytes];
                    std::memset(bloom, 0, bloom_bytes);
                    for (auto it = first; it != last; ++it)
                    {
                        uint32_t h = a5hash32((*it).key().data(), (*it).key().size(), 0);
                        if (JSONCONS_LIKELY(!bloom_may_contain(bloom, h)))
                        {
                            data_.emplace_back((*it).key(), (*it).value());
                            bloom_set(bloom, h);
                        }
                        else
                        {
                            try_emplace((*it).key(), (*it).value());
                        }
                    }

                }
                else
                {
                    for (auto it = first; it != last; ++it)
                    {
                        try_emplace((*it).key(), (*it).value());
                    }
                }
            }
        }

        template <typename InputIt>
        ordered_json_object(InputIt first, InputIt last, const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              data_(key_value_allocator_type(alloc))
        {
            std::size_t size = std::distance(first,last);
            if (size > 0)
            {
                data_.reserve(size);
                if (size <= bloom_bytes)
                {
                    std::uint8_t bloom[bloom_bytes];
                    std::memset(bloom, 0, bloom_bytes);
                    for (auto it = first; it != last; ++it)
                    {
                        uint32_t h = a5hash32((*it).key().data(), (*it).key().size(), 0);
                        if (JSONCONS_LIKELY(!bloom_may_contain(bloom, h)))
                        {
                            data_.emplace_back((*it).key(), (*it).value());
                            bloom_set(bloom, h);
                        }
                        else
                        {
                            try_emplace((*it).key(), (*it).value());
                        }
                    }

                }
                else
                {
                    for (auto it = first; it != last; ++it)
                    {
                        try_emplace((*it).key(), (*it).value());
                    }
                }
            }
        }

        ordered_json_object(std::initializer_list<std::pair<std::basic_string<char_type>,Json>> init, 
                    const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              data_(key_value_allocator_type(alloc))
        {
            data_.reserve(init.size());
            for (auto& item : init)
            {
                insert_or_assign(item.first, item.second);
            }
        }

        ~ordered_json_object() noexcept
        {
            flatten_and_destroy();
        }

        ordered_json_object& operator=(ordered_json_object&& val)
        {
            val.swap(*this);
            return *this;
        }

        ordered_json_object& operator=(const ordered_json_object& val)
        {
            data_ = val.data_;
            return *this;
        }

        void swap(ordered_json_object& other) noexcept
        {
            data_.swap(other.data_);
        }

        bool empty() const
        {
            return data_.empty();
        }

        iterator begin()
        {
            return data_.begin();
        }

        iterator end()
        {
            return data_.end();
        }

        const_iterator begin() const
        {
            return data_.begin();
        }

        const_iterator end() const
        {
            return data_.end();
        }

        std::size_t size() const {return data_.size();}

        std::size_t capacity() const {return data_.capacity();}

        void clear() 
        {
            data_.clear();
        }

        void shrink_to_fit() 
        {
            for (std::size_t i = 0; i < data_.size(); ++i)
            {
                data_[i].shrink_to_fit();
            }
            data_.shrink_to_fit();
        }

        void reserve(std::size_t n) {data_.reserve(n);}

        Json& at(std::size_t i) 
        {
            if (i >= data_.size())
            {
                JSONCONS_THROW(json_runtime_error<std::out_of_range>("Invalid array subscript"));
            }
            return data_[i].value();
        }

        const Json& at(std::size_t i) const 
        {
            if (i >= data_.size())
            {
                JSONCONS_THROW(json_runtime_error<std::out_of_range>("Invalid array subscript"));
            }
            return data_[i].value();
        }

        iterator find(string_view_type key) noexcept
        {
            const size_type length = size();
            for (size_type i = 0; i < length; ++i) 
            {
               if (data_[i].key() == key) 
               {
                   return data_.begin() + i;
               }
            }
            return data_.end();
        }

        const_iterator find(string_view_type key) const noexcept
        {
            const size_type length = size();
            for (size_type i = 0; i < length; ++i) 
            {
               if (data_[i].key() == key) 
               {
                   return data_.begin() + i;
               }
            }
            return data_.end();
        }

        iterator erase(const_iterator pos) 
        {
            if (pos != data_.end())
            {
                return data_.erase(pos);
            }
            else
            {
                return data_.end();
            }
        }

        iterator erase(const_iterator first, const_iterator last) 
        {
            std::size_t pos1 = first == data_.end() ? data_.size() : first - data_.begin();
            std::size_t pos2 = last == data_.end() ? data_.size() : last - data_.begin();

            if (pos1 < data_.size() && pos2 <= data_.size())
            {

                return data_.erase(first,last);
            }
            else
            {
                return data_.end();
            }
        }

        size_type erase(string_view_type key) 
        {
            auto pos = find(key);
            if (pos != data_.end())
            {
                data_.erase(pos);
                return 1;
            }
            return 0;
        }

        static constexpr size_t bloom_bytes = 512;
        static constexpr uint32_t bloom_bits = bloom_bytes * 8; 
        static constexpr uint32_t bloom_mask = bloom_bits - 1; 

        static void bloom_set(uint8_t* bloom, uint32_t h) noexcept
        {
           const uint32_t a = h & bloom_mask;
           const uint32_t b = (h >> 10) & bloom_mask;
           bloom[a >> 3] |= uint8_t(1) << (a & 7);
           bloom[b >> 3] |= uint8_t(1) << (b & 7);
        }

        static bool bloom_may_contain(const uint8_t* bloom, uint32_t h) noexcept
        {
           const uint32_t a = h & bloom_mask;
           const uint32_t b = (h >> 10) & bloom_mask;
           return (bloom[a >> 3] & (uint8_t(1) << (a & 7))) && (bloom[b >> 3] & (uint8_t(1) << (b & 7)));
        }

        void uninitialized_init(index_key_value<Json>* items, std::size_t size)
        {
            if (size > 0)
            {
                index_key_value<Json>* last = items + size;

                data_.reserve(size);
                if (size <= bloom_bytes)
                {
                    std::uint8_t bloom[bloom_bytes];
                    std::memset(bloom, 0, bloom_bytes);
                    for (auto* p = items; p < last; ++p)
                    {
                        uint32_t h = a5hash32(p->name.data(), p->name.size(), 0);
                        if (JSONCONS_LIKELY(!bloom_may_contain(bloom, h)))
                        {
                            data_.emplace_back(std::move(p->name), std::move(p->value));
                            bloom_set(bloom, h);
                        }
                        else
                        {
                            try_emplace(std::move(p->name), std::move(p->value));
                        }
                    }
                    
                }
                else
                {
                    for (auto* p = items; p < last; ++p)
                    {
                        try_emplace(std::move(p->name), std::move(p->value));
                    }
                }
            }
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last)
        {
            std::size_t count = std::distance(first,last);
            if (count > 0)
            {
                const size_type old_size = size();
                data_.reserve(old_size+count);
                if (old_size+count <= bloom_bytes)
                {
                    std::uint8_t bloom[bloom_bytes];
                    std::memset(bloom, 0, bloom_bytes);
                    for (std::size_t i = 0; i < old_size; ++i)
                    {
                        uint32_t h = a5hash32(data_[i].key().data(), data_[i].key().size(), 0);
                        bloom_set(bloom, h);
                    }
                    for (auto it = first; it != last; ++it)
                    {
                        uint32_t h = a5hash32((*it).first.data(), (*it).first.size(), 0);
                        if (JSONCONS_LIKELY(!bloom_may_contain(bloom, h)))
                        {                          
                            data_.emplace_back(key_type{(*it).first.c_str(), (*it).first.size(), get_allocator()}, (*it).second);
                            bloom_set(bloom, h);
                        }
                        else
                        {
                            try_emplace(key_type{(*it).first.c_str(), (*it).first.size(), get_allocator()}, (*it).second);
                        }
                    }
                }
                else
                {
                    for (auto it = first; it != last; ++it)
                    {
                        try_emplace(key_type{(*it).first.c_str(), (*it).first.size(), get_allocator()}, (*it).second);
                    }
                }
            }
        }

        template <typename InputIt>
        void insert(sorted_unique_range_tag, InputIt first, InputIt last)
        {
            for (auto it = first; it != last; ++it)
            {
                data_.emplace_back(make_key_value<KeyT,Json>()(*it));
            }
        }
   
        template <typename T,typename A=allocator_type>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            auto it = find(name);
            if (it == data_.end())
            {
                data_.emplace_back(key_type(name.begin(), name.end()), std::forward<T>(value));
                auto pos = data_.begin() + (data_.size() - 1);
                return std::make_pair(pos, true);
            }
            else
            {
                (*it).value(Json(std::forward<T>(value)));
                return std::make_pair(it,false);
            }
        }

        template <typename T,typename A=allocator_type>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            auto it = find(name);
            if (it == data_.end())
            {
                data_.emplace_back(key_type(name.begin(),name.end(),get_allocator()), std::forward<T>(value));
                auto pos = data_.begin() + (data_.size()-1);
                return std::make_pair(pos,true);
            }
            else
            {
                (*it).value(Json(std::forward<T>(value),get_allocator()));
                return std::make_pair(it,false);
            }
        }

        template <typename A=allocator_type,typename T>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& key, T&& value)
        {
            if (hint == data_.end())
            {
                auto result = insert_or_assign(key, std::forward<T>(value));
                return result.first;
            }
            else
            {
                auto it = find(hint, key);
                if (it == data_.end())
                {
                    data_.emplace_back(key_type(key.begin(), key.end()), std::forward<T>(value));
                    auto pos = data_.begin() + (data_.size() - 1);
                    return pos;
                }
                else
                {
                    (*it).value(Json(std::forward<T>(value)));
                    return it;
                }
            }
        }

        template <typename A=allocator_type,typename T>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& key, T&& value)
        {
            if (hint == data_.end())
            {
                auto result = insert_or_assign(key, std::forward<T>(value));
                return result.first;
            }
            else
            {
                auto it = find(hint, key);
                if (it == data_.end())
                {
                    data_.emplace_back(key_type(key.begin(),key.end(),get_allocator()), std::forward<T>(value));
                    auto pos = data_.begin() + (data_.size()-1);
                    return pos;
                }
                else
                {
                    (*it).value(Json(std::forward<T>(value),get_allocator()));
                    return it;
                }
            }
        }

        // merge

        void merge(const ordered_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                try_emplace((*it).key(),(*it).value());
            }
        }

        void merge(ordered_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = find((*it).key());
                if (pos == data_.end())
                {
                    try_emplace((*it).key(),std::move((*it).value()));
                }
            }
        }

        void merge(iterator hint, const ordered_json_object& source)
        {
            std::size_t pos = hint - data_.begin();
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = try_emplace(hint, (*it).key(),(*it).value());
                std::size_t newpos = hint - data_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - data_.begin();
                }
                else
                {
                    hint = data_.begin() + pos;
                }
            }
        }

        void merge(iterator hint, ordered_json_object&& source)
        {
            std::size_t pos = hint - data_.begin();

            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                hint = try_emplace(hint, (*it).key(), std::move((*it).value()));
                std::size_t newpos = hint - data_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - data_.begin();
                }
                else
                {
                    hint = data_.begin() + pos;
                }
            }
        }

        // merge_or_update

        void merge_or_update(const ordered_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                insert_or_assign((*it).key(),(*it).value());
            }
        }

        void merge_or_update(ordered_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = find((*it).key());
                if (pos == data_.end())
                {
                    insert_or_assign((*it).key(),std::move((*it).value()));
                }
                else
                {
                    pos->value(std::move((*it).value()));
                }
            }
        }

        void merge_or_update(iterator hint, const ordered_json_object& source)
        {
            std::size_t pos = hint - data_.begin();
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = insert_or_assign(hint, (*it).key(),(*it).value());
                std::size_t newpos = hint - data_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - data_.begin();
                }
                else
                {
                    hint = data_.begin() + pos;
                }
            }
        }

        void merge_or_update(iterator hint, ordered_json_object&& source)
        {
            std::size_t pos = hint - data_.begin();
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                hint = insert_or_assign(hint, (*it).key(), std::move((*it).value()));
                std::size_t newpos = hint - data_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - data_.begin();
                }
                else
                {
                    hint = data_.begin() + pos;
                }
            }
        }

        // try_emplace

        template <typename A=allocator_type,typename... Args>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            auto it = find(name);
            if (it == data_.end())
            {
                data_.emplace_back(key_type(name.begin(), name.end()), std::forward<Args>(args)...);
                auto pos = data_.begin() + (data_.size()-1);
                return std::make_pair(pos,true);
            }
            else
            {
                return std::make_pair(it,false);
            }
        }

        template <typename A=allocator_type,typename... Args>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& key, Args&&... args)
        {
            auto it = find(key);
            if (it == data_.end())
            {
                data_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                    std::forward<Args>(args)...);
                auto pos = data_.begin() + data_.size();
                return std::make_pair(pos,true);
            }
            else
            {
                return std::make_pair(it,false);
            }
        }
     
        template <typename A=allocator_type,typename ... Args>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type
        try_emplace(iterator hint, const string_view_type& key, Args&&... args)
        {
            if (hint == data_.end())
            {
                auto result = try_emplace(key, std::forward<Args>(args)...);
                return result.first;
            }
            else
            {
                auto it = find(hint, key);
                if (it == data_.end())
                {
                    data_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                        std::forward<Args>(args)...);
                    auto pos = data_.begin() + data_.size();
                    return pos;
                }
                else
                {
                    return it;
                }
            }
        }

        template <typename A=allocator_type,typename ... Args>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,iterator>::type
        try_emplace(iterator hint, const string_view_type& key, Args&&... args)
        {
            if (hint == data_.end())
            {
                auto result = try_emplace(key, std::forward<Args>(args)...);
                return result.first;
            }
            else
            {
                auto it = find(hint, key);
                if (it == data_.end())
                {
                    data_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                        std::forward<Args>(args)...);
                    auto pos = data_.begin() + data_.size();
                    return pos;
                }
                else
                {
                    return it;
                }
            }
        }

        bool operator==(const ordered_json_object& rhs) const
        {
            return data_ == rhs.data_;
        }
     
        bool operator<(const ordered_json_object& rhs) const
        {
            return data_ < rhs.data_;
        }
    private:

        iterator find(iterator hint, const string_view_type& name) noexcept
        {
            bool found = false;
            auto it = hint;
            while (!found && it != data_.end())
            {
                if ((*it).key() == name)
                {
                    found = true;
                }
                else
                {
                    ++it;
                }
            }
            return found ? it : find(name);
        }

        void flatten_and_destroy() noexcept
        {
            if (!data_.empty())
            {
                json_array<Json> temp(get_allocator());

                for (auto& kv : data_)
                {
                    switch (kv.value().storage_kind())
                    {
                        case json_storage_kind::array:
                        case json_storage_kind::object:
                            if (!kv.value().empty())
                            {
                                temp.emplace_back(std::move(kv.value()));
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    };

} // namespace jsoncons

#endif // JSONCONS_ORDERED_JSON_OBJECT_HPP
