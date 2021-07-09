// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_NORMALIZED_PATH_HPP
#define JSONCONS_JSONPATH_NORMALIZED_PATH_HPP

#include <string>
#include <vector>
#include <functional>
#include <algorithm> // std::reverse
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>
#include <jsoncons/json_type.hpp>

namespace jsoncons { 
namespace jsonpath {

    template <class CharT>
    class normalized_path; 

    enum class path_node_kind { root, index, name };

    template <class CharT>
    class path_component 
    {
        friend class normalized_path<CharT>;
    public:
        using char_type = CharT;
        using string_type = std::basic_string<CharT>;
    private:

        const path_component* parent_;
        path_node_kind node_kind_;
        string_type name_;
        std::size_t index_;
    public:
        path_component(char_type c)
            : parent_(nullptr), node_kind_(path_node_kind::root), index_(0)
        {
            name_.push_back(c);
        }

        path_component(const path_component* parent, const string_type& name)
            : parent_(parent), node_kind_(path_node_kind::name), name_(name), index_(0)
        {
        }

        path_component(const path_component* parent, std::size_t index)
            : parent_(parent), node_kind_(path_node_kind::index), index_(index)
        {
        }

        const path_component* parent() const { return parent_;}

        path_node_kind node_kind() const
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

        void swap(path_component& node)
        {
            std::swap(parent_, node.parent_);
            std::swap(node_kind_, node.node_kind_);
            std::swap(name_, node.name_);
            std::swap(index_, node.index_);
        }

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == path_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_type>{}(name_);

            return h;
        }

        int compare_node(const path_component& other) const
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
                        diff = name_.compare(other.name_);
                        break;
                    case path_node_kind::index:
                        diff = index_ < other.index_ ? -1 : index_ > other.index_ ? 1 : 0;
                        break;
                    case path_node_kind::name:
                        diff = name_.compare(other.name_);
                        break;
                }
            }
            return diff;
        }
    };

    namespace detail {

        template <class Iterator>
        class normalized_path_iterator
        { 
            Iterator it_; 

        public:
            using iterator_category = std::random_access_iterator_tag;

            using value_type = typename std::remove_pointer<typename std::iterator_traits<Iterator>::value_type>::type;
            using difference_type = typename std::iterator_traits<Iterator>::difference_type;
            using pointer = const value_type*;
            using reference = const value_type&;

            normalized_path_iterator() : it_()
            { 
            }

            explicit normalized_path_iterator(Iterator ptr) : it_(ptr)
            {
            }

            normalized_path_iterator(const normalized_path_iterator&) = default;
            normalized_path_iterator(normalized_path_iterator&&) = default;
            normalized_path_iterator& operator=(const normalized_path_iterator&) = default;
            normalized_path_iterator& operator=(normalized_path_iterator&&) = default;

            template <class Iter,
                      class=typename std::enable_if<!std::is_same<Iter,Iterator>::value && std::is_convertible<Iter,Iterator>::value>::type>
            normalized_path_iterator(const normalized_path_iterator<Iter>& other)
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

            normalized_path_iterator& operator++() 
            {
                ++it_;
                return *this;
            }

            normalized_path_iterator operator++(int) 
            {
                normalized_path_iterator temp = *this;
                ++*this;
                return temp;
            }

            normalized_path_iterator& operator--() 
            {
                --it_;
                return *this;
            }

            normalized_path_iterator operator--(int) 
            {
                normalized_path_iterator temp = *this;
                --*this;
                return temp;
            }

            normalized_path_iterator& operator+=(const difference_type offset) 
            {
                it_ += offset;
                return *this;
            }

            normalized_path_iterator operator+(const difference_type offset) const 
            {
                normalized_path_iterator temp = *this;
                return temp += offset;
            }

            normalized_path_iterator& operator-=(const difference_type offset) 
            {
                return *this += -offset;
            }

            normalized_path_iterator operator-(const difference_type offset) const 
            {
                normalized_path_iterator temp = *this;
                return temp -= offset;
            }

            difference_type operator-(const normalized_path_iterator& rhs) const noexcept
            {
                return it_ - rhs.it_;
            }

            reference operator[](const difference_type offset) const noexcept
            {
                return *(*(*this + offset));
            }

            bool operator==(const normalized_path_iterator& rhs) const noexcept
            {
                return it_ == rhs.it_;
            }

            bool operator!=(const normalized_path_iterator& rhs) const noexcept
            {
                return !(*this == rhs);
            }

            bool operator<(const normalized_path_iterator& rhs) const noexcept
            {
                return it_ < rhs.it_;
            }

            bool operator>(const normalized_path_iterator& rhs) const noexcept
            {
                return rhs < *this;
            }

            bool operator<=(const normalized_path_iterator& rhs) const noexcept
            {
                return !(rhs < *this);
            }

            bool operator>=(const normalized_path_iterator& rhs) const noexcept
            {
                return !(*this < rhs);
            }

            inline 
            friend normalized_path_iterator<Iterator> operator+(
                difference_type offset, normalized_path_iterator<Iterator> next) 
            {
                return next += offset;
            }
        };

    } // namespace detail

    template <class CharT>
    class normalized_path
    {
    public:
        using char_type = CharT;
        using string_type = std::basic_string<CharT>;
        using path_component_type = path_component<CharT>;
    private:
        std::vector<const path_component_type*> nodes_;
    public:
        using iterator = typename detail::normalized_path_iterator<typename std::vector<const path_component_type*>::iterator>;
        using const_iterator = typename detail::normalized_path_iterator<typename std::vector<const path_component_type*>::const_iterator>;

        normalized_path(const path_component_type& node)
        {
            const path_component_type* p = std::addressof(node);
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

        const path_component_type& last() const
        {
            return *nodes_.back();
        }

        string_type to_string() const
        {
            string_type buffer;

            for (const auto& node : nodes_)
            {
                switch (node->node_kind())
                {
                    case path_node_kind::root:
                        buffer.append(node->name());
                        break;
                    case path_node_kind::name:
                        buffer.push_back('[');
                        buffer.push_back('\'');
                        for (auto c : node->name())
                        {
                            if (c == '\'')
                            {
                                buffer.push_back('\\');
                                buffer.push_back('\'');
                            }
                            else
                            {
                                buffer.push_back(c);
                            }
                        }
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

        int compare(const normalized_path& other) const
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

        friend bool operator==(const normalized_path& lhs, const normalized_path& rhs) 
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator!=(const normalized_path& lhs, const normalized_path& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator<(const normalized_path& lhs, const normalized_path& rhs) 
        {
            return lhs.compare(rhs) < 0;
        }
    };

    template <class Json>
    Json* select(Json& root, const normalized_path<typename Json::char_type>& path)
    {
        Json* current = std::addressof(root);
        for (const auto& path_component : path)
        {
            if (path_component.node_kind() == path_node_kind::index)
            {
                if (current->type() != json_type::array_value || path_component.index() >= current->size())
                {
                    return nullptr; 
                }
                current = std::addressof(current->at(path_component.index()));
            }
            else if (path_component.node_kind() == path_node_kind::name)
            {
                if (current->type() != json_type::object_value)
                {
                    return nullptr;
                }
                auto it = current->find(path_component.name());
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
