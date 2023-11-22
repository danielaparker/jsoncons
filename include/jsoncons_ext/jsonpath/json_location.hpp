// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_LOCATION_HPP
#define JSONCONS_JSONPATH_JSON_LOCATION_HPP

#include <string>
#include <vector>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/jsonpath/json_location_parser.hpp>

namespace jsoncons { 
namespace jsonpath { 

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
        std::vector<path_element_type,path_element_allocator_type> elements_;
    public:
        using iterator = typename std::vector<path_element_type>::iterator;
        using const_iterator = typename std::vector<path_element_type>::const_iterator;

        basic_json_location(const allocator_type& alloc=Allocator())
            : elements_(alloc)
        {
        }

        basic_json_location(std::vector<path_element_type>&& elements)
            : elements_(std::move(elements))
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
            elements_.emplace_back(s.data(), s.size());
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
            elements_.emplace_back(s.data(), s.size());
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

        static basic_json_location parse(const jsoncons::basic_string_view<char_type>& normalized_path)
        {
            jsonpath::detail::json_location_parser<char,std::allocator<char>> parser;

            std::vector<jsonpath::path_element> location = parser.parse(normalized_path);
            return basic_json_location(std::move(location));
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
