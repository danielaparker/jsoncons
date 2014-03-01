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

template <typename Char,class Storage>
class key_compare
{
public:
    bool operator()(const typename basic_json<Char,Storage>::member_type& a, 
                    const std::basic_string<Char>& b) const
    {
        return a.name() < b;
    }
};

template <typename Char,class Storage>
class member_compare
{
public:
    bool operator()(const typename basic_json<Char,Storage>::member_type& a, 
                    const typename basic_json<Char,Storage>::member_type& b) const
    {
        return a.name() < b.name();
    }
};

template <typename Char,class Storage>
class json_array 
{
public:
    typedef typename std::vector<basic_json<Char,Storage>>::iterator iterator;
    typedef typename std::vector<basic_json<Char,Storage>>::const_iterator const_iterator;

    json_array()
    {
    }

    json_array(size_t n)
        : elements_(n)
    {
    }

    json_array(size_t n, const basic_json<Char,Storage>& val)
        : elements_(n,val)
    {
    }

    json_array(std::vector<basic_json<Char,Storage>> elements)
        : elements_(elements)
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    json_array<Char,Storage>* clone() 
    {
        return new json_array(elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

    void reserve(size_t n) {elements_.reserve(n);}

    void resize(size_t n) {elements_.resize(n);}

    void resize(size_t n, const basic_json<Char,Storage>& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    basic_json<Char,Storage>& at(size_t i) {return elements_[i];}

    const basic_json<Char,Storage>& at(size_t i) const {return elements_[i];}

    void push_back(const basic_json<Char,Storage>& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, const basic_json<Char,Storage>& value)
    {
        json_array<Char,Storage>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void push_back(basic_json<Char,Storage>&& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, basic_json<Char,Storage>&& value)
    {
        json_array<Char,Storage>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<Char,Storage>& rhs) const
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
    std::vector<basic_json<Char,Storage>> elements_;
    json_array(const json_array<Char,Storage>&);
    json_array& operator=(const json_array<Char,Storage>&);
};

template <typename Char,class Storage>
class json_object
{
public:
    typedef typename std::vector<typename basic_json<Char,Storage>::member_type>::iterator iterator;
    typedef typename std::vector<typename basic_json<Char,Storage>::member_type>::const_iterator const_iterator;

    json_object()
    {
    }

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<typename basic_json<Char,Storage>::member_type> members)
        : members_(members)
    {
    }

    json_object<Char,Storage>* clone() 
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

    const typename basic_json<Char,Storage>::member_type& get(size_t i) const 
    {
        return members_[i];
    }

    basic_json<Char,Storage>& at(size_t i) {return members_[i].value();}

    const basic_json<Char,Storage>& at(size_t i) const {return members_[i].value;}

    void set(const std::basic_string<Char>& name, const basic_json<Char,Storage>& value);

    void push_back(const std::basic_string<Char>& name, const basic_json<Char,Storage>& val)
    {
        members_.push_back(typename basic_json<Char,Storage>::member_type(name,val));
    }

    void set(std::basic_string<Char>&& name, basic_json<Char,Storage>&& value)
    {
        iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char,Storage>());
        if (it != end() && it->name() == name)
        {
            //it = remove(it);
            *it = typename basic_json<Char,Storage>::member_type(name,value);
        }
        else
        {
            insert(it,typename basic_json<Char,Storage>::member_type(name,value));
        }
    }

    void push_back(std::basic_string<Char>&& name, basic_json<Char,Storage>&& val)
    {
        members_.push_back(typename basic_json<Char,Storage>::member_type());
        members_.back().name_.swap(name);
        members_.back().value_.swap(val);
        //members_.push_back(typename basic_json<Char,Storage>::member_type(name,val)); // much slower on VS 2010
    }

    iterator remove(iterator at); 

    basic_json<Char,Storage>& get(const std::basic_string<Char>& name);

    const basic_json<Char,Storage>& get(const std::basic_string<Char>& name) const;

    iterator find(const std::basic_string<Char>& name);

    const_iterator find(const std::basic_string<Char>& name) const;

    void insert(iterator it, typename basic_json<Char,Storage>::member_type member);

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    bool operator==(const json_object<Char,Storage>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (const_iterator it = members_.begin(); it != members_.end(); ++it)
        {
            bool exists = std::binary_search(rhs.members_.begin(),rhs.members_.end(),*it,member_compare<Char,Storage>());
            if (!exists)
            {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<typename basic_json<Char,Storage>::member_type> members_;
    json_object(const json_object<Char,Storage>&);
    json_object<Char,Storage>& operator=(const json_object<Char,Storage>&);
};


template <typename Char,class Storage>
void json_object<Char,Storage>::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare<Char,Storage>());
}

template <typename Char,class Storage>
void json_object<Char,Storage>::insert(iterator it, typename basic_json<Char,Storage>::member_type member)
{
    members_.insert(it,member);
}

template <typename Char,class Storage>
typename json_object<Char,Storage>::iterator json_object<Char,Storage>::remove(iterator at)
{
    return members_.erase(at);
}

template <typename Char,class Storage>
void json_object<Char,Storage>::set(const std::basic_string<Char>& name, const basic_json<Char,Storage>& value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char,Storage>());
    if (it != end() && it->name() == name)
    {
        //it = remove(it);
        *it = typename basic_json<Char,Storage>::member_type(name,value);
    }
    else
    {
        insert(it,typename basic_json<Char,Storage>::member_type(name,value));
    }
}

template <typename Char,class Storage>
basic_json<Char,Storage>& json_object<Char,Storage>::get(const std::basic_string<Char>& name) 
{
    iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return it->value();
}

template <typename Char,class Storage>
const basic_json<Char,Storage>& json_object<Char,Storage>::get(const std::basic_string<Char>& name) const
{
    const_iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return it->value_;
}

template <typename Char,class Storage>
typename json_object<Char,Storage>::iterator json_object<Char,Storage>::find(const std::basic_string<Char>& name)
{
    key_compare<Char,Storage> comp;
    iterator it = std::lower_bound(begin(),end(), name, comp);
    return (it != end() && it->name() == name) ? it : end();
}

template <typename Char,class Storage>
typename json_object<Char,Storage>::const_iterator json_object<Char,Storage>::find(const std::basic_string<Char>& name) const
{
    key_compare<Char,Storage> comp;
    const_iterator it = std::lower_bound(begin(),end(),name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}

}

#endif
