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
    object_reference_token,
    escaped,
    unescaped
};

template<class Json>
class jsonpointer_evaluator : private basic_parsing_context<typename Json::char_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;

    pointer_state state_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    string_type buffer_;
    std::vector<const Json*> current_;
public:
    Json get_result() 
    {
        return *(current_.back());
    }

    size_t line_number() const
    {
        return line_;
    }

    size_t column_number() const
    {
        return column_;
    }

    void evaluate(const Json& root, 
                  const char_type* path, 
                  size_t length,
                  std::error_code& ec)
    {
        line_ = 1;
        column_ = 1;
        state_ = pointer_state::start;
        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        size_t index = 0;

        current_.push_back(std::addressof(root));

        while (p_ < end_input_)
        {
            switch (state_)
            {
            case pointer_state::start: 
                switch (*p_)
                {
                case '/':
                    state_ = current_.back()->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
                    break;
                default:
                    ec = jsonpointer_errc::expected_slash;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::array_reference_token: 
                switch (*p_)
                {
                case '0':
                    index = 0;
                    state_ = pointer_state::zero_array_reference_token;
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
                    index = (*p_ - '0');
                    state_ = pointer_state::nonzero_array_reference_token;
                    break;
                case '-':
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::zero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    update_current(index,ec);
                    state_ = current_.back()->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
                    index = 0;
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
                    ec = jsonpointer_errc::unexpected_leading_zero;
                    return;
                case '-':
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::nonzero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    update_current(index,ec);
                    state_ = current_.back()->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
                    index = 0;
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
                    index = index * 10 + (*p_ - '0');
                    break;
                case '-':
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer_errc::expected_digit_or_minus;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::object_reference_token: 
                switch (*p_)
                {
                case '/':
                    update_current(buffer_,ec);
                    state_ = current_.back()->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
                    index = 0;
                    buffer_.clear();
                    break;
                case '~':
                    state_ = pointer_state::escaped;
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::escaped: 
                switch (*p_)
                {
                case '0':
                    buffer_.push_back('~');
                    state_ = pointer_state::object_reference_token;
                    break;
                case '1':
                    buffer_.push_back('/');
                    state_ = pointer_state::object_reference_token;
                    break;
                default:
                    ec = jsonpointer_errc::expected_0_or_1;
                    return;
                };
                ++p_;
                ++column_;
                break;
            }
        }

        switch (state_)
        {
        case pointer_state::start: 
            break;;
        case pointer_state::zero_array_reference_token: 
        case pointer_state::nonzero_array_reference_token: 
            update_current(index,ec);
            break;
        case pointer_state::object_reference_token: 
            update_current(buffer_,ec);
            break;
        default:
            ec = jsonpointer_errc::eof;
            break;
        }
    }

private:

    void update_current(size_t index,
                        std::error_code& ec)
    {
        if (index >= current_.back()->size())
        {
            ec = jsonpointer_errc::index_exceeds_array_size;
            return;
        }
        current_.push_back(std::addressof(current_.back()->at(index)));
    }

    void update_current(const string_type& name,
                        std::error_code& ec)
    {
        if (!current_.back()->has_key(name))
        {
            ec = jsonpointer_errc::name_not_found;
            return;
        }
        current_.push_back(std::addressof(current_.back()->at(name)));
    }

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
    detail::jsonpointer_evaluator<Json> evaluator;

    std::error_code ec;
    evaluator.evaluate(root,path.data(),path.length(),ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
    return evaluator.get_result();
}

}}

#endif
