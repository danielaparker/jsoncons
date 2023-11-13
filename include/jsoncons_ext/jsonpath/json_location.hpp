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

    template <class CharT,class Allocator>
    class basic_json_location; 

    enum class path_node_kind { root, index, name };

    template <class CharT>
    class basic_path_node 
    {
        template <class Ch,class Allocator> friend class basic_json_location;
    public:
        using string_view_type = jsoncons::basic_string_view<CharT>;
        using char_type = CharT;
    private:

        char_type root_;
        const basic_path_node* parent_;
        path_node_kind node_kind_;
        string_view_type name_;
        std::size_t index_;

    public:
        basic_path_node(char_type root)
            : root_{root}, parent_(nullptr), 
              node_kind_(path_node_kind::root), 
              name_(&root_,1), index_(0)
        {
        }

        basic_path_node(const basic_path_node* parent, const string_view_type& name)
            : root_(0), parent_(parent), node_kind_(path_node_kind::name), name_(name), index_(0)
        {
        }

        basic_path_node(const basic_path_node* parent, std::size_t index)
            : root_(0), parent_(parent), node_kind_(path_node_kind::index), index_(index)
        {
        }

        basic_path_node(const basic_path_node& other)
            : root_(other.root_),
              parent_(other.parent_),
              node_kind_(other.node_kind_),
              name_(other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_),
              index_(other.index_)
        {
        }

        basic_path_node(basic_path_node&& other)
            : root_(other.root_),
              parent_(other.parent_),
              node_kind_(other.node_kind_),
              name_(other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_),
              index_(other.index_)
        {
        }

        basic_path_node& operator=(const basic_path_node& other)
        {
            root_ = other.root_;
            parent_ = other.parent_;
            node_kind_ = other.node_kind_;
            index_ = other.index_;
            name_ = other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_;
            return *this;
        }

        basic_path_node& operator=(basic_path_node&& other)
        {
            root_ = other.root_;
            parent_ = other.parent_;
            node_kind_ = other.node_kind_;
            index_ = other.index_;
            name_ = other.node_kind_ == path_node_kind::root ? string_view_type(&root_, 1) : other.name_;
            return *this;
        }

        const basic_path_node* parent() const { return parent_;}

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

        void swap(basic_path_node& node)
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
        int compare_node(const basic_path_node& other) const
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

        friend bool operator<(const basic_path_node& lhs, const basic_path_node& rhs)
        {
            std::size_t len = (std::min)(lhs.size(),rhs.size());

            const basic_path_node* p_lhs = std::addressof(lhs);
            const basic_path_node* p_rhs = std::addressof(rhs);

            bool is_less = false;
            while (p_lhs->size() > len)
            {
                p_lhs = p_lhs->parent_;
                is_less = false;
            }
            while (p_rhs->size() > len)
            {
                p_rhs = p_rhs->parent_;
                is_less = true;
            }
            while (p_lhs != nullptr)
            {
                int diff = 0;
                if (p_lhs->node_kind_ != p_rhs->node_kind_)
                {
                    diff = static_cast<int>(p_lhs->node_kind_) - static_cast<int>(p_rhs->node_kind_);
                }
                else
                {
                    switch (p_lhs->node_kind_)
                    {
                        case path_node_kind::root:
                        case path_node_kind::name:
                            diff = p_lhs->name_.compare(p_rhs->name_);
                            break;
                        case path_node_kind::index:
                            diff = static_cast<int>(p_lhs->index_) - static_cast<int>(p_rhs->index_);
                            break;
                        default:
                            break;
                    }
                }
                if (diff < 0)
                {
                    is_less = true;
                }
                else if (diff > 0)
                {
                    is_less = false;
                }

                p_lhs = p_lhs->parent_;
                p_rhs = p_rhs->parent_;
            }

            return is_less;
        }

        friend bool operator==(const basic_path_node& lhs, const basic_path_node& rhs)
        {
            if (lhs.size() != rhs.size())
            {
                return false;
            }

            const basic_path_node* p_lhs = std::addressof(lhs);
            const basic_path_node* p_rhs = std::addressof(rhs);

            bool is_equal = true;
            while (p_lhs != nullptr && is_equal)
            {
                if (p_lhs->node_kind_ != p_rhs->node_kind_)
                {
                    is_equal = false;
                }
                else
                {
                    switch (p_lhs->node_kind_)
                    {
                        case path_node_kind::root:
                        case path_node_kind::name:
                            is_equal = p_lhs->name_ == p_rhs->name_;
                            break;
                        case path_node_kind::index:
                            is_equal = p_lhs->index_ == p_rhs->index_;
                            break;
                        default:
                            break;
                    }
                }
                p_lhs = p_lhs->parent_;
                p_rhs = p_rhs->parent_;
            }

            return is_equal;
        }
    };

    template <class CharT>
    class basic_path_element 
    {
        template <class Ch, class Allocator> friend class basic_json_location;
    public:
        using char_type = CharT;
        using string_view_type = jsoncons::basic_string_view<char_type>;
    private:
        path_node_kind node_kind_;
        string_view_type name_;
        std::size_t index_;

    public:
        basic_path_element() = default;

        basic_path_element(path_node_kind node_kind, string_view_type name, std::size_t index)
            : node_kind_(node_kind), name_(name), index_(index)
        {
        }

        path_node_kind node_kind() const
        {
            return node_kind_;
        }

        const string_view_type& name() const
        {
            return name_;
        }

        std::size_t index() const 
        {
            return index_;
        }

        basic_path_element(const basic_path_element&) = default;
        basic_path_element& operator=(const basic_path_element&) = default;

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == path_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_view_type>{}(name_);

            return h;
        }

        int compare_node(const basic_path_element& other) const
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

    template <class CharT, class Allocator = std::allocator<char>>
    class basic_json_location
    {
    public:
        using char_type = CharT;
        using allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type>;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;
        using path_node_type = basic_path_node<char_type>;
        using path_element_type = basic_path_element<CharT>;
        using path_element_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<path_element_type>;
    private:
        allocator_type alloc_;
        std::vector<path_element_type> elements_;
        const path_node_type* base_node_;
    public:
        using iterator = typename std::vector<path_element_type>::iterator;
        using const_iterator = typename std::vector<path_element_type>::const_iterator;

        basic_json_location(const path_node_type& node, const allocator_type& alloc = allocator_type())
            : alloc_(alloc), elements_(node.size(), path_element_type{}), base_node_(std::addressof(node))
        {
            std::size_t len = node.size();

            const path_node_type* p = std::addressof(node);
            do
            {
                elements_[--len] = path_element_type(p->node_kind(), p->name(), p->index());
                p = p->parent_;
            }
            while (p != nullptr);
        }

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

        const basic_path_element<char_type>& base_element() const
        {
            return elements_.back();
        }

        const basic_path_node<char_type>& base_node() const
        {
            return *base_node_;
        }

        string_type to_string() const
        {
            string_type buffer(alloc_);

            for (const auto& node : elements_)
            {
                switch (node.node_kind())
                {
                    case path_node_kind::root:
                        buffer.append(node.name().data(), node.name().size());
                        break;
                    case path_node_kind::name:
                        buffer.push_back('[');
                        buffer.push_back('\'');
                        jsoncons::jsonpath::escape_string(node.name().data(), node.name().size(), buffer);
                        buffer.push_back('\'');
                        buffer.push_back(']');
                        break;
                    case path_node_kind::index:
                        buffer.push_back('[');
                        jsoncons::detail::from_integer(node.index(), buffer);
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

            auto it1 = elements_.begin();
            auto it2 = other.elements_.begin();
            while (it1 != elements_.end() && it2 != other.elements_.end())
            {
                int diff = it1->compare_node(*it2);
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

    template <class Json>
    Json* select(Json& root, const basic_json_location<typename Json::char_type>& path)
    {
        Json* current = std::addressof(root);
        for (const auto& basic_path_node : path)
        {
            if (basic_path_node.node_kind() == path_node_kind::index)
            {
                if (current->type() != json_type::array_value || basic_path_node.index() >= current->size())
                {
                    return nullptr; 
                }
                current = std::addressof(current->at(basic_path_node.index()));
            }
            else if (basic_path_node.node_kind() == path_node_kind::name)
            {
                if (current->type() != json_type::object_value)
                {
                    return nullptr;
                }
                auto it = current->find(basic_path_node.name());
                if (it == current->object_range().end())
                {
                    return nullptr;
                }
                current = std::addressof(it->value());
            }
        }
        return current;
    }

    using json_location = basic_json_location<char>;
    using wjson_location = basic_json_location<wchar_t>;
    using path_element = basic_path_element<char>;
    using wpath_element = basic_path_element<wchar_t>;
    using path_node = basic_path_node<char>;
    using wpath_node = basic_path_node<wchar_t>;

} // namespace jsonpath
} // namespace jsoncons

#endif
