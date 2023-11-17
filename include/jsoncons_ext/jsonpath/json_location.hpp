// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_LOCATION_HPP
#define JSONCONS_JSONPATH_JSON_LOCATION_HPP

#include <string>
#include <vector>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_utilities.hpp>

namespace jsoncons { 
namespace jsonpath { 

    template <class CharT,class Allocator>
    class basic_path_element 
    {
    public:
        using char_type = CharT;
        using allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT>;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;
    private:
        bool has_name_;
        string_type name_;
        std::size_t index_;

    public:
        basic_path_element(const string_type& name)
            : has_name_(true), name_(name), index_(0)
        {
        }

        basic_path_element(string_type&& name)
            : has_name_(true), name_(std::move(name)), index_(0)
        {
        }

        basic_path_element(std::size_t index)
            : has_name_(false), index_(index)
        {
        }

        basic_path_element(const basic_path_element& other) = default;

        basic_path_element& operator=(const basic_path_element& other) = default;

        bool has_name() const
        {
            return has_name_;
        }

        bool has_index() const
        {
            return !has_name_;
        }

        const string_type& name() const
        {
            return name_;
        }

        std::size_t index() const 
        {
            return index_;
        }

        int compare(const basic_path_element& other) const
        {
            int diff = 0;
            if (has_name_ != other.has_name_)
            {
                diff = static_cast<int>(has_name_) - static_cast<int>(other.has_name_);
            }
            else
            {
                if (has_name_)
                {
                    diff = name_.compare(other.name_);
                }
                else
                {
                    diff = index_ < other.index_ ? -1 : index_ > other.index_ ? 1 : 0;
                }
            }
            return diff;
        }
    };

    template <class CharT, class Allocator = std::allocator<CharT>>
    class basic_json_location
    {
    public:
        using char_type = CharT;
        using allocator_type = Allocator;
        using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type>;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;
        using string_view_type = jsoncons::basic_string_view<char_type, std::char_traits<char_type>>;
        using path_element_type = basic_path_element<CharT,Allocator>;
        using path_element_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<path_element_type>;
    private:
        allocator_type alloc_;
        std::vector<path_element_type> elements_;
    public:
        using iterator = typename std::vector<path_element_type>::iterator;
        using const_iterator = typename std::vector<path_element_type>::const_iterator;

        basic_json_location(const allocator_type& alloc=Allocator())
            : alloc_(alloc), elements_(alloc)
        {
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

        std::size_t size() const
        {
            return elements_.size();
        }

        const basic_path_element<char_type,allocator_type>& operator[](std::size_t index) const
        {
            return elements_[index];
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

        // Modifiers

        void clear()
        {
            elements_.clear();
        }

        basic_json_location& append(const string_view_type& s)
        {
            elements_.emplace_back(string_type(s.data(), s.size(), alloc_));
            return *this;
        }

        template <class IntegerType>
        typename std::enable_if<extension_traits::is_integer<IntegerType>::value, basic_json_location&>::type
            append(IntegerType val)
        {
            elements_.emplace_back(static_cast<std::size_t>(val));

            return *this;
        }

        basic_json_location& operator/=(const string_view_type& s)
        {
            elements_.emplace_back(string_type(s.data(), s.size(), alloc_));
            return *this;
        }

        template <class IntegerType>
        typename std::enable_if<extension_traits::is_integer<IntegerType>::value, basic_json_location&>::type
            operator/=(IntegerType val)
        {
            elements_.emplace_back(static_cast<std::size_t>(val));

            return *this;
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

    template<class Json>
    std::size_t json_erase(Json& instance, const basic_json_location<typename Json::char_type>& location)
    {
        std::size_t count = 0;

        Json* p_current = std::addressof(instance);

        std::size_t last = location.size() == 0 ? 0 : location.size() - 1;
        for (std::size_t i = 0; i < location.size(); ++i)
        {
            const auto& element = location[i];
            if (element.has_name())
            {
                if (p_current->is_object())
                {
                    auto it = p_current->find(element.name());
                    if (it != p_current->object_range().end())
                    {
                        if (i < last)
                        {
                            p_current = std::addressof(it->value());
                        }
                        else
                        {
                            p_current->erase(it);
                            count = 1;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else // if (element.has_index())
            {
                if (p_current->is_array() && element.index() < p_current->size())
                {
                    if (i < last)
                    {
                        p_current = std::addressof(p_current->at(element.index()));
                    }
                    else
                    {
                        p_current->erase(p_current->array_range().begin()+element.index());
                        count = 1;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        return count;
    }

    template<class Json>
    Json* json_get(Json& instance, const basic_json_location<typename Json::char_type>& location)
    {
        Json* p_current = std::addressof(instance);
        bool found = false;

        std::size_t last = location.size() == 0 ? 0 : location.size() - 1;
        for (std::size_t i = 0; i < location.size(); ++i)
        {
            const auto& element = location[i];
            if (element.has_name())
            {
                if (p_current->is_object())
                {
                    auto it = p_current->find(element.name());
                    if (it != p_current->object_range().end())
                    {
                        p_current = std::addressof(it->value());
                        if (i == last)
                        {
                            found = true;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else // if (element.has_index())
            {
                if (p_current->is_array() && element.index() < p_current->size())
                {
                    p_current = std::addressof(p_current->at(element.index()));
                    if (i == last)
                    {
                        found = true;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        return found ? p_current : nullptr;
    }

    template <class CharT, class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, std::char_traits<CharT>, Allocator> to_basic_string(const basic_json_location<CharT,Allocator>& location, 
        const Allocator& alloc = Allocator())
    {
        std::basic_string<CharT, std::char_traits<CharT>, Allocator> buffer(alloc);

        buffer.push_back('$');
        for (const auto& element : location)
        {
            if (element.has_name())
            {
                buffer.push_back('[');
                buffer.push_back('\'');
                jsoncons::jsonpath::escape_string(element.name().data(), element.name().size(), buffer);
                buffer.push_back('\'');
                buffer.push_back(']');
            }
            else
            {
                buffer.push_back('[');
                jsoncons::detail::from_integer(element.index(), buffer);
                buffer.push_back(']');
            }
        }

        return buffer;
    }

    using json_location = basic_json_location<char>;
    using wjson_location = basic_json_location<wchar_t>;
    using path_element = basic_path_element<char,std::allocator<char>>;
    using wpath_element = basic_path_element<wchar_t,std::allocator<char>>;

    inline
    std::string to_string(const json_location& location)
    {
        return to_basic_string(location);
    }

    inline
    std::wstring to_wstring(const wjson_location& location)
    {
        return to_basic_string(location);
    }

} // namespace jsonpath
} // namespace jsoncons

#endif
