// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_OBJECT_HPP
#define JSONCONS_JSON_OBJECT_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstring>
#include <algorithm> // std::sort, std::stable_sort, std::lower_bound, std::unique
#include <utility>
#include <initializer_list>
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <utility> // std::move
#include <cassert> // assert
#include <type_traits> // std::enable_if
#include <jsoncons/json_exception.hpp>
#include <jsoncons/allocator_holder.hpp>
#include <jsoncons/json_array.hpp>

namespace jsoncons {

    struct sorted_unique_range_tag
    {
        explicit sorted_unique_range_tag() = default; 
    };

    // key_value

    template <class KeyT, class ValueT>
    class key_value
    {
    public:
        using key_type = KeyT;
        using value_type = ValueT;
        using allocator_type = typename ValueT::allocator_type;
        using string_view_type = typename value_type::string_view_type;
    private:

        key_type key_;
        value_type value_;
    public:

        key_value()
        {
        }

        key_value(const key_type& name, const value_type& val)
            : key_(name), value_(val)
        {
        }

        key_value(const string_view_type& name)
            : key_(name)
        {
        }

        //template <class T>
        //key_value(key_type&& name, T&& val)
        //    : key_(std::forward<key_type>(name)), 
        //      value_(std::forward<T>(val))
        //{
        //}

        template <typename... Args>
        key_value(const key_type& name,  Args&& ... args)
            : key_(name), value_(std::forward<Args>(args)...)
        {
        }

        template <typename... Args>
        key_value(key_type&& name,  Args&& ... args) noexcept
            : key_(std::forward<key_type>(name)), value_(std::forward<Args>(args)...)
        {
        }

        key_value(const key_value& member)
            : key_(member.key_), value_(member.value_)
        {
        }

        key_value(key_value&& member) noexcept
            : key_(std::move(member.key_)), value_(std::move(member.value_))
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

        template <class T>
        void value(T&& value)
        {
            value_ = std::forward<T>(value);
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
    #if !defined(JSONCONS_NO_DEPRECATED)
        JSONCONS_DEPRECATED_MSG("Instead, use key()")
        const key_type& name() const
        {
            return key_;
        }
    #endif

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
    };

    template <class KeyT, class ValueT>
    struct get_key_value
    {
        using key_value_type = key_value<KeyT,ValueT>;

        template <class T1,class T2>
        key_value_type operator()(const std::pair<T1,T2>& p)
        {
            return key_value_type(p.first,p.second);
        }
        template <class T1,class T2>
        key_value_type operator()(std::pair<T1,T2>&& p)
        {
            return key_value_type(std::forward<T1>(p.first),std::forward<T2>(p.second));
        }
        template <class T1,class T2>
        const key_value_type& operator()(const key_value<T1,T2>& p)
        {
            return p;
        }
        template <class T1,class T2>
        key_value_type operator()(key_value<T1,T2>&& p)
        {
            return std::move(p);
        }
    };

    struct sort_key_order
    {
        explicit sort_key_order() = default; 
    };

    struct preserve_key_order
    {
        explicit preserve_key_order() = default; 
    };


    // Sort keys
    template <class KeyT,class Json,template<typename,typename> class SequenceContainer = std::vector>
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

        sorted_json_object(const sorted_json_object& val)
            : allocator_holder<allocator_type>(val.get_allocator()),
              members_(val.members_)
        {
        }

        sorted_json_object(sorted_json_object&& val)
            : allocator_holder<allocator_type>(val.get_allocator()), 
              members_(std::move(val.members_))
        {
        }

        sorted_json_object& operator=(const sorted_json_object& val)
        {
            allocator_holder<allocator_type>::operator=(val.get_allocator());
            members_ = val.members_;
            return *this;
        }

        sorted_json_object& operator=(sorted_json_object&& val)
        {
            val.swap(*this);
            return *this;
        }

        sorted_json_object(const sorted_json_object& val, const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              members_(val.members_,key_value_allocator_type(alloc))
        {
        }

        sorted_json_object(sorted_json_object&& val,const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), members_(std::move(val.members_),key_value_allocator_type(alloc))
        {
        }

        template<class InputIt>
        sorted_json_object(InputIt first, InputIt last)
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(get_key_value<KeyT,Json>()(*s));
            }
            std::stable_sort(members_.begin(),members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto it = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(it, members_.end());
        }

        template<class InputIt>
        sorted_json_object(InputIt first, InputIt last, 
                    const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc))
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(get_key_value<KeyT,Json>()(*s));
            }
            std::stable_sort(members_.begin(),members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto it = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(it, members_.end());
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

        void swap(sorted_json_object& val) noexcept
        {
            members_.swap(val.members_);
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

        void erase(const_iterator pos) 
        {
    #if defined(JSONCONS_NO_ERASE_TAKING_CONST_ITERATOR)
            iterator it = members_.begin() + (pos - members_.begin());
            members_.erase(it);
    #else
            members_.erase(pos);
    #endif
        }

        void erase(const_iterator first, const_iterator last) 
        {
    #if defined(JSONCONS_NO_ERASE_TAKING_CONST_ITERATOR)
            iterator it1 = members_.begin() + (first - members_.begin());
            iterator it2 = members_.begin() + (last - members_.begin());
            members_.erase(it1,it2);
    #else
            members_.erase(first,last);
    #endif
        }

        void erase(const string_view_type& name) 
        {
            auto it = find(name);
            if (it != members_.end())
            {
                members_.erase(it);
            }
        }

        template<class InputIt, class Convert>
        void insert(InputIt first, InputIt last, Convert convert)
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(members_.size() + count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(convert(*s));
            }
            std::stable_sort(members_.begin(),members_.end(),
                             [](const key_value_type& a, const key_value_type& b) -> bool {return a.key().compare(b.key()) < 0;});
            auto it = std::unique(members_.begin(), members_.end(),
                                  [](const key_value_type& a, const key_value_type& b) -> bool { return !(a.key().compare(b.key()));});
            members_.erase(it, members_.end());
        }

        template<class InputIt, class Convert>
        void insert(sorted_unique_range_tag, InputIt first, InputIt last, Convert convert)
        {
            if (first != last)
            {
                std::size_t count = std::distance(first,last);
                members_.reserve(members_.size() + count);

                auto it = find(convert(*first).key());
                if (it != members_.end())
                {
                    for (auto s = first; s != last; ++s)
                    {
                        it = members_.emplace(it, convert(*s));
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

        template <class T, class A=allocator_type>
        typename std::enable_if<type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), 
                                            std::forward<T>(value));
                inserted = true;
                it = members_.begin() + members_.size() - 1;
            }
            else if (it->key() == name)
            {
                it->value(Json(std::forward<T>(value)));
                inserted = false; // assigned
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end()),
                                            std::forward<T>(value));
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        template <class T, class A=allocator_type>
        typename std::enable_if<!type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), 
                                            std::forward<T>(value),get_allocator());
                inserted = true;
                it = members_.begin() + members_.size() - 1;
            }
            else if (it->key() == name)
            {
                it->value(Json(std::forward<T>(value), get_allocator()));
                inserted = false; // assigned
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end(), get_allocator()),
                                            std::forward<T>(value),get_allocator());
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        // try_emplace

        template <class A=allocator_type, class... Args>
        typename std::enable_if<type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end()), 
                                            std::forward<Args>(args)...);
                it = members_.begin() + members_.size() - 1;
                inserted = true;
            }
            else if (it->key() == name)
            {
                inserted = false;
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end()),
                                            std::forward<Args>(args)...);
                inserted = true;
            }
            return std::make_pair(it,inserted);
        }

        template <class A=allocator_type, class... Args>
        typename std::enable_if<!type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            bool inserted;
            auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                       Comp());        
            if (it == members_.end())
            {
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), 
                                            std::forward<Args>(args)...);
                it = members_.begin() + members_.size() - 1;
                inserted = true;
            }
            else if (it->key() == name)
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

        template <class A=allocator_type, class ... Args>
        typename std::enable_if<type_traits::is_stateless<A>::value,iterator>::type 
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
            else if (it->key() == name)
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

        template <class A=allocator_type, class ... Args>
        typename std::enable_if<!type_traits::is_stateless<A>::value,iterator>::type 
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
            else if (it->key() == name)
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

        template <class T, class A=allocator_type>
        typename std::enable_if<type_traits::is_stateless<A>::value,iterator>::type 
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
                members_.emplace_back(key_type(name.begin(),name.end()), 
                                            std::forward<T>(value));
                it = members_.begin() + (members_.size() - 1);
            }
            else if (it->key() == name)
            {
                it->value(Json(std::forward<T>(value)));
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end()),
                                            std::forward<T>(value));
            }
            return it;
        }

        template <class T, class A=allocator_type>
        typename std::enable_if<!type_traits::is_stateless<A>::value,iterator>::type 
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
                members_.emplace_back(key_type(name.begin(),name.end(), get_allocator()), 
                                            std::forward<T>(value),get_allocator());
                it = members_.begin() + (members_.size() - 1);
            }
            else if (it->key() == name)
            {
                it->value(Json(std::forward<T>(value),get_allocator()));
            }
            else
            {
                it = members_.emplace(it,
                                            key_type(name.begin(),name.end(), get_allocator()),
                                            std::forward<T>(value),get_allocator());
            }
            return it;
        }

        // merge

        void merge(const sorted_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                try_emplace(it->key(),it->value());
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
                if (pos == members_.end() )
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
                if (pos == members_.end() )
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
                if (pos == members_.end() )
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
                if (pos == members_.end() )
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
                        case json_storage_kind::array_value:
                        case json_storage_kind::object_value:
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

    // Preserve order
    template <class KeyT,class Json,template<typename,typename> class SequenceContainer = std::vector>
    class order_preserving_json_object : public allocator_holder<typename Json::allocator_type>
    {
    public:
        using allocator_type = typename Json::allocator_type;
        using char_type = typename Json::char_type;
        using key_type = KeyT;
        //using mapped_type = Json;
        using string_view_type = typename Json::string_view_type;
        using key_value_type = key_value<KeyT,Json>;
        //using implementation_policy = typename Json::implementation_policy;
    private:

        using key_value_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_type>;                       
        using key_value_container_type = SequenceContainer<key_value_type,key_value_allocator_type>;
        typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::size_t> index_allocator_type;
        //using index_container_type = typename implementation_policy::template sequence_container_type<std::size_t,index_allocator_type>;
        using index_container_type = SequenceContainer<std::size_t,index_allocator_type>;

        key_value_container_type members_;
        index_container_type index_;

        struct Comp
        {
            const key_value_container_type& members_;

            Comp(const key_value_container_type& members_)
                : members_(members_)
            {
            }

            bool operator() (std::size_t i, string_view_type k) const { return members_.at(i).key() < k; }
            bool operator() (string_view_type k, std::size_t i) const { return k < members_.at(i).key(); }
        };
    public:
        using iterator = typename key_value_container_type::iterator;
        using const_iterator = typename key_value_container_type::const_iterator;

        using allocator_holder<allocator_type>::get_allocator;

        order_preserving_json_object()
        {
        }
        order_preserving_json_object(const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc)), 
              index_(index_allocator_type(alloc))
        {
        }

        order_preserving_json_object(const order_preserving_json_object& val)
            : allocator_holder<allocator_type>(val.get_allocator()), 
              members_(val.members_),
              index_(val.index_)
        {
        }

        order_preserving_json_object(order_preserving_json_object&& val)
            : allocator_holder<allocator_type>(val.get_allocator()), 
              members_(std::move(val.members_)),
              index_(std::move(val.index_))
        {
        }

        order_preserving_json_object(const order_preserving_json_object& val, const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              members_(val.members_,key_value_allocator_type(alloc)),
              index_(val.index_,index_allocator_type(alloc))
        {
        }

        order_preserving_json_object(order_preserving_json_object&& val,const allocator_type& alloc) 
            : allocator_holder<allocator_type>(alloc), 
              members_(std::move(val.members_),key_value_allocator_type(alloc)),
              index_(std::move(val.index_),index_allocator_type(alloc))
        {
        }

        template<class InputIt>
        order_preserving_json_object(InputIt first, InputIt last)
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(get_key_value<KeyT,Json>()(*s));
            }

            build_index();
            auto last_unique = std::unique(index_.begin(), index_.end(),
                [&](std::size_t a, std::size_t b) { return !(members_.at(a).key().compare(members_.at(b).key())); });

            if (last_unique != index_.end())
            {
                index_.erase(last_unique, index_.end());
                std::sort(index_.begin(), index_.end());

                auto result = index_.rbegin();
                if (*result != members_.size())
                {
                    members_.erase(members_.begin() + (*result + 1), members_.end());
                }
                for (auto it = index_.rbegin() + 1; it != index_.rend(); ++it, ++result)
                {
                    if (*result - *it > 1)
                    {
                        members_.erase(members_.begin() + (*it + 1), members_.begin() + *result);
                    }
                }
            }
            build_index();
        }

        template<class InputIt>
        order_preserving_json_object(InputIt first, InputIt last, 
                    const allocator_type& alloc)
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc)), 
              index_(index_allocator_type(alloc))
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(get_key_value<KeyT,Json>()(*s));
            }

            build_index();
            auto last_unique = std::unique(index_.begin(), index_.end(),
                [&](std::size_t a, std::size_t b) { return !(members_.at(a).key().compare(members_.at(b).key())); });

            if (last_unique != index_.end())
            {
                index_.erase(last_unique, index_.end());
                std::sort(index_.begin(), index_.end());

                auto result = index_.rbegin();
                if (*result != members_.size())
                {
                    members_.erase(members_.begin() + (*result + 1), members_.end());
                }
                for (auto it = index_.rbegin() + 1; it != index_.rend(); ++it, ++result)
                {
                    if (*result - *it > 1)
                    {
                        members_.erase(members_.begin() + (*it + 1), members_.begin() + *result);
                    }
                }
            }
            build_index();
        }

        order_preserving_json_object(std::initializer_list<std::pair<std::basic_string<char_type>,Json>> init, 
                    const allocator_type& alloc = allocator_type())
            : allocator_holder<allocator_type>(alloc), 
              members_(key_value_allocator_type(alloc)), 
              index_(index_allocator_type(alloc))
        {
            members_.reserve(init.size());
            for (auto& item : init)
            {
                insert_or_assign(item.first, item.second);
            }
        }

        ~order_preserving_json_object() noexcept
        {
            flatten_and_destroy();
        }

        order_preserving_json_object& operator=(order_preserving_json_object&& val)
        {
            val.swap(*this);
            return *this;
        }

        order_preserving_json_object& operator=(const order_preserving_json_object& val)
        {
            allocator_holder<allocator_type>::operator=(val.get_allocator());
            members_ = val.members_;
            index_ = val.index_;
            return *this;
        }

        void swap(order_preserving_json_object& val) noexcept
        {
            members_.swap(val.members_);
        }

        bool empty() const
        {
            return members_.empty();
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

        void clear() 
        {
            members_.clear();
            index_.clear();
        }

        void shrink_to_fit() 
        {
            for (std::size_t i = 0; i < members_.size(); ++i)
            {
                members_[i].shrink_to_fit();
            }
            members_.shrink_to_fit();
            index_.shrink_to_fit();
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
            auto p = std::equal_range(index_.begin(),index_.end(), name, 
                                        Comp(members_));        
            return p.first == p.second ? members_.end() : members_.begin() + *p.first;
        }

        const_iterator find(const string_view_type& name) const noexcept
        {
            auto p = std::equal_range(index_.begin(),index_.end(), name, 
                                        Comp(members_));        
            return p.first == p.second ? members_.end() : members_.begin() + *p.first;
        }

        void erase(const_iterator first, const_iterator last) 
        {
            std::size_t pos1 = first == members_.end() ? members_.size() : first - members_.begin();
            std::size_t pos2 = last == members_.end() ? members_.size() : last - members_.begin();

            if (pos1 < members_.size() && pos2 <= members_.size())
            {
                erase_index_entries(pos1,pos2);

    #if defined(JSONCONS_NO_ERASE_TAKING_CONST_ITERATOR)
                iterator it1 = members_.begin() + (first - members_.begin());
                iterator it2 = members_.begin() + (last - members_.begin());
                members_.erase(it1,it2);
    #else
                members_.erase(first,last);
    #endif
                //build_index();
            }
        }

        void erase(const string_view_type& name) 
        {
            auto pos = find(name);
            if (pos != members_.end())
            {
                std::size_t pos1 = pos - members_.begin();
                std::size_t pos2 = pos1 + 1;

                erase_index_entries(pos1, pos2);
    #if defined(JSONCONS_NO_ERASE_TAKING_CONST_ITERATOR)
                iterator it = members_.begin() + (pos - members_.begin());
                members_.erase(it);
    #else
                members_.erase(pos);
    #endif
            }
        }

        template<class InputIt, class Convert>
        void insert(InputIt first, InputIt last, Convert convert)
        {
            std::size_t count = std::distance(first,last);
            members_.reserve(members_.size() + count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(convert(*s));
            }

            build_index();
            auto last_unique = std::unique(index_.begin(), index_.end(),
                [&](std::size_t a, std::size_t b) { return !(members_.at(a).key().compare(members_.at(b).key())); });

            if (last_unique != index_.end())
            {
                index_.erase(last_unique, index_.end());
                std::sort(index_.begin(), index_.end());

                auto result = index_.rbegin();
                if (*result != members_.size())
                {
                    members_.erase(members_.begin() + (*result + 1), members_.end());
                }
                for (auto it = index_.rbegin() + 1; it != index_.rend(); ++it, ++result)
                {
                    if (*result - *it > 1)
                    {
                        members_.erase(members_.begin() + (*it + 1), members_.begin() + *result);
                    }
                }
            }
            build_index();
        }

        template<class InputIt, class Convert>
        void insert(sorted_unique_range_tag, InputIt first, InputIt last, Convert convert)
        {
            std::size_t count = std::distance(first,last);

            members_.reserve(members_.size() + count);
            for (auto s = first; s != last; ++s)
            {
                members_.emplace_back(convert(*s));
            }

            build_index();
        }

        template <class T, class A=allocator_type>
        typename std::enable_if<type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            auto result = insert_index_entry(name,members_.size());
            if (result.second)
            {
                members_.emplace_back(key_type(name.begin(), name.end()), std::forward<T>(value));
                auto it = members_.begin() + result.first;
                return std::make_pair(it,true);
            }
            else
            {
                auto it = members_.begin() + result.first;
                it->value(Json(std::forward<T>(value)));
                return std::make_pair(it,false);
            }
        }

        template <class T, class A=allocator_type>
        typename std::enable_if<!type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        insert_or_assign(const string_view_type& name, T&& value)
        {
            auto result = insert_index_entry(name,members_.size());
            if (result.second)
            {
                members_.emplace_back(key_type(name.begin(),name.end(),get_allocator()), 
                                      std::forward<T>(value),get_allocator());
                auto it = members_.begin() + result.first;
                return std::make_pair(it,true);
            }
            else
            {
                auto it = members_.begin() + result.first;
                it->value(Json(std::forward<T>(value),get_allocator()));
                return std::make_pair(it,false);
            }
        }

        template <class A=allocator_type, class T>
        typename std::enable_if<type_traits::is_stateless<A>::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& key, T&& value)
        {
            if (hint == members_.end())
            {
                auto result = insert_or_assign(key, std::forward<T>(value));
                return result.first;
            }
            else
            {
                std::size_t pos = hint - members_.begin();
                auto result = insert_index_entry(key,pos);

                if (result.second)
                {
                    auto it = members_.emplace(hint, key_type(key.begin(), key.end()), std::forward<T>(value));
                    return it;
                }
                else
                {
                    auto it = members_.begin() + result.first;
                    it->value(Json(std::forward<T>(value)));
                    return it;
                }
            }
        }

        template <class A=allocator_type, class T>
        typename std::enable_if<!type_traits::is_stateless<A>::value,iterator>::type 
        insert_or_assign(iterator hint, const string_view_type& key, T&& value)
        {
            if (hint == members_.end())
            {
                auto result = insert_or_assign(key, std::forward<T>(value));
                return result.first;
            }
            else
            {
                std::size_t pos = hint - members_.begin();
                auto result = insert_index_entry(key,pos);

                if (result.second)
                {
                    auto it = members_.emplace(hint, 
                                               key_type(key.begin(),key.end(),get_allocator()), 
                                               std::forward<T>(value),get_allocator());
                    return it;
                }
                else
                {
                    auto it = members_.begin() + result.first;
                    it->value(Json(std::forward<T>(value),get_allocator()));
                    return it;
                }
            }
        }

        // merge

        void merge(const order_preserving_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                try_emplace((*it).key(),(*it).value());
            }
        }

        void merge(order_preserving_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = find((*it).key());
                if (pos == members_.end() )
                {
                    try_emplace((*it).key(),std::move((*it).value()));
                }
            }
        }

        void merge(iterator hint, const order_preserving_json_object& source)
        {
            std::size_t pos = hint - members_.begin();
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = try_emplace(hint, (*it).key(),(*it).value());
                std::size_t newpos = hint - members_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - members_.begin();
                }
                else
                {
                    hint = members_.begin() + pos;
                }
            }
        }

        void merge(iterator hint, order_preserving_json_object&& source)
        {
            std::size_t pos = hint - members_.begin();

            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                hint = try_emplace(hint, (*it).key(), std::move((*it).value()));
                std::size_t newpos = hint - members_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - members_.begin();
                }
                else
                {
                    hint = members_.begin() + pos;
                }
            }
        }

        // merge_or_update

        void merge_or_update(const order_preserving_json_object& source)
        {
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                insert_or_assign((*it).key(),(*it).value());
            }
        }

        void merge_or_update(order_preserving_json_object&& source)
        {
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                auto pos = find((*it).key());
                if (pos == members_.end() )
                {
                    insert_or_assign((*it).key(),std::move((*it).value()));
                }
                else
                {
                    pos->value(std::move((*it).value()));
                }
            }
        }

        void merge_or_update(iterator hint, const order_preserving_json_object& source)
        {
            std::size_t pos = hint - members_.begin();
            for (auto it = source.begin(); it != source.end(); ++it)
            {
                hint = insert_or_assign(hint, (*it).key(),(*it).value());
                std::size_t newpos = hint - members_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - members_.begin();
                }
                else
                {
                    hint = members_.begin() + pos;
                }
            }
        }

        void merge_or_update(iterator hint, order_preserving_json_object&& source)
        {
            std::size_t pos = hint - members_.begin();
            auto it = std::make_move_iterator(source.begin());
            auto end = std::make_move_iterator(source.end());
            for (; it != end; ++it)
            {
                hint = insert_or_assign(hint, (*it).key(), std::move((*it).value()));
                std::size_t newpos = hint - members_.begin();
                if (newpos == pos)
                {
                    ++hint;
                    pos = hint - members_.begin();
                }
                else
                {
                    hint = members_.begin() + pos;
                }
            }
        }

        // try_emplace

        template <class A=allocator_type, class... Args>
        typename std::enable_if<type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& name, Args&&... args)
        {
            auto result = insert_index_entry(name,members_.size());
            if (result.second)
            {
                members_.emplace_back(key_type(name.begin(), name.end()), std::forward<Args>(args)...);
                auto it = members_.begin() + result.first;
                return std::make_pair(it,true);
            }
            else
            {
                auto it = members_.begin() + result.first;
                return std::make_pair(it,false);
            }
        }

        template <class A=allocator_type, class... Args>
        typename std::enable_if<!type_traits::is_stateless<A>::value,std::pair<iterator,bool>>::type
        try_emplace(const string_view_type& key, Args&&... args)
        {
            auto result = insert_index_entry(key,members_.size());
            if (result.second)
            {
                members_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                                      std::forward<Args>(args)...);
                auto it = members_.begin() + result.first;
                return std::make_pair(it,true);
            }
            else
            {
                auto it = members_.begin() + result.first;
                return std::make_pair(it,false);
            }
        }
     
        template <class A=allocator_type, class ... Args>
        typename std::enable_if<type_traits::is_stateless<A>::value,iterator>::type
        try_emplace(iterator hint, const string_view_type& key, Args&&... args)
        {
            if (hint == members_.end())
            {
                auto result = try_emplace(key, std::forward<Args>(args)...);
                return result.first;
            }
            else
            {
                std::size_t pos = hint - members_.begin();
                auto result = insert_index_entry(key, pos);

                if (result.second)
                {
                    auto it = members_.emplace(hint, key_type(key.begin(), key.end()), std::forward<Args>(args)...);
                    return it;
                }
                else
                {
                    auto it = members_.begin() + result.first;
                    return it;
                }
            }
        }

        template <class A=allocator_type, class ... Args>
        typename std::enable_if<!type_traits::is_stateless<A>::value,iterator>::type
        try_emplace(iterator hint, const string_view_type& key, Args&&... args)
        {
            if (hint == members_.end())
            {
                auto result = try_emplace(key, std::forward<Args>(args)...);
                return result.first;
            }
            else
            {
                std::size_t pos = hint - members_.begin();
                auto result = insert_index_entry(key, pos);

                if (result.second)
                {
                    auto it = members_.emplace(hint, 
                                               key_type(key.begin(),key.end(), get_allocator()), 
                                               std::forward<Args>(args)...);
                    return it;
                }
                else
                {
                    auto it = members_.begin() + result.first;
                    return it;
                }
            }
        }

        bool operator==(const order_preserving_json_object& rhs) const
        {
            return members_ == rhs.members_;
        }
     
        bool operator<(const order_preserving_json_object& rhs) const
        {
            return members_ < rhs.members_;
        }
    private:

        void flatten_and_destroy() noexcept
        {
            if (!members_.empty())
            {
                json_array<Json> temp(get_allocator());

                for (auto&& kv : members_)
                {
                    if (kv.value().size() > 0)
                    {
                        temp.emplace_back(std::move(kv.value()));
                    }
                }
            }
        }

        std::pair<std::size_t,bool> insert_index_entry(const string_view_type& key, std::size_t pos)
        {
            JSONCONS_ASSERT(pos <= index_.size());

            auto it = std::lower_bound(index_.begin(),index_.end(), key, 
                                        Comp(members_));        

            if (it == index_.end())
            {
                std::size_t count = index_.size() - pos;
                for (std::size_t i = 0; count > 0 && i < index_.size(); ++i)
                {
                    if (index_[i] >= pos)
                    {
                        ++index_[i];
                        --count;
                    }
                }
                index_.push_back(pos);
                return std::make_pair(pos,true);
            }
            else if (members_.at(*it).key() != key)
            {
                std::size_t count = index_.size() - pos;
                for (std::size_t i = 0; count > 0 && i < index_.size(); ++i)
                {
                    if (index_[i] >= pos)
                    {
                        ++index_[i];
                        --count;
                    }
                }
                auto it2 = index_.insert(it, pos);
                return std::make_pair(*it2,true);
            }
            else
            {
                return std::make_pair(*it,false);
            }
        }

        void erase_index_entries(std::size_t pos1, std::size_t pos2)
        {
            JSONCONS_ASSERT(pos1 <= pos2);
            JSONCONS_ASSERT(pos2 <= index_.size());

            const size_t offset = pos2 - pos1;
            const size_t n = index_.size() - offset;
            for (std::size_t i = 0; i < index_.size(); ++i)
            {
                if (offset == index_.size())
                {
                    index_.erase(index_.begin()+i,index_.end());
                    i += offset;
                }
                else if (index_[i] >= pos1 && index_[i] < pos2)
                {
                    index_.erase(index_.begin()+i);
                }
            }
            for (std::size_t i = 0; i < index_.size(); ++i)
            {
                if (index_[i] >= pos2)
                {
                    index_[i] -= offset;
                }
            }
            JSONCONS_ASSERT(index_.size() == n);
        }

        void build_index()
        {
            index_.clear();
            index_.reserve(members_.size());
            for (std::size_t i = 0; i < members_.size(); ++i)
            {
                index_.push_back(i);
            }
            std::stable_sort(index_.begin(),index_.end(),
                             [&](std::size_t a, std::size_t b) -> bool {return members_.at(a).key().compare(members_.at(b).key()) < 0;});
        }
    };

} // namespace jsoncons

#endif
