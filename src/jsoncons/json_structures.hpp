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

template <class C>
class key_compare
{
public:
    bool operator()(const std::pair<std::basic_string<C>,basic_json<C>>& a, 
                    const std::basic_string<C>& b) const
    {
        return a.first < b;
    }
};

template <class C>
class member_compare
{
public:
    bool operator()(const std::pair<std::basic_string<C>,basic_json<C>>& a, 
                    const std::pair<std::basic_string<C>,basic_json<C>>& b) const
    {
        return a.first < b.first;
    }
};

template <class C>
class json_array 
{
public:
    typedef typename std::vector<basic_json<C>>::iterator iterator;
    typedef typename std::vector<basic_json<C>>::const_iterator const_iterator;

    json_array()
    {
    }

    json_array(size_t n)
        : elements_(n)
    {
    }

    json_array(size_t n, const basic_json<C>& val)
        : elements_(n,val)
    {
    }

    json_array(std::vector<basic_json<C>> elements)
        : elements_(elements)
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    json_array<C>* clone() 
    {
        return new json_array(elements_);
    }

    size_t size() const {return elements_.size();}

    size_t capacity() const {return elements_.capacity();}

    void clear() {elements_.clear();}

    void reserve(size_t n) {elements_.reserve(n);}

    void resize(size_t n) {elements_.resize(n);}

    void resize(size_t n, const basic_json<C>& val) {elements_.resize(n,val);}

    void remove_range(size_t from_index, size_t to_index) 
    {
        JSONCONS_ASSERT(from_index <= to_index);
        JSONCONS_ASSERT(to_index <= elements_.size());
        elements_.erase(elements_.begin()+from_index,elements_.begin()+to_index);
    }

    basic_json<C>& at(size_t i) {return elements_[i];}

    const basic_json<C>& at(size_t i) const {return elements_[i];}

    void push_back(const basic_json<C>& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, const basic_json<C>& value)
    {
        json_array<C>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

#ifndef JSONCONS_NO_CXX11_RVALUE_REFERENCES

    void push_back(basic_json<C>&& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, basic_json<C>&& value)
    {
        json_array<C>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

#endif

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<C>& rhs) const
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
    std::vector<basic_json<C>> elements_;
    json_array(const json_array<C>&);
    json_array& operator=(const json_array<C>&);
};

template <class C>
class json_object
{
public:
    typedef typename std::vector<std::pair<std::basic_string<C>,basic_json<C>>>::iterator iterator;
    typedef typename std::vector<std::pair<std::basic_string<C>,basic_json<C>>>::const_iterator const_iterator;

    json_object()
    {
    }

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<std::pair<std::basic_string<C>,basic_json<C>>> members)
        : members_(members)
    {
    }

    json_object<C>* clone() 
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

    void remove(const std::basic_string<C>& name) 
    {
        iterator it = find(name);
        if (it != members_.end())
        {
            members_.erase(it);
        }
    }

    const std::pair<std::basic_string<C>,basic_json<C>>& get(size_t i) const 
    {
        return members_[i];
    }

    basic_json<C>& at(size_t i) {return members_[i].second;}

    const basic_json<C>& at(size_t i) const {return members_[i].second;}

    void set(const std::basic_string<C>& name, const basic_json<C>& value);

    void push_back(const std::basic_string<C>& name, const basic_json<C>& val)
    {
        members_.push_back(std::pair<std::basic_string<C>,basic_json<C>>(name,val));
    }

#ifndef JSONCONS_NO_CXX11_RVALUE_REFERENCES

    void set(std::basic_string<C>&& name, basic_json<C>&& value)
    {
        iterator it = std::lower_bound(begin(),end(),name ,key_compare<C>());
        if (it != end() && (*it).first == name)
        {
            //it = remove(it);
            *it = std::pair<std::basic_string<C>,basic_json<C>>(name,value);
        }
        else
        {
            insert(it,std::pair<std::basic_string<C>,basic_json<C>>(name,value));
        }
    }

    void push_back(std::basic_string<C>&& name, basic_json<C>&& val)
    {
        members_.push_back(std::pair<std::basic_string<C>,basic_json<C>>());
        members_.back().first.swap(name);
        members_.back().second.swap(val);
    }
#endif

    iterator remove(iterator at); 

    basic_json<C>& get(const std::basic_string<C>& name);

    const basic_json<C>& get(const std::basic_string<C>& name) const;

    iterator find(const std::basic_string<C>& name);

    const_iterator find(const std::basic_string<C>& name) const;

    void insert(const_iterator it, std::pair<std::basic_string<C>,basic_json<C>> member);

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    bool operator==(const json_object<C>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (const_iterator it = members_.begin(); it != members_.end(); ++it)
        {
            bool exists = std::binary_search(rhs.members_.begin(),rhs.members_.end(),*it,member_compare<C>());
            if (!exists)
            {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::pair<std::basic_string<C>,basic_json<C>>> members_;
    json_object(const json_object<C>&);
    json_object<C>& operator=(const json_object<C>&);
};


template <class C>
void json_object<C>::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare<C>());
}

template <class C>
void json_object<C>::insert(const_iterator it, std::pair<std::basic_string<C>,basic_json<C>> member)
{
    members_.insert(it,member);
}

template <class C>
typename json_object<C>::iterator json_object<C>::remove(iterator at)
{
    return members_.erase(at);
}

template <class C>
void json_object<C>::set(const std::basic_string<C>& name, const basic_json<C>& value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<C>());
    if (it != end() && (*it).first == name)
    {
        //it = remove(it);
        *it = std::pair<std::basic_string<C>,basic_json<C>>(name,value);
    }
    else
    {
        insert(it,std::pair<std::basic_string<C>,basic_json<C>>(name,value));
    }
}

template <class C>
basic_json<C>& json_object<C>::get(const std::basic_string<C>& name) 
{
    iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return (*it).second;
}

template <class C>
const basic_json<C>& json_object<C>::get(const std::basic_string<C>& name) const
{
    const_iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return (*it).value_;
}

template <class C>
typename json_object<C>::iterator json_object<C>::find(const std::basic_string<C>& name)
{
    key_compare<C> comp;
    iterator it = std::lower_bound(begin(),end(), name, comp);
    return (it != end() && it->first == name) ? it : end();
}

template <class C>
typename json_object<C>::const_iterator json_object<C>::find(const std::basic_string<C>& name) const
{
    key_compare<C> comp;
    const_iterator it = std::lower_bound(begin(),end(),name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}

}

#endif
