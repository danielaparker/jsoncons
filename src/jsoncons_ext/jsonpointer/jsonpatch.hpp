// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPATCH_HPP
#define JSONCONS_JSONPOINTER_JSONPATCH_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error_category.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsoncons { namespace jsonpointer {

enum class operation 
{
    add,
    remove,
    replace,
    move,
    copy,
    test
};

namespace detail {

template<class Json,
         class JsonReference=Json&,
         class JsonPointer=Json*>
class jsonpatch_evaluator : private basic_parsing_context<typename Json::char_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef JsonReference json_reference;
    typedef JsonPointer json_pointer;

    pointer_state state_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    string_type buffer_;
    json_pointer current_;
public:
    Json get_result() 
    {
        return *current_;
    }

    size_t line_number() const
    {
        return line_;
    }

    size_t column_number() const
    {
        return column_;
    }

    void evaluate(json_reference root,
                  const char_type* path,
                  size_t length,
                  const Json& value,
                  operation op,
                  std::error_code& ec)
    {
        line_ = 1;
        column_ = 1;
        state_ = pointer_state::start;
        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        size_t index = 0;

        current_ = std::addressof(root);

        while (p_ < end_input_)
        {
            switch (state_)
            {
            case pointer_state::start: 
                switch (*p_)
                {
                case '/':
                    state_ = current_->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
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
                    state_ = pointer_state::after_last_array_reference_token;
                    break;
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
                    update_current(index,op,ec);
                    state_ = current_->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
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
            case pointer_state::after_last_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                default:
                    ec = jsonpointer_errc::expected_slash;
                    return;
                };
                ++p_;
                ++column_;
                break;
            case pointer_state::nonzero_array_reference_token: 
                switch (*p_)
                {
                case '/':
                    update_current(index,op,ec);
                    state_ = current_->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
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
                    update_current(buffer_,op,ec);
                    state_ = current_->is_array() ? pointer_state::array_reference_token : pointer_state::object_reference_token;
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
            break;
        case pointer_state::zero_array_reference_token: 
        case pointer_state::nonzero_array_reference_token: 
            switch (op)
            {
            case operation::add:
                if (index >= current_->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                current_->insert(current_->array_range().begin()+index,value);
                break;
            case operation::remove:
                if (index >= current_->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                current_->erase(current_->array_range().begin()+index);
                break;
            }
            break;
        case pointer_state::after_last_array_reference_token:
            switch (op)
            {
            case operation::add:
                current_->push_back(value);
                break;
            case operation::remove:
                ec = jsonpointer_errc::index_exceeds_array_size;
                break;
            }
            break;
        case pointer_state::object_reference_token: 
            switch (op)
            {
            case operation::add:
                current_->set(buffer_,value);
                break;
            case operation::remove:
                if (!current_->has_key(buffer_))
                {
                    ec = jsonpointer_errc::name_not_found;
                }
                else
                {
                    current_->erase(buffer_);
                }
                break;
            }
            break;
        default:
            ec = jsonpointer_errc::eof;
            break;
        }
    }

private:

    void update_current(size_t index,
                        operation op,
                        std::error_code& ec)
    {
        switch (op)
        {
        case operation::add:
        case operation::remove:
            if (index >= current_->size())
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            break;
        }
        current_ = std::addressof(current_->at(index));
    }

    void update_current(const string_type& name,
                        operation op,
                        std::error_code& ec)
    {
        switch (op)
        {
        case operation::add:
            if (!current_->has_key(name))
            {
                current_->set(name,Json());
            }
            break;
        case operation::remove:
            if (!current_->has_key(name))
            {
                ec = jsonpointer_errc::name_not_found;
            }
            break;
        }

        current_ = std::addressof(current_->at(name));
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
void add(Json& root, typename Json::string_view_type path, const Json& value)
{
    detail::jsonpatch_evaluator<Json> evaluator;

    std::error_code ec;
    evaluator.evaluate(root,path.data(),path.length(),value,operation::add,ec);
    if (ec)
    {
        throw parse_error(ec,evaluator.line_number(),evaluator.column_number());
    }
}

}}

#endif
