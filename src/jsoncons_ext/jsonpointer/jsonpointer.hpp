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

    void operator()(std::vector<json_pointer>& current,
                    size_t index,
                    std::error_code& ec) const
    {
        if (index >= current.back()->size())
        {
            ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
        }
        else
        {
            current.push_back(std::addressof(current.back()->at(index)));
        }
    }

    void operator()(std::vector<json_pointer>& current,
                    const string_type& name,
                    std::error_code& ec) const
    {
        if (!current.back()->has_key(name))
        {
            ec = jsonpointer::jsonpointer_errc::name_not_found;
        }
        else
        {
            current.push_back(std::addressof(current.back()->at(name)));
        }
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

    void operator()(std::vector<json_pointer>& current,
                    size_t index,
                    std::error_code& ec) const
    {
        if (index >= current.back()->size())
        {
            ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
        }
        else
        {
            current.push_back(std::addressof(current.back()->at(index)));
        }
    }

    void operator()(std::vector<json_pointer>& current,
                    const string_type& name,
                    std::error_code& ec) const
    {
        if (!current.back()->has_key(name))
        {
            current.back()->set(name,Json());
        }
        current.push_back(std::addressof(current.back()->at(name)));
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

    void select(json_reference root,
                typename Json::string_view_type path,
                std::error_code& ec)
    {
        PathResolver<Json,JsonReference,JsonPointer> op;
        evaluate(root,op,path,ec);

        switch (state_)
        {
        case pointer_state::start: 
            break;
        case pointer_state::zero_array_reference_token: 
        case pointer_state::nonzero_array_reference_token: 
            op(current_,index_,ec);
            break;
        case pointer_state::object_reference_token: 
            op(current_,buffer_,ec);
            break;
        default:
            ec = jsonpointer_errc::eof;
            break;
        }
    }

    void add(json_reference root,
             typename Json::string_view_type path,
             const Json& value,
             std::error_code& ec)
    {
        evaluate(root,PathSetter<Json,JsonReference,JsonPointer>(),path,ec);

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ >= current_.back()->size())
            {
                ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            current_.back()->insert(current_.back()->array_range().begin()+index_,value);
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            current_.back()->push_back(value);
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            current_.back()->set(buffer_,value);
            break;
        default:
            ec = jsonpointer::jsonpointer_errc::eof;
            break;
        }
    }

    void remove(json_reference root,
                typename Json::string_view_type path,
                std::error_code& ec)
    {
        evaluate(root,PathResolver<Json,JsonReference,JsonPointer>(),path,ec);

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ >= current_.back()->size())
            {
                ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            current_.back()->erase(current_.back()->array_range().begin()+index_);
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back()->has_key(buffer_))
            {
                ec = jsonpointer::jsonpointer_errc::name_not_found;
            }
            else
            {
                current_.back()->erase(buffer_);
            }
            break;
        default:
            ec = jsonpointer::jsonpointer_errc::eof;
            break;
        }
    }

    void replace(json_reference root,
                 typename Json::string_view_type path,
                 const Json& value,
                 std::error_code& ec)
    {
        evaluate(root,PathResolver<Json,JsonReference,JsonPointer>(),path,ec);

        switch (state_)
        {
        case jsonpointer::detail::pointer_state::start: 
            break;
        case jsonpointer::detail::pointer_state::zero_array_reference_token: 
        case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
            if (index_ >= current_.back()->size())
            {
                ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            (*(current_.back()))[index_] = value;
            break;
        case jsonpointer::detail::pointer_state::after_last_array_reference_token:
            ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
            break;
        case jsonpointer::detail::pointer_state::object_reference_token: 
            if (!current_.back()->has_key(buffer_))
            {
                ec = jsonpointer::jsonpointer_errc::name_not_found;
            }
            else
            {
                current_.back()->set(buffer_,value);
            }
            break;
        default:
            ec = jsonpointer::jsonpointer_errc::eof;
            break;
        }
    }

    void move(json_reference root,
              typename Json::string_view_type from,
              typename Json::string_view_type path,
              std::error_code& ec)
    {
    }

    template <class Op>
    void evaluate(json_reference root,
                  Op op,
                  typename Json::string_view_type path,
                  std::error_code& ec)
    {
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
                    state_ = current_.back()->is_array() ? jsonpointer::detail::pointer_state::array_reference_token : jsonpointer::detail::pointer_state::object_reference_token;
                    break;
                default:
                    ec = jsonpointer::jsonpointer_errc::expected_slash;
                    return;
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
                    ec = jsonpointer::jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::zero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    op(current_,index_,ec);
                    state_ = current_.back()->is_array() ? jsonpointer::detail::pointer_state::array_reference_token : jsonpointer::detail::pointer_state::object_reference_token;
                    index_ = 0;
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
                    ec = jsonpointer::jsonpointer_errc::unexpected_leading_zero;
                    return;
                case '-':
                    ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer::jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::after_last_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer::jsonpointer_errc::expected_slash;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::nonzero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    op(current_,index_,ec);
                    state_ = current_.back()->is_array() ? jsonpointer::detail::pointer_state::array_reference_token : jsonpointer::detail::pointer_state::object_reference_token;
                    index_ = 0;
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
                    ec = jsonpointer::jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer::jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case jsonpointer::detail::pointer_state::object_reference_token: 
                switch (*p_)
                {
                case '/':
                    op(current_,buffer_,ec);
                    state_ = current_.back()->is_array() ? jsonpointer::detail::pointer_state::array_reference_token : jsonpointer::detail::pointer_state::object_reference_token;
                    index_ = 0;
                    buffer_.clear();
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
                    ec = jsonpointer::jsonpointer_errc::expected_0_or_1;
                    return;
                };
                ++p_;
                ++column_;
                break;
            }
        }
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
Json select(const Json& root, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json,const Json&,const Json*> evaluator;

    std::error_code ec;
    evaluator.select(root,path,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
    return evaluator.get_result();
}

template<class Json>
void add(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    std::error_code ec;
    evaluator.add(root,path,value,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
}

template<class Json>
void remove(Json& root, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    std::error_code ec;
    evaluator.remove(root,path,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
}

template<class Json>
void replace(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    std::error_code ec;
    evaluator.replace(root,path,value,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
}

template<class Json>
void move(Json& root, typename Json::string_view_type from, typename Json::string_view_type path)
{
    detail::jsonpointer_evaluator<Json> evaluator;

    std::error_code ec;
    //evaluator.evaluate(root,path.data(),path.length(),value,operation::replace,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
}

}}

#endif
