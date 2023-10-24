// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_LOCATION_HPP
#define JSONCONS_JSONPATH_JSON_LOCATION_HPP

#include <string>
#include <vector>
#include <functional>
#include <algorithm> // std::reverse
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_utilities.hpp>

namespace jsoncons { 
namespace jsonpath {

    template <class StringT>
    class value_location; 

    enum class jsonpath_node_kind { root, index, name };

    template <class StringT>
    class jsonpath_node 
    {
        friend class value_location<StringT>;
    public:
        using string_type = StringT;
        using char_type = typename StringT::value_type;
    private:

        const jsonpath_node* parent_;
        jsonpath_node_kind node_kind_;
        jsoncons::optional<string_type> name_;
        std::size_t index_;
    public:
        jsonpath_node(string_type&& name)
            : parent_(nullptr), 
              node_kind_(jsonpath_node_kind::root), 
              name_(std::move(name)), index_(0)
        {
        }

        jsonpath_node(const jsonpath_node* parent, const string_type& name)
            : parent_(parent), node_kind_(jsonpath_node_kind::name), name_(name), index_(0)
        {
        }

        jsonpath_node(const jsonpath_node* parent, std::size_t index)
            : parent_(parent), node_kind_(jsonpath_node_kind::index), index_(index)
        {
        }

        const jsonpath_node* parent() const { return parent_;}

        jsonpath_node_kind node_kind() const
        {
            return node_kind_;
        }

        const string_type& name() const
        {
            return *name_;
        }

        std::size_t index() const 
        {
            return index_;
        }

        void swap(jsonpath_node& node)
        {
            std::swap(parent_, node.parent_);
            std::swap(node_kind_, node.node_kind_);
            std::swap(name_, node.name_);
            std::swap(index_, node.index_);
        }

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == jsonpath_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_type>{}(*name_);

            return h;
        }

        int compare_node(const jsonpath_node& other) const
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
                    case jsonpath_node_kind::root:
                    case jsonpath_node_kind::name:
                        diff = (*name_).compare(*(other.name_));
                        break;
                    case jsonpath_node_kind::index:
                        diff = index_ < other.index_ ? -1 : index_ > other.index_ ? 1 : 0;
                        break;
                    default:
                        break;
                }
            }
            return diff;
        }
    };

    namespace detail {

        template <class Iterator>
        class value_location_iterator
        { 
            Iterator it_; 

        public:
            using iterator_category = std::random_access_iterator_tag;

            using value_type = typename std::remove_pointer<typename std::iterator_traits<Iterator>::value_type>::type;
            using difference_type = typename std::iterator_traits<Iterator>::difference_type;
            using pointer = const value_type*;
            using reference = const value_type&;

            value_location_iterator() : it_()
            { 
            }

            explicit value_location_iterator(Iterator ptr) : it_(ptr)
            {
            }

            value_location_iterator(const value_location_iterator&) = default;
            value_location_iterator(value_location_iterator&&) = default;
            value_location_iterator& operator=(const value_location_iterator&) = default;
            value_location_iterator& operator=(value_location_iterator&&) = default;

            template <class Iter,
                      class=typename std::enable_if<!std::is_same<Iter,Iterator>::value && std::is_convertible<Iter,Iterator>::value>::type>
            value_location_iterator(const value_location_iterator<Iter>& other)
                : it_(other.it_)
            {
            }

            operator Iterator() const
            { 
                return it_; 
            }

            reference operator*() const 
            {
                return *(*it_);
            }

            pointer operator->() const 
            {
                return (*it_);
            }

            value_location_iterator& operator++() 
            {
                ++it_;
                return *this;
            }

            value_location_iterator operator++(int) 
            {
                value_location_iterator temp = *this;
                ++*this;
                return temp;
            }

            value_location_iterator& operator--() 
            {
                --it_;
                return *this;
            }

            value_location_iterator operator--(int) 
            {
                value_location_iterator temp = *this;
                --*this;
                return temp;
            }

            value_location_iterator& operator+=(const difference_type offset) 
            {
                it_ += offset;
                return *this;
            }

            value_location_iterator operator+(const difference_type offset) const 
            {
                value_location_iterator temp = *this;
                return temp += offset;
            }

            value_location_iterator& operator-=(const difference_type offset) 
            {
                return *this += -offset;
            }

            value_location_iterator operator-(const difference_type offset) const 
            {
                value_location_iterator temp = *this;
                return temp -= offset;
            }

            difference_type operator-(const value_location_iterator& rhs) const noexcept
            {
                return it_ - rhs.it_;
            }

            reference operator[](const difference_type offset) const noexcept
            {
                return *(*(*this + offset));
            }

            bool operator==(const value_location_iterator& rhs) const noexcept
            {
                return it_ == rhs.it_;
            }

            bool operator!=(const value_location_iterator& rhs) const noexcept
            {
                return !(*this == rhs);
            }

            bool operator<(const value_location_iterator& rhs) const noexcept
            {
                return it_ < rhs.it_;
            }

            bool operator>(const value_location_iterator& rhs) const noexcept
            {
                return rhs < *this;
            }

            bool operator<=(const value_location_iterator& rhs) const noexcept
            {
                return !(rhs < *this);
            }

            bool operator>=(const value_location_iterator& rhs) const noexcept
            {
                return !(*this < rhs);
            }

            inline 
            friend value_location_iterator<Iterator> operator+(
                difference_type offset, value_location_iterator<Iterator> next) 
            {
                return next += offset;
            }
        };

    } // namespace detail

    template <class StringT>
    class value_location
    {
    public:
        using allocator_type = typename StringT::allocator_type;
        using string_type = StringT;
        using jsonpath_node_type = jsonpath_node<StringT>;
    private:
        allocator_type alloc_;
        std::vector<const jsonpath_node_type*> nodes_;
    public:
        using iterator = typename detail::value_location_iterator<typename std::vector<const jsonpath_node_type*>::iterator>;
        using const_iterator = typename detail::value_location_iterator<typename std::vector<const jsonpath_node_type*>::const_iterator>;

        value_location(const jsonpath_node_type& node, const allocator_type& alloc = allocator_type())
            : alloc_(alloc)
        {
            const jsonpath_node_type* p = std::addressof(node);
            do
            {
                nodes_.push_back(p);
                p = p->parent_;
            }
            while (p != nullptr);

            std::reverse(nodes_.begin(), nodes_.end());
        }

        iterator begin()
        {
            return iterator(nodes_.begin());
        }

        iterator end()
        {
            return iterator(nodes_.end());
        }

        const_iterator begin() const
        {
            return const_iterator(nodes_.begin());
        }

        const_iterator end() const
        {
            return const_iterator(nodes_.end());
        }

        const jsonpath_node_type& last() const
        {
            return *nodes_.back();
        }

        string_type to_string() const
        {
            string_type buffer(alloc_);

            for (const auto& node : nodes_)
            {
                switch (node->node_kind())
                {
                    case jsonpath_node_kind::root:
                        buffer.append(node->name());
                        break;
                    case jsonpath_node_kind::name:
                        buffer.push_back('[');
                        buffer.push_back('\'');
                        jsoncons::jsonpath::escape_string(node->name().data(), node->name().size(), buffer);
                        buffer.push_back('\'');
                        buffer.push_back(']');
                        break;
                    case jsonpath_node_kind::index:
                        buffer.push_back('[');
                        jsoncons::detail::from_integer(node->index(), buffer);
                        buffer.push_back(']');
                        break;
                }
            }

            return buffer;
        }

        int compare(const value_location& other) const
        {
            if (this == &other)
            {
               return 0;
            }

            auto it1 = nodes_.begin();
            auto it2 = other.nodes_.begin();
            while (it1 != nodes_.end() && it2 != other.nodes_.end())
            {
                int diff = (*it1)->compare_node(*(*it2));
                if (diff != 0)
                {
                    return diff;
                }
                ++it1;
                ++it2;
            }
            return (nodes_.size() < other.nodes_.size()) ? -1 : (nodes_.size() == other.nodes_.size()) ? 0 : 1;
        }

        std::size_t hash() const
        {

            auto it = nodes_.begin();
            std::size_t hash = (*it).hash();
            ++it;

            while (it != nodes_.end())
            {
                hash += 17*(*it)->node_hash();
                ++it;
            }

            return hash;
        }

        friend bool operator==(const value_location& lhs, const value_location& rhs) 
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator!=(const value_location& lhs, const value_location& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator<(const value_location& lhs, const value_location& rhs) 
        {
            return lhs.compare(rhs) < 0;
        }
    };

    template <class Json>
    Json* select(Json& root, const value_location<typename Json::string_type>& path)
    {
        Json* current = std::addressof(root);
        for (const auto& jsonpath_node : path)
        {
            if (jsonpath_node.node_kind() == jsonpath_node_kind::index)
            {
                if (current->type() != json_type::array_value || jsonpath_node.index() >= current->size())
                {
                    return nullptr; 
                }
                current = std::addressof(current->at(jsonpath_node.index()));
            }
            else if (jsonpath_node.node_kind() == jsonpath_node_kind::name)
            {
                if (current->type() != json_type::object_value)
                {
                    return nullptr;
                }
                auto it = current->find(jsonpath_node.name());
                if (it == current->object_range().end())
                {
                    return nullptr;
                }
                current = std::addressof(it->value());
            }
        }
        return current;
    }

} // namespace jsonpath
} // namespace jsoncons

#endif
