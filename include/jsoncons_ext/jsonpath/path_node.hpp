// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_PATH_NODE_HPP
#define JSONCONS_JSONPATH_PATH_NODE_HPP

#include <string>
#include <vector>
#include <functional>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

namespace jsoncons { 
namespace jsonpath {
namespace detail {

    template <class CharT>
    class path_node 
    {
    public:
        using string_type = std::basic_string<CharT>;
    private:
        enum path_node_kind {root,identifier,index};

        path_node* parent_;
        path_node_kind node_kind_;
        string_type identifier_;
        std::size_t index_;
    public:
        path_node(const string_type& identifier)
            : parent_(nullptr), node_kind_(path_node_kind::root), identifier_(identifier), index_(0)
        {
        }

        path_node(path_node* parent, const string_type& identifier)
            : parent_(parent), node_kind_(path_node_kind::identifier), identifier_(identifier), index_(0)
        {
        }

        path_node(path_node* parent, std::size_t index)
            : parent_(parent), node_kind_(path_node_kind::index), index_(index)
        {
        }

        std::size_t hash() const
        {
            std::size_t h = node_kind_ == path_node_kind::index ? std::hash<std::size_t>{}(index_) : std::hash<string_type>{}(identifier_);
            if (parent_ != nullptr)
            {
                h = h + 17 * parent_>hash();
            }

            return h;
        }

        string_type to_string() const
        {
            string_type buffer;
            to_string(buffer);
            return buffer;
        }

        void to_string(string_type& buffer) const
        {
            if (parent_ != nullptr)
            {
                parent_->to_string(buffer);
            }
            switch (node_kind_)
            {
                case path_node_kind::root:
                    buffer.append(identifier_);
                    break;
                case path_node_kind::identifier:
                    buffer.push_back('[');
                    buffer.push_back('\'');
                    buffer.append(identifier_);
                    buffer.push_back('\'');
                    buffer.push_back(']');
                    break;
                case path_node_kind::index:
                    buffer.push_back('[');
                    jsoncons::detail::from_integer(index_, buffer);
                    buffer.push_back(']');
                    break;
            }
        }

        friend bool operator==(const path_node& lhs, const path_node& rhs) 
        {
            if (&lhs == &rhs)
            {
               return true;
            }

            bool areEqual;
            switch (lhs.node_kind_)
            {
                case path_node_kind::root:
                    areEqual = rhs.node_kind_ == path_node_kind::root && lhs.identifier_ == rhs.identifier_;
                    break;
                case path_node_kind::identifier:
                    areEqual = rhs.node_kind_ == path_node_kind::identifier && lhs.identifier_ == rhs.identifier_;
                    break;
                case path_node_kind::index:
                    areEqual = rhs.node_kind_ == path_node_kind::index && lhs.index_ == rhs.index_;
                    break;
                default:
                    areEqual = false;
                    break;
            }
            if (areEqual)
            {
                if (!(lhs.parent_ == nullptr && rhs.parent_ == nullptr))
                {
                    return *lhs.parent_ == *rhs.parent_;
                }
                else if (lhs.parent_ == nullptr && rhs.parent_ == nullptr)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    };


} // namespace detail
} // namespace jsonpath
} // namespace jsoncons

#endif
