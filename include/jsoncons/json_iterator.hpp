// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_ITERATOR_HPP
#define JSONCONS_JSON_ITERATOR_HPP

#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <cstring>
#include <algorithm> // std::sort, std::stable_sort, std::lower_bound, std::unique
#include <utility>
#include <initializer_list>
#include <iterator> // std::iterator_traits
#include <memory> // std::allocator
#include <utility> // std::move
#include <type_traits> // std::enable_if
#include <jsoncons/json_exception.hpp>
#include <jsoncons/allocator_holder.hpp>

namespace jsoncons {

    template <class ValueType, class DifferenceType, class Pointer, class Reference>
    struct sequence_iterator_types 
    {
        using value_type      = ValueType;
        using difference_type = DifferenceType;
        using pointer         = Pointer;
        using reference       = Reference;
    };

    // iterator for mutable sequence
    template <class ContainerTypes>
    class sequence_iterator 
    { 
        template <class Cont>
        friend class const_sequence_iterator;

        using Ptr = typename ContainerTypes::pointer;
        Ptr ptr_; 
    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = typename ContainerTypes::value_type;
        using difference_type = typename ContainerTypes::difference_type;
        using pointer = typename ContainerTypes::pointer;
        using reference = typename ContainerTypes::reference;
    public:
        sequence_iterator() : ptr_() 
        { 
        }

        sequence_iterator(Ptr ptr) : ptr_(ptr) 
        {
        }

        sequence_iterator(const sequence_iterator&) = default;
        sequence_iterator(sequence_iterator&&) = default;
        sequence_iterator& operator=(const sequence_iterator&) = default;
        sequence_iterator& operator=(sequence_iterator&&) = default;

        reference operator*() const 
        {
            return *ptr_;
        }

        pointer operator->() const 
        {
            return ptr_;
        }

        sequence_iterator& operator++() 
        {
            ++ptr_;
            return *this;
        }

        sequence_iterator operator++(int) 
        {
            sequence_iterator temp = *this;
            ++*this;
            return temp;
        }

        sequence_iterator& operator--() 
        {
            --ptr_;
            return *this;
        }

        sequence_iterator operator--(int) 
        {
            sequence_iterator temp = *this;
            --*this;
            return temp;
        }

        sequence_iterator& operator+=(const difference_type offset) 
        {
            ptr_ += offset;
            return *this;
        }

        sequence_iterator operator+(const difference_type offset) const 
        {
            sequence_iterator temp = *this;
            return temp += offset;
        }

        sequence_iterator& operator-=(const difference_type offset) 
        {
            return *this += -offset;
        }

        sequence_iterator operator-(const difference_type offset) const 
        {
            sequence_iterator temp = *this;
            return temp -= offset;
        }

        difference_type operator-(const sequence_iterator& rhs) const 
        {
            return ptr_ - rhs.ptr_;
        }

        reference operator[](const difference_type offset) const 
        {
            return *(*this + offset);
        }

        bool operator==(const sequence_iterator& rhs) const 
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator!=(const sequence_iterator& rhs) const 
        {
            return !(*this == rhs);
        }

        bool operator<(const sequence_iterator& rhs) const 
        {
            return ptr_ < rhs.ptr_;
        }

        bool operator>(const sequence_iterator& rhs) const 
        {
            return rhs < *this;
        }

        bool operator<=(const sequence_iterator& rhs) const 
        {
            return !(rhs < *this);
        }

        bool operator>=(const sequence_iterator& rhs) const 
        {
            return !(*this < rhs);
        }

        inline 
        friend sequence_iterator<ContainerTypes> operator+(
            difference_type offset, sequence_iterator<ContainerTypes> next) 
        {
            return next += offset;
        }
    };

    // iterator for immutable sequence
    template <class ContainerTypes>
    class const_sequence_iterator 
    { 
        using Ptr = typename ContainerTypes::pointer;
        Ptr ptr_; 
    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = typename ContainerTypes::value_type;
        using difference_type = typename ContainerTypes::difference_type;
        using pointer = typename ContainerTypes::pointer;
        using reference = typename ContainerTypes::reference;
    private:
        using iterator_types = sequence_iterator_types<value_type, 
                                                       difference_type,
                                                       typename std::conditional<std::is_const<typename std::remove_pointer<pointer>::type>::value,typename std::add_pointer<value_type>::type,pointer>::type, 
                                                       typename std::conditional<std::is_const<typename std::remove_pointer<pointer>::type>::value,value_type&,reference>::type>; 

    public:
        const_sequence_iterator() : ptr_() 
        { 
        }

        const_sequence_iterator(Ptr ptr) : ptr_(ptr) 
        {
        }

        const_sequence_iterator(const const_sequence_iterator&) = default;
        const_sequence_iterator(const_sequence_iterator&&) = default;
        const_sequence_iterator& operator=(const const_sequence_iterator&) = default;
        const_sequence_iterator& operator=(const_sequence_iterator&&) = default;

        const_sequence_iterator(const sequence_iterator<iterator_types>& iter)
            : ptr_(iter.ptr_)
        {
        }

        reference operator*() const 
        {
            return *ptr_;
        }

        pointer operator->() const 
        {
            return ptr_;
        }

        const_sequence_iterator& operator++() 
        {
            ++ptr_;
            return *this;
        }

        const_sequence_iterator operator++(int) 
        {
            const_sequence_iterator temp = *this;
            ++*this;
            return temp;
        }

        const_sequence_iterator& operator--() 
        {
            --ptr_;
            return *this;
        }

        const_sequence_iterator operator--(int) 
        {
            const_sequence_iterator temp = *this;
            --*this;
            return temp;
        }

        const_sequence_iterator& operator+=(const difference_type offset) 
        {
            ptr_ += offset;
            return *this;
        }

        const_sequence_iterator operator+(const difference_type offset) const 
        {
            const_sequence_iterator temp = *this;
            return temp += offset;
        }

        const_sequence_iterator& operator-=(const difference_type offset) 
        {
            return *this += -offset;
        }

        const_sequence_iterator operator-(const difference_type offset) const 
        {
            const_sequence_iterator temp = *this;
            return temp -= offset;
        }

        difference_type operator-(const const_sequence_iterator& rhs) const 
        {
            return ptr_ - rhs.ptr_;
        }

        reference operator[](const difference_type offset) const 
        {
            return *(*this + offset);
        }

        bool operator==(const const_sequence_iterator& rhs) const 
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator!=(const const_sequence_iterator& rhs) const 
        {
            return !(*this == rhs);
        }

        bool operator<(const const_sequence_iterator& rhs) const 
        {
            return ptr_ < rhs.ptr_;
        }

        bool operator>(const const_sequence_iterator& rhs) const 
        {
            return rhs < *this;
        }

        bool operator<=(const const_sequence_iterator& rhs) const 
        {
            return !(rhs < *this);
        }

        bool operator>=(const const_sequence_iterator& rhs) const 
        {
            return !(*this < rhs);
        }

        inline 
        friend const_sequence_iterator<ContainerTypes> operator+(
            difference_type offset, const_sequence_iterator<ContainerTypes> next) 
        {
            return next += offset;
        }
    };

} // namespace jsoncons

#endif
