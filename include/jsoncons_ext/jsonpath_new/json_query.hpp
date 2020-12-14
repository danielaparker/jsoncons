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
#include <jsoncons_ext/jsonpath_new/jsonpath_filter.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_function.hpp>

namespace jsoncons { namespace jsonpath_new {

    struct slice
    {
        jsoncons::optional<int64_t> start_;
        jsoncons::optional<int64_t> stop_;
        int64_t step_;

        slice()
            : start_(), stop_(), step_(1)
        {
        }

        slice(const jsoncons::optional<int64_t>& start, const jsoncons::optional<int64_t>& end, int64_t step) 
            : start_(start), stop_(end), step_(step)
        {
        }

        slice(const slice& other)
            : start_(other.start_), stop_(other.stop_), step_(other.step_)
        {
        }

        slice& operator=(const slice& rhs) 
        {
            if (this != &rhs)
            {
                if (rhs.start_)
                {
                    start_ = rhs.start_;
                }
                else
                {
                    start_.reset();
                }
                if (rhs.stop_)
                {
                    stop_ = rhs.stop_;
                }
                else
                {
                    stop_.reset();
                }
                step_ = rhs.step_;
            }
            return *this;
        }

        int64_t get_start(std::size_t size) const
        {
            if (start_)
            {
                auto len = *start_ >= 0 ? *start_ : (static_cast<int64_t>(size) + *start_);
                return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
            }
            else
            {
                if (step_ >= 0)
                {
                    return 0;
                }
                else 
                {
                    return static_cast<int64_t>(size);
                }
            }
        }

        int64_t get_stop(std::size_t size) const
        {
            if (stop_)
            {
                auto len = *stop_ >= 0 ? *stop_ : (static_cast<int64_t>(size) + *stop_);
                return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
            }
            else
            {
                return step_ >= 0 ? static_cast<int64_t>(size) : -1;
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
            jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator;
            jsoncons::jsonpath_new::detail::jsonpath_resources<Json> resources;
            evaluator.evaluate(resources, root, path);
            return evaluator.get_values();
        }
        else
        {
            jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,const Json&,detail::PathConstructor<Json>> evaluator;
            jsoncons::jsonpath_new::detail::jsonpath_resources<Json> resources;
            evaluator.evaluate(resources, root, path);
            return evaluator.get_normalized_paths();
        }
    }

    template<class Json, class T>
    typename std::enable_if<!jsoncons::detail::is_function_object<T,Json>::value,void>::type
    json_replace(Json& root, const typename Json::string_view_type& path, T&& new_value)
    {
        jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,Json&,detail::VoidPathConstructor<Json>> evaluator;
        jsoncons::jsonpath_new::detail::jsonpath_resources<Json> resources;
        evaluator.evaluate(resources, root, path);
        evaluator.replace(std::forward<T>(new_value));
    }

    template<class Json, class Op>
    typename std::enable_if<jsoncons::detail::is_function_object<Op,Json>::value,void>::type
    json_replace(Json& root, const typename Json::string_view_type& path, Op op)
    {
        jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,Json&,detail::VoidPathConstructor<Json>> evaluator;
        jsoncons::jsonpath_new::detail::jsonpath_resources<Json> resources;
        evaluator.evaluate(resources, root, path);
        evaluator.replace_fn(op);
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

        bracket_specifier,
        index_or_slice_expression,
        number,
        digit,
        rhs_slice_expression_start,
        rhs_slice_expression_stop,

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

        explicit state_item(path_state state) noexcept
            : state(state), is_recursive_descent(false), is_union(false)
        {
        }

        state_item(path_state state, const state_item& parent) noexcept
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
                return *this;
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

            virtual bool is_recursive_descent() const
            {
                return false;
            }
        };

        class identifier_selector final : public selector_base
        {
        private:
            string_type identifier_;
        public:
            identifier_selector(const string_view_type& identifier)
                : identifier_(identifier)
            {
            }

            void select(jsonpath_resources<Json>& resources,
                        const string_type& path, reference val,
                        node_set& nodes) override
            {
                if (val.is_object())
                {
                    auto it = val.find(identifier_);
                    if (it != val.object_range().end())
                    {
                        nodes.emplace_back(PathCons()(path,identifier_),std::addressof(it->value()));
                    }
                }
            }
        };

        class index_selector final : public selector_base
        {
            int64_t index_;
        public:
            index_selector(int64_t index)
                : index_(index)
            {
            }

            void select(jsonpath_resources<Json>& resources,
                        const string_type& path, reference val,
                        node_set& nodes) override
            {
                if (val.is_array())
                {
                    int64_t slen = static_cast<int64_t>(val.size());
                    if (index_ >= 0 && index_ < slen)
                    {
                        std::size_t index = static_cast<std::size_t>(index_);
                        nodes.emplace_back(PathCons()(path,""),std::addressof(val.at(index)));
                    }
                    else if ((slen + index_) >= 0 && (slen+index_) < slen)
                    {
                        std::size_t index = static_cast<std::size_t>(slen + index_);
                        nodes.emplace_back(PathCons()(path,""),std::addressof(val.at(index)));
                    }
                }
            }
        };

        class recursive_descent_selector final : public selector_base
        {
        public:

            void select(jsonpath_resources<Json>& resources,
                        const string_type& path, reference val,
                        node_set& nodes) override
            {
            }

            bool is_recursive_descent() const override
            {
                return true;
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
/*
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
*/            }
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
/*
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
                    identifier_selector selector(index.as_string_view());
                    selector.select(resources, path, val, nodes);
                }
*/
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
                /*
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
                */
            }
        };

        class slice_selector final : public selector_base
        {
        private:
            slice slice_;
        public:
            slice_selector(const slice& slic)
                : slice_(slic) 
            {
            }

            void select(jsonpath_resources<Json>&,
                        const string_type& path, reference val,
                        node_set& nodes) override
            {
                /*
                if (val.is_array())
                {
                    auto start = slice_.get_start(val.size());
                    auto end = slice_.get_stop(val.size());
                    auto step = slice_.step();

                    if (step > 0)
                    {
                        if (start < 0)
                        {
                            start = 0;
                        }
                        if (end > static_cast<int64_t>(val.size()))
                        {
                            end = val.size();
                        }
                        for (int64_t i = start; i < end; i += step)
                        {
                            std::size_t j = static_cast<std::size_t>(i);
                            nodes.emplace_back(PathCons()(path,j),std::addressof(val[j]));
                        }
                    }
                    else if (step < 0)
                    {
                        if (start >= static_cast<int64_t>(val.size()))
                        {
                            start = static_cast<int64_t>(val.size()) - 1;
                        }
                        if (end < -1)
                        {
                            end = -1;
                        }
                        for (int64_t i = start; i > end; i += step)
                        {
                            std::size_t j = static_cast<std::size_t>(i);
                            if (j < val.size())
                            {
                                nodes.emplace_back(PathCons()(path,j),std::addressof(val[j]));
                            }
                        }
                    }
                }*/
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

            if (!stack_.empty())
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

            if (!stack_.empty())
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
            if (!stack_.empty())
            {
                result.reserve(stack_.back().size());
                for (const auto& p : stack_.back())
                {
                    result.push_back(p.path);
                }
            }
            return result;
        }

        template<class Op>
        void replace_fn(Op op)
        {
            if (!stack_.empty())
            {
                for (std::size_t i = 0; i < stack_.back().size(); ++i)
                {
                    *(stack_.back()[i].val_ptr) = op(*(stack_.back()[i].val_ptr));
                }
            }
        }

        template <class T>
        void replace(T&& new_value)
        {
            if (!stack_.empty())
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

        class jsonpath_expression
        {
            jsonpath_resources<Json> resources_;
            std::vector<std::unique_ptr<selector_base>> selectors_;
        public:
            jsonpath_expression()
            {
            }

            jsonpath_expression(jsonpath_expression&& expr)
                : resources_(std::move(expr.resources_)),
                  selectors_(std::move(expr.selectors_))
            {
            }

            jsonpath_expression(jsonpath_resources<Json>&& resources,
                                std::vector<std::unique_ptr<selector_base>>&& selectors)
                : resources_(std::move(resources)), selectors_(std::move(selectors))
            {
            }

            Json evaluate(reference instance)
            {
                std::error_code ec;
                Json result = evaluate(instance, ec);
                if (ec)
                {
                    JSONCONS_THROW(jsonpath_error(ec));
                }
                return result;
            }

            Json evaluate(reference instance, std::error_code& )
            {
                jsonpath_resources<Json> dynamic_resources;

                std::vector<node_type> output_stack;
                std::vector<node_type> stack;
                std::vector<node_type> collected;
                string_type path;
                path.push_back('$');
                Json result(json_array_arg);
                bool is_recursive_descent = false;

                if (!selectors_.empty())
                {
                    stack.emplace_back(path,std::addressof(instance));
                    for (std::size_t i = 0; 
                         i < selectors_.size();
                         )
                    {
                        for (auto& item : stack)
                        {
                            output_stack.push_back(std::move(item));
                        }
                        stack.clear();
                        for (auto& item : output_stack)
                        {
                            selectors_[i]->select(dynamic_resources, path, *(item.val_ptr), stack);
                        }

                        if (!stack.empty() && !is_recursive_descent)
                        {
                            output_stack.clear();
                            ++i;
                        }
                        else if (is_recursive_descent && output_stack.empty())
                        {
                            output_stack.clear();
                            for (auto& item : collected)
                            {
                                output_stack.emplace_back(item.path,item.val_ptr);
                                stack.push_back(std::move(item));
                            }
                            collected.clear();
                            is_recursive_descent = false;
                            ++i;
                        }
                        else if (is_recursive_descent)
                        {
                            for (auto& item : stack)
                            {
                                collected.emplace_back(item.path,item.val_ptr);
                            }
                            stack.clear();
                            for (auto& item : output_stack)
                            {
                                if (item.val_ptr->is_object())
                                {
                                    for (auto& val : item.val_ptr->object_range())
                                    {
                                        stack.emplace_back(val.key(),std::addressof(val.value()));
                                    }
                                }
                                else if (item.val_ptr->is_array())
                                {
                                    for (auto& val : item.val_ptr->array_range())
                                    {
                                        stack.emplace_back("",std::addressof(val));
                                    }
                                }
                            }
                            output_stack.clear();
                        }
                        else if (selectors_[i]->is_recursive_descent())
                        {
                            is_recursive_descent = true;
                            ++i;
                        }
                        else if (!is_recursive_descent)
                        {
                            break;
                        }
                    }
                }
                if (!stack.empty())
                {
                    result.reserve(stack.size());
                    for (const auto& p : stack)
                    {
                        result.push_back(*(p.val_ptr));
                    }
                }
                return result;
            }

            static jsonpath_expression compile(const string_view_type& expr)
            {
                jsonpath_resources<Json> resources;
                jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator;
                std::error_code ec;
                auto selectors = evaluator.compile(resources, expr.data(), expr.size(), ec);
                if (ec)
                {
                    JSONCONS_THROW(jsonpath_error(ec, evaluator.line(), evaluator.column()));
                }
                return jsonpath_expression(std::move(resources), std::move(selectors));
            }

            static jsonpath_expression compile(const string_view_type& expr,
                                               std::error_code& ec)
            {
                //jsoncons::jsonpath::detail::jsonpath_evaluator<Json,const Json&> evaluator;
                //return evaluator.compile(expr.data(), expr.size(), ec);
                return jsonpath_expression();
            }
        };

        std::vector<std::unique_ptr<selector_base>> compile(jsonpath_resources<Json>& resources,
                                                            const char_type* path, 
                                                            std::size_t length,
                                                            std::error_code& ec)
        {
            Json instance;
            evaluate(resources, instance, path, length, ec);
            return std::move(selectors_);
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

            slice slic;
            std::size_t save_line = 1;
            std::size_t save_column = 1;

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
                        };
                        break;
                    }
                    case path_state::path_or_function_name:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                            {
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                //apply_selectors(resources);
                                buffer.clear();
                                state_stack_.pop_back();
                                advance_past_space_character();
                                break;
                            }
                            case '(':
                                state_stack_.back().state = path_state::arg_or_right_paren;
                                function_name = buffer;
                                buffer.clear();
                                ++p_;
                                ++column_;
                                save_line = line_;
                                save_column = column_;
                                break;
                            case '[':
                            {
                                if (buffer.size() > 0)
                                {
                                    selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                    //apply_selectors(resources);
                                    buffer.clear();
                                }
                                slic.start_ = 0;
                                buffer.clear();

                                state_stack_.back().state = path_state::bracket_specifier;
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '.':
                            {
                                if (buffer.size() > 0)
                                {
                                    selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                    //apply_selectors(resources);
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
                                jsonpath_evaluator<Json,JsonReference,PathCons> evaluator(save_line, save_column);
                                evaluator.evaluate(resources, root, buffer, ec);
                                if (ec)
                                {
                                    line_ = evaluator.line();
                                    column_ = evaluator.column();
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
                                selectors_.push_back(jsoncons::make_unique<recursive_descent_selector>());
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
                                state_stack_.back().state = path_state::bracket_specifier;
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
                                state_stack_.emplace_back(path_state::bracket_specifier, state_stack_.back());
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
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                //apply_selectors(resources);
                                slic.start_ = 0;
                                buffer.clear();
                                state_stack_.pop_back();
                                break;
                            case '.':
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                //apply_selectors(resources);
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
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                //apply_selectors(resources);
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
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                                //apply_selectors(resources);
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
                                state_stack_.back().state = path_state::bracket_specifier;
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
                    case path_state::bracket_specifier:
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
                            case ':': // slice_expression
                                state_stack_.back().state = path_state::rhs_slice_expression_start ;
                                state_stack_.emplace_back(path_state::number);
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
                            // number
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = state_item(path_state::index_or_slice_expression);
                                state_stack_.emplace_back(path_state::number);
                                break;
                            default:
                                buffer.clear();
                                buffer.push_back(*p_);
                                state_stack_.back().state = path_state::comma_or_right_bracket;
                                state_stack_.emplace_back(path_state::bracketed_unquoted_name, state_stack_.back());
                                ++p_;
                                ++column_;
                                break;
                        }
                        break;

                    case path_state::number:
                        switch(*p_)
                        {
                            case '-':
                                buffer.push_back(*p_);
                                state_stack_.back() = state_item(path_state::digit);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = state_item(path_state::digit);
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
                    case path_state::index_or_slice_expression:
                        switch(*p_)
                        {
                            case ']':
                            {
                                if (buffer.empty())
                                {
                                    ec = jsonpath_errc::invalid_number;
                                    return;
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jsonpath_errc::invalid_number;
                                        return;
                                    }
                                    selectors_.push_back(jsoncons::make_unique<index_selector>(r.value()));

                                    buffer.clear();
                                }
                                state_stack_.pop_back(); // bracket_specifier
                                ++p_;
                                ++column_;
                                break;
                            }
                            case ':':
                            {
                                if (!buffer.empty())
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jsonpath_errc::invalid_number;
                                        return;
                                    }
                                    slic.start_ = r.value();
                                    buffer.clear();
                                }
                                state_stack_.back() = state_item(path_state::rhs_slice_expression_start);
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return;
                        }
                        break;
                    case path_state::rhs_slice_expression_start:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jsonpath_errc::invalid_number;
                                return;
                            }
                            slic.stop_ = jsoncons::optional<int64_t>(r.value());
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                selectors_.push_back(jsoncons::make_unique<slice_selector>(slic));
                                slic = slice{};
                                state_stack_.pop_back(); // bracket_specifier2
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = state_item(path_state::rhs_slice_expression_stop);
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return;
                        }
                        break;
                    }
                    case path_state::rhs_slice_expression_stop:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jsonpath_errc::invalid_number;
                                return;
                            }
                            if (r.value() == 0)
                            {
                                ec = jsonpath_errc::step_cannot_be_zero;
                                return;
                            }
                            slic.step_ = r.value();
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                selectors_.push_back(jsoncons::make_unique<slice_selector>(slic));
                                buffer.clear();
                                slic = slice{};
                                state_stack_.pop_back(); // rhs_slice_expression_stop
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return;
                        }
                        break;
                    }

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
                                selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
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
                    selectors_.push_back(jsoncons::make_unique<identifier_selector>(buffer));
                    //apply_selectors(resources);
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

            switch (state_stack_.back().state)
            {
                case path_state::start:
                {
                    JSONCONS_ASSERT(!stack_.empty());
                    stack_.clear();
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.pop_back();
                    break;
                }
                default:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 2);
                    state_stack_.pop_back(); 
                    JSONCONS_ASSERT(state_stack_.back().state == path_state::start);
                    state_stack_.pop_back();
                    break;
                }
            }
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

    template <class Json>
    using jsonpath_expression = typename jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json, const Json&, detail::VoidPathConstructor<Json>>::jsonpath_expression;

    template <class Json>
    jsonpath_expression<Json> make_expression(const typename json::string_view_type& expr)
    {
        return jsonpath_expression<Json>::compile(expr);
    }

    template <class Json>
    jsonpath_expression<Json> make_expression(const typename json::string_view_type& expr,
                                              std::error_code& ec)
    {
        return jsonpath_expression<Json>::compile(expr, ec);
    }

} // namespace jsonpath_new
} // namespace jsoncons

#endif
