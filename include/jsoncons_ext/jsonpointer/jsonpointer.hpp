// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPOINTER_HPP
#define JSONCONS_JSONPOINTER_JSONPOINTER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>
#include <iterator>
#include <utility> // std::move
#include <type_traits> // std::enable_if, std::true_type
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error.hpp>
#include <jsoncons/detail/print_number.hpp>

namespace jsoncons { namespace jsonpointer {

class jsonpointer_error : public std::exception, public virtual json_exception
{
public:
    jsonpointer_error(const std::error_code& ec)
        : error_code_(ec)
    {
    }
    jsonpointer_error(const jsonpointer_error& other) = default;

    jsonpointer_error(jsonpointer_error&& other) = default;

    const char* what() const noexcept override
    {
        try
        {
            const_cast<std::string&>(buffer_) = error_code_.message();
            return buffer_.c_str();
        }
        catch (...)
        {
            return "";
        }
    }

    const std::error_code code() const
    {
        return error_code_;
    }

    jsonpointer_error& operator=(const jsonpointer_error& e) = default;
    jsonpointer_error& operator=(jsonpointer_error&& e) = default;
private:
    std::string buffer_;
    std::error_code error_code_;
};

// find_by_reference

template <class J, class Enable=void>
struct is_accessible_by_reference : std::false_type {};

template <class J>
struct is_accessible_by_reference<J, 
                            typename std::enable_if<std::is_reference<decltype(std::declval<J>().at(typename J::string_view_type()))>::value
                                                    && std::is_reference<decltype(std::declval<J>().at(0))>::value>::type> 
: std::true_type {};

namespace detail {

enum class pointer_state 
{
    start,
    escaped,
    delim
};

} // detail

// path_iterator
template <class InputIt>
class path_iterator
{
    typedef typename std::iterator_traits<InputIt>::value_type char_type;
    typedef std::basic_string<char_type> string_type;
    typedef InputIt base_iterator;

    jsonpointer::detail::pointer_state state_;
    size_t line_;
    size_t column_;
    base_iterator path_ptr_;
    base_iterator end_input_;
    base_iterator p_;
    base_iterator q_;
    std::basic_string<char_type> buffer_;
public:
    typedef string_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type* pointer;
    typedef const value_type& reference;
    typedef std::input_iterator_tag iterator_category;

    path_iterator(base_iterator first, base_iterator last)
        : path_iterator(first, last, first)
    {
        std::error_code ec;
        increment(ec);
    }

    path_iterator(base_iterator first, base_iterator last, base_iterator current)
    {
        path_ptr_ = first;
        end_input_ = last;
        p_ = current;
        q_ = current;
    }

    path_iterator(const path_iterator&) = default;

    path_iterator(path_iterator&&) = default;

    path_iterator& operator=(const path_iterator&) = default;

    path_iterator& operator=(path_iterator&&) = default;

    path_iterator& operator++()
    {
        std::error_code ec;
        increment(ec);
        if (ec)
        {
            throw jsonpointer_error(ec);
        }
        return *this;
    }

    path_iterator& increment(std::error_code& ec)
    {
        q_ = p_;
        buffer_.clear();

        bool done = false;
        while (p_ < end_input_ && !done)
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
                        ++p_;
                        ++column_;
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
                        ++p_;
                        ++column_;
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
                        ++p_;
                        ++column_;
                        break;
            }
        }
        return *this;
    }

    path_iterator operator++(int) // postfix increment
    {
        path_iterator temp(*this);
        return temp;
    }

    path_iterator& operator--()
    {
        return *this;
    }

    path_iterator operator--(int)
    {
        path_iterator temp(*this);
        return temp;
    }

    reference operator*() const
    {
        return buffer_;
    }

    friend bool operator==(const path_iterator& it1, const path_iterator& it2)
    {
        return it1.q_ == it2.q_;
    }
    friend bool operator!=(const path_iterator& it1, const path_iterator& it2)
    {
        return !(it1 == it2);
    }

private:
};

template <class Source>
Source escape_string(const Source& s)
{
    Source result;
    auto begin = s.begin();
    auto end = s.end();
    for (auto it = begin; it != end; ++it)
    {
        switch (*it)
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
                result.push_back(*it);
                break;
        }
    }
    return result;
}

// path

template <class CharT>
class basic_path
{
public:
    std::basic_string<CharT> path_;
public:
    typedef CharT char_type;
    typedef std::basic_string<char_type> string_type;
    typedef path_iterator<typename string_type::const_iterator> const_iterator;
    typedef const_iterator iterator;

    basic_path()
    {
    }
    basic_path(const std::basic_string<CharT>& path)
        : path_(path)
    {
    }
    basic_path(std::basic_string<CharT>&& path)
        : path_(std::move(path))
    {
    }
    basic_path(const basic_string_view<CharT>& path)
        : path_(path.begin(),path.end())
    {
    }
    basic_path(const CharT* s)
        : path_(s)
    {
    }

    basic_path(const basic_path&) = default;

    basic_path(basic_path&&) = default;

    basic_path& operator=(const basic_path&) = default;

    basic_path& operator=(basic_path&&) = default;

    bool empty() const
    {
      return path_.empty();
    }

    const std::basic_string<CharT>& string() const
    {
        return path_;
    }

    iterator begin()
    {
        return iterator(path_.begin(),path_.end());
    }
    iterator end()
    {
        return iterator(path_.begin(), path_.end(), path_.end());
    }

    const_iterator begin() const
    {
        return iterator(path_.begin(),path_.end());
    }
    const iterator end() const
    {
        return iterator(path_.begin(), path_.end(), path_.end());
    }

    basic_path& append(const std::basic_string<CharT>& rhs)
    {
        path_.push_back('/');
        path_.append(escape_string(rhs));

        return *this;
    }

    friend std::basic_ostream<CharT>&
    operator<<( std::basic_ostream<CharT>& os, const basic_path<CharT>& p )
    {
        os << p.path_;
        return os;
    }
};

typedef basic_path<char> path;

namespace detail {

template <class J,class JReference,class Enable = void>
class handle_type
{
public:
    using value_type = typename J::value_type;
    using type = value_type;

    handle_type(const value_type& val) noexcept
        : val_(val)
    {
    }

    handle_type(value_type&& val) noexcept
        : val_(std::move(val))
    {
    }

    handle_type(const handle_type& w) noexcept
        : val_(w.val_)
    {
    }

    handle_type& operator=(const handle_type&) noexcept = default;

    type get() const noexcept
    {
        return val_;
    }
private:
    value_type val_;
};

template <class J,class JReference>
class handle_type<J,JReference,
                  typename std::enable_if<is_accessible_by_reference<J>::value>::type>
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
class jsonpointer_evaluator : private serializing_context
{
    typedef typename handle_type<J,JReference>::type type;
    typedef typename J::string_type string_type;
    typedef typename string_type::value_type char_type;
    typedef typename J::string_view_type string_view_type;
    using reference = JReference;
    using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JReference>::type>::value,typename J::const_pointer,typename J::pointer>::type;

    size_t line_;
    size_t column_;
    string_type buffer_;
    std::vector<handle_type<J,JReference>> current_;
public:
    type get_result() 
    {
        return current_.back().get();
    }

    jsonpointer_errc get(reference root, const basic_path<char_type>& path)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }
        if (path.empty())
        {
            return jsonpointer_errc();
        }
        return resolve(current_,buffer_);
    }

    string_type normalized_path(reference root, const basic_path<char_type>& path)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return path.string();
        }
        if (current_.back().get().is_array() && buffer_.size() == 1 && buffer_[0] == '-')
        {
            string_type p = string_type(path.string().substr(0,path.string().length()-1));
            std::string s = std::to_string(current_.back().get().size());
            for (auto c : s)
            {
                p.push_back(c);
            }
            return p;
        }
        else
        {
            return path.string();
        }
    }

    jsonpointer_errc insert_or_assign(reference root, const basic_path<char_type>& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }
        if (current_.back().get().is_array())
        {
            if (buffer_.size() == 1 && buffer_[0] == '-')
            {
                current_.back().get().push_back(value);
            }
            else
            {
                if (!jsoncons::detail::is_integer(buffer_.data(), buffer_.length()))
                {
                    return jsonpointer_errc::invalid_index;
                }
                auto result = jsoncons::detail::to_integer<size_t>(buffer_.data(), buffer_.length());
                if (result.overflow)
                {
                    return jsonpointer_errc::invalid_index;
                }
                size_t index = result.value;
                current_.back().get().insert(current_.back().get().array_range().begin()+index,value);
            }
        }
        else if (current_.back().get().is_object())
        {
            current_.back().get().insert_or_assign(buffer_,value);
        }
        else
        {
            return jsonpointer_errc::expected_object_or_array;
        }
        return jsonpointer_errc();
    }

    jsonpointer_errc insert(reference root, const basic_path<char_type>& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }
        if (current_.back().get().is_array())
        {
            if (buffer_.size() == 1 && buffer_[0] == '-')
            {
                current_.back().get().push_back(value);
            }
            else
            {
                if (!jsoncons::detail::is_integer(buffer_.data(), buffer_.length()))
                {
                    return jsonpointer_errc::invalid_index;
                }
                auto result = jsoncons::detail::to_integer<size_t>(buffer_.data(), buffer_.length());
                if (result.overflow)
                {
                    return jsonpointer_errc::invalid_index;
                }
                size_t index = result.value;
                current_.back().get().insert(current_.back().get().array_range().begin()+index,value);
            }
        }
        else if (current_.back().get().is_object())
        {
            if (current_.back().get().contains(buffer_))
            {
                return jsonpointer_errc::key_already_exists;
            }
            else
            {
                current_.back().get().insert_or_assign(buffer_,value);
            }
        }
        else
        {
            return jsonpointer_errc::expected_object_or_array;
        }
        return jsonpointer_errc();
    }

    jsonpointer_errc remove(reference root, const basic_path<char_type>& path)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }
        if (current_.back().get().is_array())
        {
            if (buffer_.size() == 1 && buffer_[0] == '-')
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            else
            {
                if (!jsoncons::detail::is_integer(buffer_.data(), buffer_.length()))
                {
                    return jsonpointer_errc::invalid_index;
                }
                auto result = jsoncons::detail::to_integer<size_t>(buffer_.data(), buffer_.length());
                if (result.overflow)
                {
                    return jsonpointer_errc::invalid_index;
                }
                size_t index = result.value;
                if (index >= current_.back().get().size())
                {
                    return jsonpointer_errc::index_exceeds_array_size;
                }
                current_.back().get().erase(current_.back().get().array_range().begin()+index);
            }
        }
        else if (current_.back().get().is_object())
        {
            if (!current_.back().get().contains(buffer_))
            {
                return jsonpointer_errc::name_not_found;
            }
            else
            {
                current_.back().get().erase(buffer_);
            }
        }
        else
        {
            return jsonpointer_errc::expected_object_or_array;
        }
        return jsonpointer_errc();
    }

    jsonpointer_errc replace(reference root, const basic_path<char_type>& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }
        if (current_.back().get().is_array())
        {
            if (buffer_.size() == 1 && buffer_[0] == '-')
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            else
            {
                if (!jsoncons::detail::is_integer(buffer_.data(), buffer_.length()))
                {
                    return jsonpointer_errc::invalid_index;
                }
                auto result = jsoncons::detail::to_integer<size_t>(buffer_.data(), buffer_.length());
                if (result.overflow)
                {
                    return jsonpointer_errc::invalid_index;
                }
                size_t index = result.value;
                if (index >= current_.back().get().size())
                {
                    return jsonpointer_errc::index_exceeds_array_size;
                }
                (current_.back().get())[index] = value;
            }
        }
        else if (current_.back().get().is_object())
        {
            if (!current_.back().get().contains(buffer_))
            {
                return jsonpointer_errc::key_already_exists;
            }
            else
            {
                current_.back().get().insert_or_assign(buffer_,value);
            }
        }
        else
        {
            return jsonpointer_errc::expected_object_or_array;
        }
        return jsonpointer_errc();
    }

    jsonpointer_errc evaluate(reference root, const basic_path<char_type>& path)
    {
        jsonpointer_errc ec = jsonpointer_errc();

        current_.push_back(root);

        auto it = path.begin();
        auto end = path.end();

        while (it != end)
        {
            buffer_ = *it;
            ++it;
            if (it == end)
            {
                return jsonpointer_errc();
            }
            ec = resolve(current_, buffer_);
            if (ec != jsonpointer_errc())
                return ec;
        }
        return ec;
    }

    static jsonpointer_errc resolve(std::vector<handle_type<J,JReference>>& current,
                                    const string_view_type& buffer)
    {
        if (current.back().get().is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            else
            {
                if (!jsoncons::detail::is_integer(buffer.data(), buffer.length()))
                {
                    return jsonpointer_errc::invalid_index;
                }
                auto result = jsoncons::detail::to_integer<size_t>(buffer.data(), buffer.length());
                if (result.overflow)
                {
                    return jsonpointer_errc::invalid_index;
                }
                size_t index = result.value;
                if (index >= current.back().get().size())
                {
                    return jsonpointer_errc::index_exceeds_array_size;
                }
                current.push_back(current.back().get().at(index));
            }
        }
        else if (current.back().get().is_object())
        {
            if (!current.back().get().contains(buffer))
            {
                return jsonpointer_errc::name_not_found;
            }
            current.push_back(current.back().get().at(buffer));
        }
        else
        {
            return jsonpointer_errc::expected_object_or_array;
        }
        return jsonpointer_errc();
    }

    // serializing_context

    size_t line_number() const override
    {
        return line_;
    }

    size_t column_number() const override
    {
        return column_;
    }
};

}

template<class J>
typename J::string_type normalized_path(const J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    return evaluator.normalized_path(root,path);
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,J&>::type
get(J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,const J&>::type
get(const J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}
/*
template<class J>
typename std::enable_if<!is_accessible_by_reference<J>::value,J>::type
get(const J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}
*/
template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,J&>::type
get(J& root, const basic_path<typename J::char_type>& path, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}
/*
template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,const J&>::type
get(const J& root, const basic_path<typename J::char_type>& path, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}
*/
template<class J>
typename std::enable_if<!is_accessible_by_reference<J>::value,J>::type
get(const J& root, const basic_path<typename J::char_type>& path, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}

template<class J>
bool contains(const J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    return ec == jsonpointer_errc() ? true : false;
}

template<class J>
void insert_or_assign(J& root, const basic_path<typename J::char_type>& path, const J& value)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.insert_or_assign(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void insert_or_assign(J& root, const basic_path<typename J::char_type>& path, const J& value, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.insert_or_assign(root,path,value);
}

template<class J>
void insert(J& root, const basic_path<typename J::char_type>& path, const J& value)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.insert(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void insert(J& root, const basic_path<typename J::char_type>& path, const J& value, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.insert(root,path,value);
}

template<class J>
void remove(J& root, const basic_path<typename J::char_type>& path)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.remove(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void remove(J& root, const basic_path<typename J::char_type>& path, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.remove(root,path);
}

template<class J>
void replace(J& root, const basic_path<typename J::char_type>& path, const J& value)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.replace(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void replace(J& root, const basic_path<typename J::char_type>& path, const J& value, std::error_code& ec)
{
    jsoncons::jsonpointer::detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.replace(root,path,value);
}

template <class String>
void escape(const String& s, std::basic_ostringstream<typename String::value_type>& os)
{
    for (auto c : s)
    {
        if (c == '~')
        {
            os.put('~');
            os.put('0');
        }
        else if (c == '/')
        {
            os.put('~');
            os.put('1');
        }
        else
        {
            os.put(c);
        }
    }
}

}}

#endif
