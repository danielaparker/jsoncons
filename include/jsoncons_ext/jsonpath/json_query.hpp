// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_QUERY_HPP
#define JSONCONS_JSONPATH_JSON_QUERY_HPP

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
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_filter.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_function.hpp>

namespace jsoncons { namespace jsonpath {

struct slice
{
    int64_t start_;
    jsoncons::optional<int64_t> end_;
    int64_t step_;

    slice()
        : start_(0), end_(), step_(1)
    {
    }

    slice(int64_t start, const jsoncons::optional<int64_t>& end, int64_t step) 
        : start_(start), end_(end), step_(step)
    {
    }

    slice(const slice& other)
        : start_(other.start_), end_(other.end_), step_(other.step_)
    {
    }

    slice& operator=(const slice& rhs) 
    {
        if (this != &rhs)
        {
            start_ = rhs.start_;
            if (rhs.end_)
            {
                end_ = rhs.end_;
            }
            else
            {
                end_.reset();
            }
            step_ = rhs.step_;
        }
        return *this;
    }

    int64_t get_start(std::size_t size) const
    {
        return start_ >= 0 ? start_ : (static_cast<int64_t>(size) + start_);
    }

    int64_t get_end(std::size_t size) const
    {
        if (end_)
        {
            auto len = *end_ >= 0 ? *end_ : (static_cast<int64_t>(size) - *end_);
            return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
        }
        else
        {
            return static_cast<int64_t>(size);
        }
    }

    int64_t step() const
    {
        return step_; // Allow negative
    }
};

enum class result_type {value,path};

template<class Json>
Json json_query(const Json& root, const typename Json::string_view_type& path, result_type result_t = result_type::value)
{
    if (result_t == result_type::value)
    {
        jsoncons::jsonpath::detail::jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator;
        jsoncons::jsonpath::detail::jsonpath_resources<Json> resources;
        evaluator.evaluate(resources, root, path);
        return evaluator.get_values();
    }
    else
    {
        jsoncons::jsonpath::detail::jsonpath_evaluator<Json,const Json&,detail::PathConstructor<Json>> evaluator;
        jsoncons::jsonpath::detail::jsonpath_resources<Json> resources;
        evaluator.evaluate(resources, root, path);
        return evaluator.get_normalized_paths();
    }
}

template<class Json, class T>
void json_replace(Json& root, const typename Json::string_view_type& path, T&& new_value)
{
    jsoncons::jsonpath::detail::jsonpath_evaluator<Json,Json&,detail::VoidPathConstructor<Json>> evaluator;
    jsoncons::jsonpath::detail::jsonpath_resources<Json> resources;
    evaluator.evaluate(resources, root, path);
    evaluator.replace(std::forward<T>(new_value));
}

namespace detail {
 
enum class path_state 
{
    start,
    dot_or_left_bracket,
    name_or_left_bracket,
    name,
    unquoted_name,
    unquoted_name2,
    single_quoted_name,
    double_quoted_name,
    bracketed_unquoted_name,
    bracketed_unquoted_name2,
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
    bool is_recursive_descent;
    bool is_union;

    state_item()
        : state(path_state::start), is_recursive_descent(false), is_union(false)
    {
    }

    explicit state_item(path_state state)
        : state(state), is_recursive_descent(false), is_union(false)
    {
    }

    state_item(path_state state, const state_item& parent)
        : state(state), 
          is_recursive_descent(parent.is_recursive_descent), 
          is_union(parent.is_union)
    {
    }

    state_item(const state_item&) = default;
    state_item& operator=(const state_item&) = default;
};

JSONCONS_STRING_LITERAL(length_literal, 'l', 'e', 'n', 'g', 't', 'h')

template<class Json,
         class JsonReference,
         class PathCons>
class jsonpath_evaluator : public ser_context
{
    using char_type = typename Json::char_type;
    using char_traits_type = typename Json::char_traits_type;
    using string_type = std::basic_string<char_type,char_traits_type>;
    using string_view_type = typename Json::string_view_type;
    using reference = JsonReference;
    using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
    using const_pointer = typename Json::const_pointer;

    struct node_type
    {
        string_type path;
        pointer val_ptr;

        node_type() = default;
        node_type(const string_type& p, const pointer& valp)
            : path(p),val_ptr(valp)
        {
        }

        node_type(string_type&& p, pointer&& valp) noexcept
            : path(std::move(p)),val_ptr(valp)
        {
        }
        node_type(const node_type&) = default;

        node_type(node_type&& other) noexcept
            : path(std::move(other.path)), val_ptr(other.val_ptr)
        {

        }
        node_type& operator=(const node_type&) = default;

        node_type& operator=(node_type&& other) noexcept
        {
            path.swap(other.path);
            val_ptr = other.val_ptr;
        }

    };
    using node_set = std::vector<node_type>;

    struct node_less
    {
        bool operator()(const node_type& a, const node_type& b) const
        {
            return *(a.val_ptr) < *(b.val_ptr);
        }
    };

    class selector_base
    {
    public:
        virtual ~selector_base() noexcept = default;
        virtual void select(jsonpath_resources<Json>& resources,
                            const string_type& path, reference val, node_set& nodes) = 0;

        virtual bool is_filter() const
        {
            return false;
        }
    };

    class path_selector final : public selector_base
    {
    private:
         std::basic_string<char_type> path_;
    public:
        path_selector(const std::basic_string<char_type>& path)
            : path_(path)
        {
        }

        void select(jsonpath_resources<Json>& resources,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            std::error_code ec;
            jsonpath_evaluator<Json,JsonReference,PathCons> e;
            e.evaluate(resources, val, path_, ec);
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
         jsonpath_filter_expr<Json> result_;
    public:
        expr_selector(const jsonpath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        void select(jsonpath_resources<Json>& resources,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            auto index = result_.eval(resources, val);
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
                name_selector selector(index.as_string_view());
                selector.select(resources, path, val, nodes);
            }
        }
    };

    class filter_selector final : public selector_base
    {
    private:
         jsonpath_filter_expr<Json> result_;
    public:
        filter_selector(const jsonpath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        bool is_filter() const override
        {
            return true;
        }

        void select(jsonpath_resources<Json>& resources,
                    const string_type& path, reference val, 
                    node_set& nodes) override
        {
            //std::cout << "filter_selector select ";
            if (val.is_array())
            {
                //std::cout << "from array \n";
                for (std::size_t i = 0; i < val.size(); ++i)
                {
                    if (result_.exists(resources, val[i]))
                    {
                        nodes.emplace_back(PathCons()(path,i),std::addressof(val[i]));
                    }
                }
            }
            else if (val.is_object())
            {
                //std::cout << "from object \n";
                if (result_.exists(resources, val))
                {
                    nodes.emplace_back(path, std::addressof(val));
                }
            }
        }
    };

    class name_selector final : public selector_base
    {
    private:
        string_type name_;
    public:
        name_selector(const string_view_type& name)
            : name_(name)
        {
        }

        void select(jsonpath_resources<Json>& resources,
                    const string_type& path, reference val,
                    node_set& nodes) override
        {
            //bool is_start_positive = true;

            if (val.is_object() && val.contains(name_))
            {
                nodes.emplace_back(PathCons()(path,name_),std::addressof(val.at(name_)));
            }
            else if (val.is_array())
            {
                auto r = jsoncons::detail::to_integer_decimal<int64_t>(name_.data(), name_.size());
                if (r)
                {
                    std::size_t index = (r.value() >= 0) ? static_cast<std::size_t>(r.value()) : static_cast<std::size_t>(static_cast<int64_t>(val.size()) + r.value());
                    if (index < val.size())
                    {
                        nodes.emplace_back(PathCons()(path,index),std::addressof(val[index]));
                    }
                }
                else if (name_ == length_literal<char_type>() && val.size() > 0)
                {
                    pointer ptr = resources.create_temp(val.size());
                    nodes.emplace_back(PathCons()(path, name_), ptr);
                }
            }
            else if (val.is_string())
            {
                string_view_type sv = val.as_string_view();
                auto r = jsoncons::detail::to_integer_decimal<int64_t>(name_.data(), name_.size());
                if (r)
                {
                    std::size_t index = (r.value() >= 0) ? static_cast<std::size_t>(r.value()) : 
                                                           static_cast<std::size_t>(static_cast<int64_t>(sv.size()) + r.value());
                    auto sequence = unicons::sequence_at(sv.data(), sv.data() + sv.size(), index);
                    if (sequence.length() > 0)
                    {
                        pointer ptr = resources.create_temp(sequence.begin(),sequence.length());
                        nodes.emplace_back(PathCons()(path, index), ptr);
                    }
                }
                else if (name_ == length_literal<char_type>() && sv.size() > 0)
                {
                    std::size_t count = unicons::u32_length(sv.begin(),sv.end());
                    pointer ptr = resources.create_temp(count);
                    nodes.emplace_back(PathCons()(path, name_), ptr);
                }
            }
        }
    };

    class slice_selector final : public selector_base
    {
    private:
        slice slice_;
    public:
        slice_selector(const slice& a_slice)
            : slice_(a_slice) 
        {
        }

        void select(jsonpath_resources<Json>&,
                    const string_type& path, reference val,
                    node_set& nodes) override
        {
            if (val.is_array())
            {
                auto start = slice_.get_start(val.size());
                auto end = slice_.get_end(val.size());
                auto step = slice_.step();
                if (step >= 0)
                {
                    for (int64_t j = start; j < end; j += step)
                    {
                        std::size_t uj = static_cast<std::size_t>(j);
                        if (uj < val.size())
                        {
                            nodes.emplace_back(PathCons()(path,uj),std::addressof(val[uj]));
                        }
                    }
                }
                else
                {
                    for (int64_t j = end-1; j >= start; j += step)
                    {
                        std::size_t uj = static_cast<std::size_t>(j);
                        if (uj < val.size())
                        {
                            nodes.emplace_back(PathCons()(path,uj),std::addressof(val[uj]));
                        }
                    }
                }
            }
        }
    };

    function_table<Json,pointer> functions_;

    node_set nodes_;
    std::vector<node_set> stack_;
    std::size_t line_;
    std::size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    std::vector<std::unique_ptr<selector_base>> selectors_;
    std::vector<std::unique_ptr<Json>> temp_json_values_;

    using argument_type = std::vector<pointer>;
    std::vector<argument_type> function_stack_;
    std::vector<state_item> state_stack_;

public:
    jsonpath_evaluator()
        : line_(1), column_(1),
          begin_input_(nullptr), end_input_(nullptr),
          p_(nullptr)
    {
    }

    jsonpath_evaluator(std::size_t line, std::size_t column)
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

    Json get_values() const
    {
        Json result = typename Json::array();

        if (stack_.size() > 0)
        {
            result.reserve(stack_.back().size());
            for (const auto& p : stack_.back())
            {
                result.push_back(*(p.val_ptr));
            }
        }
        return result;
    }

    std::vector<pointer> get_pointers() const
    {
        std::vector<pointer> result;

        if (stack_.size() > 0)
        {
            result.reserve(stack_.back().size());
            for (const auto& p : stack_.back())
            {
                result.push_back(p.val_ptr);
            }
        }
        return result;
    }

    void call_function(jsonpath_resources<Json>& resources, const string_type& function_name, std::error_code& ec)
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
        pointer ptr = resources.create_temp(std::move(result));
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

    void evaluate(jsonpath_resources<Json>& resources, reference root, const string_view_type& path)
    {
        std::error_code ec;
        evaluate(resources, root, path.data(), path.length(), ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpath_error(ec, line_, column_));
        }
    }

    void evaluate(jsonpath_resources<Json>& resources, reference root, const string_view_type& path, std::error_code& ec)
    {
        JSONCONS_TRY
        {
            evaluate(resources, root, path.data(), path.length(), ec);
        }
        JSONCONS_CATCH(...)
        {
            ec = jsonpath_errc::unidentified_error;
        }
    }
 
    void evaluate(jsonpath_resources<Json>& resources,
                  reference root, 
                  const char_type* path, 
                  std::size_t length,
                  std::error_code& ec)
    {
        state_stack_.emplace_back(path_state::start);

        string_type function_name;
        string_type buffer;

        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        string_type s = {'$'};
        node_set v;
        v.emplace_back(std::move(s),std::addressof(root));
        stack_.push_back(v);

        slice a_slice;

        while (p_ < end_input_)
        {
            switch (state_stack_.back().state)
            {
                case path_state::start: 
                {
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '$':
                        {
                            state_stack_.emplace_back(path_state::dot_or_left_bracket, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        }
                        default:
                        {
                            switch (*p_)
                            {
                                case '.':
                                case '[':
                                    ec = jsonpath_errc::expected_root;
                                    return;
                                default: // might be function, validate name later
                                    state_stack_.emplace_back(path_state::dot_or_left_bracket, state_stack_.back());
                                    state_stack_.emplace_back(path_state::path_or_function_name, state_stack_.back());
                                    break;
                            }
                            break;
                        }

                        return;
                    };
                    break;
                }
                case path_state::path_or_function_name:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                        {
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            apply_selectors(resources);
                            buffer.clear();
                            state_stack_.pop_back();
                            advance_past_space_character();
                            break;
                        }
                        case '(':
                            state_stack_.back().state = path_state::arg_or_right_paren;
                            function_name = std::move(buffer);
                            buffer.clear();
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                        {
                            if (buffer.size() > 0)
                            {
                                selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                                apply_selectors(resources);
                                buffer.clear();
                            }
                            a_slice.start_ = 0;
                            buffer.clear();

                            state_stack_.back().state = path_state::wildcard_or_rpath_or_slice_or_filter;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '.':
                        {
                            if (buffer.size() > 0)
                            {
                                selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                                apply_selectors(resources);
                                buffer.clear();
                            }
                            state_stack_.back().state = path_state::dot;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '*':
                        {
                            end_all();
                            transfer_nodes();
                            state_stack_.back().state = path_state::dot;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '\'':
                        {
                            buffer.clear();
                            state_stack_.back().state = path_state::single_quoted_name;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '\"':
                        {
                            buffer.clear();
                            state_stack_.back().state = path_state::double_quoted_name;
                            ++p_;
                            ++column_;
                            break;
                        }
                        default:
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::arg_or_right_paren:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case '$':
                            buffer.clear();
                            buffer.push_back(*p_);
                            state_stack_.emplace_back(path_state::path_argument, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            buffer.clear();
                            buffer.push_back('\"');
                            state_stack_.back().state = path_state::more_args_or_right_paren;
                            state_stack_.emplace_back(path_state::single_quoted_arg, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            buffer.clear();
                            buffer.push_back('\"');
                            state_stack_.back().state = path_state::more_args_or_right_paren;
                            state_stack_.emplace_back(path_state::double_quoted_arg, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case ')':
                        {
                            jsonpath_evaluator<Json,JsonReference,PathCons> evaluator;
                            evaluator.evaluate(resources, root, buffer, ec);
                            if (ec)
                            {
                                return;
                            }
                            function_stack_.push_back(evaluator.get_pointers());

                            call_function(resources, function_name, ec);
                            if (ec)
                            {
                                return;
                            }
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        }
                        default:
                            buffer.clear();
                            state_stack_.back().state = path_state::more_args_or_right_paren;
                            state_stack_.emplace_back(path_state::unquoted_arg, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::path_argument:
                    switch (*p_)
                    {
                        case ',':
                        {
                            jsonpath_evaluator<Json, JsonReference, PathCons> evaluator;
                            evaluator.evaluate(resources, root, buffer, ec);
                            if (ec)
                            {
                                return;
                            }
                            function_stack_.push_back(evaluator.get_pointers());
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        }
                        case ')':
                        {
                            state_stack_.pop_back();
                            break;
                        }
                        default:
                            buffer.push_back(*p_); // path_argument
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::unquoted_arg:
                    switch (*p_)
                    {
                        case ',':
                            JSONCONS_TRY
                            {
                                auto val = Json::parse(buffer);
                                auto temp = resources.create_temp(val);
                                function_stack_.push_back(std::vector<pointer>{temp});
                            }
                            JSONCONS_CATCH(const ser_error&)     
                            {
                                ec = jsonpath_errc::argument_parse_error;
                                return;
                            }
                            buffer.clear();
                            //state_ = path_state::arg_or_right_paren;
                            state_stack_.pop_back();
                            break;
                        case ')':
                        {
                            JSONCONS_TRY
                            {
                                auto val = Json::parse(buffer);
                                auto temp = resources.create_temp(val);
                                function_stack_.push_back(std::vector<pointer>{temp});
                            }
                            JSONCONS_CATCH(const ser_error&)     
                            {
                                ec = jsonpath_errc::argument_parse_error;
                                return;
                            }
                            call_function(resources, function_name, ec);
                            if (ec)
                            {
                                return;
                            }
                            state_stack_.pop_back();
                            break;
                        }
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::single_quoted_arg:
                    switch (*p_)
                    {
                        case '\'':
                            buffer.push_back('\"');
                            state_stack_.pop_back();
                            break;
                        case '\"':
                            buffer.push_back('\\');
                            buffer.push_back('\"');
                            state_stack_.pop_back();
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::double_quoted_arg:
                    switch (*p_)
                    {
                        case '\"':
                            buffer.push_back('\"');
                            state_stack_.pop_back();
                            break;
                        default:
                            buffer.push_back(*p_);
                            break;
                    }
                    ++p_;
                    ++column_;
                    break;
                case path_state::more_args_or_right_paren:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;
                        case ',':
                            JSONCONS_TRY
                            {
                                auto val = Json::parse(buffer);
                                auto temp = resources.create_temp(val);
                                function_stack_.push_back(std::vector<pointer>{temp});
                            }
                            JSONCONS_CATCH(const ser_error&)     
                            {
                                ec = jsonpath_errc::argument_parse_error;
                                return;
                            }
                            buffer.clear();
                            //state_ = path_state::arg_or_right_paren;
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        case ')':
                        {
                            JSONCONS_TRY
                            {
                                auto val = Json::parse(buffer);
                                auto temp = resources.create_temp(val);
                                function_stack_.push_back(std::vector<pointer>{temp});
                            }
                            JSONCONS_CATCH(const ser_error&)     
                            {
                                ec = jsonpath_errc::argument_parse_error;
                                return;
                            }
                            call_function(resources, function_name, ec);
                            if (ec)
                            {
                                return;
                            }
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        }
                        default:
                            ec = jsonpath_errc::invalid_filter_unsupported_operator;
                            return;
                    }
                    break;
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
                            ec = jsonpath_errc::expected_key;
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
                            state_stack_.emplace_back(path_state::dot, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.emplace_back(path_state::wildcard_or_rpath_or_slice_or_filter, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jsonpath_errc::expected_separator;
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
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            apply_selectors(resources);
                            buffer.clear();
                            a_slice.start_ = 0;
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        case '.':
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            apply_selectors(resources);
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jsonpath_errc::expected_key;
                            return;
                    };
                    break;
                case path_state::single_quoted_name:
                    switch (*p_)
                    {
                        case '\'':
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            apply_selectors(resources);
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            apply_selectors(resources);
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
                            apply_selectors(resources);
                            state_stack_.pop_back();
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jsonpath_errc::expected_right_bracket;
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
                            jsonpath_filter_parser<Json> parser(line_,column_);
                            auto result = parser.parse(resources, root, p_,end_input_,&p_);
                            line_ = parser.line();
                            column_ = parser.column();
                            selectors_.push_back(jsoncons::make_unique<expr_selector>(result));
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            break;
                        }
                        case '?':
                        {
                            jsonpath_filter_parser<Json> parser(line_,column_);
                            auto result = parser.parse(resources,root,p_,end_input_,&p_);
                            line_ = parser.line();
                            column_ = parser.column();
                            selectors_.push_back(jsoncons::make_unique<filter_selector>(result));
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            break;                   
                        }
                        case ':':
                            a_slice = slice{};
                            buffer.clear();
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::slice_end_or_end_step, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '*':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_wildcard_or_path, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_single_quoted_name, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_double_quoted_name, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        default:
                            a_slice = slice();
                            buffer.clear();
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::comma_or_right_bracket;
                            state_stack_.emplace_back(path_state::bracketed_unquoted_name, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::bracketed_unquoted_name:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                        case ':':
                        case '.':
                        case '[':
                        case ',': 
                        case ']': 
                            state_stack_.back().state = path_state::bracketed_unquoted_name2;
                            break;
                        default:
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                    }
                    break;
                case path_state::bracketed_unquoted_name2:
                    switch (*p_)
                    {
                        case ' ':case '\t':case '\r':case '\n':
                            advance_past_space_character();
                            break;

                        case ':':
                        {
                            auto r = jsoncons::detail::to_integer_decimal<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jsonpath_errc::expected_slice_start;
                                return;
                            }
                            a_slice.start_ = r.value();
                            state_stack_.back().state = path_state::slice_end_or_end_step;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '.':
                        case '[':
                        case ',': 
                        case ']': 
                            state_stack_.back().state = path_state::bracketed_name_or_path;
                            break;
                        default:
                            ec = jsonpath_errc::expected_colon_dot_left_bracket_comma_or_right_bracket;
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
                            selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                            buffer.clear();
                            state_stack_.pop_back();
                            break;
                        default:
                            ec = jsonpath_errc::expected_right_bracket;
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
                            ec = jsonpath_errc::expected_right_bracket;
                            return;
                    }
                    break;
                case path_state::path:
                    switch (*p_)
                    {
                        case '\'':
                            buffer.push_back(*p_);
                            state_stack_.emplace_back(path_state::path_single_quoted, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            buffer.push_back(*p_);
                            state_stack_.emplace_back(path_state::path_double_quoted, state_stack_.back());
                            ++p_;
                            ++column_;
                            break;
                        case ',': 
                        case ']': 
                            if (!buffer.empty())
                            {
                                selectors_.push_back(jsoncons::make_unique<path_selector>(buffer));
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
                            buffer.clear();
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::slice_end;
                            ++p_;
                            ++column_;
                            break;
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            buffer.clear();
                            buffer.push_back(*p_);
                            state_stack_.back().state = path_state::slice_end;
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                            state_stack_.back().state = path_state::slice_step;
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                        {
                            selectors_.push_back(jsoncons::make_unique<slice_selector>(a_slice));
                            state_stack_.pop_back();
                            break;
                        }
                        default:
                            ec = jsonpath_errc::expected_minus_or_digit_or_colon_or_comma_or_right_bracket;
                            return;
                    }
                    break;
                case path_state::slice_end:
                    switch (*p_)
                    {
                        case ':':
                        {
                            auto r = jsoncons::detail::to_integer_decimal<int64_t>(buffer.data(), buffer.size());
                            if (!r || r.value() == 0)
                            {
                                ec = jsonpath_errc::expected_slice_end;
                                return;
                            }
                            a_slice.end_ = jsoncons::optional<int64_t>(r.value());
                            buffer.clear();
                            state_stack_.back().state = path_state::slice_step;
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                        {
                            auto r = jsoncons::detail::to_integer_decimal<int64_t>(buffer.data(), buffer.size());
                            if (!r || r.value() == 0)
                            {
                                ec = jsonpath_errc::expected_slice_end;
                                return;
                            }
                            a_slice.end_ = jsoncons::optional<int64_t>(r.value());
                            selectors_.push_back(jsoncons::make_unique<slice_selector>(a_slice));
                            state_stack_.pop_back();
                            break;
                        }
                        default:
                            ec = jsonpath_errc::expected_digit_or_colon_or_comma_or_right_bracket;
                            return;
                    }
                    break;
                case path_state::slice_step:
                    switch (*p_)
                    {
                        case '-':
                            buffer.clear();
                            state_stack_.back().state = path_state::slice_step2;
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            buffer.clear();
                            state_stack_.back().state = path_state::slice_step2;
                            break;
                    }
                    break;
                case path_state::slice_step2:
                    switch (*p_)
                    {
                        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            buffer.push_back(*p_);
                            ++p_;
                            ++column_;
                            break;
                        case ',':
                        case ']':
                        {
                            auto r = jsoncons::detail::to_integer_decimal<int64_t>(buffer.data(), buffer.size());
                            if (!r || r.value() == 0)
                            {
                                ec = jsonpath_errc::expected_slice_step;
                                return;
                            }
                            a_slice.step_ = r.value();
                            selectors_.push_back(jsoncons::make_unique<slice_selector>(a_slice));
                            state_stack_.pop_back();
                            break;
                        }
                        default:
                            ec = jsonpath_errc::expected_minus_or_digit_or_comma_or_right_bracket;
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
                                ec = jsonpath_errc::unexpected_end_of_input;
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
            }
        }

        switch (state_stack_.back().state)
        {
            case path_state::unquoted_name: 
            case path_state::unquoted_name2: 
            {
                selectors_.push_back(jsoncons::make_unique<name_selector>(buffer));
                apply_selectors(resources);
                buffer.clear();
                state_stack_.pop_back(); // unquoted_name
                break;
            }
            default:
                break;
        }

        if (state_stack_.size() > 2)
        {
            ec = jsonpath_errc::unexpected_end_of_input;
            return;
        }

        JSONCONS_ASSERT(state_stack_.size() == 2);
        state_stack_.pop_back(); 

        JSONCONS_ASSERT(state_stack_.back().state == path_state::start);
        state_stack_.pop_back();
    }

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

    void apply_selectors(jsonpath_resources<Json>& resources)
    {
        //std::cout << "apply_selectors count: " << selectors_.size() << "\n";
        if (selectors_.size() > 0)
        {
            for (auto& node : stack_.back())
            {
                //std::cout << "apply selector to:\n" << pretty_print(*(node.val_ptr)) << "\n";
                for (auto& selector : selectors_)
                {
                    apply_selector(resources, node.path, *(node.val_ptr), *selector, true);
                }
            }
            selectors_.clear();
        }
        transfer_nodes();
    }

    void apply_selector(jsonpath_resources<Json>& resources,
                        const string_type& path, 
                        reference val, 
                        selector_base& selector, 
                        bool process)
    {
        if (process)
        {
            selector.select(resources, path, val, nodes_);
        }
        //std::cout << "*it: " << val << "\n";
        //std::cout << "apply_selectors 1 done\n";
        if (state_stack_.back().is_recursive_descent)
        {
            //std::cout << "is_recursive_descent\n";
            if (val.is_object())
            {
                //std::cout << "is_object\n";
                for (auto& nvp : val.object_range())
                {
                    if (nvp.value().is_array() || nvp.value().is_object())
                    {                        
                        apply_selector(resources, PathCons()(path,nvp.key()), nvp.value(), selector, true);
                    } 
                }
            }
            else if (val.is_array())
            {
                //std::cout << "is_array\n";
                auto first = val.array_range().begin();
                auto last = val.array_range().end();
                for (auto it = first; it != last; ++it)
                {
                    if (it->is_array())
                    {
                        apply_selector(resources, PathCons()(path,it - first), *it,selector, true);
                        //std::cout << "*it: " << *it << "\n";
                    }
                    else if (it->is_object())
                    {
                        apply_selector(resources, PathCons()(path,it - first), *it, selector, !selector.is_filter());
                    }
                }
            }
        }
    }

    void transfer_nodes()
    {
        if (state_stack_.back().is_union)
        {
            std::set<node_type, node_less> index;
            std::vector<node_type> temp;
            for (const auto& node : nodes_)
            {
                if (index.count(node) == 0)
                {
                    temp.emplace_back(node);
                    index.emplace(node);
                }
            }
            stack_.emplace_back(std::move(temp));
        }
        else
        {
            stack_.push_back(std::move(nodes_));
        }
        nodes_.clear();
        state_stack_.back().is_recursive_descent = false;
        state_stack_.back().is_union = false;
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
