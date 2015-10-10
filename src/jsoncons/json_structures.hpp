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
    bool operator()(const std::pair<std::basic_string<Char>,basic_json<Char,Alloc>>& a, 
                    const std::basic_string<Char>& b) const
    {
        return a.first < b;
    }
};

template <typename Char,class Alloc>
class member_compare
{
public:
    bool operator()(const std::pair<std::basic_string<Char>,basic_json<Char,Alloc>>& a, 
                    const std::pair<std::basic_string<Char>,basic_json<Char,Alloc>>& b) const
    {
        return a.first < b.first;
    }
};

template <typename Char,class Alloc>
class json_array_impl 
{
public:
    typedef typename std::vector<basic_json<Char,Alloc>>::iterator iterator;
    typedef typename std::vector<basic_json<Char,Alloc>>::const_iterator const_iterator;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<json_array_impl>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<json_array_impl>::other().deallocate(static_cast<json_array_impl*>(ptr), 1); }

    json_array_impl()
    {
    }

    json_array_impl(size_t n)
        : elements_(n)
    {
    }

    json_array_impl(size_t n, const basic_json<Char,Alloc>& val)
        : elements_(n,val)
    {
    }

    json_array_impl(std::vector<basic_json<Char,Alloc>> elements)
        : elements_(elements)
    {
    }

    template <class InputIterator>
    json_array_impl(InputIterator begin, InputIterator end)
        : elements_(begin,end)
    {
    }

    json_array_impl<Char,Alloc>* clone() 
    {
        return new json_array_impl(elements_);
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
        json_array_impl<Char,Alloc>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    void push_back(basic_json<Char,Alloc>&& value)
    {
        elements_.push_back(value);
    }

    void add(size_t index, basic_json<Char,Alloc>&& value)
    {
        json_array_impl<Char,Alloc>::iterator position = index < elements_.size() ? elements_.begin() + index : elements_.end();
        elements_.insert(position, value);
    }

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array_impl<Char,Alloc>& rhs) const
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

    json_array_impl(const json_array_impl<Char,Alloc>& val)
        : elements_(val.elements_)
    {
    }
private:
    std::vector<basic_json<Char,Alloc>> elements_;
    json_array_impl& operator=(const json_array_impl<Char,Alloc>&);
};

template <typename Char, typename Alloc, bool IsConst = false>
class object_iterator
{
    typedef typename basic_json<Char,Alloc>::member_type value_types;
    typedef std::ptrdiff_t difference_type;
    typedef typename std::conditional<IsConst, const typename basic_json<Char,Alloc>::member_type*, typename basic_json<Char,Alloc>::member_type*>::type pointer;
    typedef typename std::conditional<IsConst, const typename basic_json<Char,Alloc>::member_type&, typename basic_json<Char,Alloc>::member_type&>::type reference;
    typedef std::bidirectional_iterator_tag  iterator_category;
    typedef typename std::vector<std::pair<std::basic_string<Char>,basic_json<Char,Alloc>>>::iterator iterator_impl;
    class deref_proxy;
    friend class deref_proxy;

    class deref_proxy
    {
    public:
      deref_proxy(object_iterator const * it)
        : it_(it)
      {}

    public:
        const std::basic_string<Char>& name() const
        {
            return (it_->it_)->first;
        }

        basic_json<Char,Alloc>& value() const
        {
            return (it_->it_)->second;
        }

        operator value_types() const
        {
            return value_types(name(),value());
        }

        void operator =(value_types const& value)
        {
          it_->invoke_(value);
        }

        deref_proxy const * operator->() const
        {
            return this;
        }
    private:
      object_iterator const * const it_;

    // Not to be implemented
    private:
      void operator =(deref_proxy const &);
    };
public:
    object_iterator(iterator_impl it)
        : it_(it)
    {
    }

    object_iterator(const object_iterator<Char,Alloc,false>& it)
        : it_(it.it_)
    {
    }

    object_iterator& operator=(object_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    object_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    object_iterator operator++(int) // postfix increment
    {
        object_iterator temp(*this);
        ++it_;
        return temp;
    }

    object_iterator& operator--()
    {
        --it_;
        return *this;
    }

    object_iterator operator--(int)
    {
        object_iterator temp(*this);
        --it_;
        return temp;
    }

    deref_proxy operator*() const
    {
        return deref_proxy(this);
    }

    deref_proxy operator->() const
    {
        return deref_proxy(this);
    }

    friend bool operator==(const object_iterator& it1, const object_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const object_iterator& it1, const object_iterator& it2)
    {
        return it1.it_ != it2.it_;
    }
    friend void swap(object_iterator& lhs, object_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
    }
//private:

    iterator_impl it_;
    typename basic_json<Char,Alloc>::member_type member_;

    void invoke_(value_types const & value)
    {
        *it_ = value;
    }
};

template <typename Char,class Alloc>
class json_object_impl
{
public:
    typedef object_iterator<Char,Alloc,false> iterator;
    typedef object_iterator<Char,Alloc,true> const_iterator;
	typedef std::pair<std::basic_string<Char>,basic_json<Char,Alloc>> member_type;
    typedef typename std::vector<member_type>::iterator internal_iterator;
    typedef typename std::vector<member_type>::const_iterator const_internal_iterator;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<json_object_impl>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<json_object_impl>::other().deallocate(static_cast<json_object_impl*>(ptr), 1); }

    json_object_impl()
    {
    }

    json_object_impl(size_t n)
        : members_(n)
    {
    }

    json_object_impl(std::vector<member_type> members)
        : members_(members)
    {
    }

    json_object_impl<Char,Alloc>* clone() 
    {
        return new json_object_impl(members_);
    }

    size_t size() const {return members_.size();}

    size_t capacity() const {return members_.capacity();}

    void clear() {members_.clear();}

    void reserve(size_t n) {members_.reserve(n);}

    iterator find(const std::basic_string<Char>& name)
    {
        key_compare<Char,Alloc> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->first == name) ? iterator(it) : end();
    }
 
    // Fixed by cperthuis
    const_iterator find(const std::basic_string<Char>& name) const
    {
        key_compare<Char,Alloc> comp;
        auto it = std::lower_bound(members_.begin(),members_.end(), name, comp);
        return (it != members_.end() && it->first == name) ? const_iterator(it) : end();
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
        if (it != members_.end() && it->first == name)
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
        if (it != members_.end() && it->first == name)
        {
            *it = member_type(name,value);
        }
        else
        {
            members_.insert(it,member_type(name,value));
        }
    }

    void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)
    {
        auto it = std::lower_bound(members_.begin(),members_.end(),name ,key_compare<Char,Alloc>());
        if (it != members_.end() && it->first == name)
        {
            *it = member_type(name,value);
        }
        else
        {
            members_.insert(it,member_type(name,value));
        }
    }

    /*void push_back(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& val)
    {
        members_.push_back(member_type(name,val));
    }*/

    void push_back(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& val)
    {
        members_.push_back(member_type());
        members_.back().first.swap(name);
        members_.back().second.swap(val);
        //members_.push_back(typename basic_json<Char,Alloc>::member_type(name,val)); // much slower on VS 2010
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

    const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const
    {
        auto it = find(name);
        if (it == end())
        {
            JSONCONS_THROW_EXCEPTION_1("Member %s not found.",name);
        }
        return it->second;
    }

	void sort_members()
	{
		std::sort(members_.begin(),members_.end(),member_compare<Char,Alloc>());
	}

    iterator begin() {return iterator(members_.begin());}

    iterator end() {return iterator(members_.end());}

    const_iterator begin() const {return const_iterator(members_.begin());}

    const_iterator end() const {return const_iterator(members_.end());}

    bool operator==(const json_object_impl<Char,Alloc>& rhs) const
    {
        if (size() != rhs.size())
        {
            return false;
        }
        for (auto it = members_.begin(); it != members_.end(); ++it)
        {

            auto rhs_it = std::lower_bound(rhs.members_.begin(), rhs.members_.end(), *it, member_compare<Char, Alloc>());
            // member_compare actually only compares keys, so we need to check the value separately
            if (rhs_it == rhs.members_.end() || rhs_it->first != it->first || rhs_it->second != it->second)
            {
                return false;
            }
        }
        return true;
    }

    json_object_impl(const json_object_impl<Char,Alloc>& val)
        : members_(val.members_)
    {
    }

private:

    std::vector<member_type> members_;
    json_object_impl<Char,Alloc>& operator=(const json_object_impl<Char,Alloc>&);
};



}

#endif
