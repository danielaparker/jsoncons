// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_LOCATION_HPP
#define JSONCONS_JSON_LOCATION_HPP

#include <string>
#include <vector>
#include <functional>
#include <algorithm> // std::reverse
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons/json_type.hpp>

namespace jsoncons { 

    enum class location_element_kind { root, name, index };

    template <class CharT,class Allocator>
    class basic_location_element 
    {
    public:
        using char_type = CharT;
        using allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT>;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;
    private:
        location_element_kind node_kind_;
        string_type name_;
        std::size_t index_;

    public:
        basic_location_element(location_element_kind node_kind, 
            const string_type& name, std::size_t index)
            : node_kind_(node_kind), name_(name), index_(index)
        {
        }

        basic_location_element(location_element_kind node_kind, 
            string_type&& name, std::size_t index)
            : node_kind_(node_kind), name_(std::move(name)), index_(index)
        {
        }

        basic_location_element(const basic_location_element& other) = default;

        basic_location_element& operator=(const basic_location_element& other) = default;

        location_element_kind node_kind() const
        {
            return node_kind_;
        }

        const string_type& name() const
        {
            return name_;
        }

        std::size_t index() const 
        {
            return index_;
        }

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == location_element_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_type>{}(name_);

            return h;
        }

        int compare(const basic_location_element& other) const
        {
            int diff = 0;
            if (node_kind_ != other.node_kind_)
            {
                diff = static_cast<int>(node_kind_) - static_cast<int>(other.node_kind_);
            }
            else
            {
                switch (node_kind_)
                {
                    case location_element_kind::root:
                    case location_element_kind::name:
                        diff = name_.compare(other.name_);
                        break;
                    case location_element_kind::index:
                        diff = index_ < other.index_ ? -1 : index_ > other.index_ ? 1 : 0;
                        break;
                    default:
                        break;
                }
            }
            return diff;
        }
    };

    template <class CharT, class Allocator = std::allocator<char>>
    class basic_json_location
    {
    public:
        using char_type = CharT;
        using allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type>;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;
        using location_element_type = basic_location_element<CharT,Allocator>;
        using location_element_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<location_element_type>;
    private:
        allocator_type alloc_;
        std::vector<location_element_type> elements_;
    public:
        using iterator = typename std::vector<location_element_type>::iterator;
        using const_iterator = typename std::vector<location_element_type>::const_iterator;

        iterator begin()
        {
            return elements_.begin();
        }

        iterator end()
        {
            return elements_.end();
        }

        const_iterator begin() const
        {
            return elements_.begin();
        }

        const_iterator end() const
        {
            return elements_.end();
        }

        int compare(const basic_json_location& other) const
        {
            if (this == &other)
            {
               return 0;
            }

            auto it1 = elements_.begin();
            auto it2 = other.elements_.begin();
            while (it1 != elements_.end() && it2 != other.elements_.end())
            {
                int diff = it1->compare(*it2);
                if (diff != 0)
                {
                    return diff;
                }
                ++it1;
                ++it2;
            }
            return (elements_.size() < other.elements_.size()) ? -1 : (elements_.size() == other.elements_.size()) ? 0 : 1;
        }

        std::size_t hash() const
        {

            auto it = elements_.begin();
            std::size_t hash = (*it).hash();
            ++it;

            while (it != elements_.end())
            {
                hash += 17*(*it)->node_hash();
                ++it;
            }

            return hash;
        }

        friend bool operator==(const basic_json_location& lhs, const basic_json_location& rhs) 
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator!=(const basic_json_location& lhs, const basic_json_location& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator<(const basic_json_location& lhs, const basic_json_location& rhs) 
        {
            return lhs.compare(rhs) < 0;
        }
    };

    using json_location = basic_json_location<char>;
    using wjson_location = basic_json_location<wchar_t>;
    using location_element = basic_location_element<char,std::allocator<char>>;
    using wlocation_element = basic_location_element<wchar_t,std::allocator<char>>;

} // namespace jsoncons

#endif
