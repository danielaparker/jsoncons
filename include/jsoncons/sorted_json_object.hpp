// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_OBJECT_HPP
#define JSONCONS_JSON_OBJECT_HPP

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
#include <jsoncons/key_value.hpp>
#include <jsoncons/utility/a5hash.hpp>

namespace jsoncons {

    // Sort keys
    template <typename KeyT,typename Json,template <typename,typename> class SequenceContainer = std::vector>
    class sorted_json_object : public allocator_holder<typename Json::allocator_type>    
    {
    public:
        using allocator_type = typename Json::allocator_type;
        using key_type = KeyT;
        using key_value_type = key_value<KeyT,Json>;
        using char_type = typename Json::char_type;
        using string_view_type = typename Json::string_view_type;
    private:
        struct Comp
        {
            bool operator() (const key_value_type& kv, string_view_type k) const { return kv.key() < k; }
            bool operator() (string_view_type k, const key_value_type& kv) const { return k < kv.key(); }
        };

        using key_value_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_type>;
        using key_value_container_type = SequenceContainer<key_value_type,key_value_allocator_type>;

        key_value_container_type members_;
    public:
        using iterator = typename key_value_container_type::iterator;
        using const_iterator = typename key_value_container_type::const_iterator;

        using allocator_holder<allocator_type>::get_allocator;

        sorted_json_object()
        {
        }

        explicit sorted_json_object(const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc))
        {
        }

        sorted_json_object(const sorted_json_object& other)
            : allocator_holder<allocator_type>(other.get_allocator()),
              members_(other.members_)
        {
        }

        sorted_json_object(const sorted_json_object& other, const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              members_(other.members_,key_value_allocator_type(alloc))
        {
        }

        sorted_json_object(sorted_json_object&& other) noexcept
            : allocator_holder<allocator_type>(other.get_allocator()), 
              members_(std::move(other.members_))
        {
        }

        sorted_json_object(sorted_json_object&& other,const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), members_(std::move(other.members_),key_value_allocator_type(alloc))
        {
        }

        sorted_json_object& operator=(const sorted_json_object& other)
        {
            members_ = other.members_;
            return *this;
        }

        sorted_json_object& operator=(sorted_json_object&& other) noexcept
        {
            other.swap(*this);
            return *this;
        }

        template <typename InputIt>
        sorted_json_object(InputIt first, InputIt last)
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto it = first; it != last; ++it)
            {
                auto kv = make_key_value<KeyT,Json>()(*it);
                members_.emplace_back(std::move(kv));
            }
            std::stable_sort(members_.begin(),members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto last2 = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(last2, members_.end());
        }

        template <typename InputIt>
        sorted_json_object(InputIt first, InputIt last, const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc))
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto it = first; it != last; ++it)
            {
                auto kv = make_key_value<KeyT,Json>()(*it, alloc);
                members_.emplace_back(std::move(kv));
            }
            std::stable_sort(members_.begin(), members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto last2 = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(last2, members_.end());
        }

        sorted_json_object(const std::initializer_list<std::pair<std::basic_string<char_type>,Json>>& init, 
                    const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc))
        {
            members_.reserve(init.size());
            for (auto& item : init)
            {
                insert_or_assign(item.first, item.second);
            }
        }

        ~sorted_json_object() noexcept
        {
            flatten_and_destroy();
        }

        bool empty() const
        {
            return members_.empty();
        }

        void swap(sorted_json_object& other) noexcept
        {
            members_.swap(other.members_);
        }

        iterator begin()
        {
            return members_.begin();
        }

        iterator end()
        {
            return members_.end();
        }

        const_iterator begin() const
        {
            return members_.begin();
        }

        const_iterator end() const
        {
            return members_.end();
        }

        std::size_t size() const {return members_.size();}

        std::size_t capacity() const {return members_.capacity();}

        void clear() {members_.clear();}

        void shrink_to_fit() 
        {
            for (std::size_t i = 0; i < members_.size(); ++i)
            {
                members_[i].shrink_to_fit();
            }
            members_.shrink_to_fit();
        }

        void reserve(std::size_t n) {members_.reserve(n);}

        Json& at(std::size_t i) 
        {
            if (i >= members_.size())
            {
                JSONCONS_THROW(json_runtime_error<std::out_of_range>("Invalid array subscript"));
            }
            return members_[i].value();
        }

        const Json& at(std::size_t i) const 
        {
            if (i >= members_.size())
            {
                JSONCONS_THROW(json_runtime_error<std::out_of_range>("Invalid array subscript"));
            }
            return members_[i].value();
        }

        iterator find(const string_view_type& name) noexcept
        {
            auto p = std::equal_range(members_.begin(),members_.end(), name, 
                                       Comp());        
            return p.first == p.second ? members_.end() : p.first;
        }

        const_iterator find(const string_view_type& name) const noexcept
        {
            auto p = std::equal_range(members_.begin(),members_.end(), name, 
                                       Comp());        
            return p.first == p.second ? members_.end() : p.first;
        }

        iterator erase(const_iterator pos) 
        {
            return members_.erase(pos);
        }

        iterator erase(const_iterator first, const_iterator last) 
        {
            return members_.erase(first,last);
        }

        void erase(const string_view_type& name) 
        {
            auto it = find(name);
            if (it != members_.end())
            {
                members_.erase(it);
            }
        }

        static bool compare(const index_key_value<Json>& item1, const index_key_value<Json>& item2)
        {
            int comp = item1.name.compare(item2.name); 
            if (comp < 0) return true;
            if (comp == 0) return item1.index < item2.index;

            return false;
        }

        void uninitialized_init(index_key_value<Json>* items, std::size_t count)
        {
            if (count > 0)
            {
                members_.reserve(count);

                std::sort(items, items+count, compare);
                members_.emplace_back(key_type(items[0].name.data(), items[0].name.size(), get_allocator()), std::move(items[0].value));
                
                for (std::size_t i = 1; i < count; ++i)
                {
                    auto& item = items[i];
                    if (item.name != items[i-1].name)
                    {
                        members_.emplace_back(key_type(item.name.data(), item.name.size(), get_allocator()), std::move(item.value));
                    }
                }
            }
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last)
        {
            for (auto it = first; it != last; ++it)
            {
                members_.emplace_back(key_type((*it).first.c_str(), (*it).first.size(), get_allocator()), (*it).second);
            }
            std::stable_sort(members_.begin(),members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto last2 = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(last2, members_.end());
        }

        template <typename InputIt>
        void insert(sorted_unique_range_tag, InputIt first, InputIt last)
        {
            if (first != last)
            {
                auto it = find(convert(*first).key());
                if (it != members_.end())
                {
                    for (auto s = first; s != last; ++s)
                    {
                        it = members_.emplace(it, key_type(s->first, get_allocator()), s->second);
                    }
                }
                else
                {
                    for (auto s = first; s != last; ++s)
                    {
                        members_.emplace_back(convert(*s));
                    }
                }
            }
        }

        // insert_or_assign

        template <typename T,typename A=allocator_type>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), std::forward<T>(value));
                inserted = true;
                it = members_.begin() + members_.size() - 1;
            }
            else if ((*it).key() == name)
            {
                (*it).value(Json(std::forward<T>(value)));
                inserted = false; // assigned
            }
            else
            {
                it = members_.emplace(it, key_type(name.begin(),name.end()), std::forward<T>(value));
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        template <typename T,typename A=allocator_type>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), std::forward<T>(value));
                inserted = true;
                it = members_.begin() + members_.size() - 1;
            }
            else if ((*it).key() == name)
            {
                (*it).value(Json(std::forward<T>(value), get_allocator()));
                inserted = false; // assigned
            }
            else
            {
                it = members_.emplace(it, key_type(name.begin(),name.end(), get_allocator()),
                    std::forward<T>(value));
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        // try_emplace

        template <typename A=allocator_type,typename... Args>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), std::forward<Args>(args)...);
                it = members_.begin() + members_.size() - 1;
                inserted = true;
            }
            else if ((*it).key() == name)
            {
                inserted = false;
            }
            else
            {
                it = members_.emplace(it, key_type(name.begin(),name.end()),
                                            std::forward<Args>(args)...);
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        template <typename A=allocator_type,typename... Args>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), std::forward<Args>(args)...);
                it = members_.begin() + members_.size() - 1;
                inserted = true;
            }
            else if ((*it).key() == name)
            {
                inserted = false;
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end(), get_allocator()),
                                            std::forward<Args>(args)...);
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        template <typename A=allocator_type,typename ... Args>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        try_emplace(iterator hint, const string_view_type& name, Args&&... args)
        {
            iterator it = hint;

            if (hint != members_.end() && hint->key() <= name)
            {
                it = std::lower_bound(hint,members_.end(), name, 
                                      Comp());        
            }
            else
            {
                it = std::lower_bound(members_.begin(),members_.end(), name, 
                                      Comp());        
            }

            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), 
                    std::forward<Args>(args)...);
                it = members_.begin() + (members_.size() - 1);
            }
            else if ((*it).key() == name)
            {
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end()),
                                            std::forward<Args>(args)...);
            }

            return it;
        }

        template <typename A=allocator_type,typename ... Args>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        try_emplace(iterator hint, const string_view_type& name, Args&&... args)
        {
            iterator it = hint;
            if (hint != members_.end() && hint->key() <= name)
            {
                it = std::lower_bound(hint,members_.end(), name, 
                                      Comp());        
            }
            else
            {
                it = std::lower_bound(members_.begin(),members_.end(), name, 
                                      Comp());        
            }

            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), 
                    std::forward<Args>(args)...);
                it = members_.begin() + (members_.size() - 1);
            }
            else if ((*it).key() == name)
            {
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end(), get_allocator()),
                                            std::forward<Args>(args)...);
            }
            return it;
        }

        // insert_or_assign

        template <typename T,typename A=allocator_type>
        typename std::enable_if<std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& name, T&& value)
        {
            iterator it;
            if (hint != members_.end() && hint->key() <= name)
            {
                it = std::lower_bound(hint,members_.end(), name, 
                                      [](const key_value_type& a, const string_view_type& k) -> bool {return string_view_type(a.key()).compare(k) < 0;});        
            }
            else
            {
                it = std::lower_bound(members_.begin(),members_.end(), name, 
                                      [](const key_value_type& a, const string_view_type& k) -> bool {return string_view_type(a.key()).compare(k) < 0;});        
            }

            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), std::forward<T>(value));
                it = members_.begin() + (members_.size() - 1);
            }
            else if ((*it).key() == name)
            {
                (*it).value(Json(std::forward<T>(value)));
            }
            else
            {
                it = members_.emplace(it, key_type(name.begin(),name.end()), std::forward<T>(value));
            }
            return it;
        }

        template <typename T,typename A=allocator_type>
        typename std::enable_if<!std::allocator_traits<A>::is_always_equal::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& name, T&& value)
        {
            iterator it;
            if (hint != members_.end() && hint->key() <= name)
            {
                it = std::lower_bound(hint,members_.end(), name, 
                                      Comp());        
            }
            else
            {
                it = std::lower_bound(members_.begin(),members_.end(), name, 
                                      Comp());        
            }

            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), std::forward<T>(value));
                it = members_.begin() + (members_.size() - 1);
            }
            else if ((*it).key() == name)
            {
                (*it).value(Json(std::forward<T>(value),get_allocator()));
            }
            else
            {
                it = members_.emplace(it, key_type(name.begin(),name.end(), get_allocator()),
                    std::forward<T>(value));
            }
            return it;
        }

        // merge

        void merge(const sorted_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                try_emplace((*it).key(),(*it).value());
            }
        }

        void merge(sorted_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = std::lower_bound(members_.begin(),members_.end(), (*it).key(), 
                                            Comp());   
                if (pos == members_.end())
                {
                    members_.emplace_back(*it);
                }
                else if ((*it).key() != pos->key())
                {
                    members_.emplace(pos,*it);
                }
            }
        }

        void merge(iterator hint, const sorted_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = try_emplace(hint, (*it).key(),(*it).value());
            }
        }

        void merge(iterator hint, sorted_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                iterator pos;
                if (hint != members_.end() && hint->key() <= (*it).key())
                {
                    pos = std::lower_bound(hint,members_.end(), (*it).key(), 
                                          Comp());        
                }
                else
                {
                    pos = std::lower_bound(members_.begin(),members_.end(), (*it).key(), 
                                          Comp());        
                }
                if (pos == members_.end())
                {
                    members_.emplace_back(*it);
                    hint = members_.begin() + (members_.size() - 1);
                }
                else if ((*it).key() != pos->key())
                {
                    hint = members_.emplace(pos,*it);
                }
            }
        }

        // merge_or_update

        void merge_or_update(const sorted_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                insert_or_assign((*it).key(),(*it).value());
            }
        }

        void merge_or_update(sorted_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = std::lower_bound(members_.begin(),members_.end(), (*it).key(), 
                                            Comp());   
                if (pos == members_.end())
                {
                    members_.emplace_back(*it);
                }
                else 
                {
                    pos->value((*it).value());
                }
            }
        }

        void merge_or_update(iterator hint, const sorted_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = insert_or_assign(hint, (*it).key(),(*it).value());
            }
        }

        void merge_or_update(iterator hint, sorted_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                iterator pos;
                if (hint != members_.end() && hint->key() <= (*it).key())
                {
                    pos = std::lower_bound(hint,members_.end(), (*it).key(), 
                                          Comp());        
                }
                else
                {
                    pos = std::lower_bound(members_.begin(),members_.end(), (*it).key(), 
                                          Comp());        
                }
                if (pos == members_.end())
                {
                    members_.emplace_back(*it);
                    hint = members_.begin() + (members_.size() - 1);
                }
                else 
                {
                    pos->value((*it).value());
                    hint = pos;
                }
            }
        }

        bool operator==(const sorted_json_object& rhs) const
        {
            return members_ == rhs.members_;
        }

        bool operator<(const sorted_json_object& rhs) const
        {
            return members_ < rhs.members_;
        }
    private:

        void flatten_and_destroy() noexcept
        {
            if (!members_.empty())
            {
                json_array<Json> temp(get_allocator());

                for (auto& kv : members_)
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

#endif // JSONCONS_JSON_OBJECT_HPP
