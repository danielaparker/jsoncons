// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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
#include <new>
#include "jsoncons/jsoncons.hpp"

namespace jsoncons {

template <class ValueT>
class compare_with_string
{
public:
    typedef typename ValueT::char_type char_type;

    bool operator()(const ValueT& a, 
                    const std::basic_string<char_type>& b) const
    {
        return a.name() < b;
    }
};

template <class ValueT>
class compare_with_cstring
{
    size_t length_;
public:
    typedef typename ValueT::char_type char_type;

    compare_with_cstring(size_t length)
        : length_(length)
    {
    }

    bool operator()(const ValueT& a, char_type const * b) const
    {
        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name().length(),length_);
        return std::char_traits<char_type>::compare(a.name().c_str(),b,len) < 0;
    }
};

template <class ValueT>
class member_compare
{
public:
    bool operator()(const ValueT& a, const ValueT& b) const
    {
        return a.name() < b.name();
    }
};

template <class JsonT>
class json_array 
{
public:
    typedef typename JsonT::char_type char_type;
    typedef JsonT& reference; 
    typedef const JsonT& const_reference; 
    typedef typename std::vector<JsonT>::iterator iterator;
    typedef typename std::vector<JsonT>::const_iterator const_iterator;

    // Allocation
    //static void* operator new(std::size_t) { return typename Alloc::template rebind<json_array>::other().allocate(1); }
    //static void operator delete(void* ptr) { return typename Alloc::template rebind<json_array>::other().deallocate(static_cast<json_array*>(ptr), 1); }

    json_array()
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

    json_array(size_t n)
        : elements_(n)
    {
    }

    json_array(size_t n, const JsonT& val)
        : elements_(n,val)
    {
    }

    /*json_array(std::vector<JsonT> elements)
        : elements_(elements)
    {
    }*/

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    void swap(json_array<JsonT>& val)
    {
        elements_.swap(val.elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

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

    bool operator==(const json_array<JsonT>& rhs) const
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
    std::vector<JsonT> elements_;
    json_array& operator=(const json_array<JsonT>&);
};

template <class JsonT>
class json_object_member
{
public:
    typedef typename JsonT::char_type char_type;

    json_object_member()
    {
    }
    json_object_member(const json_object_member& pair)
        : name_(pair.name_), value_(pair.value_)
    {
    }
    json_object_member(json_object_member&& pair)
        : name_(std::move(pair.name_)), value_(std::move(pair.value_))
    {
        //name_.swap(pair.name_);
        //value_.swap(pair.value_);
    }
    json_object_member(const std::basic_string<char_type>& name, const JsonT& value)
        : name_(name), value_(value)
    {
    }

    json_object_member(std::basic_string<char_type>&& name, JsonT&& value)
        : name_(std::move(name)), value_(std::move(value))
    {
    }

    json_object_member(std::basic_string<char_type>&& name, const JsonT& value)
        : name_(std::move(name)), value_(value)
    {
    }

    json_object_member(const std::basic_string<char_type>& name, JsonT&& value)
        : name_(name), value_(std::move(std::move(value)))
    {
    }

    const std::basic_string<char_type>& name() const
    {
        return name_;
    }

    JsonT& value()
    {
        return value_;
    }

    const JsonT& value() const
    {
        return value_;
    }

    void swap(json_object_member& pair)
    {
        name_.swap(pair.name_);
        value_.swap(pair.value_);
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

private:
    std::basic_string<char_type> name_;
    JsonT value_;
};

template <class JsonT>
class json_object
{
public:
    typedef json_object_member<JsonT> value_type;
    typedef typename JsonT::char_type char_type;
    typedef typename JsonT::allocator_type allocator_type;
    typedef value_type& reference; 
    typedef const value_type& const_reference; 
    typedef typename std::vector<value_type>::iterator iterator;
    typedef typename std::vector<value_type>::const_iterator const_iterator;

    // Allocation
    static void* operator new(std::size_t) { return allocator_type::rebind<json_object>::other().allocate(1); }
    static void operator delete(void* ptr) { return allocator_type::rebind<json_object>::other().deallocate(static_cast<json_object*>(ptr), 1); }

    json_object()
    {
    }

    json_object(const json_object& val)
        : members_(val.members_)
    {
    }

    json_object(json_object&& val)
        : members_(std::move(val.members_))
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

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<value_type> members)
        : members_(members)
    {
    }

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void reserve(size_t n) {members_.reserve(n);}

    iterator find(char_type const * name)
    {
        size_t length = std::char_traits<char_type>::length(name);
        compare_with_cstring<value_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    const_iterator find(char_type const * name) const
    {
        size_t length = std::char_traits<char_type>::length(name);
        compare_with_cstring<value_type> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    iterator find(const std::basic_string<char_type>& name)
    {
        compare_with_string<value_type> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }
 
    // Fixed by cperthuis
    const_iterator find(const std::basic_string<char_type>& name) const
    {
        compare_with_string<value_type> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
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
        compare_with_string<value_type> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
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
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        if (it == members_.end())
        {
            members_.push_back(value_type(name, value));
        }
        else if (it->name() == name)
        {
            *it = value_type(name,value);
        }
        else
        {
            members_.insert(it,value_type(name,value));
        }
    }

    void set(std::basic_string<char_type>&& name, const JsonT& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), value));
        }
        else if (it->name() == name)
        {
            *it = value_type(std::move(name),value);
        }
        else
        {
            members_.insert(it,value_type(std::move(name),value));
        }
    }

    void set(const std::basic_string<char_type>& name, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        if (it == members_.end())
        {
            members_.push_back(value_type(name, std::move(value)));
        }
        else if (it->name() == name)
        {
            *it = value_type(name,std::move(value));
        }
        else
        {
            members_.insert(it,value_type(name,std::move(value)));
        }
    }

    void set(std::basic_string<char_type>&& name, JsonT&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), std::move(value)));
        }
        else if (it->name() == name)
        {
            *it = value_type(std::move(name),std::move(value));
        }
        else
        {
            members_.insert(it,value_type(std::move(name),std::move(value)));
        }
    }

    iterator set(iterator hint, const std::basic_string<char_type>& name, const JsonT& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name ,compare_with_string<value_type>());
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        }
        
        if (it == members_.end())
        {
            members_.push_back(value_type(name, value));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            *it = value_type(name,value);
        }
        else
        {
           it = members_.insert(it,value_type(name,value));
        }
        return it;
    }

    iterator set(iterator hint, std::basic_string<char_type>&& name, const JsonT& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name ,compare_with_string<value_type>());
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), value));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            *it = value_type(std::move(name),value);
        }
        else
        {
            it = members_.insert(it,value_type(std::move(name),value));
        }
        return it;
    }

    iterator set(iterator hint, const std::basic_string<char_type>& name, JsonT&& value)
    {
        iterator it;
        if (hint != end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name ,compare_with_string<value_type>());
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(name, std::move(value)));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            *it = value_type(name,std::move(value));
        }
        else
        {
            it = members_.insert(it,value_type(name,std::move(value)));
        }
        return it;
    }

    iterator set(iterator hint, std::basic_string<char_type>&& name, JsonT&& value)
    {
        iterator it;
        if (hint != members_.end() && hint->name() <= name)
        {
            it = std::lower_bound(hint,end(),name ,compare_with_string<value_type>());
        }
        else
        {
            it = std::lower_bound(members_.begin(),members_.end(),name ,compare_with_string<value_type>());
        }

        if (it == members_.end())
        {
            members_.push_back(value_type(std::move(name), std::move(value)));
            it = members_.end();
        }
        else if (it->name() == name)
        {
            *it = value_type(std::move(name),std::move(value));
        }
        else
        {
            it = members_.insert(it,value_type(std::move(name),std::move(value)));
        }
        return it;
    }

    void push_back(std::basic_string<char_type>&& name, JsonT&& val)
    {
        members_.push_back(value_type(std::move(name), std::move(val)));
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

    JsonT& get(char_type const * name) 
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

    bool operator==(const json_object<JsonT>& rhs) const
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

private:

    std::vector<value_type> members_;
    json_object<JsonT>& operator=(const json_object<JsonT>&);
};



}

#endif
