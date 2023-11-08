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

    template <class Json>
    class basic_json_location; 

    enum class path_node_kind { root, index, name };

    template <class CharT>
    class path_node 
    {
        template<class Json> friend class basic_json_location;
    public:
        using string_view_type = jsoncons::basic_string_view<CharT>;
        using char_type = CharT;
    private:

        char_type root_;
        const path_node* parent_;
        path_node_kind node_kind_;
        string_view_type name_;
        std::size_t index_;

    public:
        path_node(char_type root)
            : root_{root}, parent_(nullptr), 
              node_kind_(path_node_kind::root), 
              name_(&root_,1), index_(0)
        {
        }

        path_node(const path_node* parent, const string_view_type& name)
            : root_(0), parent_(parent), node_kind_(path_node_kind::name), name_(name), index_(0)
        {
        }

        path_node(const path_node* parent, std::size_t index)
            : root_(0), parent_(parent), node_kind_(path_node_kind::index), index_(index)
        {
        }

        path_node(const path_node& other)
            : root_(other.root_),
              parent_(other.parent_),
              node_kind_(other.node_kind_),
              name_(other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_),
              index_(other.index_)
        {
        }

        path_node(path_node&& other)
            : root_(other.root_),
              parent_(other.parent_),
              node_kind_(other.node_kind_),
              name_(other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_),
              index_(other.index_)
        {
        }

        path_node& operator=(const path_node& other)
        {
            root_ = other.root_;
            parent_ = other.parent_;
            node_kind_ = other.node_kind_;
            index_ = other.index_;
            name_ = other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_;
            return *this;
        }

        path_node& operator=(path_node&& other)
        {
            root_ = other.root_;
            parent_ = other.parent_;
            node_kind_ = other.node_kind_;
            index_ = other.index_;
            name_ = other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_;
            return *this;
        }

        const path_node* parent() const { return parent_;}

        path_node_kind node_kind() const
        {
            return node_kind_;
        }

        const string_view_type& name() const
        {
            return name_;
        }

        std::size_t size() const
        {
            std::size_t len = 1;
            
            for (auto p = parent_; p != nullptr; p = p->parent_)
            {
                ++len;
            }
            return len;
        }

        std::size_t index() const 
        {
            return index_;
        }

        void swap(path_node& node)
        {
            std::swap(parent_, node.parent_);
            std::swap(node_kind_, node.node_kind_);
            std::swap(name_, node.name_);
            std::swap(index_, node.index_);
        }

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == path_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_view_type>{}(name_);

            return h;
        }

        int compare_node(const path_node& other) const
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
                    case path_node_kind::root:
                    case path_node_kind::name:
                        diff = name_.compare(other.name_);
                        break;
                    case path_node_kind::index:
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
        class json_location_iterator
        { 
            Iterator it_; 

        public:
            using iterator_category = std::random_access_iterator_tag;

            using value_type = typename std::remove_pointer<typename std::iterator_traits<Iterator>::value_type>::type;
            using difference_type = typename std::iterator_traits<Iterator>::difference_type;
            using pointer = const value_type*;
            using reference = const value_type&;

            json_location_iterator() : it_()
            { 
            }

            explicit json_location_iterator(Iterator ptr) : it_(ptr)
            {
            }

            json_location_iterator(const json_location_iterator&) = default;
            json_location_iterator(json_location_iterator&&) = default;
            json_location_iterator& operator=(const json_location_iterator&) = default;
            json_location_iterator& operator=(json_location_iterator&&) = default;

            template <class Iter,
                      class=typename std::enable_if<!std::is_same<Iter,Iterator>::value && std::is_convertible<Iter,Iterator>::value>::type>
            json_location_iterator(const json_location_iterator<Iter>& other)
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

            json_location_iterator& operator++() 
            {
                ++it_;
                return *this;
            }

            json_location_iterator operator++(int) 
            {
                json_location_iterator temp = *this;
                ++*this;
                return temp;
            }

            json_location_iterator& operator--() 
            {
                --it_;
                return *this;
            }

            json_location_iterator operator--(int) 
            {
                json_location_iterator temp = *this;
                --*this;
                return temp;
            }

            json_location_iterator& operator+=(const difference_type offset) 
            {
                it_ += offset;
                return *this;
            }

            json_location_iterator operator+(const difference_type offset) const 
            {
                json_location_iterator temp = *this;
                return temp += offset;
            }

            json_location_iterator& operator-=(const difference_type offset) 
            {
                return *this += -offset;
            }

            json_location_iterator operator-(const difference_type offset) const 
            {
                json_location_iterator temp = *this;
                return temp -= offset;
            }

            difference_type operator-(const json_location_iterator& rhs) const noexcept
            {
                return it_ - rhs.it_;
            }

            reference operator[](const difference_type offset) const noexcept
            {
                return *(*(*this + offset));
            }

            bool operator==(const json_location_iterator& rhs) const noexcept
            {
                return it_ == rhs.it_;
            }

            bool operator!=(const json_location_iterator& rhs) const noexcept
            {
                return !(*this == rhs);
            }

            bool operator<(const json_location_iterator& rhs) const noexcept
            {
                return it_ < rhs.it_;
            }

            bool operator>(const json_location_iterator& rhs) const noexcept
            {
                return rhs < *this;
            }

            bool operator<=(const json_location_iterator& rhs) const noexcept
            {
                return !(rhs < *this);
            }

            bool operator>=(const json_location_iterator& rhs) const noexcept
            {
                return !(*this < rhs);
            }

            inline 
            friend json_location_iterator<Iterator> operator+(
                difference_type offset, json_location_iterator<Iterator> next) 
            {
                return next += offset;
            }
        };

    } // namespace detail

    template <class Json>
    class basic_json_location
    {
    public:
        using allocator_type = typename Json::allocator_type;
        using string_type = typename Json::string_type;
        using path_node_type = path_node<typename Json::char_type>;
    private:
        allocator_type alloc_;
        std::vector<const path_node_type*> nodes_;
    public:
        using iterator = typename detail::json_location_iterator<typename std::vector<const path_node_type*>::iterator>;
        using const_iterator = typename detail::json_location_iterator<typename std::vector<const path_node_type*>::const_iterator>;

        basic_json_location(const path_node_type& node, const allocator_type& alloc = allocator_type())
            : alloc_(alloc)
        {
            std::size_t len = node.size();
            nodes_.resize(len);

            const path_node_type* p = std::addressof(node);
            do
            {
                nodes_[--len] = p;
                p = p->parent_;
            }
            while (p != nullptr);
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

        const path_node_type& last() const
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
                    case path_node_kind::root:
                        buffer.append(node->name().data(), node->name().size());
                        break;
                    case path_node_kind::name:
                        buffer.push_back('[');
                        buffer.push_back('\'');
                        jsoncons::jsonpath::escape_string(node->name().data(), node->name().size(), buffer);
                        buffer.push_back('\'');
                        buffer.push_back(']');
                        break;
                    case path_node_kind::index:
                        buffer.push_back('[');
                        jsoncons::detail::from_integer(node->index(), buffer);
                        buffer.push_back(']');
                        break;
                }
            }

            return buffer;
        }

        int compare(const basic_json_location& other) const
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

    template <class Json>
    Json* select(Json& root, const basic_json_location<typename std::remove_cv<Json>::type>& path)
    {
        Json* current = std::addressof(root);
        for (const auto& path_node : path)
        {
            if (path_node.node_kind() == path_node_kind::index)
            {
                if (current->type() != json_type::array_value || path_node.index() >= current->size())
                {
                    return nullptr; 
                }
                current = std::addressof(current->at(path_node.index()));
            }
            else if (path_node.node_kind() == path_node_kind::name)
            {
                if (current->type() != json_type::object_value)
                {
                    return nullptr;
                }
                auto it = current->find(path_node.name());
                if (it == current->object_range().end())
                {
                    return nullptr;
                }
                current = std::addressof(it->value());
            }
        }
        return current;
    }

    using json_location = basic_json_location<json>;
    using wjson_location = basic_json_location<wjson>;

} // namespace jsonpath
} // namespace jsoncons

#endif
