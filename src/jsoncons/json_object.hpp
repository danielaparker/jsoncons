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
#include <jsoncons/json_traits.hpp>
#include <jsoncons/jsoncons_util.hpp>

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
    typedef typename KeyT::allocator_type allocator_type;

    key_value_pair()
    {
    }

    key_value_pair(const key_type& name, const ValueT& val)
        : key_(name), value_(val)
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
    key_value_pair(key_type&& name, 
                   T&& val, 
                   const allocator_type& allocator)
        : key_(std::forward<key_type&&>(name)), value_(std::forward<T&&>(val), allocator)
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

template <class KeyT,class Json,bool PreserveOrder>
class json_object
{
};

// Do not preserve order
template <class KeyT,class Json>
class json_object<KeyT,Json,false>
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_object> self_allocator_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::key_allocator_type key_allocator_type;
    typedef KeyT key_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value_pair<KeyT,Json> value_type;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> kvp_allocator_type;
    typedef typename Json::base_object_type base_object_type;

    typedef typename base_object_type::iterator iterator;
    typedef typename base_object_type::const_iterator const_iterator;

private:
    self_allocator_type self_allocator_;
    base_object_type members_;
public:
    json_object()
        : self_allocator_(), members_()
    {
    }
    json_object(const allocator_type& allocator)
        : self_allocator_(allocator), 
          members_(kvp_allocator_type(allocator))
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
        members_(val.members_,kvp_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) :
        self_allocator_(allocator), members_(std::move(val.members_),kvp_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init)
        : self_allocator_()
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

    json_object(std::initializer_list<typename Json::array> init, 
                const allocator_type& allocator)
        : self_allocator_(allocator),
          members_(kvp_allocator_type(allocator))
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
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        auto result = (it != members_.end() && name_eq_string(it->key(),name,length)) ? it : members_.end();
        return result;
    }

    const_iterator find(const char_type* name, size_t length) const
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        auto result = (it != members_.end() && name_eq_string(it->key(),name,length)) ? it : members_.end();
        return result;
    }

    void erase(iterator first, iterator last) 
    {
        members_.erase(first,last);
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

    template <class T>
    void bulk_insert(key_type&& name, T&& value)
    {
        members_.emplace_back(std::forward<key_type&&>(name), 
                              std::forward<T&&>(value));
    }
    void end_bulk_insert()
    {
        std::stable_sort(members_.begin(),members_.end(),member_lt_member<value_type>());
        auto it = std::unique(members_.rbegin(), members_.rend(),
                              [](const value_type& a, const value_type& b){ return !(a.key().compare(b.key()));});
        members_.erase(members_.begin(),it.base());
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<is_stateless<U>::value
           >::type* = nullptr>
    void set(string_view_type name, T&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data(),member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length()), 
                                  std::forward<T&&>(value));
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            members_.emplace(it,
                             key_type(name.data(),name.length()),
                             std::forward<T&&>(value));
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<!is_stateless<U>::value
           >::type* = nullptr>
    void set(string_view_type name, T&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data(),member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(), get_self_allocator()),
                                  std::forward<T&&>(value),get_self_allocator() );
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator() ));
        }
        else
        {
            members_.emplace(it,
                             key_type(name.data(),name.length(), get_self_allocator()),
                             std::forward<T&&>(value),get_self_allocator() );
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<is_stateless<U>::value
           >::type* = nullptr>
    void set_(key_type&& name, T&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data(),member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value));
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            members_.emplace(it,
                             std::forward<key_type&&>(name),
                             std::forward<T&&>(value));
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<!is_stateless<U>::value
           >::type* = nullptr>
    void set_(key_type&& name, T&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data(),member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value),get_self_allocator() );
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator() ));
        }
        else
        {
            members_.emplace(it,
                             std::forward<key_type&&>(name),
                             std::forward<T&&>(value),get_self_allocator() );
        }
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    set(iterator hint, string_view_type name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length()), 
                                  std::forward<T&&>(value));
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            it = members_.emplace(it,
                                  key_type(name.data(),name.length()),
                                  std::forward<T&&>(value));
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<!is_stateless<U>::value,iterator>::type 
    set(iterator hint, string_view_type name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(), get_self_allocator()), 
                                  std::forward<T&&>(value),get_self_allocator() );
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator() ));
        }
        else
        {
            it = members_.emplace(it,
                                  key_type(name.data(),name.length(), get_self_allocator()),
                                  std::forward<T&&>(value),get_self_allocator() );
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    set_(iterator hint, key_type&& name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value));
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            it = members_.emplace(it,
                                  std::forward<key_type&&>(name),
                                  std::forward<T&&>(value));
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<!is_stateless<U>::value,iterator>::type 
    set_(iterator hint, key_type&& name, T&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->key() <= name)
        {
            it = std::lower_bound(hint,members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value),get_self_allocator() );
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator() ));
        }
        else
        {
            it = members_.emplace(it,
                                  std::forward<key_type&&>(name),
                                  std::forward<T&&>(value),get_self_allocator() );
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

template <class KeyT,class Json>
class json_object<KeyT,Json,true>
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_object> self_allocator_type;
    typedef typename Json::char_type char_type;
    typedef typename Json::key_allocator_type key_allocator_type;
    typedef KeyT key_type;
    typedef typename Json::string_view_type string_view_type;
    typedef key_value_pair<KeyT,Json> value_type;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> kvp_allocator_type;
    typedef std::vector<value_type, kvp_allocator_type> base_object_type;

    typedef typename base_object_type::iterator iterator;
    typedef typename base_object_type::const_iterator const_iterator;

private:
    self_allocator_type self_allocator_;
    base_object_type members_;
public:
    json_object()
        : self_allocator_(), members_()
    {
    }
    json_object(const allocator_type& allocator)
        : self_allocator_(allocator), 
          members_(kvp_allocator_type(allocator))
    {
    }

    json_object(const json_object& val)
        : self_allocator_(val.get_self_allocator()), 
          members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : self_allocator_(val.get_self_allocator()), 
          members_(std::move(val.members_))
    {
    }

    json_object(const json_object& val, const allocator_type& allocator) 
        : self_allocator_(allocator), 
          members_(val.members_,kvp_allocator_type(allocator))
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) :
        self_allocator_(allocator), members_(std::move(val.members_),kvp_allocator_type(allocator))
    {
    }

    json_object(std::initializer_list<typename Json::array> init)
        : self_allocator_()
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

    json_object(std::initializer_list<typename Json::array> init, 
                const allocator_type& allocator)
        : self_allocator_(allocator),
          members_(kvp_allocator_type(allocator))
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
        members_.erase(first,last);
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

    template <class T> inline
    void bulk_insert(key_type&& name, T&& value)
    {
        members_.emplace_back(std::forward<key_type&&>(name), 
                              std::forward<T&&>(value));
    }
    void end_bulk_insert()
    {
        auto it = unique_unsorted(members_.begin(), members_.end(),
                                  member_lt_member<value_type>(),
                              [](const value_type& a, const value_type& b){ return !(a.key().compare(b.key()));});
        members_.erase(it,members_.end());
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<is_stateless<U>::value
           >::type* = nullptr>
    void set(string_view_type name, T&& value)
    {
        equals_pred<value_type,char_type> comp(name.data(), name.length());
        auto it = std::find_if(members_.begin(),members_.end(), comp);

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length()), 
                                  std::forward<T&&>(value));
        }
        else
        {
            it->value(Json(std::forward<T&&>(value)));
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<!is_stateless<U>::value
           >::type* = nullptr>
    void set(string_view_type name, T&& value)
    {
        equals_pred<value_type,char_type> comp(name.data(), name.length());
        auto it = std::find_if(members_.begin(),members_.end(), comp);

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(), get_self_allocator()), 
                                  std::forward<T&&>(value),get_self_allocator());
        }
        else
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator()));
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<is_stateless<U>::value
           >::type* = nullptr>
    void set_(key_type&& name, T&& value)
    {
        equals_pred<value_type,char_type> comp(name.data(), name.length());
        auto it = std::find_if(members_.begin(),members_.end(), comp);

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value));
        }
        else
        {
            it->value(Json(std::forward<T&&>(value)));
        }
    }

    template <class T, class U=allocator_type,
        typename std::enable_if<!is_stateless<U>::value
           >::type* = nullptr>
    void set_(key_type&& name, T&& value)
    {
        equals_pred<value_type,char_type> comp(name.data(), name.length());
        auto it = std::find_if(members_.begin(),members_.end(), comp);

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value),get_self_allocator());
        }
        else
        {
            it->value(Json(std::forward<T&&>(value),get_self_allocator()));
        }
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    set(iterator hint, string_view_type name, T&& value)
    {
        iterator it = hint;

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(), get_self_allocator()), 
                                  std::forward<T&&>(value));
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            it = members_.emplace(it,
                                  key_type(name.data(),name.length()),
                                  std::forward<T&&>(value));
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<!is_stateless<U>::value,iterator>::type 
    set(iterator hint, string_view_type name, T&& value)
    {
        iterator it = hint;

        if (it == members_.end())
        {
            members_.emplace_back(key_type(name.data(),name.length(),get_self_allocator()), 
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
                                  key_type(name.data(),name.length(),get_self_allocator()),
                                  std::forward<T&&>(value),get_self_allocator());
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    set_(iterator hint, key_type&& name, T&& value)
    {
        iterator it = hint;

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value));
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value)));
        }
        else
        {
            it = members_.emplace(it,
                                  std::forward<key_type&&>(name),
                                  std::forward<T&&>(value));
        }
        return it;
    }

    template <class T, class U=allocator_type>
        typename std::enable_if<!is_stateless<U>::value,iterator>::type 
    set_(iterator hint, key_type&& name, T&& value)
    {
        iterator it = hint;

        if (it == members_.end())
        {
            members_.emplace_back(std::forward<key_type&&>(name), 
                                  std::forward<T&&>(value), get_self_allocator());
            it = members_.begin() + (members_.size() - 1);
        }
        else if (it->key() == name)
        {
            it->value(Json(std::forward<T&&>(value), get_self_allocator()));
        }
        else
        {
            it = members_.emplace(it,
                                  std::forward<key_type&&>(name),
                                  std::forward<T&&>(value), get_self_allocator());
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
