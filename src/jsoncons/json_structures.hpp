// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_STRUCTURES_HPP
#define JSONCONS_JSON_STRUCTURES_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "jsoncons/json1.hpp"

namespace jsoncons {

template <class Char>
class basic_name_value_pair 
{
public:
    basic_name_value_pair()
    {
    }
    basic_name_value_pair(const std::basic_string<Char>& name, const basic_json<Char>& value)
        : name_(name), value_(value)
    {
    }
    basic_name_value_pair(std::basic_string<Char>&& name, basic_json<Char>&& value)
        : name_(name), value_(value)
    {
    }
    const std::basic_string<Char>& name() const
    {
        return name_;
    }
    const basic_json<Char>& value() const
    {
        return value_;
    }

    std::basic_string<Char> name_;
    basic_json<Char> value_;
};

template <class Char>
class key_compare
{
public:
    bool operator()(const basic_name_value_pair<Char>& a, 
                    const std::basic_string<Char>& b) const
    {
        return a.name_ < b;
    }
};

template <class Char>
class member_compare
{
public:
    bool operator()(const basic_name_value_pair<Char>& a, 
                    const basic_name_value_pair<Char>& b) const
    {
        return a.name_ < b.name_;
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

    basic_json<Char>& at(size_t i) {return elements_[i];}

    const basic_json<Char>& at(size_t i) const {return elements_[i];}

    void push_back(basic_json<Char> value);

    std::vector<basic_json<Char>> elements_;
};

template <class Char>
class json_object
{
public:
    typedef typename std::vector<basic_name_value_pair<Char>>::iterator iterator;
    typedef typename std::vector<basic_name_value_pair<Char>>::const_iterator const_iterator;

    json_object()
    {
    }

    json_object(std::vector<basic_name_value_pair<Char>> members)
        : members_(members)
    {
    }

    ~json_object()
    {
    }

    json_object<Char>* clone() 
    {
        std::vector<basic_name_value_pair<Char>> members(members_.size());
        for (size_t i = 0; i < members_.size(); ++i)
        {
            
            members[i] = basic_name_value_pair<Char>(members_[i].name_,members_[i].value_);
        }
        return new json_object(members);
    }

    size_t size() const {return members_.size();}

    const basic_name_value_pair<Char>& get(size_t i) const 
    {
        return members_[i];
    }

    basic_json<Char>& at(size_t i) {return members_[i].value_;}

    const basic_json<Char>& at(size_t i) const {return members_[i].value_;}

    void set_member(const std::basic_string<Char>& name, basic_json<Char> value);

    void remove(iterator at); 

    basic_json<Char>& get(const std::basic_string<Char>& name);

    const basic_json<Char>& get(const std::basic_string<Char>& name) const;

    iterator find(const std::basic_string<Char>& name);

    const_iterator find(const std::basic_string<Char>& name) const;

    void insert(const_iterator it, basic_name_value_pair<Char> member);

    void push_back(basic_name_value_pair<Char> member)
    {
        members_.push_back(member);
    }

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    std::vector<basic_name_value_pair<Char>> members_;
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
void json_object<Char>::insert(const_iterator it, basic_name_value_pair<Char> member)
{
    members_.insert(it,member);
}

template <class Char>
void json_object<Char>::remove(iterator at)
{
    members_.erase(at);
}

template <class Char>
void json_object<Char>::set_member(const std::basic_string<Char>& name, basic_json<Char> value)
{
    iterator it = std::lower_bound(begin(),end(),name ,key_compare<Char>());
    if (it != end() && (*it).name_ == name)
    {
        remove(it);
    }
    insert(it,basic_name_value_pair<Char>(name,value));
}

template <class Char>
basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) 
{
    iterator it = find(name);
    JSONCONS_ASSERT((it != end()));
    return (*it).value_;
}

template <class Char>
const basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) const
{
    iterator it = find(name);
    JSONCONS_ASSERT((it != end()));
    return (*it).value_;
}

template <class Char>
typename json_object<Char>::iterator json_object<Char>::find(const std::basic_string<Char>& name)
{
    key_compare<Char> comp;
    iterator it = std::lower_bound(begin(),end(), name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}

template <class Char>
typename json_object<Char>::const_iterator json_object<Char>::find(const std::basic_string<Char>& name) const
{
    key_compare comp;
    const_iterator it = std::lower_bound(begin(),end(),name, comp);
    return (it != end() && it->name_ == name) ? it : end();
}

}

#endif
