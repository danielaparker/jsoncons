// Copyright 2020 Daniel Parkerpath_single_quoted
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JMESPATH_JMESPATH_HPP
#define JSONCONS_JMESPATH_JMESPATH_HPP

#include <array> // std::array
#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::is_const
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <regex>
#include <set> // std::set
#include <iterator> // std::make_move_iterator
#include <functional> // <functional>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath_error.hpp>

namespace jsoncons { namespace jmespath {

struct array_slice
{
    int64_t start_;
    optional<int64_t> end_;
    int64_t step_;

    array_slice()
        : start_(0), end_(), step_(1)
    {
    }

    array_slice(int64_t start, optional<int64_t> end, int64_t step) 
        : start_(start), end_(end), step_(step)
    {
    }

    int64_t get_start(std::size_t size) const
    {
        return start_ >= 0 ? start_ : (static_cast<int64_t>(size) - start_);
    }

    int64_t get_end(std::size_t size) const
    {
        if (end_)
        {
            auto len = end_.value() >= 0 ? end_.value() : (static_cast<int64_t>(size) - end_.value());
            return len <= size ? len : size;
        }
        else
        {
            return size;
        }
    }

    int64_t step() const
    {
        return step_; // Allow negative
    }

    array_slice(const array_slice&) = default;

    array_slice& operator=(const array_slice&) = default;
};

// work around for std::make_unique not being available until C++14
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_ptr(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

enum class result_type {value,path};

namespace detail {
template <class Json,
    class JsonReference>
    class jmespath_evaluator;
}

template<class Json>
Json select(const Json& root, const typename Json::string_view_type& path, result_type result_t = result_type::value)
{
    if (result_t == result_type::value)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        return evaluator.evaluate(root, path);
    }
    else
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        return evaluator.evaluate(root, path);
    }
}

template<class Json, class T>
void json_replace(Json& root, const typename Json::string_view_type& path, T&& new_value)
{
    jsoncons::jmespath::detail::jmespath_evaluator<Json,Json&> evaluator;
    evaluator.evaluate(root, path);
    evaluator.replace(std::forward<T>(new_value));
}

namespace detail {
 
enum class path_state 
{
    start,
    expression,

    identifier,
    quoted_string,
    unquoted_string,
    expression1,
    index_expression,
    number,
    digit,
    bracket_specifier,
    bracket_specifier2,
    bracket_specifier3,
    bracket_specifier4,

    dot_or_left_bracket,
    name_or_left_bracket,
    name,
    unquoted_name,
    unquoted_name2,
    single_quoted_name,
    double_quoted_name,
    bracketed_unquoted_name,
    bracketed_single_quoted_name,
    bracketed_double_quoted_name,
    bracketed_name_or_path,
    bracketed_wildcard_or_path,
    wildcard_or_rpath_or_slice_or_filter,
    slice_end_or_end_step,
    slice_end,
    slice_step,
    slice_step2,
    comma_or_right_bracket,
    path_or_function_name,
    function,
    arg_or_right_paren,
    path_argument,
    unquoted_arg,
    single_quoted_arg,
    double_quoted_arg,
    more_args_or_right_paren,
    dot,
    path,
    path2,
    path_single_quoted,
    path_double_quoted
};

struct state_item
{
    path_state state;

    state_item()
        : state(path_state::start)
    {
    }

    explicit state_item(path_state state)
        : state(state)
    {
    }

    state_item(const state_item&) = default;
    state_item& operator=(const state_item&) = default;
};

JSONCONS_STRING_LITERAL(length, 'l', 'e', 'n', 'g', 't', 'h')

template<class Json,
         class JsonReference>
class jmespath_evaluator : public ser_context
{
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef typename Json::string_view_type string_view_type;
    typedef JsonReference reference;
    using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
    typedef typename Json::const_pointer const_pointer;

    class selector_base
    {
    public:
        virtual ~selector_base()
        {
        }

        virtual void add_selector(std::unique_ptr<selector_base>&&) = 0;

        virtual reference select(reference val) = 0;
    };

    class sub_expression_selector final : public selector_base
    {
    public:
        std::vector<std::unique_ptr<selector_base>> selectors_;

        sub_expression_selector()
        {
        }

        void add_selector(std::unique_ptr<selector_base>&& selector) 
        {
            selectors_.emplace_back(std::move(selector));
        }

        reference select(reference val) override
        {
            pointer ptr = std::addressof(val);
            for (auto& selector : selectors_)
            {
                ptr = std::addressof(selector->select(*ptr));
            }
            return *ptr;
        }
    };

    class list_projection_selector final : public selector_base
    {
    public:
        std::unique_ptr<selector_base> lhs_selector_;
        std::vector<std::unique_ptr<selector_base>> rhs_selectors_;
        Json result_;

        list_projection_selector(std::unique_ptr<selector_base>&& lhs_selector)
           : result_(json_array_arg)
        {
            lhs_selector_ = std::move(lhs_selector);
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            rhs_selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val) override
        {
            static Json null{null_type()};
            auto j = lhs_selector_->select(val);
            if (!j.is_array())
            {
                return null;
            }

            for (reference item : j.array_range())
            {
                pointer ptr = std::addressof(item);
                for (auto& selector : rhs_selectors_)
                {
                    ptr = std::addressof(selector->select(*ptr));
                }
                if (!ptr->is_null())
                {
                    result_.push_back(*ptr);
                }
            }
            return result_;
        }
    };

    class identifier_selector final : public selector_base
    {
    private:
        string_type identifier_;
    public:
        identifier_selector(const string_view_type& name)
            : identifier_(name)
        {
        }

        void add_selector(std::unique_ptr<selector_base>&& selector) override
        {
            // Error
        }

        reference select(reference val) override
        {
            static Json null{null_type()};

            if (val.is_object() && val.contains(identifier_))
            {
                return val.at(identifier_);
            }
            /* else if (val.is_array())
            {
                std::size_t pos = 0;
                if (try_string_to_index(identifier_.data(), identifier_.size(), &pos, &is_start_positive))
                {
                    std::size_t index = is_start_positive ? pos : val.size() - pos;
                    if (index < val.size())
                    {
                        nodes.emplace_back(PathCons()(path,index),std::addressof(val[index]));
                    }
                }
                else if (identifier_ == length_literal<char_type>() && val.size() > 0)
                {
                    pointer ptr = evaluator.create_temp(val.size());
                    nodes.emplace_back(PathCons()(path, identifier_), ptr);
                }
            }
            else if (val.is_string())
            {
                std::size_t pos = 0;
                string_view_type sv = val.as_string_view();
                if (try_string_to_index(identifier_.data(), identifier_.size(), &pos, &is_start_positive))
                {
                    std::size_t index = is_start_positive ? pos : sv.size() - pos;
                    auto sequence = unicons::sequence_at(sv.data(), sv.data() + sv.size(), index);
                    if (sequence.length() > 0)
                    {
                        pointer ptr = evaluator.create_temp(sequence.begin(),sequence.length());
                        nodes.emplace_back(PathCons()(path, index), ptr);
                    }
                }
                else if (identifier_ == length_literal<char_type>() && sv.size() > 0)
                {
                    std::size_t count = unicons::u32_length(sv.begin(),sv.end());
                    pointer ptr = evaluator.create_temp(count);
                    nodes.emplace_back(PathCons()(path, identifier_), ptr);
                }
            }*/
            return null;
        }
    };

    class index_selector final : public selector_base
    {
    private:
        string_type identifier_;
    public:
        index_selector(const string_view_type& name)
            : identifier_(name)
        {
        }

        void add_selector(std::unique_ptr<selector_base>&& selector) 
        {
            // Error
        }

        reference select(reference val) override
        {
            static Json null{null_type()};

            if (val.is_array())
            {
                int64_t slen = static_cast<int64_t>(val.size());
                auto result = jsoncons::detail::to_integer<int64_t>(identifier_.data(), identifier_.size());
                if (!result)
                {
                    return null;
                }
                else if (result.value() >= 0 && result.value() < slen)
                {
                    std::size_t index = static_cast<std::size_t>(result.value());
                    return val.at(index);
                }
                else if (result.value() < 0 && (slen+result.value()) < slen)
                {
                    std::size_t index = static_cast<std::size_t>(slen + result.value());
                    return val.at(index);
                }
                else
                {
                    return null;
                }
            }
            /* else if (val.is_string())
            {
                std::size_t pos = 0;
                string_view_type sv = val.as_string_view();
                if (try_string_to_index(identifier_.data(), identifier_.size(), &pos, &is_start_positive))
                {
                    std::size_t index = is_start_positive ? pos : sv.size() - pos;
                    auto sequence = unicons::sequence_at(sv.data(), sv.data() + sv.size(), index);
                    if (sequence.length() > 0)
                    {
                        pointer ptr = evaluator.create_temp(sequence.begin(),sequence.length());
                        nodes.emplace_back(PathCons()(path, index), ptr);
                    }
                }
                else if (identifier_ == length_literal<char_type>() && sv.size() > 0)
                {
                    std::size_t count = unicons::u32_length(sv.begin(),sv.end());
                    pointer ptr = evaluator.create_temp(count);
                    nodes.emplace_back(PathCons()(path, identifier_), ptr);
                }
            }*/
            return null;
        }
    };

    class slice_selector final : public selector_base
    {
    private:
        array_slice slice_;
        Json a_; 
    public:
        slice_selector(const array_slice& slice)
            : slice_(slice), a_(json_array_arg) 
        {
        }

        void add_selector(std::unique_ptr<selector_base>&& selector) override
        {
            // Error
        }

        reference select(reference val) override
        {
            static Json null{null_type()};

            if (val.is_array())
            {
                auto start = slice_.get_start(val.size());
                auto end = slice_.get_end(val.size());
                auto step = slice_.step();
                if (step >= 0)
                {
                    for (int64_t j = start; j < end; j += step)
                    {
                        a_.emplace_back(val[static_cast<std::size_t>(j)]);
                    }
                    return a_; 
                }
                else
                {
                    for (int64_t j = end-1; j >= start; j += step)
                    {
                        a_.emplace_back(val[static_cast<std::size_t>(j)]);
                    }
                    return a_; 
                }
            }
            else
            {
                return null;
            }
        }
    };

    std::size_t line_;
    std::size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;

    std::vector<state_item> state_stack_;
    std::unique_ptr<selector_base> selector_;

public:
    jmespath_evaluator()
        : line_(1), column_(1),
          begin_input_(nullptr), end_input_(nullptr),
          p_(nullptr), selector_(make_unique_ptr<sub_expression_selector>())
    {
    }

    jmespath_evaluator(std::size_t line, std::size_t column)
        : line_(line), column_(column),
          begin_input_(nullptr), end_input_(nullptr),
          p_(nullptr)
    {
    }

    std::size_t line() const
    {
        return line_;
    }

    std::size_t column() const
    {
        return column_;
    }

    reference evaluate(reference root, const string_view_type& path)
    {
        std::error_code ec;
        reference j = evaluate(root, path.data(), path.length(), ec);
        if (ec)
        {
            JSONCONS_THROW(jmespath_error(ec, line_, column_));
        }
        return j;
    }

    reference evaluate(reference root, const string_view_type& path, std::error_code& ec)
    {
        JSONCONS_TRY
        {
            return evaluate(root, path.data(), path.length(), ec);
        }
        JSONCONS_CATCH(...)
        {
            ec = jmespath_errc::unidentified_error;
        }
    }
 
    reference evaluate(reference root, 
                       const char_type* path, 
                       std::size_t length,
                       std::error_code& ec)
    {
        static Json result{null_type()};

        state_stack_.emplace_back(path_state::start);

        string_type function_name;
        string_type buffer;

        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        array_slice slice;

        while (p_ < end_input_)
        {
            switch (state_stack_.back().state)
            {
                case path_state::start: 
                {
                    state_stack_.back().state = path_state::expression1;
                    break;
                }
                case path_state::identifier: 
                {
                    switch (*p_)
                    {
                        case '[':
                            state_stack_.back().state = path_state::index_expression;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        case '.':
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                state_stack_.back().state = path_state::identifier;
                                state_stack_.emplace_back(path_state::unquoted_string);
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                            }
                            else
                            {
                                ec = jmespath_errc::expected_identifier;
                                return result;
                            }
                            break;
                    };
                    break;
                }
                case path_state::expression1: 
                {
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '\"':
                            state_stack_.emplace_back(path_state::quoted_string);
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.back().state = path_state::index_expression;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                state_stack_.emplace_back(path_state::unquoted_string);
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                            }
                            else
                            {
                                ec = jmespath_errc::expected_identifier;
                                return result;
                            }
                            break;
                    };
                    break;
                }
                case path_state::unquoted_string: 
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            selector_->add_selector(make_unique_ptr<identifier_selector>(buffer));
                            buffer.clear();
                            state_stack_.pop_back(); // unquoted_string
                            advance_past_space_character();
                            break;
                        case '.':
                            selector_->add_selector(make_unique_ptr<identifier_selector>(buffer));
                            state_stack_.pop_back(); // unquoted_string
                            state_stack_.back().state = path_state::expression1;
                            buffer.clear();
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            selector_->add_selector(make_unique_ptr<identifier_selector>(buffer));
                            state_stack_.pop_back(); // unquoted_string
                            buffer.clear();
                            state_stack_.back().state = path_state::index_expression;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= '0' && *p_ <= '9') || (*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                            }
                            else
                            {
                                ec = jmespath_errc::expected_A_Za_Z_;
                                return result;
                            }
                            break;
                    };
                    break;
                case path_state::number:
                    switch(*p_)
                    {
                        case '-':
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::digit;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            state_stack_.back().state = path_state::digit;
                            break;
                    }
                    break;
                case path_state::digit:
                    switch(*p_)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            state_stack_.pop_back(); // digit
                            break;
                    }
                    break;
                case path_state::index_expression:
                    switch(*p_)
                    {
                        case '.':
                            ++p_;
                            ++column_;
                            state_stack_.back().state = path_state::expression1;
                            break;
                        case '[':
                            //++p_;
                            //++column_;
                            state_stack_.back().state = path_state::expression1;
                            break;
                        default:
                            ec = jmespath_errc::expected_index;
                            return result;
                    }
                    break;
                case path_state::bracket_specifier:
                    switch(*p_)
                    {
                        case ']':
                            selector_->add_selector(make_unique_ptr<index_selector>(buffer));
                            buffer.clear();
                            state_stack_.pop_back(); // bracket_specifier
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                        {
                            if (!buffer.empty())
                            {
                                auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                if (!r)
                                {
                                    ec = jmespath_errc::invalid_number;
                                    return result;
                                }
                                slice.start_ = r.value();
                                buffer.clear();
                            }
                            state_stack_.back().state = path_state::bracket_specifier2;
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '*':
                            selector_ = make_unique_ptr<list_projection_selector>(std::move(selector_));
                            state_stack_.back().state = path_state::bracket_specifier4;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return result;
                    }
                    break;
                case path_state::bracket_specifier2:
                {
                    if (!buffer.empty())
                    {
                        auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                        if (!r)
                        {
                            ec = jmespath_errc::invalid_number;
                            return result;
                        }
                        slice.end_ = optional<int64_t>(r.value());
                    }
                    switch(*p_)
                    {
                        case ']':
                            selector_->add_selector(make_unique_ptr<slice_selector>(slice));
                            buffer.clear();
                            slice = array_slice();
                            state_stack_.pop_back(); // bracket_specifier2
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                            state_stack_.back().state = path_state::bracket_specifier3;
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return result;
                    }
                    break;
                }
                case path_state::bracket_specifier3:
                {
                    if (!buffer.empty())
                    {
                        auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                        if (!r)
                        {
                            ec = jmespath_errc::invalid_number;
                            return result;
                        }
                        slice.step_ = r.value();
                        buffer.clear();
                    }
                    switch(*p_)
                    {
                        case ']':
                            selector_->add_selector(make_unique_ptr<slice_selector>(slice));
                            buffer.clear();
                            slice = array_slice();
                            state_stack_.pop_back(); // bracket_specifier3
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return result;
                    }
                    break;
                }
                case path_state::bracket_specifier4:
                {
                    switch(*p_)
                    {
                        case ']':
                            state_stack_.pop_back(); // bracket_specifier4
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return result;
                    }
                    break;
                }
            }
        }

        switch (state_stack_.back().state)
        {
            case path_state::unquoted_string: 
            {
                selector_->add_selector(make_unique_ptr<identifier_selector>(buffer));
                buffer.clear();
                state_stack_.pop_back(); // unquoted_name
                break;
            }
            default:
                break;
        }

        if (state_stack_.size() > 1)
        {
            ec = jmespath_errc::unexpected_end_of_input;
            return result;
        }

        JSONCONS_ASSERT(state_stack_.size() == 1);
        JSONCONS_ASSERT(state_stack_.back().state == path_state::identifier ||
                        state_stack_.back().state == path_state::expression1 ||
                        state_stack_.back().state == path_state::index_expression);
        state_stack_.pop_back();

        reference r = selector_->select(root); 
        return r;
    }

    void advance_past_space_character()
    {
        switch (*p_)
        {
            case ' ':case '\t':
                ++p_;
                ++column_;
                break;
            case '\r':
                if (p_+1 < end_input_ && *(p_+1) == '\n')
                    ++p_;
                ++line_;
                column_ = 1;
                ++p_;
                break;
            case '\n':
                ++line_;
                column_ = 1;
                ++p_;
                break;
            default:
                break;
        }
    }
};

}

}}

#endif
