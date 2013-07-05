// Copyright 2013 Daniel Parker
// Distributed under Boost license

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

template <class Char>
class key_compare
{
public:
    bool operator()(const std::pair<std::basic_string<Char>,basic_json<Char>>& a, 
                    const std::basic_string<Char>& b) const
    {
        return a.first < b;
    }
};

template <class Char>
class member_compare
{
public:
    bool operator()(const std::pair<std::basic_string<Char>,basic_json<Char>>& a, 
                    const std::pair<std::basic_string<Char>,basic_json<Char>>& b) const
    {
        return a.first < b.first;
    }
};

template <class Char>
class json_array 
{
public:
    typedef typename std::vector<basic_json<Char>>::iterator iterator;
    typedef typename std::vector<basic_json<Char>>::const_iterator const_iterator;

    json_array()
    {
    }

    json_array(size_t n)
        : elements_(n)
    {
    }
    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    json_array<Char>* clone() 
    {
        return new json_array(elements_.begin(),elements_.end());
    }

    ~json_array()
    {
    }

    size_t size() const {return elements_.size();}

    void clear() {elements_.clear();}

    void reserve(size_t n) {elements_.reserve(n);}

    basic_json<Char>& at(size_t i) {return elements_[i];}

    const basic_json<Char>& at(size_t i) const {return elements_[i];}

    void push_back(basic_json<Char> value);

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    std::vector<basic_json<Char>> elements_;
};

template <class Char>
class json_object
{
public:
    typedef typename std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>>::iterator iterator;
    typedef typename std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>>::const_iterator const_iterator;

    json_object()
    {
    }

    json_object(size_t n)
        : members_(n)
    {
    }

    json_object(std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>> members)
        : members_(members)
    {
    }

    ~json_object()
    {
    }

    json_object<Char>* clone() 
    {
        std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>> members(members_.size());
        for (size_t i = 0; i < members_.size(); ++i)
        {
            
            members[i] = std::pair<std::basic_string<Char>,basic_json<Char>>(members_[i].first,members_[i].second);
        }
        return new json_object(members);
    }

    size_t size() const {return members_.size();}

    void clear() {members_.clear();}

    void reserve(size_t n) {members_.reserve(n);}

    const std::pair<std::basic_string<Char>,basic_json<Char>>& get(size_t i) const 
    {
        return members_[i];
    }

    basic_json<Char>& at(size_t i) {return members_[i].value_;}

    const basic_json<Char>& at(size_t i) const {return members_[i].value_;}

    void insert_member(const std::basic_string<Char>& name, const basic_json<Char>& value);

    void insert_member(std::basic_string<Char>&& name, basic_json<Char>&& value);

    void remove(iterator at); 

    basic_json<Char>& get(const std::basic_string<Char>& name);

    const basic_json<Char>& get(const std::basic_string<Char>& name) const;

    iterator find(const std::basic_string<Char>& name);

    const_iterator find(const std::basic_string<Char>& name) const;

    void insert(const_iterator it, std::pair<std::basic_string<Char>,basic_json<Char>> member);

    void push_back(std::pair<std::basic_string<Char>,basic_json<Char>> member)
    {
        members_.push_back(member);
    }

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>> members_;
};

template <class Char>
void json_array<Char>::push_back(basic_json<Char> value)
{
    elements_.push_back(value);
}

template <class Char>
void json_object<Char>::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare<Char>());
}

template <class Char>
void json_object<Char>::insert(const_iterator it, std::pair<std::basic_string<Char>,basic_json<Char>> member)
{
    members_.insert(it,member);
}

template <class Char>
void json_object<Char>::remove(iterator at)
{
    members_.erase(at);
}

template <class Char>
void json_object<Char>::insert_member(const std::basic_string<Char>& name, const basic_json<Char>& value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char>());
    if (it != end() && (*it).first == name)
    {
        remove(it);
    }
    insert(it,std::pair<std::basic_string<Char>,basic_json<Char>>(name,value));
}

template <class Char>
void json_object<Char>::insert_member(std::basic_string<Char>&& name, basic_json<Char>&& value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char>());
    if (it != end() && (*it).first == name)
    {
        remove(it);
    }
    insert(it,std::pair<std::basic_string<Char>,basic_json<Char>>(name,value));
}

template <class Char>
basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) 
{
    auto it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return (*it).second;
}

template <class Char>
const basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) const
{
    const_iterator it = find(name);
    if (it == end())
    {
        JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
    }
    return (*it).value_;
}

template <class Char>
typename json_object<Char>::iterator json_object<Char>::find(const std::basic_string<Char>& name)
{
    key_compare<Char> comp;
    iterator it = std::lower_bound(begin(),end(), name, comp);
    return (it != end() && it->first == name) ? it : end();
}

template <class Char>
typename json_object<Char>::const_iterator json_object<Char>::find(const std::basic_string<Char>& name) const
{
    key_compare<Char> comp;
    const_iterator it = std::lower_bound(begin(),end(),name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}



}

#endif
