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

template<class Json,class JsonReference,class JsonPointer>
struct PathResolver
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef JsonReference json_reference;
    typedef JsonPointer json_pointer;

    jsonpointer_errc operator()(std::vector<json_pointer>& current,
                                size_t index) const
    {
        if (index >= current.back()->size())
        {
            return jsonpointer_errc::index_exceeds_array_size;
        }
        current.push_back(std::addressof(current.back()->at(index)));
        return jsonpointer_errc();
    }

    jsonpointer_errc operator()(std::vector<json_pointer>& current,
                                const string_type& name) const
    {
        if (!current.back()->has_key(name))
        {
            return jsonpointer_errc::name_not_found;
        }
        current.push_back(std::addressof(current.back()->at(name)));
        return jsonpointer_errc();
    }
};

template<class Json, class JsonReference, class JsonPointer>
struct PathSetter
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef JsonReference json_reference;
    typedef JsonPointer json_pointer;

    jsonpointer_errc operator()(std::vector<json_pointer>& current,
                                size_t index) const
    {
        if (index >= current.back()->size())
        {
            return jsonpointer_errc::index_exceeds_array_size;
        }
        current.push_back(std::addressof(current.back()->at(index)));
        return jsonpointer_errc();
    }

    jsonpointer_errc operator()(std::vector<json_pointer>& current,
                                const string_type& name) const
    {
        jsonpointer_errc ec = jsonpointer_errc();
        if (!current.back()->has_key(name))
        {
            return jsonpointer_errc::name_not_found;
        }
        current.push_back(std::addressof(current.back()->at(name)));
        return ec;
    }
};

template<class Json,
         class JsonReference=Json&,
         class JsonPointer=Json*>
class jsonpointer_evaluator : private basic_parsing_context<typename Json::char_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef JsonReference json_reference;
    typedef JsonPointer json_pointer;

    jsonpointer::detail::pointer_state state_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    string_type buffer_;
    size_t index_;
    std::vector<json_pointer> current_;
public:
    Json get_result() 
    {
        return std::move(*(current_.back()));
    }

    size_t line_number() const
    {
        return line_;
    }

    size_t column_number() const
    {
        return column_;
    }

    jsonpointer_errc select(json_reference root,
                            typename Json::string_view_type path)
    {
        PathResolver<Json,JsonReference,JsonPointer> op;
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

    string_type normalized_path(json_reference root, typename Json::string_view_type path)
    {
        jsonpointer_errc ec = evaluate(root,PathSetter<Json,JsonReference,JsonPointer>(),path);
        if (ec != jsonpointer_errc())
        {
            return path;
        }
        if (state_ == jsonpointer::detail::pointer_state::after_last_array_reference_token)
        {
            string_type p = path.substr(0,path.length()-1);
            std::string s = std::to_string(current_.back()->size());
            for (auto c : s)
            {
                p.push_back(c);
            }
            return p;
        }
        else
        {
            return path;
        }
    }

    jsonpointer_errc add(json_reference root,
                         typename Json::string_view_type path,
                         const Json& value)
    {
        jsonpointer_errc ec = evaluate(root,PathSetter<Json,JsonReference,JsonPointer>(),path);
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
            if (index_ > current_.back()->size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            if (index_ == current_.back()->size())
            {
                current_.back()->push_back(value);
            }
            else
            {
                current_.back()->insert(current_.back()->array_range().begin()+index_,value);
            }
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            current_.back()->push_back(value);
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            current_.back()->set(buffer_,value);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc insert(json_reference root,
                            typename Json::string_view_type path,
                            const Json& value)
    {
        jsonpointer_errc ec = evaluate(root,PathSetter<Json,JsonReference,JsonPointer>(),path);
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
            if (index_ > current_.back()->size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            if (index_ == current_.back()->size())
            {
                current_.back()->push_back(value);
            }
            else
            {
                current_.back()->insert(current_.back()->array_range().begin()+index_,value);
            }
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            current_.back()->push_back(value);
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (current_.back()->has_key(buffer_))
            {
                ec = jsonpointer_errc::member_already_exists;
            }
            else
            {
                current_.back()->set(buffer_,value);
            }
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc remove(json_reference root,
                            typename Json::string_view_type path)
    {
        jsonpointer_errc ec = evaluate(root,PathResolver<Json,JsonReference,JsonPointer>(),path);
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
            if (index_ >= current_.back()->size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            current_.back()->erase(current_.back()->array_range().begin()+index_);
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            return jsonpointer_errc::index_exceeds_array_size;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back()->has_key(buffer_))
            {
                return jsonpointer_errc::name_not_found;
            }
            current_.back()->erase(buffer_);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return ec;
    }

    jsonpointer_errc replace(json_reference root,
                             typename Json::string_view_type path,
                             const Json& value)
    {
        jsonpointer_errc ec = evaluate(root,PathResolver<Json,JsonReference,JsonPointer>(),path);
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
            if (index_ >= current_.back()->size())
            {
                return jsonpointer_errc::index_exceeds_array_size;
            }
            (*(current_.back()))[index_] = value;
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            return jsonpointer_errc::index_exceeds_array_size;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back()->has_key(buffer_))
            {
                return jsonpointer_errc::name_not_found;
            }
            current_.back()->set(buffer_,value);
            break;
        default:
            return jsonpointer_errc::end_of_input;
        }
        return jsonpointer_errc();
    }

    template <class Op>
    jsonpointer_errc evaluate(json_reference root,
                              Op op,
                              typename Json::string_view_type path)
    {
        jsonpointer_errc ec = jsonpointer_errc();

        line_ = 1;
        column_ = 1;
        state_ = jsonpointer::detail::pointer_state::start;
        begin_input_ = path.data();
        end_input_ = path.data() + path.length();
        p_ = begin_input_;

        index_ = 0;

        current_.push_back(std::addressof(root));

        while (p_ < end_input_)
        {
            switch (state_)
            {
            case jsonpointer::detail::pointer_state::start: 
                switch (*p_)
                {
                case '/':
                    if (current_.back()->is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back()->is_object())
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
                    if (current_.back()->is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back()->is_object())
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
                    if (current_.back()->is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back()->is_object())
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
                    if (current_.back()->is_array())
                    {
                        state_ = jsonpointer::detail::pointer_state::array_reference_token;
                        index_ = 0;
                    }
                    else if (current_.back()->is_object())
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

    // basic_parsing_context

    size_t do_line_number() const override
    {
        return line_;
    }

    size_t do_column_number() const override
    {
        return column_;
    }

    char_type do_current_char() const override
    {
        return p_ < end_input_? *p_ : 0;
    }
};

}

template<class Json>
typename Json::string_type normalized_path(const Json& root, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json, const Json&, const Json*> evaluator;
    return evaluator.normalized_path(root,path);
}

template<class Json>
std::tuple<Json,jsonpointer_errc> select(const Json& root, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json,const Json&,const Json*> evaluator;
    jsonpointer_errc ec = evaluator.select(root,path);
    return std::make_tuple(evaluator.get_result(),ec);
}

template<class Json>
jsonpointer_errc add(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    return evaluator.add(root,path,value);
}

template<class Json>
jsonpointer_errc insert(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    return evaluator.insert(root,path,value);
}

template<class Json>
jsonpointer_errc remove(Json& root, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    return evaluator.remove(root,path);
}

template<class Json>
jsonpointer_errc replace(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    return evaluator.replace(root,path,value);
}

}}

#endif
