// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_ARRAY_HPP
#define JSONCONS_JSON_ARRAY_HPP

#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <utility>
#include <initializer_list>
#include <jsoncons/json_text_traits.hpp>
#include <jsoncons/jsoncons_util.hpp>

namespace jsoncons {

template <class Json>
class Json_array_base_
{
public:
    typedef typename Json::allocator_type allocator_type;

public:
    Json_array_base_()
        : self_allocator_()
    {
    }
    Json_array_base_(const allocator_type& allocator)
        : self_allocator_(allocator)
    {
    }

    allocator_type get_self_allocator() const
    {
        return self_allocator_;
    }

    allocator_type self_allocator_;
};

template <class Json>
class json_array : public Json_array_base_<Json>
{
public:
    typedef typename Json::allocator_type allocator_type;
    typedef Json value_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<value_type> val_allocator_type;

    typedef typename Json::base_array_type base_array_type;

    typedef typename base_array_type::iterator iterator;
    typedef typename base_array_type::const_iterator const_iterator;

    typedef typename std::iterator_traits<iterator>::reference reference;
    typedef typename std::iterator_traits<const_iterator>::reference const_reference;

    using Json_array_base_<Json>::get_self_allocator;

    json_array()
        : Json_array_base_(), 
          elements_()
    {
    }

    explicit json_array(const allocator_type& allocator)
        : Json_array_base_(allocator), 
          elements_(val_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const allocator_type& allocator = allocator_type())
        : Json_array_base_(allocator), 
          elements_(n,Json(),val_allocator_type(allocator))
    {
    }

    explicit json_array(size_t n, 
                        const Json& value, 
                        const allocator_type& allocator = allocator_type())
        : Json_array_base_(allocator), 
          elements_(n,value,val_allocator_type(allocator))
    {
    }

    template <class InputIterator>
    json_array(InputIterator begin, InputIterator end, const allocator_type& allocator = allocator_type())
        : Json_array_base_(allocator), 
          elements_(begin,end,val_allocator_type(allocator))
    {
    }
    json_array(const json_array& val)
        : Json_array_base_(val.get_self_allocator()),
          elements_(val.elements_)
    {
    }
    json_array(const json_array& val, const allocator_type& allocator)
        : Json_array_base_(allocator), 
          elements_(val.elements_,val_allocator_type(allocator))
    {
    }

    json_array(json_array&& val) JSONCONS_NOEXCEPT
        : Json_array_base_(val.get_self_allocator()), 
          elements_(std::move(val.elements_))
    {
    }
    json_array(json_array&& val, const allocator_type& allocator)
        : Json_array_base_(allocator), 
          elements_(std::move(val.elements_),val_allocator_type(allocator))
    {
    }

    json_array(std::initializer_list<Json> init)
        : Json_array_base_(), 
          elements_(std::move(init))
    {
    }

    json_array(std::initializer_list<Json> init, 
               const allocator_type& allocator)
        : Json_array_base_(allocator), 
          elements_(std::move(init),val_allocator_type(allocator))
    {
    }
    ~json_array()
    {
    }

    void swap(json_array<Json>& val)
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

    void resize(size_t n, const Json& val) {elements_.resize(n,val);}

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

    Json& operator[](size_t i) {return elements_[i];}

    const Json& operator[](size_t i) const {return elements_[i];}

    template <class T, class U=allocator_type,
             typename std::enable_if<is_stateless<U>::value
                >::type* = nullptr>
    void add(T&& value)
    {
        elements_.emplace_back(Json(std::forward<T&&>(value)));
    }

    template <class T, class U=allocator_type,
             typename std::enable_if<!is_stateless<U>::value
                >::type* = nullptr>
    void add(T&& value)
    {
        elements_.emplace_back(std::forward<T&&>(value),get_self_allocator());
    }

#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
    // work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54577
    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    add(const_iterator pos, T&& value)
    {
        iterator it = elements_.begin() + (pos - elements_.begin());
        return elements_.emplace(it, Json(std::forward<T&&>(value)));
    }
#else
    template <class T, class U=allocator_type>
        typename std::enable_if<is_stateless<U>::value,iterator>::type 
    add(const_iterator pos, T&& value)
    {
        return elements_.emplace(pos, Json(std::forward<T&&>(value)));
    }
#endif

    iterator begin() {return elements_.begin();}

    iterator end() {return elements_.end();}

    const_iterator begin() const {return elements_.begin();}

    const_iterator end() const {return elements_.end();}

    bool operator==(const json_array<Json>& rhs) const
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
    base_array_type elements_;

    json_array& operator=(const json_array<Json>&) = delete;
};

}

#endif
