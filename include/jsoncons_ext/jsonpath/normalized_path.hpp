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

namespace jsoncons { 
namespace jsonpath {
namespace detail { 

    template <class CharT>
    class normalized_path; 

    enum class path_node_kind { root, index, identifier };

    template <class CharT>
    class path_node 
    {
        friend class normalized_path<CharT>;
    public:
        using char_type = CharT;
        using string_type = std::basic_string<CharT>;
    private:

        const path_node* parent_;
        path_node_kind node_kind_;
        string_type identifier_;
        std::size_t index_;
    public:
        path_node(char_type c)
            : parent_(nullptr), node_kind_(path_node_kind::root), index_(0)
        {
            identifier_.push_back(c);
        }

        path_node(const path_node* parent, const string_type& identifier)
            : parent_(parent), node_kind_(path_node_kind::identifier), identifier_(identifier), index_(0)
        {
        }

        path_node(const path_node* parent, std::size_t index)
            : parent_(parent), node_kind_(path_node_kind::index), index_(index)
        {
        }

        const path_node* parent() const { return parent_;}

        path_node_kind node_kind() const
        {
            return node_kind_;
        }

        const string_type& identifier() const
        {
            return identifier_;
        }

        std::size_t index() const 
        {
            return index_;
        }

        void swap(path_node& node)
        {
            std::swap(parent_, node.parent_);
            std::swap(node_kind_, node.node_kind_);
            std::swap(identifier_, node.identifier_);
            std::swap(index_, node.index_);
        }

    private:

        std::size_t node_hash() const
        {
            std::size_t h = node_kind_ == path_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_type>{}(identifier_);

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
                        diff = identifier_.compare(other.identifier_);
                        break;
                    case path_node_kind::index:
                        diff = index_ < other.index_ ? -1 : index_ > other.index_ ? 1 : 0;
                        break;
                    case path_node_kind::identifier:
                        diff = identifier_.compare(other.identifier_);
                        break;
                }
            }
            return diff;
        }
    };

    template <class CharT>
    class normalized_path
    {
    public:
        using char_type = CharT;
        using string_type = std::basic_string<CharT>;
        using path_node_type = path_node<CharT>;
    private:
        std::vector<const path_node_type*> nodes_;
    public:
        normalized_path(const path_node_type& node)
        {
            const path_node_type* p = std::addressof(node);
            do
            {
                nodes_.push_back(p);
                p = p->parent_;
            }
            while (p != nullptr);

            std::reverse(nodes_.begin(), nodes_.end());
        }

        const path_node_type& root() const
        {
            return *nodes_.front();
        }

        const path_node_type& stem() const
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
                        buffer.append(node->identifier());
                        break;
                    case path_node_kind::identifier:
                        buffer.push_back('[');
                        buffer.push_back('\'');
                        for (auto c : node->identifier())
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


} // namespace detail
} // namespace jsonpath
} // namespace jsoncons

#endif
