// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_STRUCTURES_HPP
#define JSONCONS_JSON_STRUCTURES_HPP

#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_type_traits.hpp>

namespace jsoncons {

// json_array

template <class Json>
class Json_array_base_
{
public:
    typedef typename Json::allocator_type allocator_type;

public:
    Json_array_base_()
        : self_allocator_()
    {
    }
    Json_array_base_(const allocator_type& allocator)
        : self_allocator_(allocator)
    {
    }

    allocator_type get_allocator() const
    {
        return self_allocator_;
    }

    allocator_type self_allocator_;
};

// json_array

template <class Json>
class json_array: public Json_array_base_<Json>
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef Json value_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> val_allocator_type;

    typedef typename Json::array_storage_type array_storage_type;

    typedef typename array_storage_type::iterator iterator;
    typedef typename array_storage_type::const_iterator const_iterator;

    typedef typename std::iterator_traits<iterator>::reference reference;
    typedef typename std::iterator_traits<const_iterator>::reference const_reference;

    using Json_array_base_<Json>::get_allocator;

    json_array()
        : Json_array_base_<Json>(), 
          elements_()
    {
    }

    explicit json_array(const allocator_type& allocator)
        : Json_array_base_<Json>(allocator), 
          elements_(val_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const allocator_type& allocator = allocator_type())
        : Json_array_base_<Json>(allocator), 
          elements_(n,Json(),val_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const Json& value, 
                        const allocator_type& allocator = allocator_type())
        : Json_array_base_<Json>(allocator), 
          elements_(n,value,val_allocator_type(allocator))
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end, const allocator_type& allocator = allocator_type())
        : Json_array_base_<Json>(allocator), 
          elements_(begin,end,val_allocator_type(allocator))
    {
    }
    json_array(const json_array& val)
        : Json_array_base_<Json>(val.get_allocator()),
          elements_(val.elements_)
    {
    }
    json_array(const json_array& val, const allocator_type& allocator)
        : Json_array_base_<Json>(allocator), 
          elements_(val.elements_,val_allocator_type(allocator))
    {
    }

    json_array(json_array&& val) JSONCONS_NOEXCEPT
        : Json_array_base_<Json>(val.get_allocator()), 
          elements_(std::move(val.elements_))
    {
    }
    json_array(json_array&& val, const allocator_type& allocator)
        : Json_array_base_<Json>(allocator), 
          elements_(std::move(val.elements_),val_allocator_type(allocator))
    {
    }

    json_array(std::initializer_list<Json> init)
        : Json_array_base_<Json>(), 
          elements_(std::move(init))
    {
    }

    json_array(std::initializer_list<Json> init, 
               const allocator_type& allocator)
        : Json_array_base_<Json>(allocator), 
          elements_(std::move(init),val_allocator_type(allocator))
    {
    }
    ~json_array()
    {
    }

    void swap(json_array<Json>& val)
    {
        elements_.swap(val.elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            elements_[i].shrink_to_fit();
        }
        elements_.shrink_to_fit();
    }

    void reserve(size_t n) {elements_.reserve(n);}

    void resize(size_t n) {elements_.resize(n);}

    void resize(size_t n, const Json& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    void erase(const_iterator pos) 
    {
        elements_.erase(pos);
    }

    void erase(const_iterator first, const_iterator last) 
    {
        elements_.erase(first,last);
    }

    Json& operator[](size_t i) {return elements_[i];}

    const Json& operator[](size_t i) const {return elements_[i];}

    // push_back

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,void>::type 
    push_back(T&& value)
    {
        elements_.emplace_back(std::forward<T>(value));
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,void>::type 
    push_back(T&& value)
    {
        elements_.emplace_back(std::forward<T>(value),get_allocator());
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    insert(const_iterator pos, T&& value)
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, std::forward<T>(value));
#else
        return elements_.emplace(pos, std::forward<T>(value));
#endif
    }
    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    insert(const_iterator pos, T&& value)
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, std::forward<T>(value), get_allocator());
#else
        return elements_.emplace(pos, std::forward<T>(value), get_allocator());
#endif
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.insert(it, first, last);
#else
        return elements_.insert(pos, first, last);
#endif
    }

#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
    template <class A=allocator_type, class... Args>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    emplace(const_iterator pos, Args&&... args)
    {
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, std::forward<Args>(args)...);
    }
#else
    template <class A=allocator_type, class... Args>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    emplace(const_iterator pos, Args&&... args)
    {
        return elements_.emplace(pos, std::forward<Args>(args)...);
    }
#endif
    template <class... Args>
    Json& emplace_back(Args&&... args)
    {
        elements_.emplace_back(std::forward<Args>(args)...);
        return elements_.back();
    }

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<Json>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (size_t i = 0; i < size(); ++i)
        {
            if (elements_[i] != rhs.elements_[i])
            {
                return false;
            }
        }
        return true;
    }
private:
    array_storage_type elements_;

    json_array& operator=(const json_array<Json>&) = delete;
};

// json_object

template <class BidirectionalIt,class BinaryPredicate>
BidirectionalIt last_wins_unique_sequence(BidirectionalIt first, BidirectionalIt last, BinaryPredicate compare)
{
    
    if (first == last)
    {
        return last;
    }

    typedef typename BidirectionalIt::value_type value_type;
    typedef typename BidirectionalIt::pointer pointer;
    std::vector<value_type> dups;
    {
        std::vector<pointer> v(std::distance(first,last));
        auto p = v.begin();
        for (auto it = first; it != last; ++it)
        {
            *p++ = &(*it);
        }
        std::sort(v.begin(), v.end(), [&](pointer a, pointer b){return compare(*a,*b)<0;});
        auto it = v.begin();
        auto end = v.end();
        for (auto begin = it+1; begin != end; ++it, ++begin)
        {
            if (compare(*(*it),*(*begin)) == 0)
            {
                dups.push_back(*(*it));
            }
        }
    }
    if (dups.size() == 0)
    {
        return last;
    }
    
    auto it = last;
    for (auto p = first; p != last && p != it; )
    {
        bool no_dup = true;
        if (dups.size() > 0)
        {
            for (auto q = dups.begin(); no_dup && q != dups.end();)
            {
                if (compare(*p,*q) == 0)
                {
                    dups.erase(q);
                    no_dup = false;
                }
                else
                {
                    ++q;
                }
            }
        }
        if (!no_dup)
        {
            --it;
            for (auto r = p; r != it; ++r)
            {
                *r = std::move(*(r+1));
            }
        }
        else
        {
            ++p;
        }
    }
    
    return it;
}

template <class KeyT, class ValueT>
class key_value_pair
{
public:
    typedef KeyT key_storage_type;
    typedef typename KeyT::value_type char_type;
    typedef typename KeyT::allocator_type allocator_type;
    typedef typename ValueT::string_view_type string_view_type;

    key_value_pair()
    {
    }

    key_value_pair(const key_storage_type& name, const ValueT& val)
        : key_(name), value_(val)
    {
    }

    template <class T>
    key_value_pair(key_storage_type&& name, T&& val)
        : key_(std::forward<key_storage_type>(name)), 
          value_(std::forward<T>(val))
    {
    }

    template <class T>
    key_value_pair(key_storage_type&& name, 
                   T&& val, 
                   const allocator_type& allocator)
        : key_(std::forward<key_storage_type>(name)), value_(std::forward<T>(val), allocator)
    {
    }

    key_value_pair(const key_value_pair& member)
        : key_(member.key_), value_(member.value_)
    {
    }

    key_value_pair(key_value_pair&& member)
        : key_(std::move(member.key_)), value_(std::move(member.value_))
    {
    }

    string_view_type key() const
    {
        return string_view_type(key_.data(),key_.size());
    }

    ValueT& value()
    {
        return value_;
    }

    const ValueT& value() const
    {
        return value_;
    }

    template <class T>
    void value(T&& value)
    {
        value_ = std::forward<T>(value);
    }

    void swap(key_value_pair& member)
    {
        key_.swap(member.key_);
        value_.swap(member.value_);
    }

    key_value_pair& operator=(const key_value_pair& member)
    {
        if (this != & member)
        {
            key_ = member.key_;
            value_ = member.value_;
        }
        return *this;
    }

    key_value_pair& operator=(key_value_pair&& member)
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
    const key_storage_type& name() const
    {
        return key_;
    }
#endif
private:
    key_storage_type key_;
    ValueT value_;
};

template <class KeyT,class Json>
class Json_object_
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::char_allocator_type char_allocator_type;
    typedef KeyT key_storage_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value_pair<KeyT,Json> value_type;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> kvp_allocator_type;
    typedef typename Json::object_storage_type object_storage_type;

    typedef typename object_storage_type::iterator iterator;
    typedef typename object_storage_type::const_iterator const_iterator;

protected:
    allocator_type self_allocator_;
    object_storage_type members_;
public:
    Json_object_()
        : self_allocator_(), members_()
    {
    }
    Json_object_(const allocator_type& allocator)
        : self_allocator_(allocator), 
          members_(kvp_allocator_type(allocator))
    {
    }

    Json_object_(const Json_object_& val)
        : self_allocator_(val.get_allocator()), members_(val.members_)
    {
    }

    Json_object_(Json_object_&& val)
        : self_allocator_(val.get_allocator()), 
          members_(std::move(val.members_))
    {
    }

    Json_object_(const Json_object_& val, const allocator_type& allocator) :
        self_allocator_(allocator), 
        members_(val.members_,kvp_allocator_type(allocator))
    {
    }

    Json_object_(Json_object_&& val,const allocator_type& allocator) :
        self_allocator_(allocator), members_(std::move(val.members_),kvp_allocator_type(allocator))
    {
    }

    void swap(Json_object_& val)
    {
        members_.swap(val.members_);
    }

    allocator_type get_allocator() const
    {
        return this->self_allocator_;
    }
};

// json_object

template <class KeyT,class Json,bool PreserveOrder>
class json_object
{
};

// Do not preserve order
template <class KeyT,class Json>
class json_object<KeyT,Json,false> final : public Json_object_<KeyT,Json>
{
public:
    using typename Json_object_<KeyT,Json>::allocator_type;
    using typename Json_object_<KeyT,Json>::char_type;
    using typename Json_object_<KeyT,Json>::char_allocator_type;
    using typename Json_object_<KeyT,Json>::key_storage_type;
    using typename Json_object_<KeyT,Json>::string_view_type;
    using typename Json_object_<KeyT,Json>::value_type;
    using typename Json_object_<KeyT,Json>::kvp_allocator_type;
    using typename Json_object_<KeyT,Json>::object_storage_type;
    using typename Json_object_<KeyT,Json>::iterator;
    using typename Json_object_<KeyT,Json>::const_iterator;
    using Json_object_<KeyT,Json>::get_allocator;

    json_object()
        : Json_object_<KeyT,Json>()
    {
    }
    json_object(const allocator_type& allocator)
        : Json_object_<KeyT,Json>(allocator)
    {
    }

    json_object(const json_object& val)
        : Json_object_<KeyT,Json>(val)
    {
    }

    json_object(json_object&& val)
        : Json_object_<KeyT,Json>(std::forward<json_object>(val))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) 
        : Json_object_<KeyT,Json>(val,allocator)
    {
    }

    json_object(json_object&& val,const allocator_type& allocator)
        : Json_object_<KeyT,Json>(std::forward<json_object>(val),allocator)
    {
    }

    json_object(std::initializer_list<std::pair<string_view_type,Json>> init)
        : Json_object_<KeyT,Json>()
    {
        this->members_.reserve(init.size());
        for (auto& item : init)
        {
            insert_or_assign(item.first, std::move(item.second));
        }
    }

    json_object(std::initializer_list<std::pair<string_view_type,Json>> init, 
                const allocator_type& allocator)
        : Json_object_<KeyT,Json>(allocator)
    {
        this->members_.reserve(init.size());
        for (auto& item : init)
        {
            insert_or_assign(item.first, std::move(item.second), allocator);
        }
    }

    void swap(json_object& val)
    {
        Json_object_<KeyT,Json>::swap(val);
    }

    iterator begin()
    {
        return this->members_.begin();
    }

    iterator end()
    {
        return this->members_.end();
    }

    const_iterator begin() const
    {
        return this->members_.begin();
    }

    const_iterator end() const
    {
        return this->members_.end();
    }

    size_t size() const {return this->members_.size();}

    size_t capacity() const {return this->members_.capacity();}

    void clear() {this->members_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < this->members_.size(); ++i)
        {
            this->members_[i].shrink_to_fit();
        }
        this->members_.shrink_to_fit();
    }

    void reserve(size_t n) {this->members_.reserve(n);}

    Json& at(size_t i) 
    {
        if (i >= this->members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return this->members_[i].value();
    }

    const Json& at(size_t i) const 
    {
        if (i >= this->members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return this->members_[i].value();
    }

    iterator find(const string_view_type& name)
    {
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        auto result = (it != this->members_.end() && it->key() == name) ? it : this->members_.end();
        return result;
    }

    const_iterator find(const string_view_type& name) const
    {
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), 
                                   name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});
        auto result = (it != this->members_.end() && it->key() == name) ? it : this->members_.end();
        return result;
    }

    void erase(const_iterator pos) 
    {
        this->members_.erase(pos);
    }

    void erase(const_iterator first, const_iterator last) 
    {
        this->members_.erase(first,last);
    }

    void erase(const string_view_type& name) 
    {
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it != this->members_.end() && it->key() == name)
        {
            this->members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        this->members_.reserve(this->members_.size() + count);
        for (auto s = first; s != last; ++s)
        {
            this->members_.emplace_back(pred(*s));
        }
        std::stable_sort(this->members_.begin(),this->members_.end(),
                         [](const value_type& a, const value_type& b){return a.key().compare(b.key()) < 0;});
        auto it = std::unique(this->members_.rbegin(), this->members_.rend(),
                              [](const value_type& a, const value_type& b){ return !(a.key().compare(b.key()));});
        this->members_.erase(this->members_.begin(),it.base());
    }

    // merge

    void merge(const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            try_emplace(it->key(),it->value());
        }
    }

    void merge(json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::lower_bound(this->members_.begin(),this->members_.end(), it->key(), 
                                        [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});   
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
            }
            else if (it->key() != pos->key())
            {
                this->members_.emplace(pos,*it);
            }
        }
    }

    void merge(iterator hint, const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            hint = try_emplace(hint, it->key(),it->value());
        }
    }

    void merge(iterator hint, json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            iterator pos;
            if (hint != this->members_.end() && hint->key() <= it->key())
            {
                pos = std::lower_bound(hint,this->members_.end(), it->key(), 
                                      [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            else
            {
                pos = std::lower_bound(this->members_.begin(),this->members_.end(), it->key(), 
                                      [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
                hint = this->members_.begin() + (this->members_.size() - 1);
            }
            else if (it->key() != pos->key())
            {
                hint = this->members_.emplace(pos,*it);
            }
        }
    }

    // merge_or_update

    void merge_or_update(const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            insert_or_assign(it->key(),it->value());
        }
    }

    void merge_or_update(json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::lower_bound(this->members_.begin(),this->members_.end(), it->key(), 
                                        [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});   
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
            }
            else 
            {
                pos->value(it->value());
            }
        }
    }

    void merge_or_update(iterator hint, const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            hint = insert_or_assign(hint, it->key(),it->value());
        }
    }

    void merge_or_update(iterator hint, json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            iterator pos;
            if (hint != this->members_.end() && hint->key() <= it->key())
            {
                pos = std::lower_bound(hint,this->members_.end(), it->key(), 
                                      [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            else
            {
                pos = std::lower_bound(this->members_.begin(),this->members_.end(), it->key(), 
                                      [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
                hint = this->members_.begin() + (this->members_.size() - 1);
            }
            else 
            {
                pos->value(it->value());
                hint = pos;
            }
        }
    }

    // insert_or_assign

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end()), 
                                        std::forward<T>(value));
            inserted = true;
            it = this->members_.begin() + this->members_.size() - 1;
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T>(value)));
            inserted = false; // assigned
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end()),
                                        std::forward<T>(value));
            inserted = true;
        }
        return std::make_pair(it,inserted);
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end(), get_allocator()), 
                                        std::forward<T>(value),get_allocator());
            inserted = true;
            it = this->members_.begin() + this->members_.size() - 1;
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T>(value), get_allocator()));
            inserted = false; // assigned
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end(), get_allocator()),
                                        std::forward<T>(value),get_allocator());
            inserted = true;
        }
        return std::make_pair(it,inserted);
    }

    // try_emplace

    template <class A=allocator_type, class... Args>
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& name, Args&&... args)
    {
        bool inserted;
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;
        }
        else if (it->key() == name)
        {
            inserted = false;
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end()),
                                        std::forward<Args>(args)...);
            inserted = true;
        }
        return std::make_pair(it,inserted);
    }

    template <class A=allocator_type, class... Args>
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& name, Args&&... args)
    {
        bool inserted;
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end(), get_allocator()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;
        }
        else if (it->key() == name)
        {
            inserted = false;
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end(), get_allocator()),
                                        std::forward<Args>(args)...);
            inserted = true;
        }
        return std::make_pair(it,inserted);
    }

    template <class A=allocator_type, class ... Args>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    try_emplace(iterator hint, const string_view_type& name, Args&&... args)
    {
        iterator it;
        if (hint != this->members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == name)
        {
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end()),
                                        std::forward<Args>(args)...);
        }
        return it;
    }

    template <class A=allocator_type, class ... Args>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    try_emplace(iterator hint, const string_view_type& name, Args&&... args)
    {
        iterator it;
        if (hint != this->members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end(), get_allocator()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == name)
        {
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end(), get_allocator()),
                                        std::forward<Args>(args)...);
        }
        return it;
    }

    // set_

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,void>::type 
    set_(key_storage_type&& name, T&& value)
    {
        string_view_type s(name.data(), name.size());
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), s, 
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(name), 
                                  std::forward<T>(value));
        }
        else if (string_view_type(it->key().data(),it->key().length()) == s)
        {
            it->value(Json(std::forward<T>(value)));
        }
        else
        {
            this->members_.emplace(it,
                             std::forward<key_storage_type>(name),
                             std::forward<T>(value));
        }
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,void>::type 
    set_(key_storage_type&& name, T&& value)
    {
        string_view_type s(name.data(), name.size());
        auto it = std::lower_bound(this->members_.begin(),this->members_.end(), s,
                                   [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(name), 
                                  std::forward<T>(value),get_allocator() );
        }
        else if (string_view_type(it->key().data(), it->key().length()) == s)
        {
            it->value(Json(std::forward<T>(value),get_allocator() ));
        }
        else
        {
            this->members_.emplace(it,
                             std::forward<key_storage_type>(name),
                             std::forward<T>(value),get_allocator() );
        }
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& name, T&& value)
    {
        iterator it;
        if (hint != this->members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end()), 
                                        std::forward<T>(value));
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T>(value)));
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end()),
                                        std::forward<T>(value));
        }
        return it;
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& name, T&& value)
    {
        iterator it;
        if (hint != this->members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), name, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end(), get_allocator()), 
                                        std::forward<T>(value),get_allocator());
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T>(value),get_allocator()));
        }
        else
        {
            it = this->members_.emplace(it,
                                        key_storage_type(name.begin(),name.end(), get_allocator()),
                                        std::forward<T>(value),get_allocator());
        }
        return it;
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    set_(iterator hint, key_storage_type&& name, T&& value)
    {
        string_view_type s(name.data(), name.size());
        iterator it;
        if (hint != this->members_.end() && hint->key() <= s)
        {
            it = std::lower_bound(hint,this->members_.end(), s, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), s, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(name), 
                                  std::forward<T>(value));
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (string_view_type(it->key().data(), it->key().length()) == s)
        {
            it->value(Json(std::forward<T>(value)));
        }
        else
        {
            it = this->members_.emplace(it,
                                  std::forward<key_storage_type>(name),
                                  std::forward<T>(value));
        }
        return it;
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    set_(iterator hint, key_storage_type&& name, T&& value)
    {
        string_view_type s(name.data(), name.size());
        iterator it;
        if (hint != this->members_.end() && hint->key() <= s)
        {
            it = std::lower_bound(hint,this->members_.end(), s, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(this->members_.begin(),this->members_.end(), s, 
                                  [](const value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(name), 
                                  std::forward<T>(value),get_allocator() );
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (string_view_type(it->key().data(), it->key().length()) == s)
        {
            it->value(Json(std::forward<T>(value),get_allocator() ));
        }
        else
        {
            it = this->members_.emplace(it,
                                  std::forward<key_storage_type>(name),
                                  std::forward<T>(value),get_allocator() );
        }
        return it;
    }

    bool operator==(const json_object& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = this->members_.begin(); it != this->members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.begin(), rhs.end(), *it, 
                                           [](const value_type& a, const value_type& b){return a.key().compare(b.key()) < 0;});
            if (rhs_it == rhs.end() || rhs_it->key() != it->key() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }
private:
    json_object& operator=(const json_object&) = delete;
};

// Preserve order
template <class KeyT,class Json>
class json_object<KeyT,Json,true> final : public Json_object_<KeyT,Json>
{
public:
    using typename Json_object_<KeyT,Json>::allocator_type;
    using typename Json_object_<KeyT,Json>::char_type;
    using typename Json_object_<KeyT,Json>::char_allocator_type;
    using typename Json_object_<KeyT,Json>::key_storage_type;
    using typename Json_object_<KeyT,Json>::string_view_type;
    using typename Json_object_<KeyT,Json>::value_type;
    using typename Json_object_<KeyT,Json>::kvp_allocator_type;
    using typename Json_object_<KeyT,Json>::object_storage_type;
    using typename Json_object_<KeyT,Json>::iterator;
    using typename Json_object_<KeyT,Json>::const_iterator;
    using Json_object_<KeyT,Json>::get_allocator;

    json_object()
        : Json_object_<KeyT,Json>()
    {
    }
    json_object(const allocator_type& allocator)
        : Json_object_<KeyT,Json>(allocator)
    {
    }

    json_object(const json_object& val)
        : Json_object_<KeyT,Json>(val)
    {
    }

    json_object(json_object&& val)
        : Json_object_<KeyT,Json>(std::forward<json_object>(val))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) 
        : Json_object_<KeyT,Json>(val,allocator)
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) 
        : Json_object_<KeyT,Json>(std::forward<json_object>(val),allocator)
    {
    }

    json_object(std::initializer_list<typename Json::array> init)
        : Json_object_<KeyT,Json>()
    {
        for (const auto& element : init)
        {
            if (element.size() != 2 || !element[0].is_string())
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Cannot create object from initializer list"));
                break;
            }
        }
        for (auto& element : init)
        {
            insert_or_assign(element[0].as_string_view(), std::move(element[1]));
        }
    }

    json_object(std::initializer_list<typename Json::array> init, 
                const allocator_type& allocator)
        : Json_object_<KeyT,Json>(allocator)
    {
        for (const auto& element : init)
        {
            if (element.size() != 2 || !element[0].is_string())
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Cannot create object from initializer list"));
                break;
            }
        }
        for (auto& element : init)
        {
            insert_or_assign(element[0].as_string_view(), std::move(element[1]));
        }
    }

    void swap(json_object& val)
    {
        Json_object_<KeyT,Json>::swap(val);
    }

    iterator begin()
    {
        return this->members_.begin();
    }

    iterator end()
    {
        return this->members_.end();
    }

    const_iterator begin() const
    {
        return this->members_.begin();
    }

    const_iterator end() const
    {
        return this->members_.end();
    }

    size_t size() const {return this->members_.size();}

    size_t capacity() const {return this->members_.capacity();}

    void clear() {this->members_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < this->members_.size(); ++i)
        {
            this->members_[i].shrink_to_fit();
        }
        this->members_.shrink_to_fit();
    }

    void reserve(size_t n) {this->members_.reserve(n);}

    Json& at(size_t i) 
    {
        if (i >= this->members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return this->members_[i].value();
    }

    const Json& at(size_t i) const 
    {
        if (i >= this->members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return this->members_[i].value();
    }

    iterator find(const string_view_type& name)
    {
        return std::find_if(this->members_.begin(),this->members_.end(), 
                            [name](const value_type& kv){return kv.key() == name;});
    }

    const_iterator find(const string_view_type& name) const
    {
        return std::find_if(this->members_.begin(),this->members_.end(), 
                            [name](const value_type& kv){return kv.key() == name;});
    }

    void erase(const_iterator first, const_iterator last) 
    {
        this->members_.erase(first,last);
    }

    void erase(const string_view_type& name) 
    {
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                              [name](const value_type& kv){return kv.key() == name;});
        if (it != this->members_.end())
        {
            this->members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        this->members_.reserve(this->members_.size() + count);
        for (auto s = first; s != last; ++s)
        {
            this->members_.emplace_back(pred(*s));
        }
        auto it = last_wins_unique_sequence(this->members_.begin(), this->members_.end(),
                              [](const value_type& a, const value_type& b){ return a.key().compare(b.key());});
        this->members_.erase(it,this->members_.end());
    }

    // insert_or_assign

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [name](const value_type& a){return a.key() == name;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end()), 
                                        std::forward<T>(value));
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;
        }
        else
        {
            it->value(Json(std::forward<T>(value)));
            inserted = false; // assigned
        }
        return std::make_pair(it,inserted);
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [name](const value_type& a){return a.key() == name;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(name.begin(),name.end(),get_allocator()), 
                                        std::forward<T>(value),get_allocator());
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;
        }
        else
        {
            it->value(Json(std::forward<T>(value),get_allocator()));
            inserted = false; // assigned
        }
        return std::make_pair(it,inserted);
    }

    template <class A=allocator_type, class T>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& key, T&& value)
    {
        iterator it;
        if (hint == this->members_.end())
        {
            auto result = insert_or_assign(key, std::forward<T>(value));
            it = result.first;
        }
        else
        {
            it = std::find_if(this->members_.begin(),this->members_.end(), 
                              [key](const value_type& a){return a.key() == key;});

            if (it == this->members_.end())
            {
                this->members_.emplace_back(key_storage_type(key.begin(),key.end()), 
                                            std::forward<T>(value));
                it = this->members_.begin() + this->members_.size() - 1;
            }
            else
            {
                it->value(Json(std::forward<T>(value)));
            }
        }
        return it;
    }

    template <class A=allocator_type, class T>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& key, T&& value)
    {
        iterator it;
        if (hint == this->members_.end())
        {
            auto result = insert_or_assign(key, std::forward<T>(value));
            it = result.first;
        }
        else
        {
            it = std::find_if(this->members_.begin(),this->members_.end(), 
                              [key](const value_type& a){return a.key() == key;});

            if (it == this->members_.end())
            {
                this->members_.emplace_back(key_storage_type(key.begin(),key.end(),get_allocator()), 
                                            std::forward<T>(value),get_allocator());
                it = this->members_.begin() + this->members_.size() - 1;
            }
            else
            {
                it->value(Json(std::forward<T>(value),get_allocator()));
            }
        }
        return it;
    }

    // merge

    void merge(const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            try_emplace(it->key(),it->value());
        }
    }

    void merge(json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::find_if(this->members_.begin(),this->members_.end(), 
                                    [it](const value_type& a){return a.key() == it->key();});
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
            }
        }
    }

    void merge(iterator hint, const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            hint = try_emplace(hint, it->key(),it->value());
        }
    }

    void merge(iterator hint, json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::find_if(this->members_.begin(),this->members_.end(), 
                                    [it](const value_type& a){return a.key() == it->key();});
            if (pos == this->members_.end() )
            {
                hint = this->members_.emplace(hint,*it);
            }
        }
    }

    // merge_or_update

    void merge_or_update(const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            insert_or_assign(it->key(),it->value());
        }
    }

    void merge_or_update(json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::find_if(this->members_.begin(),this->members_.end(), 
                                    [it](const value_type& a){return a.key() == it->key();});
            if (pos == this->members_.end() )
            {
                this->members_.emplace_back(*it);
            }
            else
            {
                pos->value(it->value());
            }
        }
    }

    void merge_or_update(iterator hint, const json_object& source)
    {
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            hint = insert_or_assign(hint, it->key(),it->value());
        }
    }

    void merge_or_update(iterator hint, json_object&& source)
    {
        auto it = std::make_move_iterator(source.begin());
        auto end = std::make_move_iterator(source.end());
        for (; it != end; ++it)
        {
            auto pos = std::find_if(this->members_.begin(),this->members_.end(), 
                                    [it](const value_type& a){return a.key() == it->key();});
            if (pos == this->members_.end() )
            {
                hint = this->members_.emplace(hint,*it);
            }
            else
            {
                pos->value(it->value());
                hint = pos;
            }
        }
    }

    // try_emplace
    template <class A=allocator_type, class... Args>
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& key, Args&&... args)
    {
        bool inserted;
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [key](const value_type& a){return a.key() == key;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(key.begin(),key.end()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;

        }
        else 
        {
            inserted = false;
        }
        return std::make_pair(it,inserted);
    }

    template <class A=allocator_type, class... Args>
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& key, Args&&... args)
    {
        bool inserted;
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [key](const value_type& a){return a.key() == key;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(key_storage_type(key.begin(),key.end(), get_allocator()), 
                                        std::forward<Args>(args)...);
            it = this->members_.begin() + this->members_.size() - 1;
            inserted = true;

        }
        else 
        {
            inserted = false;
        }
        return std::make_pair(it,inserted);
    }

    template <class A=allocator_type, class ... Args>
    typename std::enable_if<is_stateless<A>::value,iterator>::type
    try_emplace(iterator hint, const string_view_type& key, Args&&... args)
    {
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [key](const value_type& a){return a.key() == key;});

        if (it == this->members_.end())
        {
            if (hint == this->members_.end())
            {
                this->members_.emplace_back(key_storage_type(key.begin(),key.end()), 
                                            std::forward<Args>(args)...);
                it = this->members_.begin() + (this->members_.size() - 1);
            }
            else
            {
                it = this->members_.emplace(hint, 
                                            key_storage_type(key.begin(),key.end()), 
                                            std::forward<Args>(args)...);
            }
        }
        return it;
    }

    template <class A=allocator_type, class ... Args>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type
    try_emplace(iterator hint, const string_view_type& key, Args&&... args)
    {
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [key](const value_type& a){return a.key() == key;});

        if (it == this->members_.end())
        {
            if (hint == this->members_.end())
            {
                this->members_.emplace_back(key_storage_type(key.begin(),key.end(), get_allocator()), 
                                            std::forward<Args>(args)...);
                it = this->members_.begin() + (this->members_.size() - 1);
            }
            else
            {
                it = this->members_.emplace(hint, 
                                            key_storage_type(key.begin(),key.end(), get_allocator()), 
                                            std::forward<Args>(args)...);
            }
        }
        return it;
    }

    // set_

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,void>::type 
    set_(key_storage_type&& key, T&& value)
    {
        string_view_type s(key.data(),key.size());
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [s](const value_type& a){return a.key().compare(s) == 0;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(key), 
                                  std::forward<T>(value));
        }
        else
        {
            it->value(Json(std::forward<T>(value)));
        }
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,void>::type 
    set_(key_storage_type&& key, T&& value)
    {
        string_view_type s(key.data(),key.size());
        auto it = std::find_if(this->members_.begin(),this->members_.end(), 
                               [s](const value_type& a){return a.key().compare(s) == 0;});

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(key), 
                                  std::forward<T>(value),get_allocator());
        }
        else
        {
            it->value(Json(std::forward<T>(value),get_allocator()));
        }
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    set_(iterator hint, key_storage_type&& key, T&& value)
    {
        iterator it = hint;

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(key), 
                                  std::forward<T>(value));
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == key)
        {
            it->value(Json(std::forward<T>(value)));
        }
        else
        {
            it = this->members_.emplace(it,
                                  std::forward<key_storage_type>(key),
                                  std::forward<T>(value));
        }
        return it;
    }

    template <class T, class A=allocator_type>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    set_(iterator hint, key_storage_type&& key, T&& value)
    {
        iterator it = hint;

        if (it == this->members_.end())
        {
            this->members_.emplace_back(std::forward<key_storage_type>(key), 
                                  std::forward<T>(value), get_allocator());
            it = this->members_.begin() + (this->members_.size() - 1);
        }
        else if (it->key() == key)
        {
            it->value(Json(std::forward<T>(value), get_allocator()));
        }
        else
        {
            it = this->members_.emplace(it,
                                  std::forward<key_storage_type>(key),
                                  std::forward<T>(value), get_allocator());
        }
        return it;
    }

    bool operator==(const json_object& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = this->members_.begin(); it != this->members_.end(); ++it)
        {
            auto rhs_it = std::find_if(rhs.begin(),rhs.end(), 
                                       [it](const value_type& a){return a.key() == it->key();});
            if (rhs_it == rhs.end() || rhs_it->key() != it->key() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }
private:
    json_object& operator=(const json_object&) = delete;
};

}

#endif
