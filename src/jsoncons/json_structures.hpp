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
    typedef typename std::vector<basic_json<Char,Alloc>>::iterator iterator;
    typedef typename std::vector<basic_json<Char,Alloc>>::const_iterator const_iterator;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<json_array>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<json_array>::other().deallocate(static_cast<json_array*>(ptr), 1); }

    json_array()
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

    json_array(std::vector<basic_json<Char,Alloc>> elements)
        : elements_(elements)
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    json_array<Char,Alloc>* clone() 
    {
        return new json_array(elements_);
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

    void add(size_t index, const basic_json<Char,Alloc>& value)
    {
        json_array<Char,Alloc>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void push_back(basic_json<Char,Alloc>&& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, basic_json<Char,Alloc>&& value)
    {
        json_array<Char,Alloc>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
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

    json_array(const json_array<Char,Alloc>& val)
        : elements_(val.elements_)
    {
    }
private:
    std::vector<basic_json<Char,Alloc>> elements_;
    json_array& operator=(const json_array<Char,Alloc>&);
};

template <typename Char,class Alloc>
class json_object
{
public:
    typedef typename std::vector<typename basic_json<Char,Alloc>::member_type>::iterator iterator;
    typedef typename std::vector<typename basic_json<Char,Alloc>::member_type>::const_iterator const_iterator;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<json_object>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<json_object>::other().deallocate(static_cast<json_object*>(ptr), 1); }

    json_object()
    {
    }

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<typename basic_json<Char,Alloc>::member_type> members)
        : members_(members)
    {
    }

    json_object<Char,Alloc>* clone() 
    {
        return new json_object(members_);
    }

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void reserve(size_t n) {members_.reserve(n);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= members_.size());
        members_.erase(members_.begin()+from_index,members_.begin()+to_index);
    }

    void remove(const std::basic_string<Char>& name) 
    {
        iterator it = find(name);
        if (it != members_.end())
        {
            members_.erase(it);
        }
    }

    const typename basic_json<Char,Alloc>::member_type& get(size_t i) const 
    {
        return members_[i];
    }

    basic_json<Char,Alloc>& at(size_t i) {return members_[i].value();}

    const basic_json<Char,Alloc>& at(size_t i) const {return members_[i].value;}

    void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value);

    void push_back(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& val)
    {
        members_.push_back(typename basic_json<Char,Alloc>::member_type(name,val));
    }

    void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)
    {
        iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char,Alloc>());
        if (it != end() && it->name() == name)
        {
            //it = remove(it);
            *it = typename basic_json<Char,Alloc>::member_type(name,value);
        }
        else
        {
            insert(it,typename basic_json<Char,Alloc>::member_type(name,value));
        }
    }

    void push_back(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& val)
    {
        members_.push_back(typename basic_json<Char,Alloc>::member_type());
        members_.back().name_.swap(name);
        members_.back().value_.swap(val);
        //members_.push_back(typename basic_json<Char,Alloc>::member_type(name,val)); // much slower on VS 2010
    }

    iterator remove(iterator at); 

    basic_json<Char,Alloc>& get(const std::basic_string<Char>& name);

    const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const;

    iterator find(const std::basic_string<Char>& name);

    const_iterator find(const std::basic_string<Char>& name) const;

    void insert(iterator it, typename basic_json<Char,Alloc>::member_type member);

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    bool operator==(const json_object<Char,Alloc>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (const_iterator it = members_.begin(); it != members_.end(); ++it)
        {

            const_iterator rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_compare<Char, Alloc>());
            // member_compare actually only compares keys, so we need to check the value separately
            if (rhs_it == rhs.end() || rhs_it->value() != it->value())
            {
                return false;
            }
        }
        return true;
    }

    json_object(const json_object<Char,Alloc>& val)
        : members_(val.members_)
    {
    }

private:
    std::vector<typename basic_json<Char,Alloc>::member_type> members_;
    json_object<Char,Alloc>& operator=(const json_object<Char,Alloc>&);
};


template <typename Char,class Alloc>
void json_object<Char,Alloc>::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare<Char,Alloc>());
}

template <typename Char,class Alloc>
void json_object<Char,Alloc>::insert(iterator it, typename basic_json<Char,Alloc>::member_type member)
{
    members_.insert(it,member);
}

template <typename Char,class Alloc>
typename json_object<Char,Alloc>::iterator json_object<Char,Alloc>::remove(iterator at)
{
    return members_.erase(at);
}

template <typename Char,class Alloc>
void json_object<Char,Alloc>::set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char,Alloc>());
    if (it != end() && it->name() == name)
    {
        //it = remove(it);
        *it = typename basic_json<Char,Alloc>::member_type(name,value);
    }
    else
    {
        insert(it,typename basic_json<Char,Alloc>::member_type(name,value));
    }
}

template <typename Char,class Alloc>
basic_json<Char,Alloc>& json_object<Char,Alloc>::get(const std::basic_string<Char>& name) 
{
    iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return it->value();
}

template <typename Char,class Alloc>
const basic_json<Char,Alloc>& json_object<Char,Alloc>::get(const std::basic_string<Char>& name) const
{
    const_iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return it->value_;
}

template <typename Char,class Alloc>
typename json_object<Char,Alloc>::iterator json_object<Char,Alloc>::find(const std::basic_string<Char>& name)
{
    key_compare<Char,Alloc> comp;
    iterator it = std::lower_bound(begin(),end(), name, comp);
    return (it != end() && it->name() == name) ? it : end();
}

template <typename Char,class Alloc>
typename json_object<Char,Alloc>::const_iterator json_object<Char,Alloc>::find(const std::basic_string<Char>& name) const
{
    key_compare<Char,Alloc> comp;
    const_iterator it = std::lower_bound(begin(),end(),name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}

}

#endif
