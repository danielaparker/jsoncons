// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPOINTER_HPP
#define JSONCONS_JSONPOINTER_JSONPOINTER_HPP

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <iterator>
#include <utility> // std::move
#include <system_error> // system_error
#include <type_traits> // std::enable_if, std::true_type
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error.hpp>
#include <jsoncons/detail/write_number.hpp>

namespace jsoncons { namespace jsonpointer {

namespace detail {

enum class pointer_state 
{
    start,
    escaped,
    delim
};

} // detail

    // json_ptr_iterator
    template <class InputIt>
    class json_ptr_iterator
    {
        using char_type = typename std::iterator_traits<InputIt>::value_type;
        using string_type = std::basic_string<char_type>;
        using base_iterator = InputIt;

        base_iterator path_ptr_;
        base_iterator end_input_;
        base_iterator p_;
        base_iterator q_;
        jsonpointer::detail::pointer_state state_;
        std::size_t line_;
        std::size_t column_;
        std::basic_string<char_type> buffer_;
    public:
        using value_type = string_type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::input_iterator_tag;

        json_ptr_iterator(base_iterator first, base_iterator last)
            : json_ptr_iterator(first, last, first)
        {
            std::error_code ec;
            increment(ec);
        }

        json_ptr_iterator(base_iterator first, base_iterator last, base_iterator current)
            : path_ptr_(first), end_input_(last), p_(current), q_(current), state_(jsonpointer::detail::pointer_state::start)
        {
        }

        json_ptr_iterator(const json_ptr_iterator&) = default;

        json_ptr_iterator(json_ptr_iterator&&) = default;

        json_ptr_iterator& operator=(const json_ptr_iterator&) = default;

        json_ptr_iterator& operator=(json_ptr_iterator&&) = default;

        json_ptr_iterator& operator++()
        {
            std::error_code ec;
            increment(ec);
            if (ec)
            {
                JSONCONS_THROW(jsonpointer_error(ec));
            }
            return *this;
        }

        json_ptr_iterator& increment(std::error_code& ec)
        {
            q_ = p_;
            buffer_.clear();

            bool done = false;
            while (p_ != end_input_ && !done)
            {
                switch (state_)
                {
                    case jsonpointer::detail::pointer_state::start: 
                        switch (*p_)
                        {
                            case '/':
                                state_ = jsonpointer::detail::pointer_state::delim;
                                break;
                            default:
                                ec = jsonpointer_errc::expected_slash;
                                done = true;
                                break;
                        };
                        break;
                    case jsonpointer::detail::pointer_state::delim: 
                        switch (*p_)
                        {
                            case '/':
                                state_ = jsonpointer::detail::pointer_state::delim;
                                done = true;
                                break;
                            case '~':
                                state_ = jsonpointer::detail::pointer_state::escaped;
                                break;
                            default:
                                buffer_.push_back(*p_);
                                break;
                        };
                        break;
                    case jsonpointer::detail::pointer_state::escaped: 
                        switch (*p_)
                        {
                            case '0':
                                buffer_.push_back('~');
                                state_ = jsonpointer::detail::pointer_state::delim;
                                break;
                            case '1':
                                buffer_.push_back('/');
                                state_ = jsonpointer::detail::pointer_state::delim;
                                break;
                            default:
                                ec = jsonpointer_errc::expected_0_or_1;
                                done = true;
                                break;
                        };
                        break;
                    default:
                        JSONCONS_UNREACHABLE();
                        break;
                }
                ++p_;
                ++column_;
            }
            return *this;
        }

        json_ptr_iterator operator++(int) // postfix increment
        {
            json_ptr_iterator temp(*this);
            ++(*this);
            return temp;
        }

        reference operator*() const
        {
            return buffer_;
        }

        friend bool operator==(const json_ptr_iterator& it1, const json_ptr_iterator& it2)
        {
            return it1.q_ == it2.q_;
        }
        friend bool operator!=(const json_ptr_iterator& it1, const json_ptr_iterator& it2)
        {
            return !(it1 == it2);
        }

    private:
    };

    template <class CharT>
    std::basic_string<CharT> escape_string(const std::basic_string<CharT>& s)
    {
        std::basic_string<CharT> result;
        for (auto c : s)
        {
            switch (c)
            {
                case '~':
                    result.push_back('~');
                    result.push_back('0');
                    break;
                case '/':
                    result.push_back('~');
                    result.push_back('1');
                    break;
                default:
                    result.push_back(c);
                    break;
            }
        }
        return result;
    }

    // basic_json_ptr

    template <class CharT>
    class basic_json_ptr
    {
    public:
        std::basic_string<CharT> path_;
    public:
        // Member types
        using char_type = CharT;
        using string_type = std::basic_string<char_type>;
        using string_view_type = jsoncons::basic_string_view<char_type>;
        using const_iterator = json_ptr_iterator<typename string_type::const_iterator>;
        using iterator = const_iterator;

        // Constructors
        basic_json_ptr()
        {
        }
        explicit basic_json_ptr(const string_type& s)
            : path_(s)
        {
        }
        explicit basic_json_ptr(string_type&& s)
            : path_(std::move(s))
        {
        }
        explicit basic_json_ptr(const CharT* s)
            : path_(s)
        {
        }

        basic_json_ptr(const basic_json_ptr&) = default;

        basic_json_ptr(basic_json_ptr&&) = default;

        // operator=
        basic_json_ptr& operator=(const basic_json_ptr&) = default;

        basic_json_ptr& operator=(basic_json_ptr&&) = default;

        // Modifiers

        void clear()
        {
            path_.clear();
        }

        basic_json_ptr& operator/=(const string_type& s)
        {
            path_.push_back('/');
            path_.append(escape_string(s));

            return *this;
        }

        basic_json_ptr& operator+=(const basic_json_ptr& p)
        {
            path_.append(p.path_);
            return *this;
        }

        // Accessors
        bool empty() const
        {
          return path_.empty();
        }

        const string_type& string() const
        {
            return path_;
        }

        operator string_view_type() const
        {
            return path_;
        }

        // Iterators
        iterator begin() const
        {
            return iterator(path_.begin(),path_.end());
        }
        iterator end() const
        {
            return iterator(path_.begin(), path_.end(), path_.end());
        }

        // Non-member functions
        friend basic_json_ptr<CharT> operator/(const basic_json_ptr<CharT>& lhs, const string_type& rhs)
        {
            basic_json_ptr<CharT> p(lhs);
            p /= rhs;
            return p;
        }

        friend basic_json_ptr<CharT> operator+( const basic_json_ptr<CharT>& lhs, const basic_json_ptr<CharT>& rhs )
        {
            basic_json_ptr<CharT> p(lhs);
            p += rhs;
            return p;
        }

        friend bool operator==( const basic_json_ptr& lhs, const basic_json_ptr& rhs )
        {
            return lhs.path_ == rhs.path_;
        }

        friend bool operator!=( const basic_json_ptr& lhs, const basic_json_ptr& rhs )
        {
            return lhs.path_ != rhs.path_;
        }

        friend std::basic_ostream<CharT>&
        operator<<( std::basic_ostream<CharT>& os, const basic_json_ptr<CharT>& p )
        {
            os << p.path_;
            return os;
        }
    };

    using json_ptr = basic_json_ptr<char>;
    using wjson_ptr = basic_json_ptr<wchar_t>;

    #if !defined(JSONCONS_NO_DEPRECATED)
    template<class CharT>
    using basic_address = basic_json_ptr<CharT>;
    JSONCONS_DEPRECATED_MSG("Instead, use json_ptr") typedef json_ptr address;
    #endif

    namespace detail {

    template <class J,class JReference>
    class handle_type
    {
    public:
        using reference = JReference;
        using type = reference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JReference>::type>::value,typename J::const_pointer,typename J::pointer>::type;

        handle_type(reference ref) noexcept
            : ptr_(std::addressof(ref))
        {
        }

        handle_type(const handle_type&) noexcept = default;

        handle_type& operator=(const handle_type&) noexcept = default;

        type get() const noexcept
        {
            return *ptr_;
        }
    private:
        pointer ptr_;
    };

    template<class J,class JReference>
    class jsonpointer_evaluator : public ser_context
    {
        using type = typename handle_type<J,JReference>::type;
        using char_type = typename J::char_type;
        using string_type = typename std::basic_string<char_type>;
        using string_view_type = typename J::string_view_type;
        using reference = JReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JReference>::type>::value,typename J::const_pointer,typename J::pointer>::type;

        std::size_t line_;
        std::size_t column_;
        string_type buffer_;
        std::vector<handle_type<J,JReference>> current_;
    public:
        type get_result() 
        {
            return current_.back().get();
        }

        void get(reference root, const string_view_type& path, std::error_code& ec)
        {
            evaluate(root, path, ec);
            if (ec)
            {
                return;
            }
            if (path.empty())
            {
                return;
            }
            resolve(current_, buffer_, ec);
        }

        string_type normalized_path(reference root, const string_view_type& path)
        {
            std::error_code ec;
            evaluate(root, path, ec);
            if (ec)
            {
                return string_type(path);
            }
            if (current_.back().get().is_array() && buffer_.size() == 1 && buffer_[0] == '-')
            {
                string_type p = string_type(path.substr(0,path.length()-1));
                std::string s = std::to_string(current_.back().get().size());
                for (auto c : s)
                {
                    p.push_back(c);
                }
                return p;
            }
            else
            {
                return string_type(path);
            }
        }

        void insert_or_assign(reference root, const string_view_type& path, const J& value, std::error_code& ec)
        {
            evaluate(root, path, ec);
            if (ec)
            {
                return;
            }
            if (current_.back().get().is_array())
            {
                if (buffer_.size() == 1 && buffer_[0] == '-')
                {
                    current_.back().get().push_back(value);
                }
                else
                {
                    if (!jsoncons::detail::is_base10(buffer_.data(), buffer_.length()))
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    auto result = jsoncons::detail::to_integer<std::size_t>(buffer_.data(), buffer_.length());
                    if (!result)
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    std::size_t index = result.value();
                    if (index > current_.back().get().size())
                    {
                        ec = jsonpointer_errc::index_exceeds_array_size;
                        return;
                    }
                    if (index == current_.back().get().size())
                    {
                        current_.back().get().push_back(value);
                    }
                    else
                    {
                        current_.back().get().insert(current_.back().get().array_range().begin()+index,value);
                    }
                }
            }
            else if (current_.back().get().is_object())
            {
                current_.back().get().insert_or_assign(buffer_,value);
            }
            else
            {
                ec = jsonpointer_errc::expected_object_or_array;
                return;
            }
        }

        void insert(reference root, const string_view_type& path, const J& value, std::error_code& ec)
        {
            evaluate(root, path, ec);
            if (ec)
            {
                return;
            }
            if (current_.back().get().is_array())
            {
                if (buffer_.size() == 1 && buffer_[0] == '-')
                {
                    current_.back().get().push_back(value);
                }
                else
                {
                    if (!jsoncons::detail::is_base10(buffer_.data(), buffer_.length()))
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    auto result = jsoncons::detail::to_integer<std::size_t>(buffer_.data(), buffer_.length());
                    if (!result)
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    std::size_t index = result.value();
                    if (index > current_.back().get().size())
                    {
                        ec = jsonpointer_errc::index_exceeds_array_size;
                        return;
                    }
                    if (index == current_.back().get().size())
                    {
                        current_.back().get().push_back(value);
                    }
                    else
                    {
                        current_.back().get().insert(current_.back().get().array_range().begin()+index,value);
                    }
                }
            }
            else if (current_.back().get().is_object())
            {
                if (current_.back().get().contains(buffer_))
                {
                    ec = jsonpointer_errc::key_already_exists;
                    return;
                }
                else
                {
                    current_.back().get().insert_or_assign(buffer_,value);
                }
            }
            else
            {
                ec = jsonpointer_errc::expected_object_or_array;
                return;
            }
        }

        void remove(reference root, const string_view_type& path, std::error_code& ec)
        {
            evaluate(root, path, ec);
            if (ec)
            {
                return;
            }
            if (current_.back().get().is_array())
            {
                if (buffer_.size() == 1 && buffer_[0] == '-')
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                else
                {
                    if (!jsoncons::detail::is_base10(buffer_.data(), buffer_.length()))
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    auto result = jsoncons::detail::to_integer<std::size_t>(buffer_.data(), buffer_.length());
                    if (!result)
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    std::size_t index = result.value();
                    if (index >= current_.back().get().size())
                    {
                        ec = jsonpointer_errc::index_exceeds_array_size;
                        return;
                    }
                    current_.back().get().erase(current_.back().get().array_range().begin()+index);
                }
            }
            else if (current_.back().get().is_object())
            {
                if (!current_.back().get().contains(buffer_))
                {
                    ec = jsonpointer_errc::name_not_found;
                    return;
                }
                else
                {
                    current_.back().get().erase(buffer_);
                }
            }
            else
            {
                ec = jsonpointer_errc::expected_object_or_array;
                return;
            }
        }

        void replace(reference root, const string_view_type& path, const J& value, std::error_code& ec)
        {
            evaluate(root, path, ec);
            if (ec)
            {
                return;
            }
            if (current_.back().get().is_array())
            {
                if (buffer_.size() == 1 && buffer_[0] == '-')
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                else
                {
                    if (!jsoncons::detail::is_base10(buffer_.data(), buffer_.length()))
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    auto result = jsoncons::detail::to_integer<std::size_t>(buffer_.data(), buffer_.length());
                    if (!result)
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    std::size_t index = result.value();
                    if (index >= current_.back().get().size())
                    {
                        ec = jsonpointer_errc::index_exceeds_array_size;
                        return;
                    }
                    (current_.back().get())[index] = value;
                }
            }
            else if (current_.back().get().is_object())
            {
                if (!current_.back().get().contains(buffer_))
                {
                    ec = jsonpointer_errc::key_already_exists;
                    return;
                }
                else
                {
                    current_.back().get().insert_or_assign(buffer_,value);
                }
            }
            else
            {
                ec = jsonpointer_errc::expected_object_or_array;
                return;
            }
        }

        void evaluate(reference root, const string_view_type& path, std::error_code& ec)
        {
            current_.push_back(root);

            json_ptr_iterator<typename string_view_type::iterator> it(path.begin(), path.end());
            json_ptr_iterator<typename string_view_type::iterator> end(path.begin(), path.end(), path.end());
            while (it != end)
            {
                buffer_ = *it;
                it.increment(ec);
                if (ec)
                    return;
                if (it == end)
                {
                    return;
                }
                resolve(current_, buffer_, ec);
                if (ec)
                    return;
            }
        }

        static void resolve(std::vector<handle_type<J,JReference>>& current,
                            const string_view_type& buffer,
                            std::error_code& ec)
        {
            if (current.back().get().is_array())
            {
                if (buffer.size() == 1 && buffer[0] == '-')
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                else
                {
                    if (!jsoncons::detail::is_base10(buffer.data(), buffer.length()))
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    auto result = jsoncons::detail::to_integer<std::size_t>(buffer.data(), buffer.length());
                    if (!result)
                    {
                        ec = jsonpointer_errc::invalid_index;
                        return;
                    }
                    std::size_t index = result.value();
                    if (index >= current.back().get().size())
                    {
                        ec = jsonpointer_errc::index_exceeds_array_size;
                        return;
                    }
                    current.push_back(current.back().get().at(index));
                }
            }
            else if (current.back().get().is_object())
            {
                if (!current.back().get().contains(buffer))
                {
                    ec = jsonpointer_errc::name_not_found;
                    return;
                }
                current.push_back(current.back().get().at(buffer));
            }
            else
            {
                ec = jsonpointer_errc::expected_object_or_array;
                return;
            }
        }

        // ser_context

        std::size_t line() const override
        {
            return line_;
        }

        std::size_t column() const override
        {
            return column_;
        }
    };

    }

    template<class J>
    std::basic_string<typename J::char_type> normalized_path(const J& root, const typename J::string_view_type& path)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
        return evaluator.normalized_path(root,path);
    }

    template<class J>
    J& get(J& root, const typename J::string_view_type& path)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;
        std::error_code ec;
        evaluator.get(root, path, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
        return evaluator.get_result();
    }

    template<class J>
    const J& get(const J& root, const typename J::string_view_type& path)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;

        std::error_code ec;
        evaluator.get(root, path, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
        return evaluator.get_result();
    }

    template<class J>
    J& get(J& root, const typename J::string_view_type& path, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;
        evaluator.get(root, path, ec);
        return evaluator.get_result();
    }

    template<class J>
    const J& get(const J& root, const typename J::string_view_type& path, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
        evaluator.get(root, path, ec);
        return evaluator.get_result();
    }

    template<class J>
    bool contains(const J& root, const typename J::string_view_type& path)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
        std::error_code ec;
        evaluator.get(root, path, ec);
        return !ec ? true : false;
    }

    template<class J>
    void insert_or_assign(J& root, const typename J::string_view_type& path, const J& value)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        std::error_code ec;
        evaluator.insert_or_assign(root, path, value, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    template<class J>
    void insert_or_assign(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        evaluator.insert_or_assign(root, path, value, ec);
    }

    template<class J>
    void insert(J& root, const typename J::string_view_type& path, const J& value)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        std::error_code ec;
        evaluator.insert(root, path, value, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    template<class J>
    void insert(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        evaluator.insert(root, path, value, ec);
    }

    template<class J>
    void remove(J& root, const typename J::string_view_type& path)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        std::error_code ec;
        evaluator.remove(root, path, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    template<class J>
    void remove(J& root, const typename J::string_view_type& path, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        evaluator.remove(root, path, ec);
    }

    template<class J>
    void replace(J& root, const typename J::string_view_type& path, const J& value)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        std::error_code ec;
        evaluator.replace(root, path, value, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    template<class J>
    void replace(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
    {
        jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

        evaluator.replace(root, path, value, ec);
    }

    template <class String,class Result>
    typename std::enable_if<std::is_convertible<typename String::value_type,typename Result::value_type>::value>::type
    escape(const String& s, Result& result)
    {
        for (auto c : s)
        {
            if (c == '~')
            {
                result.push_back('~');
                result.push_back('0');
            }
            else if (c == '/')
            {
                result.push_back('~');
                result.push_back('1');
            }
            else
            {
                result.push_back(c);
            }
        }
    }

    template <class CharT>
    std::basic_string<CharT> escape(const jsoncons::basic_string_view<CharT>& s)
    {
        std::basic_string<CharT> result;

        for (auto c : s)
        {
            if (c == '~')
            {
                result.push_back('~');
                result.push_back('0');
            }
            else if (c == '/')
            {
                result.push_back('~');
                result.push_back('1');
            }
            else
            {
                result.push_back(c);
            }
        }
        return result;
    }

    // flatten

    template<class Json>
    void flatten_(const std::basic_string<typename Json::char_type>& parent_key,
                  const Json& parent_value,
                  Json& result)
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        switch (parent_value.type())
        {
            case json_type::array_value:
            {
                if (parent_value.empty())
                {
                    // Flatten empty array to null
                    result.try_emplace(parent_key, null_type{});
                }
                else
                {
                    for (std::size_t i = 0; i < parent_value.size(); ++i)
                    {
                        string_type key(parent_key);
                        key.push_back('/');
                        jsoncons::detail::write_integer(i,key);
                        flatten_(key, parent_value.at(i), result);
                    }
                }
                break;
            }

            case json_type::object_value:
            {
                if (parent_value.empty())
                {
                    // Flatten empty object to null
                    result.try_emplace(parent_key, null_type{});
                }
                else
                {
                    for (const auto& item : parent_value.object_range())
                    {
                        string_type key(parent_key);
                        key.push_back('/');
                        escape(jsoncons::basic_string_view<char_type>(item.key().data(),item.key().size()), key);
                        flatten_(key, item.value(), result);
                    }
                }
                break;
            }

            default:
            {
                // add primitive parent_value with its reference string
                result[parent_key] = parent_value;
                break;
            }
        }
    }

    template<class Json>
    Json flatten(const Json& value)
    {
        Json result;
        std::basic_string<typename Json::char_type> parent_key;
        flatten_(parent_key, value, result);
        return result;
    }


    // unflatten

    enum class unflatten_options {none,assume_object = 1
    #if !defined(JSONCONS_NO_DEPRECATED)
,object = assume_object
#endif
};

    template<class Json>
    Json safe_unflatten (Json& value)
    {
        if (!value.is_object())
        {
            return value;
        }
        bool safe = true;
        std::size_t index = 0;
        for (const auto& item : value.object_range())
        {
            auto r = jsoncons::detail::to_integer<std::size_t>(item.key().data(),item.key().size());
            if (!r || (index++ != r.value()))
            {
                safe = false;
                break;
            }
        }

        if (safe)
        {
            Json j(json_array_arg);
            j.reserve(value.size());
            for (auto& item : value.object_range())
            {
                j.emplace_back(std::move(item.value()));
            }
            Json a(json_array_arg);
            for (auto& item : j.array_range())
            {
                a.emplace_back(safe_unflatten (item));
            }
            return a;
        }
        else
        {
            Json o(json_object_arg);
            for (auto& item : value.object_range())
            {
                o.try_emplace(item.key(), safe_unflatten (item.value()));
            }
            return o;
        }
    }

    template<class Json>
    jsoncons::optional<Json> try_unflatten_array(const Json& value)
    {
        using char_type = typename Json::char_type;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            JSONCONS_THROW(jsonpointer_error(jsonpointer_errc::argument_to_unflatten_invalid));
        }
        Json result;

        for (const auto& item: value.object_range())
        {
            Json* part = &result;
            basic_json_ptr<char_type> ptr(item.key());
            std::size_t index = 0;
            for (auto it = ptr.begin(); it != ptr.end(); )
            {
                auto s = *it;
                auto r = jsoncons::detail::to_integer<size_t>(s.data(), s.size());
                if (r && (index++ == r.value()))
                {
                    if (!part->is_array())
                    {
                        *part = Json(json_array_arg);
                    }
                    if (++it != ptr.end())
                    {
                        if (r.value()+1 > part->size())
                        {
                            Json& ref = part->emplace_back();
                            part = std::addressof(ref);
                        }
                        else
                        {
                            part = &part->at(r.value());
                        }
                    }
                    else
                    {
                        Json& ref = part->emplace_back(item.value());
                        part = std::addressof(ref);
                    }
                }
                else if (part->is_object())
                {
                    if (++it != ptr.end())
                    {
                        auto res = part->try_emplace(s,Json());
                        part = &(res.first->value());
                    }
                    else
                    {
                        auto res = part->try_emplace(s, item.value());
                        part = &(res.first->value());
                    }
                }
                else 
                {
                    return jsoncons::optional<Json>();
                }
            }
        }

        return result;
    }

    template<class Json>
    Json unflatten_to_object(const Json& value, unflatten_options options = unflatten_options::none)
    {
        using char_type = typename Json::char_type;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            JSONCONS_THROW(jsonpointer_error(jsonpointer_errc::argument_to_unflatten_invalid));
        }
        Json result;

        for (const auto& item: value.object_range())
        {
            Json* part = &result;
            basic_json_ptr<char_type> ptr(item.key());
            for (auto it = ptr.begin(); it != ptr.end(); )
            {
                auto s = *it;
                if (++it != ptr.end())
                {
                    auto res = part->try_emplace(s,Json());
                    part = &(res.first->value());
                }
                else
                {
                    auto res = part->try_emplace(s, item.value());
                    part = &(res.first->value());
                }
            }
        }

        return options == unflatten_options::none ? safe_unflatten (result) : result;
    }

    template<class Json>
    Json unflatten(const Json& value, unflatten_options options = unflatten_options::none)
    {
        if (options == unflatten_options::none)
        {
            jsoncons::optional<Json> j = try_unflatten_array(value);
            return j ? *j : unflatten_to_object(value,options);
        }
        else
        {
            return unflatten_to_object(value,options);
        }
    }

} // namespace jsonpointer
} // namespace jsoncons

#endif
