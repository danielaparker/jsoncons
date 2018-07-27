// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPOINTER_HPP
#define JSONCONS_JSONPOINTER_JSONPOINTER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error_category.hpp>

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

    const char* what() const JSONCONS_NOEXCEPT override
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
    array_reference_token,
    zero_array_reference_token,
    nonzero_array_reference_token,
    after_last_array_reference_token,
    object_reference_token,
    escaped
};
template <class J,class JReference,class Enable = void>
class handle_type
{
public:
    using value_type = typename J::value_type;
    using type = value_type;

    handle_type(const value_type& val) JSONCONS_NOEXCEPT
        : val_(val)
    {
    }

    handle_type(value_type&& val) JSONCONS_NOEXCEPT
        : val_(std::move(val))
    {
    }

    handle_type(const handle_type& w) JSONCONS_NOEXCEPT
        : val_(w.val_)
    {
    }

    handle_type& operator=(const handle_type&) JSONCONS_NOEXCEPT = default;

    type get() const JSONCONS_NOEXCEPT
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

    handle_type(reference ref) JSONCONS_NOEXCEPT
        : ptr_(std::addressof(ref))
    {
    }

    handle_type(const handle_type&) JSONCONS_NOEXCEPT = default;

    handle_type& operator=(const handle_type&) JSONCONS_NOEXCEPT = default;

    type get() const JSONCONS_NOEXCEPT
    {
        return *ptr_;
    }
private:
    pointer ptr_;
};

template<class J,class JReference>
struct path_resolver
{
    typedef typename J::string_view_type string_view_type;

    jsonpointer_errc operator()(std::vector<handle_type<J,JReference>>& current,
                                size_t index) const
    {
        if (index >= current.back().get().size())
        {
            return jsonpointer_errc::index_exceeds_array_size;
        }
        current.push_back(current.back().get().at(index));
        return jsonpointer_errc();
    }

    jsonpointer_errc operator()(std::vector<handle_type<J,JReference>>& current,
                                const string_view_type& name) const
    {
        if (!current.back().get().has_key(name))
        {
            return jsonpointer_errc::name_not_found;
        }
        current.push_back(current.back().get().at(name));
        return jsonpointer_errc();
    }
};

template<class J, class JReference>
struct path_setter
{
    typedef typename J::string_view_type string_view_type;

    jsonpointer_errc operator()(std::vector<handle_type<J,JReference>>& current,
                                size_t index) const
    {
        if (index >= current.back().get().size())
        {
            return jsonpointer_errc::index_exceeds_array_size;
        }
        current.push_back(current.back().get().at(index));
        return jsonpointer_errc();
    }

    jsonpointer_errc operator()(std::vector<handle_type<J,JReference>>& current,
                                const string_view_type& name) const
    {
        jsonpointer_errc ec = jsonpointer_errc();
        if (!current.back().get().has_key(name))
        {
            return jsonpointer_errc::name_not_found;
        }
        current.push_back(current.back().get().at(name));
        return ec;
    }
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

    jsonpointer::detail::pointer_state state_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    string_type buffer_;
    size_t index_;
    std::vector<handle_type<J,JReference>> current_;
public:
    type get_result() 
    {
        return current_.back().get();
    }

    size_t line_number() const
    {
        return line_;
    }

    size_t column_number() const
    {
        return column_;
    }

    jsonpointer_errc get(reference root, const string_view_type& path)
    {
        path_resolver<J,reference> op;
        jsonpointer_errc ec = evaluate(root,op,path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }

        switch (state_)
        {
        case pointer_state::start: 
            break;
        case pointer_state::zero_array_reference_token: 
        case pointer_state::nonzero_array_reference_token: 
            ec = op(current_,index_);
            break;
        case pointer_state::object_reference_token: 
            ec = op(current_,buffer_);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    string_type normalized_path(reference root, const string_view_type& path)
    {
        jsonpointer_errc ec = evaluate(root,path_setter<J,reference>(),path);
        if (ec != jsonpointer_errc())
        {
            return string_type(path);
        }
        if (state_ == jsonpointer::detail::pointer_state::after_last_array_reference_token)
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

    jsonpointer_errc insert_or_assign(reference root, const string_view_type& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path_setter<J,reference>(),path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ > current_.back().get().size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            if (index_ == current_.back().get().size())
            {
                current_.back().get().push_back(value);
            }
            else
            {
                current_.back().get().insert(current_.back().get().array_range().begin()+index_,value);
            }
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            current_.back().get().push_back(value);
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            current_.back().get().insert_or_assign(buffer_,value);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc insert(reference root, const string_view_type& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path_setter<J,reference>(),path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ > current_.back().get().size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            if (index_ == current_.back().get().size())
            {
                current_.back().get().push_back(value);
            }
            else
            {
                current_.back().get().insert(current_.back().get().array_range().begin()+index_,value);
            }
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            current_.back().get().push_back(value);
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (current_.back().get().has_key(buffer_))
            {
                ec = jsonpointer_errc::key_already_exists;
            }
            else
            {
                current_.back().get().insert_or_assign(buffer_,value);
            }
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc remove(reference root, const string_view_type& path)
    {
        jsonpointer_errc ec = evaluate(root,path_resolver<J,reference>(),path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ >= current_.back().get().size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            current_.back().get().erase(current_.back().get().array_range().begin()+index_);
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            return jsonpointer_errc::index_exceeds_array_size;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back().get().has_key(buffer_))
            {
                return jsonpointer_errc::name_not_found;
            }
            current_.back().get().erase(buffer_);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc replace(reference root, const string_view_type& path, const J& value)
    {
        jsonpointer_errc ec = evaluate(root,path_resolver<J,reference>(),path);
        if (ec != jsonpointer_errc())
        {
            return ec;
        }

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ >= current_.back().get().size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            (current_.back().get())[index_] = value;
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            return jsonpointer_errc::index_exceeds_array_size;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back().get().has_key(buffer_))
            {
                return jsonpointer_errc::name_not_found;
            }
            current_.back().get().insert_or_assign(buffer_,value);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return jsonpointer_errc();
    }

    template <class Op>
    jsonpointer_errc evaluate(reference root, Op op, const string_view_type& path)
    {
        jsonpointer_errc ec = jsonpointer_errc();

        line_ = 1;
        column_ = 1;
        state_ = jsonpointer::detail::pointer_state::start;
        begin_input_ = path.data();
        end_input_ = path.data() + path.length();
        p_ = begin_input_;

        index_ = 0;

        current_.push_back(root);

        while (p_ < end_input_)
        {
            switch (state_)
            {
            case jsonpointer::detail::pointer_state::start: 
                switch (*p_)
                {
                case '/':
                    if (current_.back().get().is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back().get().is_object())
                    {
                        state_ = jsonpointer::detail::pointer_state::object_reference_token;
                        buffer_.clear();
                    }
                    else
                    {
                        return jsonpointer_errc::expected_object_or_array;
                    }
                    break;
                default:
                    return jsonpointer_errc::expected_slash;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::array_reference_token: 
                switch (*p_)
                {
                case '0':
                    index_ = 0;
                    state_ = jsonpointer::detail::pointer_state::zero_array_reference_token;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    index_ = (*p_ - '0');
                    state_ = jsonpointer::detail::pointer_state::nonzero_array_reference_token;
                    break;
                case '-':
                    state_ = jsonpointer::detail::pointer_state::after_last_array_reference_token;
                    break;
                default:
                    return jsonpointer_errc::expected_digit_or_dash;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::zero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    ec = op(current_,index_);
                    if (ec != jsonpointer_errc())
                    {
                        return ec;
                    }
                    if (current_.back().get().is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back().get().is_object())
                    {
                        state_ = jsonpointer::detail::pointer_state::object_reference_token;
                        buffer_.clear();
                    }
                    else
                    {
                        return jsonpointer_errc::expected_object_or_array;
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return jsonpointer_errc::unexpected_leading_zero;
                case '-':
                    return jsonpointer_errc::index_exceeds_array_size;
                default:
                    return jsonpointer_errc::expected_digit_or_dash;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::after_last_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    return jsonpointer_errc::index_exceeds_array_size;
                default:
                    return jsonpointer_errc::expected_slash;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    ec = op(current_,index_);
                    if (ec != jsonpointer_errc())
                    {
                        return ec;
                    }
                    if (current_.back().get().is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back().get().is_object())
                    {
                        state_ = jsonpointer::detail::pointer_state::object_reference_token;
                        buffer_.clear();
                    }
                    else
                    {
                        return jsonpointer_errc::expected_object_or_array;
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    index_ = index_ * 10 + (*p_ - '0');
                    break;
                case '-':
                    return jsonpointer_errc::index_exceeds_array_size;
                default:
                    return jsonpointer_errc::expected_digit_or_dash;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::object_reference_token: 
                switch (*p_)
                {
                case '/':
                    ec = op(current_,buffer_);
                    if (ec != jsonpointer_errc())
                    {
                        return ec;
                    }
                    if (current_.back().get().is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back().get().is_object())
                    {
                        state_ = jsonpointer::detail::pointer_state::object_reference_token;
                        buffer_.clear();
                    }
                    else
                    {
                        return jsonpointer_errc::expected_object_or_array;
                    }
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
                    state_ = jsonpointer::detail::pointer_state::object_reference_token;
                    break;
                case '1':
                    buffer_.push_back('/');
                    state_ = jsonpointer::detail::pointer_state::object_reference_token;
                    break;
                default:
                    return jsonpointer_errc::expected_0_or_1;
                };
                ++p_;
                ++column_;
                break;
            }
        }
        return ec;
    }

private:

    // serializing_context

    size_t do_line_number() const override
    {
        return line_;
    }

    size_t do_column_number() const override
    {
        return column_;
    }
};

}

template<class J>
typename J::string_type normalized_path(const J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    return evaluator.normalized_path(root,path);
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,J&>::type
get(J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,const J&>::type
get(const J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<!is_accessible_by_reference<J>::value,J>::type
get(const J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,J&>::type
get(J& root, const typename J::string_view_type& path, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<is_accessible_by_reference<J>::value,const J&>::type
get(const J& root, const typename J::string_view_type& path, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}

template<class J>
typename std::enable_if<!is_accessible_by_reference<J>::value,J>::type
get(const J& root, const typename J::string_view_type& path, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    ec = evaluator.get(root,path);
    return evaluator.get_result();
}

template<class J>
bool contains(const J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,const J&> evaluator;
    jsonpointer_errc ec = evaluator.get(root,path);
    return ec == jsonpointer_errc() ? true : false;
}

template<class J>
void insert_or_assign(J& root, const typename J::string_view_type& path, const J& value)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.insert_or_assign(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void insert_or_assign(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.insert_or_assign(root,path,value);
}

template<class J>
void insert(J& root, const typename J::string_view_type& path, const J& value)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.insert(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void insert(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.insert(root,path,value);
}

template<class J>
void remove(J& root, const typename J::string_view_type& path)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.remove(root,path);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void remove(J& root, const typename J::string_view_type& path, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    ec = evaluator.remove(root,path);
}

template<class J>
void replace(J& root, const typename J::string_view_type& path, const J& value)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

    jsonpointer_errc ec = evaluator.replace(root,path,value);
    if (ec != jsonpointer_errc())
    {
        JSONCONS_THROW(jsonpointer_error(ec));
    }
}

template<class J>
void replace(J& root, const typename J::string_view_type& path, const J& value, std::error_code& ec)
{
    detail::jsonpointer_evaluator<J,J&> evaluator;

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
