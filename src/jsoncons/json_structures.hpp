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
#include "jsoncons/json1.hpp"

namespace jsoncons {

template <typename Char,class Alloc>
class key_compare
{
public:
    bool operator()(const typename basic_json<Char,Alloc>::member_type& a, 
                    const std::basic_string<Char>& b) const
    {
        return a.name() < b;
    }
};

template <typename Char,class Alloc>
class key_compare2
{
    size_t length_;
public:
    key_compare2(size_t length)
        : length_(length)
    {
    }

    bool operator()(const typename basic_json<Char,Alloc>::member_type& a, 
                    Char const * b) const
    {
        size_t len = std::min JSONCONS_NO_MACRO_EXP(a.name().length(),length_);
        return std::char_traits<Char>::compare(a.name().c_str(),b,len) < 0;
    }
};

template <typename Char,class Alloc>
class member_compare
{
public:
    bool operator()(const typename basic_json<Char,Alloc>::member_type& a, 
                    const typename basic_json<Char,Alloc>::member_type& b) const
    {
        return a.name() < b.name();
    }
};

template <typename Char,class Alloc>
class json_array 
{
public:
    typedef basic_json<Char,Alloc>& reference; 
    typedef const basic_json<Char,Alloc>& const_reference; 
    typedef typename std::vector<basic_json<Char,Alloc>>::iterator iterator;
    typedef typename std::vector<basic_json<Char,Alloc>>::const_iterator const_iterator;

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

    json_array(size_t n, const basic_json<Char,Alloc>& val)
        : elements_(n,val)
    {
    }

    /*json_array(std::vector<basic_json<Char,Alloc>> elements)
        : elements_(elements)
    {
    }*/

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    void swap(json_array<Char,Alloc>& val)
    {
        elements_.swap(val.elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

    void reserve(size_t n) {elements_.reserve(n);}

    void resize(size_t n) {elements_.resize(n);}

    void resize(size_t n, const basic_json<Char,Alloc>& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    basic_json<Char,Alloc>& at(size_t i) {return elements_[i];}

    const basic_json<Char,Alloc>& at(size_t i) const {return elements_[i];}

    void push_back(const basic_json<Char,Alloc>& value)
    {
        elements_.push_back(value);
    }

    void push_back(basic_json<Char,Alloc>&& value)
    {
        elements_.push_back(std::move(value));
    }

    void add(size_t index, const basic_json<Char,Alloc>& value)
    {
        auto position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void add(size_t index, basic_json<Char,Alloc>&& value)
    {
        auto it = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(it, std::move(value));
    }

    void add(const_iterator pos, const basic_json<Char,Alloc>& value)
    {
        elements_.insert(pos, value);
    }

    void add(const_iterator pos, basic_json<Char,Alloc>&& value)
    {
        elements_.insert(pos, std::move(value));
    }

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<Char,Alloc>& rhs) const
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
    std::vector<basic_json<Char,Alloc>> elements_;
    json_array& operator=(const json_array<Char,Alloc>&);
};

template <typename Char,class Alloc>
class json_object
{
public:
    typedef typename basic_json<Char,Alloc>::member_type member_type;
    typedef member_type& reference; 
    typedef const member_type& const_reference; 
    typedef typename std::vector<member_type>::iterator iterator;
    typedef typename std::vector<member_type>::const_iterator const_iterator;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<json_object>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<json_object>::other().deallocate(static_cast<json_object*>(ptr), 1); }

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

    void swap(json_object& val)
    {
        members_.swap(val.members_);
    }

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<member_type> members)
        : members_(members)
    {
    }

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void reserve(size_t n) {members_.reserve(n);}

    iterator find(Char const * name)
    {
        size_t length = std::char_traits<Char>::length(name);
        key_compare2<Char,Alloc> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    const_iterator find(Char const * name) const
    {
        size_t length = std::char_traits<Char>::length(name);
        key_compare2<Char,Alloc> comp(length);
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    iterator find(const std::basic_string<Char>& name)
    {
        key_compare<Char,Alloc> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }
 
    // Fixed by cperthuis
    const_iterator find(const std::basic_string<Char>& name) const
    {
        key_compare<Char,Alloc> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->name() == name) ? it : end();
    }

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= members_.size());
        members_.erase(members_.begin()+from_index,members_.begin()+to_index);
    }

    // Fixed by cperthuis
    void remove(const std::basic_string<Char>& name) 
    {
        key_compare<Char,Alloc> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        if (it != members_.end() && it->name() == name)
        {
            members_.erase(it);
        }
    }

    const typename basic_json<Char,Alloc>::member_type& get(size_t i) const 
    {
        return members_[i];
    }

    void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,key_compare<Char,Alloc>());
        if (it != members_.end() && it->name() == name)
        {
            *it = member_type(name,value);
        }
        else
        {
            members_.insert(it,member_type(name,value));
        }
    }

    void set(std::basic_string<Char>&& name, const basic_json<Char,Alloc>& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,key_compare<Char,Alloc>());
        if (it != members_.end() && it->name() == name)
        {
            *it = member_type(std::move(name),value);
        }
        else
        {
            members_.insert(it,member_type(std::move(name),value));
        }
    }

    void set(const std::basic_string<Char>& name, basic_json<Char,Alloc>&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,key_compare<Char,Alloc>());
        if (it != members_.end() && it->name() == name)
        {
            *it = member_type(name,std::move(value));
        }
        else
        {
            members_.insert(it,member_type(name,std::move(value)));
        }
    }

    void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,key_compare<Char,Alloc>());
        if (it != members_.end() && it->name() == name)
        {
            *it = member_type(std::move(name),std::move(value));
        }
        else
        {
            members_.insert(it,member_type(std::move(name),std::move(value)));
        }
    }

    /*void push_back(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& val)
    {
        members_.push_back(member_type(name,val));
    }*/

    void push_back(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& val)
    {
        members_.push_back(member_type(std::move(name), std::move(val)));
        //members_.back().name().swap(std::move(name));
        //members_.back().value().swap(std::move(val));
        //members_.push_back(std::pair<std::basic_string<Char>, basic_json<Char, Alloc>>(std::move(name),
        //    std::move(val))); // much slower on VS 2010
    }

    basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) 
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
        }
        return it->value();
    }

    basic_json<Char,Alloc>& get(Char const * name) 
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
        }
        return it->value();
    }

    const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
        }
        return it->value();
    }

    const basic_json<Char,Alloc>& get(Char const * name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
        }
        return it->value();
    }

	void sort_members()
	{
		std::sort(members_.begin(),members_.end(),member_compare<Char,Alloc>());
	}

    iterator begin() {return iterator(members_.begin());}

    iterator end() {return iterator(members_.end());}

    const_iterator begin() const {return const_iterator(members_.begin());}

    const_iterator end() const {return const_iterator(members_.end());}

    bool operator==(const json_object<Char,Alloc>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_compare<Char, Alloc>());
            // member_compare actually only compares keys, so we need to check the value separately
            if (rhs_it == rhs.members_.end() || rhs_it->name() != it->name() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }

private:

    std::vector<member_type> members_;
    json_object<Char,Alloc>& operator=(const json_object<Char,Alloc>&);
};



}

#endif
