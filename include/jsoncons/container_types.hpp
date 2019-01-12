// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONTAINER_TYPES_HPP
#define JSONCONS_CONTAINER_TYPES_HPP

#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <cstring>
#include <algorithm> // std::sort, std::stable_sort, std::lower_bound, std::unique
#include <utility>
#include <initializer_list>
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <utility> // std::move
#include <type_traits> // std::enable_if
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_type_traits.hpp>

namespace jsoncons {

// key_value

template <class KeyT, class ValueT>
class key_value
{
public:
    typedef KeyT key_type;
    typedef ValueT value_type;
    typedef typename KeyT::allocator_type allocator_type;
    typedef typename value_type::string_view_type string_view_type;
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

    template <class T>
    key_value(key_type&& name, T&& val)
        : key_(std::forward<key_type>(name)), 
          value_(std::forward<T>(val))
    {
    }

    template <class T>
    key_value(key_type&& name, 
                   T&& val, 
                   const allocator_type& allocator)
        : key_(std::forward<key_type>(name)), value_(std::forward<T>(val), allocator)
    {
    }

    key_value(const key_value& member)
        : key_(member.key_), value_(member.value_)
    {
    }

    key_value(key_value&& member)
        : key_(std::move(member.key_)), value_(std::move(member.value_))
    {
    }

    string_view_type key() const
    {
        return string_view_type(key_.data(),key_.size());
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

    void swap(key_value& member)
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

    key_value& operator=(key_value&& member)
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
    const key_type& name() const
    {
        return key_;
    }
#endif
};

// json_array

template <class Allocator>
class container_base
{
public:
    typedef Allocator allocator_type;
private:
    allocator_type allocator_;
public:
    container_base()
        : allocator_()
    {
    }
    container_base(const allocator_type& allocator)
        : allocator_(allocator)
    {
    }

    allocator_type get_allocator() const
    {
        return allocator_;
    }
};

// json_array

template <class Json>
class json_array: public container_base<typename Json::allocator_type>
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef Json value_type;
private:
    typedef typename Json::implementation_policy implementation_policy;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> value_allocator_type;
    using value_container_type = typename implementation_policy::template sequence_container_type<value_type,value_allocator_type>;
    value_container_type elements_;
public:
    typedef typename value_container_type::iterator iterator;
    typedef typename value_container_type::const_iterator const_iterator;
    typedef typename std::iterator_traits<iterator>::reference reference;
    typedef typename std::iterator_traits<const_iterator>::reference const_reference;

    using container_base<allocator_type>::get_allocator;

    json_array()
        : container_base<allocator_type>(), 
          elements_()
    {
    }

    explicit json_array(const allocator_type& allocator)
        : container_base<allocator_type>(allocator), 
          elements_(value_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const allocator_type& allocator = allocator_type())
        : container_base<allocator_type>(allocator), 
          elements_(n,Json(),value_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const Json& value, 
                        const allocator_type& allocator = allocator_type())
        : container_base<allocator_type>(allocator), 
          elements_(n,value,value_allocator_type(allocator))
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end, const allocator_type& allocator = allocator_type())
        : container_base<allocator_type>(allocator), 
          elements_(begin,end,value_allocator_type(allocator))
    {
    }
    json_array(const json_array& val)
        : container_base<allocator_type>(val.get_allocator()),
          elements_(val.elements_)
    {
    }
    json_array(const json_array& val, const allocator_type& allocator)
        : container_base<allocator_type>(allocator), 
          elements_(val.elements_,value_allocator_type(allocator))
    {
    }

    json_array(json_array&& val) noexcept
        : container_base<allocator_type>(val.get_allocator()), 
          elements_(std::move(val.elements_))
    {
    }
    json_array(json_array&& val, const allocator_type& allocator)
        : container_base<allocator_type>(allocator), 
          elements_(std::move(val.elements_),value_allocator_type(allocator))
    {
    }

    json_array(std::initializer_list<Json> init)
        : container_base<allocator_type>(), 
          elements_(std::move(init))
    {
    }

    json_array(std::initializer_list<Json> init, 
               const allocator_type& allocator)
        : container_base<allocator_type>(allocator), 
          elements_(std::move(init),value_allocator_type(allocator))
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

#if !defined(JSONCONS_NO_DEPRECATED)
    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.cbegin()+from_index,elements_.cbegin()+to_index);
    }
#endif
    void erase(const_iterator pos) 
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
        iterator it = elements_.begin() + (pos - elements_.begin());
        elements_.erase(it);
#else
        elements_.erase(pos);
#endif
    }

    void erase(const_iterator first, const_iterator last) 
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
        iterator it1 = elements_.begin() + (first - elements_.begin());
        iterator it2 = elements_.begin() + (last - elements_.begin());
        elements_.erase(it1,it2);
#else
        elements_.erase(first,last);
#endif
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
        elements_.insert(it, first, last);
        return first == last ? it : it + 1;
#else
        return elements_.insert(pos, first, last);
#endif
    }

    template <class A=allocator_type, class... Args>
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    emplace(const_iterator pos, Args&&... args)
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
        // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, std::forward<Args>(args)...);
#else
        return elements_.emplace(pos, std::forward<Args>(args)...);
#endif
    }

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

// json_object

template <class KeyT,class Json,bool PreserveOrder>
class json_object
{
};

// Sort keys
template <class KeyT,class Json>
class json_object<KeyT,Json,false> final 
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef KeyT key_type;
    typedef key_value<KeyT,Json> key_value_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::string_view_type string_view_type;
private:
    typedef typename Json::implementation_policy implementation_policy;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_type> key_value_allocator_type;
    using key_value_container_type = typename implementation_policy::template sequence_container_type<key_value_type,key_value_allocator_type>;

    allocator_type allocator_;
    key_value_container_type members_;
public:
    typedef typename key_value_container_type::iterator iterator;
    typedef typename key_value_container_type::const_iterator const_iterator;

    json_object()
    {
    }
    json_object(const allocator_type& allocator)
        : allocator_(allocator), 
          members_(key_value_allocator_type(allocator))
    {
    }

    json_object(const json_object& val)
        : allocator_(val.get_allocator()), members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : allocator_(val.get_allocator()), 
          members_(std::move(val.members_))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) 
        : allocator_(allocator), 
          members_(val.members_,key_value_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) 
        : allocator_(allocator), members_(std::move(val.members_),key_value_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init)
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
        : allocator_(allocator), 
          members_(key_value_allocator_type(allocator))
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

    allocator_type get_allocator() const
    {
        return allocator_;
    }

    void swap(json_object& val)
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

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < members_.size(); ++i)
        {
            members_[i].shrink_to_fit();
        }
        members_.shrink_to_fit();
    }

    void reserve(size_t n) {members_.reserve(n);}

    Json& at(size_t i) 
    {
        if (i >= members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return members_[i].value();
    }

    const Json& at(size_t i) const 
    {
        if (i >= members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return members_[i].value();
    }

    iterator find(const string_view_type& name)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        auto result = (it != members_.end() && it->key() == name) ? it : members_.end();
        return result;
    }

    const_iterator find(const string_view_type& name) const
    {
        auto it = std::lower_bound(members_.begin(),members_.end(), 
                                   name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});
        auto result = (it != members_.end() && it->key() == name) ? it : members_.end();
        return result;
    }

    void erase(const_iterator pos) 
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
        iterator it = members_.begin() + (pos - members_.begin());
        members_.erase(it);
#else
        members_.erase(pos);
#endif
    }

    void erase(const_iterator first, const_iterator last) 
    {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
        iterator it1 = members_.begin() + (first - members_.begin());
        iterator it2 = members_.begin() + (last - members_.begin());
        members_.erase(it1,it2);
#else
        members_.erase(first,last);
#endif
    }

    void erase(const string_view_type& name) 
    {
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        if (it != members_.end() && it->key() == name)
        {
            members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        members_.reserve(members_.size() + count);
        for (auto s = first; s != last; ++s)
        {
            members_.emplace_back(pred(*s));
        }
        std::stable_sort(members_.begin(),members_.end(),
                         [](const key_value_type& a, const key_value_type& b){return a.key().compare(b.key()) < 0;});
        auto it = std::unique(members_.rbegin(), members_.rend(),
                              [](const key_value_type& a, const key_value_type& b){ return !(a.key().compare(b.key()));});
        members_.erase(members_.begin(),it.base());
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
            auto pos = std::lower_bound(members_.begin(),members_.end(), it->key(), 
                                        [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});   
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
            }
            else if (it->key() != pos->key())
            {
                members_.emplace(pos,*it);
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
            if (hint != members_.end() && hint->key() <= it->key())
            {
                pos = std::lower_bound(hint,members_.end(), it->key(), 
                                      [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            else
            {
                pos = std::lower_bound(members_.begin(),members_.end(), it->key(), 
                                      [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
                hint = members_.begin() + (members_.size() - 1);
            }
            else if (it->key() != pos->key())
            {
                hint = members_.emplace(pos,*it);
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
            auto pos = std::lower_bound(members_.begin(),members_.end(), it->key(), 
                                        [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});   
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
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
            if (hint != members_.end() && hint->key() <= it->key())
            {
                pos = std::lower_bound(hint,members_.end(), it->key(), 
                                      [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            else
            {
                pos = std::lower_bound(members_.begin(),members_.end(), it->key(), 
                                      [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
            }
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
                hint = members_.begin() + (members_.size() - 1);
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
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& name, Args&&... args)
    {
        bool inserted;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<!is_stateless<A>::value,std::pair<iterator,bool>>::type
    try_emplace(const string_view_type& name, Args&&... args)
    {
        bool inserted;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, 
                                   [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    try_emplace(iterator hint, const string_view_type& name, Args&&... args)
    {
        iterator it = hint;

        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    try_emplace(iterator hint, const string_view_type& name, Args&&... args)
    {
        iterator it = hint;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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
    typename std::enable_if<!is_stateless<A>::value,iterator>::type 
    insert_or_assign(iterator hint, const string_view_type& name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(), name, 
                                  [](const key_value_type& a, const string_view_type& k){return a.key().compare(k) < 0;});        
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

    bool operator==(const json_object& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.begin(), rhs.end(), *it, 
                                           [](const key_value_type& a, const key_value_type& b){return a.key().compare(b.key()) < 0;});
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
class json_object<KeyT,Json,true> final
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef typename Json::char_type char_type;
    typedef KeyT key_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value<KeyT,Json> key_value_type;
private:
    typedef typename Json::implementation_policy implementation_policy;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_type> key_value_allocator_type;
    using key_value_container_type = typename implementation_policy::template sequence_container_type<key_value_type,key_value_allocator_type>;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<size_t> index_allocator_type;
    using index_container_type = typename implementation_policy::template sequence_container_type<size_t,index_allocator_type>;

    allocator_type allocator_;
    key_value_container_type members_;
    index_container_type index_;
public:
    typedef typename key_value_container_type::iterator iterator;
    typedef typename key_value_container_type::const_iterator const_iterator;

    json_object()
    {
    }
    json_object(const allocator_type& allocator)
        : allocator_(allocator), 
          members_(key_value_allocator_type(allocator))
    {
    }

    json_object(const json_object& val)
        : allocator_(val.get_allocator()), members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : allocator_(val.get_allocator()), 
          members_(std::move(val.members_))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) 
        : allocator_(allocator), 
          members_(val.members_,key_value_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) 
        : allocator_(allocator), members_(std::move(val.members_),key_value_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init)
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
        : allocator_(allocator), 
          members_(key_value_allocator_type(allocator))
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

    allocator_type get_allocator() const
    {
        return allocator_;
    }

    void swap(json_object& val)
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

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void shrink_to_fit() 
    {
        for (size_t i = 0; i < members_.size(); ++i)
        {
            members_[i].shrink_to_fit();
        }
        members_.shrink_to_fit();
    }

    void reserve(size_t n) {members_.reserve(n);}

    Json& at(size_t i) 
    {
        if (i >= members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return members_[i].value();
    }

    const Json& at(size_t i) const 
    {
        if (i >= members_.size())
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("Invalid array subscript"));
        }
        return members_[i].value();
    }

    iterator find(const string_view_type& name)
    {
        return std::find_if(members_.begin(),members_.end(), 
                            [name](const key_value_type& kv){return kv.key() == name;});
    }

    const_iterator find(const string_view_type& name) const
    {
        return std::find_if(members_.begin(),members_.end(), 
                            [name](const key_value_type& kv){return kv.key() == name;});
    }

    void erase(const_iterator first, const_iterator last) 
    {
        members_.erase(first,last);
    }

    void erase(const string_view_type& name) 
    {
        auto it = std::find_if(members_.begin(),members_.end(), 
                              [name](const key_value_type& kv){return kv.key() == name;});
        if (it != members_.end())
        {
            members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        members_.reserve(members_.size() + count);
        for (auto s = first; s != last; ++s)
        {
            members_.emplace_back(pred(*s));
        }
        auto it = last_wins_unique_sequence(members_.begin(), members_.end(),
                              [](const key_value_type& a, const key_value_type& b){ return a.key().compare(b.key());});
        members_.erase(it,members_.end());
    }

    // insert_or_assign

    template <class T, class A=allocator_type>
    typename std::enable_if<is_stateless<A>::value,std::pair<iterator,bool>>::type
    insert_or_assign(const string_view_type& name, T&& value)
    {
        bool inserted;
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [name](const key_value_type& a){return a.key() == name;});

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.begin(),name.end()), 
                                        std::forward<T>(value));
            it = members_.begin() + members_.size() - 1;
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
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [name](const key_value_type& a){return a.key() == name;});

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.begin(),name.end(),get_allocator()), 
                                        std::forward<T>(value),get_allocator());
            it = members_.begin() + members_.size() - 1;
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
        if (hint == members_.end())
        {
            auto result = insert_or_assign(key, std::forward<T>(value));
            it = result.first;
        }
        else
        {
            it = std::find_if(members_.begin(),members_.end(), 
                              [key](const key_value_type& a){return a.key() == key;});

            if (it == members_.end())
            {
                members_.emplace_back(key_type(key.begin(),key.end()), 
                                            std::forward<T>(value));
                it = members_.begin() + members_.size() - 1;
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
        if (hint == members_.end())
        {
            auto result = insert_or_assign(key, std::forward<T>(value));
            it = result.first;
        }
        else
        {
            it = std::find_if(members_.begin(),members_.end(), 
                              [key](const key_value_type& a){return a.key() == key;});

            if (it == members_.end())
            {
                members_.emplace_back(key_type(key.begin(),key.end(),get_allocator()), 
                                            std::forward<T>(value),get_allocator());
                it = members_.begin() + members_.size() - 1;
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
            auto pos = std::find_if(members_.begin(),members_.end(), 
                                    [it](const key_value_type& a){return a.key() == it->key();});
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
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
            auto pos = std::find_if(members_.begin(),members_.end(), 
                                    [it](const key_value_type& a){return a.key() == it->key();});
            if (pos == members_.end() )
            {
                hint = members_.emplace(hint,*it);
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
            auto pos = std::find_if(members_.begin(),members_.end(), 
                                    [it](const key_value_type& a){return a.key() == it->key();});
            if (pos == members_.end() )
            {
                members_.emplace_back(*it);
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
            auto pos = std::find_if(members_.begin(),members_.end(), 
                                    [it](const key_value_type& a){return a.key() == it->key();});
            if (pos == members_.end() )
            {
                hint = members_.emplace(hint,*it);
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
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [key](const key_value_type& a){return a.key() == key;});

        if (it == members_.end())
        {
            members_.emplace_back(key_type(key.begin(),key.end()), 
                                        std::forward<Args>(args)...);
            it = members_.begin() + members_.size() - 1;
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
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [key](const key_value_type& a){return a.key() == key;});

        if (it == members_.end())
        {
            members_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                                        std::forward<Args>(args)...);
            it = members_.begin() + members_.size() - 1;
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
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [key](const key_value_type& a){return a.key() == key;});

        if (it == members_.end())
        {
            if (hint == members_.end())
            {
                members_.emplace_back(key_type(key.begin(),key.end()), 
                                            std::forward<Args>(args)...);
                it = members_.begin() + (members_.size() - 1);
            }
            else
            {
                it = members_.emplace(hint, 
                                            key_type(key.begin(),key.end()), 
                                            std::forward<Args>(args)...);
            }
        }
        return it;
    }

    template <class A=allocator_type, class ... Args>
    typename std::enable_if<!is_stateless<A>::value,iterator>::type
    try_emplace(iterator hint, const string_view_type& key, Args&&... args)
    {
        auto it = std::find_if(members_.begin(),members_.end(), 
                               [key](const key_value_type& a){return a.key() == key;});

        if (it == members_.end())
        {
            if (hint == members_.end())
            {
                members_.emplace_back(key_type(key.begin(),key.end(), get_allocator()), 
                                            std::forward<Args>(args)...);
                it = members_.begin() + (members_.size() - 1);
            }
            else
            {
                it = members_.emplace(hint, 
                                            key_type(key.begin(),key.end(), get_allocator()), 
                                            std::forward<Args>(args)...);
            }
        }
        return it;
    }

    bool operator==(const json_object& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {
            auto rhs_it = std::find_if(rhs.begin(),rhs.end(), 
                                       [it](const key_value_type& a){return a.key() == it->key();});
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
