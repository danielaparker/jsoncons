// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_OBJECT_HPP
#define JSONCONS_JSON_OBJECT_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>
#include <jsoncons/json_text_traits.hpp>

namespace jsoncons {

template <class Pointer,class Compare>
struct compare_pointer
{
    Compare c_;

    compare_pointer(Compare c)
        : c_(c)
    {
    }
    bool operator()(Pointer a, Pointer b)
    {
        return c_(*a,*b);
    }
};

template <class ForwardIt,class Compare,class BinaryPredicate>
ForwardIt unique_unsorted(ForwardIt first, ForwardIt last, Compare comp, BinaryPredicate predicate)
{
    if (first == last)
    {
        return last;
    }
    std::vector<typename ForwardIt::pointer> dups;
    {
        typedef typename ForwardIt::pointer pointer;

        std::vector<pointer> v(std::distance(first,last));
        auto p = v.begin();
        for (auto it = first; it != last; ++it)
        {
            *p++ = &(*it);
        }
        compare_pointer<pointer,Compare> c(comp);
        std::sort(v.begin(), v.end(), c);
        auto it = v.begin();
        auto end = v.end();
        for (auto begin = it+1; begin != end; ++it, ++begin)
        {
            if (predicate(*(*it),*(*begin)))
            {
                dups.push_back(*it);
            }
        }
    }
    if (dups.size() == 0)
    {
        return last;
    }

    auto it = first;
    for (auto p = first; p != last; ++p)
    {
        bool no_dup = true;
        if (dups.size() > 0)
        {
            for (auto q = dups.begin(); no_dup && q != dups.end();)
            {
                if (predicate(*p,*(*q)))
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
        if (no_dup && it != p)
        {
            *it = std::move(*p);
            ++it;
        }
    }
    
    return it;
}

template <class ValueT,class CharT>
class member_lt_string
{
    size_t length_;
public:
    member_lt_string(size_t length)
        : length_(length)
    {
    }

    bool operator()(const ValueT& a, const CharT* b) const
    {
        size_t len = (std::min)(a.key().length(),length_);
        int result = std::char_traits<CharT>::compare(a.key().data(),b,len);
        if (result != 0)
        {
            return result < 0;
        }

        return a.key().length() < length_;
    }
};

template <class CharT,class ValueT>
class string_lt_member
{
    size_t length_;
public:
    string_lt_member(size_t length)
        : length_(length)
    {
    }

    bool operator()(const CharT* a, const ValueT& b) const
    {
        size_t len = (std::min)(b.key().length(),length_);
        int result = std::char_traits<CharT>::compare(a, b.key().data(),len);
        if (result != 0)
        {
            return result < 0;
        }

        return length_ < b.key().length();
    }
};

template <class StringT,class CharT>
bool name_le_string(const StringT& a, const CharT* b, size_t length)
{
    size_t min_len = (std::min)(a.length(),length);
    int result = std::char_traits<CharT>::compare(a.data(),b, min_len);
    if (result != 0)
    {
        return result < 0;
    }

    return a.length() <= length;
}

template <class StringT,class CharT>
bool name_eq_string(const StringT& a, const CharT* b, size_t length)
{
    return a.length() == length && std::char_traits<CharT>::compare(a.data(),b,length) == 0;
}

template <class ValueT>
class member_lt_member
{
    typedef typename ValueT::char_type char_type;
public:
    bool operator()(const ValueT& a, const ValueT& b) const
    {
        if (a.key().length() == b.key().length())
        {
            return std::char_traits<char_type>::compare(a.key().data(),b.key().data(),a.key().length()) < 0;
        }

        size_t len = (std::min)(a.key().length(),b.key().length());
        int result = std::char_traits<char_type>::compare(a.key().data(),b.key().data(),len);
        if (result != 0)
        {
            return result < 0;
        }

        return a.key().length() < b.key().length();
    }
};

template <class ValueT,class CharT>
class equals_pred
{
    const CharT* b_;
    size_t length_;
public:
    equals_pred(const CharT* b, size_t length)
        : b_(b), length_(length)
    {
    }

    bool operator()(const ValueT& a) const
    {
        size_t len = (std::min)(a.key().length(),length_);
        int result = std::char_traits<CharT>::compare(a.key().data(),b_,len);

        return result == 0 && a.key().length() == length_;
    }
};

template <class KeyT, class ValueT>
class key_value_pair
{
public:
    typedef KeyT key_type;
    typedef typename KeyT::value_type char_type;
    typedef typename ValueT::allocator_type allocator_type;

    key_value_pair()
    {
    }
    key_value_pair(const key_type& name)
        : key_(name)
    {
    }
    key_value_pair(key_type&& name)
        : key_(std::forward<key_type&&>(name))
    {
    }

    key_value_pair(const key_type& name, const ValueT& val)
        : key_(name), value_(val)
    {
    }
    key_value_pair(key_type&& name, const ValueT& val)
        : key_(std::forward<key_type>(name)), value_(val)
    {
    }
    key_value_pair(const key_type& name, ValueT&& val)
        : key_(name), value_(std::forward<ValueT&&>(val))
    {
    }
    key_value_pair(key_type&& name, ValueT&& val)
        : key_(std::forward<key_type&&>(name)), 
          value_(std::forward<ValueT&&>(val))
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
    template <class T>
    key_value_pair(const key_type& name, T&& val, 
                   const allocator_type& allocator)
        : key_(name), value_(std::forward<T&&>(val), allocator)
    {
    }

    const key_type& key() const
    {
        return key_;
    }

    ValueT& value()
    {
        return value_;
    }

    const ValueT& value() const
    {
        return value_;
    }

    void value(const ValueT& value)
    {
        value_ = value;
    }

    void value(ValueT&& value)
    {
        value_ = std::forward<ValueT&&>(value);
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
    const key_type& name() const
    {
        return key_;
    }
#endif
private:
    key_type key_;
    ValueT value_;
};

template <class IteratorT,class NonConstIteratorT>
class json_object_iterator
{
public:
    typedef IteratorT iterator;
    typedef typename std::iterator_traits<IteratorT>::value_type value_type;
    typedef typename std::iterator_traits<IteratorT>::difference_type difference_type;
    typedef typename std::iterator_traits<IteratorT>::pointer pointer;
    typedef typename std::iterator_traits<IteratorT>::reference reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    json_object_iterator()
    {
    }

    json_object_iterator(iterator it)
        : it_(it)
    {
    }

    json_object_iterator(const json_object_iterator<NonConstIteratorT,NonConstIteratorT>& it)
        : it_(it.it_)
    {
    }

    json_object_iterator& operator=(json_object_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_object_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_object_iterator operator++(int) // postfix increment
    {
        json_object_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_object_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_object_iterator operator--(int)
    {
        json_object_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return *it_;
    }

    pointer operator->() const
    {
        return &(*it_);
    }

    friend bool operator==(const json_object_iterator& it1, const json_object_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_object_iterator& it1, const json_object_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_object_iterator& lhs, json_object_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
    }

    iterator get()
    {
        return it_;
    }

//private:
    IteratorT it_;
};

template <class KeyT,class Json,bool PreserveOrder,class Allocator>
class json_object
{
};

// Do not preserve order
template <class KeyT,class Json,class Allocator>
class json_object<KeyT,Json,false,Allocator>
{
public:
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<json_object> self_allocator_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::key_allocator_type key_allocator_type;
    typedef KeyT key_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value_pair<KeyT,Json> value_type;
    typedef typename std::vector<value_type, allocator_type>::iterator base_iterator;
    typedef typename std::vector<value_type, allocator_type>::const_iterator const_base_iterator;

    typedef json_object_iterator<base_iterator,base_iterator> iterator;
    typedef json_object_iterator<const_base_iterator,base_iterator> const_iterator;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<value_type> vector_allocator_type;
private:
    self_allocator_type self_allocator_;
    std::vector<value_type,vector_allocator_type> members_;
public:
    json_object()
        : self_allocator_(), members_(vector_allocator_type())
    {
    }
    json_object(const allocator_type& allocator)
        : self_allocator_(allocator), members_(vector_allocator_type(allocator))
    {
    }

    json_object(const json_object& val)
        : self_allocator_(val.get_self_allocator()), members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : self_allocator_(val.get_self_allocator()), 
          members_(std::move(val.members_))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) :
        self_allocator_(allocator), 
        members_(val.members_,vector_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) :
        self_allocator_(allocator), members_(std::move(val.members_),vector_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init, 
                const Allocator& allocator = Allocator())
        : self_allocator_(allocator),
          members_(vector_allocator_type(allocator))
    {
        for (const auto& element : init)
        {
            if (element.size() != 2 || !element[0].is_string())
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error, "Cannot create object from initializer list");
                break;
            }
        }
        for (auto& element : init)
        {
            set(element[0].as_string_view(), std::move(element[1]));
        }
    }

    self_allocator_type get_self_allocator() const
    {
        return self_allocator_;
    }

    iterator begin()
    {
        //return members_.begin();
        return iterator(members_.begin());
    }

    iterator end()
    {
        return iterator(members_.end());
    }

    const_iterator begin() const
    {
        return const_iterator(members_.begin());
    }

    const_iterator end() const
    {
        return const_iterator(members_.end());
    }

    void swap(json_object& val)
    {
        members_.swap(val.members_);
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

    Json& at(size_t) 
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
    }

    const Json& at(size_t) const 
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
    }

    iterator find(const char_type* name, size_t length)
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        auto result = (it != members_.end() && name_eq_string(it->key(),name,length)) ? it : members_.end();
        return iterator(result);
    }

    const_iterator find(const char_type* name, size_t length) const
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        auto result = (it != members_.end() && name_eq_string(it->key(),name,length)) ? it : members_.end();
        return const_iterator(result);
    }

    void erase(iterator first, iterator last) 
    {
        members_.erase(first.get(),last.get());
    }

    void erase(const char_type* name, size_t length) 
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        if (it != members_.end() && name_eq_string(it->key(),name,length))
        {
            members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        size_t pos = members_.size();
        members_.resize(pos+count);
        auto d = members_.begin()+pos;
        for (auto s = first; s != last; ++s, ++d)
        {
            *d = pred(*s);
        }
        std::stable_sort(members_.begin(),members_.end(),member_lt_member<value_type>());
        auto it = std::unique(members_.rbegin(), members_.rend(),
                              [](const value_type& a, const value_type& b){ return !(a.key().compare(b.key()));});
        members_.erase(members_.begin(),it.base());
    }

    template <class T>
    void set(string_view_type name, T&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data(),member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())), 
                                  std::forward<T&&>(value),get_self_allocator());
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value), get_self_allocator()));
        }
        else
        {
            members_.emplace(it,
                             key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())),
                             std::forward<T&&>(value),get_self_allocator());
        }
    }

    template <class T>
    iterator set(iterator hint, string_view_type name, T&& value)
    {
        base_iterator it;
        if (hint.get() != members_.end() && hint.get()->key() <= name)
        {
            it = std::lower_bound(hint.get(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())), 
                                  std::forward<T&&>(value),get_self_allocator());
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator()));
        }
        else
        {
            it = members_.emplace(it,
                                  key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())),
                                  std::forward<T&&>(value),get_self_allocator());
        }
        return iterator(it);
    }

    bool operator==(const json_object& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_lt_member<value_type>());
            // member_lt_member only compares keys, so we need to check the value separately
            if (rhs_it == rhs.members_.end() || rhs_it->key() != it->key() || rhs_it->value() != it->value())
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

template <class KeyT,class Json,class Allocator>
class json_object<KeyT,Json,true,Allocator>
{
public:
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<json_object> self_allocator_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::key_allocator_type key_allocator_type;
    typedef KeyT key_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value_pair<KeyT,Json> value_type;
    typedef typename std::vector<value_type, allocator_type>::iterator iterator;
    typedef typename std::vector<value_type, allocator_type>::const_iterator const_iterator;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<value_type> vector_allocator_type;
private:
    self_allocator_type self_allocator_;
    std::vector<value_type,vector_allocator_type> members_;
public:
    json_object()
        : self_allocator_(), members_(vector_allocator_type())
    {
    }
    json_object(const allocator_type& allocator)
        : self_allocator_(allocator), members_(vector_allocator_type(allocator))
    {
    }

    json_object(const json_object& val)
        : self_allocator_(), members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : self_allocator_(), members_(std::move(val.members_))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) :
        self_allocator_(allocator), members_(val.members_,vector_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) :
        self_allocator_(allocator), members_(std::move(val.members_),vector_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init, 
                const Allocator& allocator = Allocator())
        : self_allocator_(allocator),
          members_(vector_allocator_type(allocator))
    {
        for (const auto& element : init)
        {
            if (element.size() != 2 || !element[0].is_string())
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error, "Cannot create object from initializer list");
                break;
            }
        }
        for (auto& element : init)
        {
            set(element[0].as_string_view(), std::move(element[1]));
        }
    }
    ~json_object()
    {
    }

    self_allocator_type get_self_allocator() const
    {
        return self_allocator_;
    }

    iterator begin()
    {
        return iterator(members_.begin());
    }

    iterator end()
    {
        return iterator(members_.end());
    }

    const_iterator begin() const
    {
        return const_iterator(members_.begin());
    }

    const_iterator end() const
    {
        return const_iterator(members_.end());
    }

    void swap(json_object& val)
    {
        members_.swap(val.members_);
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
            JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
        }
        return members_[i].value();
    }

    const Json& at(size_t i) const 
    {
        if (i >= members_.size())
        {
            JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
        }
        return members_[i].value();
    }

    iterator find(const char_type* name, size_t length)
    {
        equals_pred<value_type,char_type> comp(name, length);
        auto it = std::find_if(members_.begin(),members_.end(), comp);
        return it;
    }

    const_iterator find(const char_type* name, size_t length) const
    {
        equals_pred<value_type,char_type> comp(name, length);
        auto it = std::find_if(members_.begin(),members_.end(), comp);
        return it;
    }

    void erase(iterator first, iterator last) 
    {
        members_.erase(first.get(),last.get());
    }

    void erase(const char_type* name, size_t length) 
    {
        equals_pred<value_type,char_type> comp(name, length);
        auto it = std::find_if(members_.begin(),members_.end(), comp);
        if (it != members_.end())
        {
            members_.erase(it);
        }
    }

    template<class InputIt, class UnaryPredicate>
    void insert(InputIt first, InputIt last, UnaryPredicate pred)
    {
        size_t count = std::distance(first,last);
        size_t pos = members_.size();
        members_.resize(pos+count);
        auto d = members_.begin()+pos;
        for (auto s = first; s != last; ++s, ++d)
        {
            *d = pred(*s);
        }
        auto it = unique_unsorted(members_.begin(), members_.end(),
                                  member_lt_member<value_type>(),
                              [](const value_type& a, const value_type& b){ return !(a.key().compare(b.key()));});
        members_.erase(it,members_.end());
    }

    template <class T>
    void set(string_view_type name, T&& value)
    {
        equals_pred<value_type,char_type> comp(name.data(), name.length());
        auto it = std::find_if(members_.begin(),members_.end(), comp);

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())), 
                                  std::forward<T&&>(value),get_self_allocator());
        }
        else
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator()));
        }
    }

    template <class T>
    iterator set(iterator hint, string_view_type name, T&& value)
    {
        typename std::vector<value_type,allocator_type>::iterator it = hint;

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())), 
                                  std::forward<T&&>(value),get_self_allocator());
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator()));
        }
        else
        {
            it = members_.emplace(it,
                                  key_type(name.data(),name.length(),key_allocator_type(get_self_allocator())),
                                  std::forward<T&&>(value),get_self_allocator());
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
            equals_pred<value_type,char_type> comp(it->key().data(), it->key().length());
            auto rhs_it = std::find_if(rhs.members_.begin(),rhs.members_.end(), comp);
            if (rhs_it == rhs.members_.end() || rhs_it->key() != it->key() || rhs_it->value() != it->value())
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
