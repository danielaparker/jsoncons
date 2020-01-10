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
#include <jsoncons_ext/jmespath/jmespath_filter.hpp>
#include <jsoncons_ext/jmespath/jmespath_error.hpp>
#include <jsoncons_ext/jmespath/jmespath_function.hpp>

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

template<class Json>
Json select(const Json& root, const typename Json::string_view_type& path, result_type result_t = result_type::value)
{
    if (result_t == result_type::value)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator;
        return evaluator.evaluate(root, path);
    }
    else
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&,detail::PathConstructor<Json>> evaluator;
        return evaluator.evaluate(root, path);
    }
}

template<class Json, class T>
void json_replace(Json& root, const typename Json::string_view_type& path, T&& new_value)
{
    jsoncons::jmespath::detail::jmespath_evaluator<Json,Json&,detail::VoidPathConstructor<Json>> evaluator;
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
         class JsonReference,
         class PathCons>
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

        virtual reference select(jmespath_evaluator& evaluator,
                                 const string_type& path, 
                                 reference val) = 0;
    };
#if 0
    class path_selector final : public selector_base
    {
    private:
         std::basic_string<char_type> path_;
    public:
        path_selector(const std::basic_string<char_type>& path)
            : path_(path)
        {
        }

        void select(jmespath_evaluator&,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            std::error_code ec;
            jmespath_evaluator<Json,JsonReference,PathCons> e;
            e.evaluate(val, path_, ec);
            if (!ec)
            {
                for (auto ptr : e.get_pointers())
                {
                    nodes.emplace_back(PathCons()(path,path_),ptr);
                }
            }
        }
    };

    class expr_selector final : public selector_base
    {
    private:
         jmespath_filter_expr<Json> result_;
    public:
        expr_selector(const jmespath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        void select(jmespath_evaluator& evaluator,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            auto index = result_.eval(val);
            if (index.template is<std::size_t>())
            {
                std::size_t start = index.template as<std::size_t>();
                if (val.is_array() && start < val.size())
                {
                    nodes.emplace_back(PathCons()(path,start),std::addressof(val[start]));
                }
            }
            else if (index.is_string())
            {
                identifier_selector selector(index.as_string_view());
                selector.select(evaluator, path, val, nodes);
            }
        }
    };

    class filter_selector final : public selector_base
    {
    private:
         jmespath_filter_expr<Json> result_;
    public:
        filter_selector(const jmespath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        bool is_filter() const override
        {
            return true;
        }

        void select(jmespath_evaluator&,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            //std::cout << "filter_selector select ";
            if (val.is_array())
            {
                //std::cout << "from array \n";
                for (std::size_t i = 0; i < val.size(); ++i)
                {
                    if (result_.exists(val[i]))
                    {
                        nodes.emplace_back(PathCons()(path,i),std::addressof(val[i]));
                    }
                }
            }
            else if (val.is_object())
            {
                //std::cout << "from object \n";
                if (result_.exists(val))
                {
                    nodes.emplace_back(path, std::addressof(val));
                }
            }
        }
    };
#endif
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

        reference select(jmespath_evaluator& evaluator,
                         const string_type& path, 
                         reference val) override
        {
            pointer ptr = std::addressof(val);
            for (auto& selector : selectors_)
            {
                ptr = std::addressof(selector->select(evaluator,path,*ptr));
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

        reference select(jmespath_evaluator& evaluator,
                         const string_type& path, 
                         reference val) override
        {
            static Json null{null_type()};
            auto j = lhs_selector_->select(evaluator,path,val);
            if (!j.is_array())
            {
                return null;
            }

            for (reference item : j.array_range())
            {
                pointer ptr = std::addressof(item);
                for (auto& selector : rhs_selectors_)
                {
                    ptr = std::addressof(selector->select(evaluator,path,*ptr));
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

        reference select(jmespath_evaluator& evaluator,
                         const string_type& path, 
                         reference val) override
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

        reference select(jmespath_evaluator& evaluator,
                         const string_type& path, 
                         reference val) override
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

        reference select(jmespath_evaluator& evaluator,
                         const string_type& path, 
                         reference val) override
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

    function_table<Json,pointer> functions_;

    std::size_t line_;
    std::size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    std::vector<std::size_t> sub_expression_indices_;

    std::vector<std::unique_ptr<Json>> temp_json_values_;

    typedef std::vector<pointer> argument_type;
    std::vector<argument_type> function_stack_;
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

#if 0
    void call_function(const string_type& function_name, std::error_code& ec)
    {
        auto f = functions_.get(function_name, ec);
        if (ec)
        {
            return;
        }
        auto result = f(function_stack_, ec);
        if (ec)
        {
            return;
        }

        string_type s = {'$'};
        node_set v;
        pointer ptr = create_temp(std::move(result));
        v.emplace_back(s,ptr);
        stack_.push_back(v);
    }

    Json get_normalized_paths() const
    {
        Json result = typename Json::array();
        if (stack_.size() > 0)
        {
            result.reserve(stack_.back().size());
            for (const auto& p : stack_.back())
            {
                result.push_back(p.path);
            }
        }
        return result;
    }

    template <class T>
    void replace(T&& new_value)
    {
        if (stack_.size() > 0)
        {
            for (std::size_t i = 0; i < stack_.back().size(); ++i)
            {
                *(stack_.back()[i].val_ptr) = new_value;
            }
        }
    }
#endif
    template <typename... Args>
    pointer create_temp(Args&& ... args)
    {
        auto temp = make_unique_ptr<Json>(std::forward<Args>(args)...);
        pointer ptr = temp.get();
        temp_json_values_.emplace_back(std::move(temp));
        return ptr;
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
#if 0
                case path_state::dot:
                    switch (*p_)
                    {
                        case '.':
                            state_stack_.back().is_recursive_descent = true;
                            ++p_;
                            ++column_;
                            state_stack_.back().state = path_state::name_or_left_bracket;
                            break;
                        default:
                            state_stack_.back().state = path_state::name;
                            break;
                    }
                    break;
                case path_state::name_or_left_bracket: 
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '[': // [ can follow ..
                            state_stack_.back().state = path_state::wildcard_or_rpath_or_slice_or_filter;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            buffer.clear();
                            state_stack_.back().state = path_state::name;
                            break;
                    }
                    break;
                case path_state::name: 
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '*':
                            end_all();
                            transfer_nodes();
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.back().state = path_state::single_quoted_name;
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            state_stack_.back().state = path_state::double_quoted_name;
                            ++p_;
                            ++column_;
                            break;
                        case '[': 
                        case '.':
                            ec = jmespath_errc::expected_name;
                            return;
                        default:
                            buffer.clear();
                            state_stack_.back().state = path_state::unquoted_name;
                            break;
                    }
                    break;
                case path_state::dot_or_left_bracket: 
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '.':
                            state_stack_.emplace_back(path_state::dot);
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.emplace_back(path_state::wildcard_or_rpath_or_slice_or_filter);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_separator;
                            return;
                    };
                    break;
                case path_state::unquoted_name: 
                    switch (*p_)
                    {
                        case '[':
                        case '.':
                        case ' ':case '\t':
                        case '\r':
                        case '\n':
                            state_stack_.back().state = path_state::unquoted_name2;
                            break;
                        default:
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                    };
                    break;
                case path_state::unquoted_name2: 
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '[':
                            selector_stack_.push_back(make_unique_ptr<identifier_selector>(buffer));
                            apply_selectors();
                            buffer.clear();
                            slice.start_ = 0;
                            state_stack_.pop_back();
                            break;
                        case '.':
                            selector_stack_.push_back(make_unique_ptr<identifier_selector>(buffer));
                            apply_selectors();
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_name;
                            return;
                    };
                    break;
                case path_state::single_quoted_name:
                    switch (*p_)
                    {
                        case '\'':
                            selector_stack_.push_back(make_unique_ptr<identifier_selector>(buffer));
                            apply_selectors();
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    };
                    ++p_;
                    ++column_;
                    break;
                case path_state::double_quoted_name: 
                    switch (*p_)
                    {
                        case '\"':
                            selector_stack_.push_back(make_unique_ptr<identifier_selector>(buffer));
                            apply_selectors();
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    };
                    ++p_;
                    ++column_;
                    break;
                case path_state::comma_or_right_bracket:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case ',':
                            state_stack_.back().is_union = true;
                            state_stack_.back().state = path_state::wildcard_or_rpath_or_slice_or_filter;
                            ++p_;
                            ++column_;
                            break;
                        case ']':
                            apply_selectors();
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return;
                    }
                    break;
                case path_state::wildcard_or_rpath_or_slice_or_filter:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '(':
                        {
                            jmespath_filter_parser<Json> parser(line_,column_);
                            auto result = parser.parse(root, p_,end_input_,&p_);
                            line_ = parser.line();
                            column_ = parser.column();
                            selector_stack_.push_back(make_unique_ptr<expr_selector>(result));
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            break;
                        }
                        case '?':
                        {
                            jmespath_filter_parser<Json> parser(line_,column_);
                            auto result = parser.parse(root,p_,end_input_,&p_);
                            line_ = parser.line();
                            column_ = parser.column();
                            selector_stack_.push_back(make_unique_ptr<filter_selector>(result));
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            break;                   
                        }
                        case ':':
                            slice = array_slice();
                            buffer.clear();
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::slice_end_or_end_step);
                            ++p_;
                            ++column_;
                            break;
                        case '*':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_wildcard_or_path);
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_single_quoted_name);
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_double_quoted_name);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            slice = array_slice();
                            buffer.clear();
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_unquoted_name);
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::bracketed_unquoted_name:
                    switch (*p_)
                    {
                        case ':':
                            if (!try_string_to_index(buffer.data(), buffer.size(), &slice.start_, &slice.is_start_positive))
                            {
                                ec = jmespath_errc::expected_slice_start;
                                return;
                            }
                            state_stack_.back().state = path_state::slice_end_or_end_step;
                            ++p_;
                            ++column_;
                            break;
                        case '.':
                        case '[':
                        case ',': 
                        case ']': 
                            state_stack_.back().state = path_state::bracketed_name_or_path;
                            break;
                        default:
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::bracketed_name_or_path:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '.':
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::path;
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::path2;
                            ++p_;
                            ++column_;
                            break;
                        case ',': 
                        case ']': 
                            if (!buffer.empty())
                            {
                                selector_stack_.push_back(make_unique_ptr<identifier_selector>(buffer));
                                buffer.clear();
                            }
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return;
                    }
                    break;
                case path_state::bracketed_wildcard_or_path:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '.':
                            buffer.push_back('*');
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::path;
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            buffer.push_back('*');
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::path2;
                            ++p_;
                            ++column_;
                            break;
                        case ',': 
                        case ']': 
                            end_all();
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return;
                    }
                    break;
                case path_state::path:
                    switch (*p_)
                    {
                        case '\'':
                            buffer.push_back(*p_);
                            state_stack_.emplace_back(path_state::path_single_quoted);
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            buffer.push_back(*p_);
                            state_stack_.emplace_back(path_state::path_double_quoted);
                            ++p_;
                            ++column_;
                            break;
                        case ',': 
                        case ']': 
                            if (!buffer.empty())
                            {
                                selector_stack_.push_back(make_unique_ptr<path_selector>(buffer));
                                buffer.clear();
                            }
                            state_stack_.pop_back();
                            break;
                        default:
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::path_double_quoted:
                    switch (*p_)
                    {
                        case '\"': 
                            buffer.push_back(*p_);
                            state_stack_.pop_back();
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::path_single_quoted:
                    switch (*p_)
                    {
                        case '\'': 
                            buffer.push_back(*p_);
                            state_stack_.pop_back();
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::path2:
                    switch (*p_)
                    {
                        case ']': 
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::path;
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::slice_end_or_end_step:
                    switch (*p_)
                    {
                        case '-':
                            slice.is_end_positive = false;
                            state_stack_.back().state = path_state::slice_end;
                            ++p_;
                            ++column_;
                            break;
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            slice.is_end_defined = true;
                            slice.end_ = static_cast<std::size_t>(*p_-'0');
                            state_stack_.back().state = path_state::slice_end;
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                            slice.step_ = 0;
                            state_stack_.back().state = path_state::slice_step;
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                            selector_stack_.push_back(make_unique_ptr<slice_selector>(slice));
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_minus_or_digit_or_colon_or_comma_or_right_bracket;
                            return;
                    }
                    break;
                case path_state::slice_end:
                    switch (*p_)
                    {
                        case ':':
                            slice.step_ = 0;
                            state_stack_.back().state = path_state::slice_step;
                            ++p_;
                            ++column_;
                            break;
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            slice.is_end_defined = true;
                            slice.end_ = slice.end_*10 + static_cast<std::size_t>(*p_-'0');
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                            if (!slice.is_end_defined)
                            {
                                ec = jmespath_errc::expected_slice_end;
                                return;
                            }
                            selector_stack_.push_back(make_unique_ptr<slice_selector>(slice));
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_digit_or_colon_or_comma_or_right_bracket;
                            return;
                    }
                    break;
                case path_state::slice_step:
                    switch (*p_)
                    {
                        case '-':
                            slice.is_step_positive = false;
                            slice.step_ = 0;
                            state_stack_.back().state = path_state::slice_step2;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            slice.step_ = 0;
                            state_stack_.back().state = path_state::slice_step2;
                            break;
                    }
                    break;
                case path_state::slice_step2:
                    switch (*p_)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            slice.step_ = slice.step_*10 + static_cast<std::size_t>(*p_-'0');
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                            if (slice.step_ == 0)
                            {
                                ec = jmespath_errc::expected_slice_step;
                                return;
                            }
                            selector_stack_.push_back(make_unique_ptr<slice_selector>(slice));
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jmespath_errc::expected_minus_or_digit_or_comma_or_right_bracket;
                            return;
                    }
                    break;
                case path_state::bracketed_single_quoted_name:
                    switch (*p_)
                    {
                        case '\'':
                            state_stack_.back().state = path_state::bracketed_name_or_path;
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    };
                    ++p_;
                    ++column_;
                    break;
                case path_state::bracketed_double_quoted_name: 
                    switch (*p_)
                    {
                        case '\"':
                            state_stack_.back().state = path_state::bracketed_name_or_path;
                            break;
                        case '\\':
                            if (p_+1 < end_input_)
                            {
                                ++p_;
                                ++column_;
                                buffer.push_back(*p_);
                            }
                            else
                            {
                                ec = jmespath_errc::unexpected_end_of_input;
                                return;
                            }
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    };
                    ++p_;
                    ++column_;
                    break;
                default:
                    ++p_;
                    ++column_;
                    break;
#endif
            }
        }

        std::cout << "stack size: " << state_stack_.size() << "\n";
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

        reference r = selector_->select(*this, string_type(), root); 
        return r;
    }
#if 0
    void end_all()
    {
        for (const auto& node : stack_.back())
        {
            const auto& path = node.path;
            pointer p = node.val_ptr;
            end_all(path, *p);
        }
    }

    void end_all(const string_type& path, reference val)
    {
        if (val.is_array())
        {
            for (auto it = val.array_range().begin(); it != val.array_range().end(); ++it)
            {
                nodes_.emplace_back(PathCons()(path,it - val.array_range().begin()),std::addressof(*it));
            }
        }
        else if (val.is_object())
        {
            for (auto it = val.object_range().begin(); it != val.object_range().end(); ++it)
            {
                nodes_.emplace_back(PathCons()(path,it->key()),std::addressof(it->value()));
            }
        }
        if (state_stack_.back().is_recursive_descent)
        {
            if (val.is_array())
            {
                for (auto it = val.array_range().begin(); it != val.array_range().end(); ++it)
                {
                    end_all(PathCons()(path, it - val.array_range().begin()),*it);
                }
            }
            else if (val.is_object())
            {
                for (auto it = val.object_range().begin(); it != val.object_range().end(); ++it)
                {
                    end_all(PathCons()(path,it->key()),it->value());
                }
            }
        }
    }

#endif
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
