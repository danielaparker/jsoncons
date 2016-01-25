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

template <class JsonT, typename Alloc>
class json_array 
{
    std::vector<JsonT,Alloc> elements_;
    json_array& operator=(const json_array<JsonT,Alloc>&);
public:
    typedef typename std::vector<JsonT,Alloc>::allocator_type allocator_type;
    typedef typename std::vector<JsonT,Alloc>::reference reference;
    typedef typename std::vector<JsonT,Alloc>::const_reference const_reference;
    typedef typename std::vector<JsonT,Alloc>::iterator iterator;
    typedef typename std::vector<JsonT,Alloc>::const_iterator const_iterator;

    json_array(const Alloc& allocator = Alloc())
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

#if !defined(JSONCONS_NO_CXX11_COPY_CONSTRUCTOR)
    json_array(const json_array& val, const Alloc& allocator)
        : elements_(val.elements_,allocator)
    {
    }
    json_array(json_array&& val, const Alloc& allocator)
        : elements_(std::move(val.elements_),allocator)
    {
    }
#endif

    json_array(size_t n, const Alloc& allocator = Alloc())
        : elements_(n,allocator)
    {
    }

    json_array(size_t n, const JsonT& val, const Alloc& allocator = Alloc())
        : elements_(n,val,allocator)
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

    JsonT& at(size_t i) {return elements_.at(i);}

    const JsonT& at(size_t i) const {return elements_.at(i);}

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

    void add(const_iterator pos, const JsonT& value)
    {
        elements_.insert(pos, value);
    }

    void add(const_iterator pos, JsonT&& value)
    {
        elements_.insert(pos, std::move(value));
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
};

template <typename CharT,class ValueT>
class compare_with_string
{
    size_t length_;
public:
    compare_with_string(size_t length)
        : length_(length)
    {
    }

    bool operator()(const ValueT& a, const CharT* b) const
    {
        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name_length(),length_);
        int result = std::char_traits<CharT>::compare(a.name_data(),b,len);
        if (result < 0 || result > 0)
        {
            return result < 0;
        }

        return a.name_length() < length_;
    }
};

template <class ValueT>
class member_compare
{
    typedef typename ValueT::char_type char_type;
public:
    bool operator()(const ValueT& a, const ValueT& b) const
    {
        if (a.name_length() == b.name_length())
        {
            return std::char_traits<char_type>::compare(a.name_data(),b.name_data(),a.name_length()) < 0;
        }

        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name_length(),b.name_length());
        int result = std::char_traits<char_type>::compare(a.name_data(),b.name_data(),len);
        if (result < 0 || result > 0)
        {
            return result < 0;
        }

        return a.name_length() < b.name_length();
    }
};

template <class JsonT, class Alloc>
class json_object_member
{
public:
    typedef typename JsonT::char_type char_type;
    typedef std::basic_string<char_type> name_type;

    json_object_member()
    {
    }
    json_object_member(const char_type* name_data, size_t name_length)
        : name_(name_data,name_length)
    {
    }
    json_object_member(const char_type* name_data, size_t name_length, const JsonT& val)
        : name_(name_data,name_length), value_(val)
    {
    }
    json_object_member(const char_type* name_data, size_t name_length, JsonT&& val)
        : name_(name_data,name_length), value_(std::move(val))
    {
    }
    json_object_member(const json_object_member& member)
        : name_(member.name_), value_(member.value_)
    {
    }
    json_object_member(json_object_member&& member)
        : name_(std::move(member.name_)), value_(std::move(member.value_))
    {
    }
    const char_type* name_data() const
    {
        return name_.data();
    }

    const size_t name_length() const
    {
        return name_.length();
    }

    std::basic_string<char_type> name() const
    {
        return std::basic_string<char_type>(name_.data(),name_.length());
    }

    JsonT& value()
    {
        return value_;
    }

    const JsonT& value() const
    {
        return value_;
    }

    void value(const JsonT& value)
    {
        value_ = value;
    }

    void value(JsonT&& value)
    {
        value_ = std::move(value);
    }

    void swap(json_object_member& member)
    {
        name_.swap(member.name_);
        value_.swap(member.value_);
    }

    json_object_member& operator=(json_object_member const & member)
    {
        if (this != & member)
        {
            name_ = member.name;
            value_ = member.value;
        }
        return *this;
    }

    json_object_member& operator=(json_object_member&& member)
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
    std::basic_string<char_type> name_;
    JsonT value_;
};

template <class JsonT,typename Alloc>
class json_object
{
    json_object<JsonT,Alloc>& operator=(const json_object<JsonT,Alloc>&);
public:
    typedef json_object_member<JsonT,Alloc> value_type;
    typedef typename JsonT::char_type char_type;
    typedef Alloc allocator_type;
    typedef value_type& reference; 
    typedef const value_type& const_reference; 
    typedef typename std::vector<value_type,Alloc>::iterator iterator;
    typedef typename std::vector<value_type,Alloc>::const_iterator const_iterator;
private:
    std::vector<value_type,Alloc> members_;
public:
    json_object(const Alloc& allocator = Alloc())
        : members_(allocator)
    {
    }

    json_object(const json_object<JsonT,Alloc>& val)
        : members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : members_(std::move(val.members_))
    {
    }

#if !defined(JSONCONS_NO_CXX11_COPY_CONSTRUCTOR)
    json_object(const json_object<JsonT,Alloc>& val, const Alloc& allocator)
        : members_(val.members_,allocator)
    {
    }

    json_object(json_object&& val,const Alloc& allocator)
        : members_(std::move(val.members_),allocator)
    {
    }
#endif

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
        size_t length = std::char_traits<char_type>::length(name);
        compare_with_string<char_type,value_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    const_iterator find(const char_type* name) const
    {
        size_t length = std::char_traits<char_type>::length(name);
        compare_with_string<char_type,value_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    iterator find(const std::basic_string<char_type>& name)
    {
        compare_with_string<char_type,value_type> comp(name.length());
        auto it = std::lower_bound(members_.begin(),members_.end(), name.data(), comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }
 
    // Fixed by cperthuis
    const_iterator find(const std::basic_string<char_type>& name) const
    {
        compare_with_string<char_type,value_type> comp(name.length());
        auto it = std::lower_bound(members_.begin(),members_.end(), name.data(), comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    void erase(iterator first, iterator last) 
    {
        members_.erase(first,last);
    }

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= members_.size());
        members_.erase(members_.begin()+from_index,members_.begin()+to_index);
    }

    // Fixed by cperthuis
    void remove(const std::basic_string<char_type>& name) 
    {
        compare_with_string<char_type,value_type> comp(name.length());
        auto it = std::lower_bound(members_.begin(),members_.end(), name.data(), comp);
        if (it != members_.end() && it->name() == name)
        {
            members_.erase(it);
        }
    }

    const value_type& get(size_t i) const 
    {
        return members_[i];
    }

    void set(const std::basic_string<char_type>& name, const JsonT& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(), name.length(),value));
        }
        else if (it->name() == name)
        {
            it->value(value);
        }
        else
        {
            members_.insert(it,value_type(name.data(),name.length(),value));
        }
    }

    void set(std::basic_string<char_type>&& name, const JsonT& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
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
            members_.insert(it,value_type(name.data(),name.length(),value));
        }
    }

    void set(const std::basic_string<char_type>& name, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length(),std::move(value)));
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            members_.insert(it,value_type(name.data(),name.length(),std::move(value)));
        }
    }

    void set(std::basic_string<char_type>&& name, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length(), std::move(value)));
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            members_.insert(it,value_type(name.data(),name.length(),std::move(value)));
        }
    }

    iterator set(iterator hint, const std::basic_string<char_type>& name, const JsonT& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }
        
        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length(), value));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(value);
        }
        else
        {
           it = members_.insert(it,value_type(name.data(),name.length().data(),name.length(),value));
        }
        return it;
    }

    iterator set(iterator hint, std::basic_string<char_type>&& name, const JsonT& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length(), value));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(value);
        }
        else
        {
            it = members_.insert(it,value_type(name.data(),name.length().data(),name.length(),value));
        }
        return it;
    }

    iterator set(iterator hint, const std::basic_string<char_type>& name, JsonT&& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length().data(),name.length(),std::move(value)));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            it = members_.insert(it,value_type(name.data(),name.length(),std::move(value)));
        }
        return it;
    }

    iterator set(iterator hint, std::basic_string<char_type>&& name, JsonT&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name.data() ,compare_with_string<char_type,value_type>(name.length()));
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(name.data(),name.length(), std::move(value)));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            it->value(std::move(value));
        }
        else
        {
            it = members_.insert(it,value_type(name.data(),name.length(),std::move(value)));
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

    JsonT& at(const std::basic_string<char_type>& name) 
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"Member %s not found.",name);
        }
        return it->value();
    }

    const JsonT& at(const std::basic_string<char_type>& name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"Member %s not found.",name);
        }
        return it->value();
    }

    JsonT& get(const std::basic_string<char_type>& name)
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::exception, "Member %s not found.", name);
        }
        return it->value();
    }

    const JsonT& get(const std::basic_string<char_type>& name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::exception, "Member %s not found.", name);
        }
        return it->value();
    }

    JsonT& get(const char_type* name) 
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1(std::exception,"Member %s not found.",name);
        }
        return it->value();
    }

	void sort_members()
	{
		std::sort(members_.begin(),members_.end(),member_compare<value_type>());
	}

    bool operator==(const json_object<JsonT,Alloc>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_compare<value_type>());
            // member_compare actually only compares keys, so we need to check the value separately
            if (rhs_it == rhs.members_.end() || rhs_it->name() != it->name() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }
};



}

#endif
