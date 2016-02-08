// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_STRUCTURES_HPP
#define JSONCONS_JSON_STRUCTURES_HPP

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
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template <class JsonT, class Alloc>
class json_array 
{
public:
    typedef Alloc allocator_type;
    typedef typename std::vector<JsonT,Alloc>::reference reference;
    typedef typename std::vector<JsonT,Alloc>::const_reference const_reference;
    typedef typename std::vector<JsonT,Alloc>::iterator iterator;
    typedef typename std::vector<JsonT,Alloc>::const_iterator const_iterator;
    typedef typename std::vector<JsonT,Alloc>::value_type value_type;

    explicit json_array(const Alloc& allocator = Alloc())
        : elements_(allocator)
    {
    }

    json_array(const json_array& val)
        : elements_(val.elements_)
    {
    }
    json_array(json_array&& val)
        : elements_(std::move(val.elements_))
    {
    }

    json_array(const json_array& val, const Alloc& allocator)
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        : elements_(val.elements_,allocator)
#else
        : elements_(val.elements_)
#endif
    {
    }
    json_array(json_array&& val, const Alloc& allocator)
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        : elements_(std::move(val.elements_),allocator)
#else
        : elements_(val.elements_)
#endif
    {
    }

    explicit json_array(size_t n, const Alloc& allocator = Alloc())
        : elements_(n,JsonT(),allocator)
    {
    }

    explicit json_array(size_t n, const JsonT& value, const Alloc& allocator = Alloc())
        : elements_(n,value,allocator)
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end, const Alloc& allocator = Alloc())
        : elements_(begin,end,allocator)
    {
    }

    void swap(json_array<JsonT,Alloc>& val)
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

    void resize(size_t n, const JsonT& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    void erase(iterator first, iterator last) 
    {
        elements_.erase(first,last);
    }

    JsonT& operator[](size_t i) {return elements_[i];}

    const JsonT& operator[](size_t i) const {return elements_[i];}

    void push_back(const JsonT& value)
    {
        elements_.push_back(value);
    }

    void push_back(JsonT&& value)
    {
        elements_.push_back(std::move(value));
    }

    void add(size_t index, const JsonT& value)
    {
        auto position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void add(size_t index, JsonT&& value)
    {
        auto it = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(it, std::move(value));
    }

    iterator add(const_iterator pos, const JsonT& value)
    {
        return elements_.insert(pos, value);
    }

    iterator add(const_iterator pos, JsonT&& value)
    {
        return elements_.insert(pos, std::move(value));
    }

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<JsonT,Alloc>& rhs) const
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
    json_array& operator=(const json_array<JsonT,Alloc>&);
    std::vector<JsonT,Alloc> elements_;
};

template <class ValueT,typename CharT>
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
        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name().length(),length_);
        int result = std::char_traits<CharT>::compare(a.name().data(),b,len);
        if (result != 0)
        {
            return result < 0;
        }

        return a.name().length() < length_;
    }
};

template <class StringT,typename CharT>
bool name_le_string(const StringT& a, const CharT* b, size_t length)
{
    size_t min_len = std::min JSONCONS_NO_MACRO_EXP(a.length(),length);
    int result = std::char_traits<CharT>::compare(a.data(),b, min_len);
    if (result != 0)
    {
        return result < 0;
    }

    return a.length() <= length;
}

template <class StringT,typename CharT>
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
        if (a.name().length() == b.name().length())
        {
            return std::char_traits<char_type>::compare(a.name().data(),b.name().data(),a.name().length()) < 0;
        }

        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name().length(),b.name().length());
        int result = std::char_traits<char_type>::compare(a.name().data(),b.name().data(),len);
        if (result != 0)
        {
            return result < 0;
        }

        return a.name().length() < b.name().length();
    }
};

template <class StringT, class ValueT>
class name_value_pair
{
public:
    typedef StringT string_type;
    typedef typename StringT::value_type char_type;

    name_value_pair()
    {
    }
    name_value_pair(const string_type& name)
        : name_(name)
    {
    }
    name_value_pair(string_type&& name)
        : name_(std::move(name))
    {
    }

    name_value_pair(const string_type& name, const ValueT& val)
        : name_(name), value_(val)
    {
    }
    name_value_pair(string_type&& name, const ValueT& val)
        : name_(std::move(name)), value_(val)
    {
    }
    name_value_pair(const string_type& name, ValueT&& val)
        : name_(name), value_(std::move(val))
    {
    }
    name_value_pair(string_type&& name, ValueT&& val)
        : name_(std::move(name)), value_(std::move(val))
    {
    }
    name_value_pair(const name_value_pair& member)
        : name_(member.name_), value_(member.value_)
    {
    }
    name_value_pair(name_value_pair&& member)
        : name_(std::move(member.name_)), value_(std::move(member.value_))
    {
    }

    const string_type& name() const
    {
        return name_;
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
        value_ = std::move(value);
    }

    void swap(name_value_pair& member)
    {
        name_.swap(member.name_);
        value_.swap(member.value_);
    }

    name_value_pair& operator=(const name_value_pair& member)
    {
        if (this != & member)
        {
            name_ = member.name_;
            value_ = member.value_;
        }
        return *this;
    }

    name_value_pair& operator=(name_value_pair&& member)
    {
        if (this != &member)
        {
            name_.swap(member.name_);
            value_.swap(member.value_);
        }
        return *this;
    }

    void shrink_to_fit() 
    {
        name_.shrink_to_fit();
        value_.shrink_to_fit();
    }
private:
    string_type name_;
    ValueT value_;
};

template <class StringT,class JsonT,class Alloc>
class json_object
{
public:
    typedef Alloc allocator_type;
    typedef typename JsonT::char_type char_type;
    typedef StringT string_type;
    typedef name_value_pair<StringT,JsonT> value_type;

    typedef typename std::vector<value_type,allocator_type>::iterator iterator;
    typedef typename std::vector<value_type,allocator_type>::const_iterator const_iterator;
    typedef typename std::vector<value_type,allocator_type>::reference reference;
    typedef typename std::vector<value_type,allocator_type>::const_reference const_reference;
private:
    std::vector<value_type,allocator_type> members_;
public:
    json_object(const allocator_type& allocator = allocator_type())
        : members_(allocator)
    {
    }

    json_object(const json_object<StringT,JsonT,Alloc>& val)
        : members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : members_(std::move(val.members_))
    {
    }

    json_object(const json_object<StringT,JsonT,Alloc>& val, const allocator_type& allocator) :
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        members_(val.members_,allocator)
#else
        members_(val.members_)
#endif
    {
    }

    json_object(json_object&& val,const allocator_type& allocator) :
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        members_(std::move(val.members_),allocator)
#else
        members_(std::move(val.members_))
#endif
    {
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

    const_iterator cbegin() const
    {
        return members_.begin();
    }

    const_iterator cend() const
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

    iterator find(const char_type* name)
    {
        return find(name, std::char_traits<char_type>::length(name));
    }

    const_iterator find(const char_type* name) const
    {
        return find(name, std::char_traits<char_type>::length(name));
    }

    iterator find(const char_type* name, size_t length)
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && name_eq_string(it->name(),name,length)) ? it : end();
    }

    const_iterator find(const char_type* name, size_t length) const
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && name_eq_string(it->name(),name,length)) ? it : end();
    }

    iterator find(const string_type& name)
    {
        return find(name.data(), name.length());
    }
 
    const_iterator find(const string_type& name) const
    {
        return find(name.data(), name.length());
    }

    JsonT& at(const string_type& name) 
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"Member %s not found.",name);
        }
        return it->value();
    }

    const JsonT& at(const string_type& name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"Member %s not found.",name);
        }
        return it->value();
    }

    void erase(iterator first, iterator last) 
    {
        members_.erase(first,last);
    }

    void erase(const char_type* name) 
    {
        erase(name, std::char_traits<char_type>::length(name));
    }

    void erase(const char_type* name, size_t length) 
    {
        member_lt_string<value_type,char_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        if (it != members_.end() && name_eq_string(it->name(),name,length))
        {
            members_.erase(it);
        }
    }

    void erase(const string_type& name) 
    {
        return erase(name.data(),name.length());
    }

    void set(const char_type* s, size_t length, const JsonT& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),s,member_lt_string<value_type,char_type>(length));
        if (it == members_.end())
        {
            members_.push_back(value_type(string_type(s,length),value));
        }
        else if (name_eq_string(it->name(),s,length))
        {
            it->value(value);
        }
        else
        {
            members_.insert(it,value_type(string_type(s,length),value));
        }
    }

    void set(const char_type* s, size_t length, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),s,member_lt_string<value_type,char_type>(length));
        if (it == members_.end())
        {
            members_.push_back(value_type(string_type(s,length),std::move(value)));
        }
        else if (name_eq_string(it->name(),s,length))
        {
            it->value(std::move(value));
        }
        else
        {
            members_.insert(it,value_type(string_type(s,length),std::move(value)));
        }
    }

    void set(string_type&& name, const JsonT& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), value));
        }
        else if (it->name() == name)
        {
            it->value(value);
        }
        else
        {
            members_.insert(it,value_type(std::move(name),value));
        }
    }

    void set(const string_type& name, const JsonT& value)
    {
        set(name.data(),name.length(),value);
    }

    void set(const string_type& name, JsonT&& value)
    {
        set(name.data(),name.length(),std::move(value));
    }

    void set(string_type&& name, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), std::move(value)));
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            members_.insert(it,value_type(std::move(name),std::move(value)));
        }
    }

    iterator set(iterator hint, const char_type* name, const JsonT& value)
    {
        return set(hint, name, std::char_traits<char_type>::length(name), value);
    }

    iterator set(iterator hint, const char_type* name, JsonT&& value)
    {
        return set(hint, name, std::char_traits<char_type>::length(name), std::move(value));
    }

    iterator set(iterator hint, const char_type* s, size_t length, const JsonT& value)
    {
        iterator it;
        if (hint != end() && name_le_string(hint->name(), s, length))
        {
            it = std::lower_bound(hint,end(),s,member_lt_string<value_type,char_type>(length));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),s, member_lt_string<value_type,char_type>(length));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(string_type(s, length), value));
            it = members_.end();
        }
        else if (name_eq_string(it->name(),s,length))
        {
            it->value(value);
        }
        else
        {
           it = members_.insert(it,value_type(string_type(s,length),value));
        }
        return it;
    }

    iterator set(iterator hint, const char_type* s, size_t length, JsonT&& value)
    {
        iterator it;
        if (hint != end() && name_le_string(hint->name(), s, length))
        {
            it = std::lower_bound(hint,end(),s,member_lt_string<value_type,char_type>(length));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),s, member_lt_string<value_type,char_type>(length));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(string_type(s, length), std::move(value)));
            it = members_.end();
        }
        else if (name_eq_string(it->name(),s,length))
        {
            it->value(std::move(value));
        }
        else
        {
           it = members_.insert(it,value_type(string_type(s,length),std::move(value)));
        }
        return it;
    }

    iterator set(iterator hint, const string_type& name, const JsonT& value)
    {
        return set(hint,name.data(),name.length(),value);
    }

    iterator set(iterator hint, string_type&& name, const JsonT& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), value));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(value);
        }
        else
        {
            it = members_.insert(it,value_type(std::move(name),value));
        }
        return it;
    }

    iterator set(iterator hint, const string_type& name, JsonT&& value)
    {
        return set(hint,name.data(),name.length(),std::move(value));
    }

    iterator set(iterator hint, string_type&& name, JsonT&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,member_lt_string<value_type,char_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), std::move(value)));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            it = members_.insert(it,value_type(std::move(name),std::move(value)));
        }
        return it;
    }

    void bulk_insert(const value_type& member)
    {
        members_.push_back(member);
    }

    void bulk_insert(value_type&& member)
    {
        members_.push_back(std::move(member));
    }

	void end_bulk_insert()
	{
		std::sort(members_.begin(),members_.end(),member_lt_member<value_type>());
	}

    bool operator==(const json_object<StringT,JsonT,Alloc>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_lt_member<value_type>());
            // member_lt_member actually only compares keys, so we need to check the value separately
            if (rhs_it == rhs.members_.end() || rhs_it->name() != it->name() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }
private:
    json_object<StringT,JsonT,Alloc>& operator=(const json_object<StringT,JsonT,Alloc>&);
};



}

#endif
