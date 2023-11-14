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
#include <jsoncons/json_location.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_utilities.hpp>

namespace jsoncons { 
namespace jsonpath {

    template <class CharT>
    class basic_path_node 
    {
    public:
        using string_view_type = jsoncons::basic_string_view<CharT>;
        using char_type = CharT;
    private:

        const basic_path_node* parent_;
        std::size_t size_;
        location_element_kind node_kind_;
        char_type root_;
        string_view_type name_;
        std::size_t index_;

    public:
        basic_path_node(char_type root)
            : parent_(nullptr), size_(1),
              node_kind_(location_element_kind::root), 
              root_{root}, name_(&root_,1), index_(0)
        {
        }

        basic_path_node(const basic_path_node* parent, const string_view_type& name)
            : parent_(parent), size_(parent == nullptr ? 1 : parent->size()+1), 
              node_kind_(location_element_kind::name), root_(0), name_(name), index_(0)
        {
        }

        basic_path_node(const basic_path_node* parent, std::size_t index)
            : parent_(parent), size_(parent == nullptr ? 1 : parent->size()+1), 
              node_kind_(location_element_kind::index), root_(0), index_(index)
        {
        }

        basic_path_node(const basic_path_node& other)
            : parent_(other.parent_), size_(other.size()),
              node_kind_(other.node_kind_),
              root_(other.root_), name_(other.node_kind_ == location_element_kind::root ? string_view_type(&root_, 1) : other.name_),
              index_(other.index_)
        {
        }

        basic_path_node& operator=(const basic_path_node& other)
        {
            parent_ = other.parent_;
            size_ = other.size();
            node_kind_ = other.node_kind_;
            index_ = other.index_;
            root_ = other.root_;
            name_ = other.node_kind_ == location_element_kind::root ? string_view_type(&root_, 1) : other.name_;
            return *this;
        }

        const basic_path_node* parent() const { return parent_;}

        location_element_kind node_kind() const
        {
            return node_kind_;
        }

        const string_view_type& name() const
        {
            return name_;
        }

        std::size_t size() const
        {
            return size_;
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
            std::size_t h = node_kind_ == location_element_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_view_type>{}(name_);

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
                        case location_element_kind::root:
                        case location_element_kind::name:
                            diff = p_lhs->name_.compare(p_rhs->name_);
                            break;
                        case location_element_kind::index:
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
                        case location_element_kind::root:
                        case location_element_kind::name:
                            is_equal = p_lhs->name_ == p_rhs->name_;
                            break;
                        case location_element_kind::index:
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

    template <class Json>
    Json* select(Json& root, const basic_path_node<typename Json::char_type>& path)
    {
        using path_node_type = basic_path_node<typename Json::char_type>;

        std::vector<const path_node_type*> nodes(path.size(), nullptr);
        std::size_t len = nodes.size();
        const path_node_type* p = std::addressof(path);
        while (p != nullptr)
        {
            nodes[--len] = p;
            p = p->parent();
        }
        while (p != nullptr);

        Json* current = std::addressof(root);
        for (auto node : nodes)
        {
            if (node->node_kind() == location_element_kind::index)
            {
                if (current->type() != json_type::array_value || node->index() >= current->size())
                {
                    return nullptr; 
                }
                current = std::addressof(current->at(node->index()));
            }
            else if (node->node_kind() == location_element_kind::name)
            {
                if (current->type() != json_type::object_value)
                {
                    return nullptr;
                }
                auto it = current->find(node->name());
                if (it == current->object_range().end())
                {
                    return nullptr;
                }
                current = std::addressof(it->value());
            }
        }
        return current;
    }

    template <class CharT, class Allocator=std::allocator<CharT>>
    std::basic_string<CharT,std::char_traits<CharT>,Allocator> to_jsonpath(const basic_path_node<CharT>& path, const Allocator& alloc=Allocator())
    {
        std::basic_string<CharT,std::char_traits<CharT>,Allocator> buffer(alloc);

        using path_node_type = basic_path_node<CharT>;

        std::vector<const path_node_type*> nodes(path.size(), nullptr);
        std::size_t len = nodes.size();
        const path_node_type* p = std::addressof(path);
        while (p != nullptr)
        {
            nodes[--len] = p;
            p = p->parent();
        }
        while (p != nullptr);

        for (auto node : nodes)
        {
            switch (node->node_kind())
            {
                case location_element_kind::root:
                    buffer.append(node->name().data(), node->name().size());
                    break;
                case location_element_kind::name:
                    buffer.push_back('[');
                    buffer.push_back('\'');
                    jsoncons::jsonpath::escape_string(node->name().data(), node->name().size(), buffer);
                    buffer.push_back('\'');
                    buffer.push_back(']');
                    break;
                case location_element_kind::index:
                    buffer.push_back('[');
                    jsoncons::detail::from_integer(node->index(), buffer);
                    buffer.push_back(']');
                    break;
            }
        }

        return buffer;
    }

    using json_location = basic_json_location<char>;
    using wjson_location = basic_json_location<wchar_t>;
    using location_element = basic_location_element<char>;
    using wlocation_element = basic_location_element<wchar_t>;
    using path_node = basic_path_node<char>;
    using wpath_node = basic_path_node<wchar_t>;

} // namespace jsonpath
} // namespace jsoncons

#endif
