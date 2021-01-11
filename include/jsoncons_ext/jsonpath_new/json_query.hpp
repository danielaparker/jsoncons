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
//#include <jsoncons_ext/jsonpath_new/jsonpath_filter.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_expression.hpp>

namespace jsoncons { namespace jsonpath_new {

    // token

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

    namespace detail {
     
    enum class path_state 
    {
        start,
        rhs_expression,
        recursive_descent_or_lhs_expression,
        lhs_expression,
        path_or_literal_or_function,
        json_text_or_function_expr,
        literal,
        identifier_or_function_expr,
        name_or_left_bracket,
        unquoted_string,
        function_expression,
        argument,
        identifier,
        single_quoted_string,
        double_quoted_string,
        bracketed_unquoted_name_or_union,
        union_expression,
        single_quoted_name_or_union,
        double_quoted_name_or_union,
        identifier_or_union,
        wildcard_or_union,
        bracket_specifier_or_union,
        index_or_slice_or_union,
        index,
        integer,
        digit,
        rhs_slice_expression_start,
        rhs_slice_expression_stop,
        comma_or_right_bracket,
        expect_right_bracket,
        quoted_string_escape_char,
        escape_u1, 
        escape_u2, 
        escape_u3, 
        escape_u4, 
        escape_expect_surrogate_pair1, 
        escape_expect_surrogate_pair2, 
        escape_u5, 
        escape_u6, 
        escape_u7, 
        escape_u8,
        filter,
        expression,
        comparator_expression,
        eq_or_regex,
        expect_regex,
        regex,
        cmp_lt_or_lte,
        cmp_gt_or_gte,
        cmp_ne,
        expect_or,
        expect_and
    };

    JSONCONS_STRING_LITERAL(length_literal, 'l', 'e', 'n', 'g', 't', 'h')

    template<class Json,
             class JsonReference>
    class jsonpath_evaluator : public ser_context
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = typename Json::string_view_type;
        using path_node_type = path_node<Json,JsonReference>;
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename path_node_type::pointer;
        using selector_base_type = selector_base<Json,JsonReference>;
        using token_type = token<Json,JsonReference>;
        using path_expression_type = path_expression<Json,JsonReference>;
        using function_base_type = function_base<Json,JsonReference>;

    private:
        class identifier_selector final : public selector_base_type
        {
            string_type identifier_;
            using selector_base_type::generate_path;
        public:
            identifier_selector(const string_view_type& identifier)
                : identifier_(identifier)
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference /* root */,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                //std::cout << "identifier_selector " << identifier_ << ", val: " << val << "\n";
                if (val.is_object())
                {
                    auto it = val.find(identifier_);
                    if (it != val.object_range().end())
                    {
                        nodes.emplace_back(generate_path(path,identifier_,flags),std::addressof(it->value()));
                    }
                }
                else if (val.is_array() && identifier_ == length_literal<char_type>())
                {
                    pointer ptr = resources.create_json(val.size());
                    nodes.emplace_back(generate_path(path, identifier_, flags), ptr);
                }
                else if (val.is_string() && identifier_ == length_literal<char_type>())
                {
                    string_view_type sv = val.as_string_view();
                    std::size_t count = unicons::u32_length(sv.begin(), sv.end());
                    pointer ptr = resources.create_json(count);
                    nodes.emplace_back(generate_path(path, identifier_, flags), ptr);
                }
                //std::cout << "end identifier_selector\n";
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("identifier: ");
                s.append(identifier_);

                return s;
            }
        };

        class current_node final : public selector_base_type
        {
            using selector_base_type::generate_path;
        public:
            current_node()
            {
            }

            void select(dynamic_resources<Json>& /*resources*/,
                        const string_type& path, 
                        reference,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags) const override
            {
                nodes.emplace_back(path, std::addressof(val));
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("current_node");

                return s;
            }
        };

        class root_node final : public selector_base_type
        {
            using selector_base_type::generate_path;
        public:
            root_node()
            {
            }

            void select(dynamic_resources<Json>& /*resources*/,
                        const string_type& path, 
                        reference root,
                        reference,
                        std::vector<path_node_type>& nodes,
                        result_flags) const override
            {
                nodes.emplace_back(path, std::addressof(root));
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("root_node");

                return s;
            }
        };

        class index_selector final : public selector_base_type
        {
            int64_t index_;
            using selector_base_type::generate_path;
        public:
            index_selector(int64_t index)
                : index_(index)
            {
            }

            void select(dynamic_resources<Json>& /*resources*/,
                        const string_type& path, 
                        reference /* root */,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                if (val.is_array())
                {
                    int64_t slen = static_cast<int64_t>(val.size());
                    if (index_ >= 0 && index_ < slen)
                    {
                        std::size_t index = static_cast<std::size_t>(index_);
                        nodes.emplace_back(generate_path(path, "", flags),std::addressof(val.at(index)));
                    }
                    else if ((slen + index_) >= 0 && (slen+index_) < slen)
                    {
                        std::size_t index = static_cast<std::size_t>(slen + index_);
                        nodes.emplace_back(generate_path(path,"",flags),std::addressof(val.at(index)));
                    }
                }
            }
        };

        // projection_base
        class projection_base : public selector_base_type
        {
            using selector_base_type::generate_path;
            std::vector<std::unique_ptr<selector_base_type>> selectors_;
        public:
            projection_base(bool is_filter, std::size_t precedence_level)
                : selector_base_type(true, is_filter, precedence_level)
            {
            }

            void add_selector(std::unique_ptr<selector_base_type>&& expr) override
            {
                if (!selectors_.empty() && selectors_.back()->is_projection() && 
                    (expr->precedence_level() < selectors_.back()->precedence_level() ||
                     (expr->precedence_level() == selectors_.back()->precedence_level() && expr->is_right_associative())))
                {
                    selectors_.back()->add_selector(std::move(expr));
                }
                else
                {
                    selectors_.emplace_back(std::move(expr));
                }
            }

            void apply_expressions(dynamic_resources<Json>& resources,
                                   const string_type& path, 
                                   reference root,
                                   reference val,
                                   std::vector<path_node_type>& nodes,
                                   result_flags flags) const
            {
                if (selectors_.empty())
                {
                    nodes.emplace_back(path, std::addressof(val));
                }
                else
                {
                    std::vector<path_node_type> collect;
                    collect.emplace_back(path,std::addressof(val));
                    for (auto& selector : selectors_)
                    {
                        std::vector<path_node_type> temp;
                        for (auto& item : collect)
                            selector->select(resources, path, root, *(item.val_ptr), temp, flags);
                        collect = std::move(temp);
                    }
                    for (auto& item : collect)
                    {
                        nodes.emplace_back(std::move(item));
                    }
                }
            }

            virtual std::string to_string(int level = 0) const override
            {
                std::string s;
                for (auto& sel : selectors_)
                {
                    s.append(sel->to_string(level+1));
                }

                return s;
            }
        };

        class wildcard_selector final : public projection_base
        {
            using projection_base::generate_path;
        public:
            wildcard_selector()
                : projection_base(false, 11)
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                //std::cout << "wildcard_selector: " << val << "\n";
                if (val.is_array())
                {
                    for (auto& item : val.array_range())
                    {
                        this->apply_expressions(resources, path, root, item, nodes, flags);
                    }
                }
                else if (val.is_object())
                {
                    for (auto& item : val.object_range())
                    {
                        this->apply_expressions(resources, path, root, item.value(), nodes, flags);
                    }
                }
                //std::cout << "end wildcard_selector\n";
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("wildcard\n");
                s.append(projection_base::to_string(level));

                return s;
            }
        };

        class recursive_selector final : public projection_base
        {
            using projection_base::generate_path;
        public:
            recursive_selector()
                : projection_base(false, 11)
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                //std::cout << "wildcard_selector: " << val << "\n";
                if (val.is_array())
                {
                    this->apply_expressions(resources, path, root, val, nodes, flags);
                    for (auto& item : val.array_range())
                    {
                        select(resources, path, root, item, nodes, flags);
                    }
                }
                else if (val.is_object())
                {
                    this->apply_expressions(resources, path, root, val, nodes, flags);
                    for (auto& item : val.object_range())
                    {
                        select(resources, path, root, item.value(), nodes, flags);
                    }
                }
                //std::cout << "end wildcard_selector\n";
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("wildcard\n");
                s.append(projection_base::to_string(level));

                return s;
            }
        };

        class union_selector final : public projection_base
        {
            std::vector<path_expression_type> expressions_;

            using projection_base::generate_path;
        public:
            union_selector(std::vector<path_expression_type>&& expressions)
                : projection_base(false, 11), expressions_(std::move(expressions))
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val, 
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                //std::cout << "union select val: " << val << "\n";
                auto callback = [&](path_node_type& node)
                {
                    //std::cout << "union select callback: node: " << *node.val_ptr << "\n";
                    //nodes.push_back(node);
                    this->apply_expressions(resources, path, root, *node.val_ptr, nodes, flags);
                };
                for (auto& expr : expressions_)
                {
                    expr.evaluate(resources, root, val, callback, flags);
                }
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level * 2, ' ');
                }
                s.append("union selector\n");
                //s.append(expr_.to_string(level));

                return s;
            }
        };

        static bool is_false(const std::vector<path_node_type>& nodes)
        {
            if (nodes.size() != 1)
            {
                return nodes.empty();
            }
            auto valp = nodes.front().val_ptr; 
            return ((valp->is_array() && valp->empty()) ||
                     (valp->is_object() && valp->empty()) ||
                     (valp->is_string() && valp->as_string_view().empty()) ||
                     (valp->is_bool() && !valp->as_bool()) ||
                     valp->is_null());
        }

        static bool is_true(const std::vector<path_node_type>& nodes)
        {
            return !is_false(nodes);
        }

        class filter_selector final : public projection_base
        {
            path_expression_type expr_;

            using projection_base::generate_path;
        public:

            filter_selector(path_expression_type&& expr)
                : projection_base(true, 11), expr_(std::move(expr))
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val, 
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                if (val.is_array())
                {
                    for (std::size_t i = 0; i < val.size(); ++i)
                    {
                        std::vector<path_node_type> temp;
                        auto callback = [&temp](path_node_type& node)
                        {
                            temp.push_back(node);
                        };
                        expr_.evaluate(resources, root, val[i], callback, flags);
                        if (is_true(temp))
                        {
                            this->apply_expressions(resources, generate_path(path,i,flags), root, val[i], nodes, flags);
                        }
                    }
                }
                else if (val.is_object())
                {
                    for (auto& member : val.object_range())
                    {
                        std::vector<path_node_type> temp;
                        auto callback = [&temp](path_node_type& node)
                        {
                            temp.push_back(node);
                        };
                        expr_.evaluate(resources, root, member.value(), callback, flags);
                        if (is_true(temp))
                        {
                            this->apply_expressions(resources, generate_path(path,member.key(),flags), root, member.value(), nodes, flags);
                        }
                    }

                    /*std::vector<path_node_type> temp;
                    auto callback = [&temp](path_node_type& node)
                    {
                        temp.push_back(node);
                    };
                    expr_.evaluate(resources, root, val, callback);
                    if (is_true(temp))
                    {
                        this->apply_expressions(resources, path, root, val, nodes);
                    }
                    */
                }
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level * 2, ' ');
                }
                s.append("filter selector\n");
                //s.append(expr_.to_string(level));

                return s;
            }
        };

        class expression_selector final : public projection_base
        {
            using projection_base::generate_path;
            path_expression_type expr_;

        public:

            expression_selector(path_expression_type&& expr)
                : projection_base(true, 11), expr_(std::move(expr))
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val, 
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                std::vector<path_node_type> temp;
                auto callback = [&temp](path_node_type& node)
                {
                    temp.push_back(node);
                };
                expr_.evaluate(resources, root, val, callback, flags);
                if (!temp.empty())
                {
                    auto& j = *temp.back().val_ptr;
                    if (j.template is<std::size_t>() && val.is_array())
                    {
                        std::size_t start = j.template as<std::size_t>();
                        this->apply_expressions(resources, path, root, val.at(start), nodes, flags);
                    }
                    else if (j.is_string() && val.is_object())
                    {
                        this->apply_expressions(resources, path, root, val.at(j.as_string_view()), nodes, flags);
                    }
                }
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level * 2, ' ');
                }
                s.append("expression selector\n");
                //s.append(expr_.to_string(level));

                return s;
            }
        };

        class slice_selector final : public projection_base
        {
        private:
            using projection_base::generate_path;
            slice slice_;
        public:
            slice_selector(const slice& slic)
                : projection_base(false, 11), slice_(slic) 
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& path, 
                        reference root,
                        reference val,
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
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
                            //nodes.emplace_back(generate_path(path,j),std::addressof(val[j]));
                            this->apply_expressions(resources, path, root, val[j], nodes, flags);
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
                                this->apply_expressions(resources, generate_path(path,j,flags), root, val[j], nodes, flags);
                            }
                        }
                    }
                }
            }
        };

        class function_expression final : public selector_base_type
        {
        public:
            path_expression_type expr_;

            function_expression(path_expression_type&& expr)
                : expr_(std::move(expr))
            {
            }

            void select(dynamic_resources<Json>& resources,
                        const string_type& /*path*/, 
                        reference root,
                        reference val, 
                        std::vector<path_node_type>& nodes,
                        result_flags flags) const override
            {
                //std::cout << "function val: " << val << "\n";
                auto callback = [&nodes](path_node_type& node)
                {
                    nodes.push_back(node);
                };
                return expr_.evaluate(resources, root, val, callback, flags);
            }

            std::string to_string(int level = 0) const override
            {
                std::string s;
                if (level > 0)
                {
                    s.append("\n");
                    s.append(level*2, ' ');
                }
                s.append("function expression");

                return s;
            }
        };

        std::size_t line_;
        std::size_t column_;
        const char_type* begin_input_;
        const char_type* end_input_;
        const char_type* p_;

        using argument_type = std::vector<pointer>;
        std::vector<argument_type> function_stack_;
        std::vector<path_state> state_stack_;
        std::vector<token_type> output_stack_;
        std::vector<token_type> operator_stack_;

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
/*
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
*/
        path_expression_type compile(static_resources<value_type,reference>& resources, const string_view_type& path)
        {
            std::error_code ec;
            auto result = compile(resources, path.data(), path.length(), ec);
            if (ec)
            {
                JSONCONS_THROW(jsonpath_error(ec, line_, column_));
            }
            return std::move(result);
        }

        path_expression_type compile(static_resources<value_type,reference>& resources, const string_view_type& path, std::error_code& ec)
        {
            JSONCONS_TRY
            {
                return compile(resources, path.data(), path.length(), ec);
            }
            JSONCONS_CATCH(...)
            {
                ec = jsonpath_errc::unidentified_error;
            }
        }

        path_expression_type compile(static_resources<value_type,reference>& resources,
                                     const char_type* path, 
                                     std::size_t length)
        {
            std::error_code ec;
            auto result = compile(resources, path, length, ec);
            if (ec)
            {
                JSONCONS_THROW(jsonpath_error(ec, line_, column_));
            }
            return std::move(result);
        }

        path_expression_type compile(static_resources<value_type,reference>& resources,
                                     const char_type* path, 
                                     std::size_t length,
                                     std::error_code& ec)
        {

            state_stack_.emplace_back(path_state::start);

            string_type function_name;
            string_type buffer;
            uint32_t cp = 0;
            uint32_t cp2 = 0;

            begin_input_ = path;
            end_input_ = path + length;
            p_ = begin_input_;

            string_type s = {'$'};

            slice slic;
            int paren_level = 0;

            while (p_ < end_input_)
            {
                switch (state_stack_.back())
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
                                push_token(root_node_arg, ec);
                                state_stack_.emplace_back(path_state::rhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                state_stack_.back() = path_state::rhs_expression;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                            }
                        }
                        break;
                    }
                    case path_state::recursive_descent_or_lhs_expression:
                        switch (*p_)
                        {
                            case '.':
                                push_token(token_type(jsoncons::make_unique<recursive_selector>()), ec);
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::name_or_left_bracket;
                                break;
                            default:
                                state_stack_.back() = path_state::lhs_expression;
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
                                state_stack_.back() = path_state::bracket_specifier_or_union;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.clear();
                                state_stack_.back() = path_state::lhs_expression;
                                break;
                        }
                        break;
                    case path_state::literal:
                    {
                        //std::cout << "literal: " << buffer << "\n";
                        push_token(token_type(literal_arg, Json(buffer)), ec);
                        buffer.clear();
                        state_stack_.pop_back(); // json_value
                        ++p_;
                        ++column_;
                        break;
                    }
                    case path_state::path_or_literal_or_function: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '$':
                                push_token(root_node_arg, ec);
                                push_token(token_type(jsoncons::make_unique<root_node>()), ec);
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '@':
                                push_token(current_node_arg, ec);
                                push_token(token_type(jsoncons::make_unique<current_node>()), ec);
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                ++paren_level;
                                push_token(lparen_arg, ec);
                                break;
                            }
                            case '\'':
                                state_stack_.back() = path_state::literal;
                                state_stack_.emplace_back(path_state::single_quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\"':
                                state_stack_.back() = path_state::literal;
                                state_stack_.emplace_back(path_state::double_quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                push_token(token_type(resources.get_unary_not()), ec);
                                break;
                            }
                            case '-':
                            {
                                ++p_;
                                ++column_;
                                push_token(token_type(resources.get_unary_minus()), ec);
                                break;
                            }
                            default:
                            {
                                state_stack_.back() = path_state::json_text_or_function_expr;
                                state_stack_.emplace_back(path_state::unquoted_string);
                                break;
                            }
                        }
                        break;
                    }
                    case path_state::json_text_or_function_expr:
                    {
                        switch(*p_)
                        {
                            case '(':
                            {
                                auto f = resources.get_function(buffer, ec);
                                if (ec)
                                {
                                    return path_expression_type();
                                }
                                buffer.clear();
                                ++paren_level;
                                push_token(current_node_arg, ec);
                                push_token(token_type(begin_function_arg), ec);
                                push_token(token_type(f), ec);
                                state_stack_.back() = path_state::function_expression;
                                state_stack_.emplace_back(path_state::argument);
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                json_decoder<Json> decoder;
                                basic_json_parser<char_type> parser;
                                parser.update(buffer.data(),buffer.size());
                                parser.parse_some(decoder, ec);
                                if (ec)
                                {
                                    return path_expression_type();
                                }
                                parser.finish_parse(decoder, ec);
                                if (ec)
                                {
                                    return path_expression_type();
                                }
                                push_token(token_type(literal_arg, decoder.get_result()), ec);
                                buffer.clear();
                                state_stack_.pop_back();
                                break;
                            }
                        }
                        break;
                    }
                    case path_state::lhs_expression: 
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '*':
                                push_token(token_type(jsoncons::make_unique<wildcard_selector>()), ec);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::identifier;
                                state_stack_.emplace_back(path_state::single_quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\"':
                                state_stack_.back() = path_state::identifier;
                                state_stack_.emplace_back(path_state::double_quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.emplace_back(path_state::bracket_specifier_or_union);
                                ++p_;
                                ++column_;
                                break;
                            case '$':
                                ++p_;
                                ++column_;
                                push_token(token_type(root_node_arg), ec);
                                push_token(token_type(jsoncons::make_unique<root_node>()), ec);
                                state_stack_.pop_back();
                                break;
                            case '@':
                                ++p_;
                                ++column_;
                                push_token(token_type(current_node_arg), ec);
                                push_token(token_type(jsoncons::make_unique<current_node>()), ec);
                                state_stack_.pop_back();
                                break;
                            case '.':
                                ec = jsonpath_errc::expected_key;
                                return path_expression_type();
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                ++paren_level;
                                push_token(lparen_arg, ec);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                push_token(token_type(resources.get_unary_not()), ec);
                                break;
                            }
                            case '?':
                            {
                                push_token(token_type(begin_filter_arg), ec);
                                state_stack_.back() = path_state::filter;
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                ++p_;
                                ++column_;
                                break;
                            }
                            // integer
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = path_state::index;
                                state_stack_.emplace_back(path_state::integer);
                                break;
                            default:
                                buffer.clear();
                                state_stack_.back() = path_state::identifier_or_function_expr;
                                state_stack_.emplace_back(path_state::unquoted_string);
                                break;
                        }
                        break;
                    case path_state::identifier_or_function_expr:
                    {
                        switch(*p_)
                        {
                            case '(':
                            {
                                auto f = resources.get_function(buffer, ec);
                                if (ec)
                                {
                                    return path_expression_type();
                                }
                                buffer.clear();
                                ++paren_level;
                                push_token(current_node_arg, ec);
                                push_token(token_type(begin_function_arg), ec);
                                push_token(token_type(f), ec);
                                state_stack_.back() = path_state::function_expression;
                                state_stack_.emplace_back(path_state::argument);
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            }
                        }
                        break;
                    }
                    case path_state::function_expression:
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                push_token(token_type(current_node_arg), ec);
                                state_stack_.emplace_back(path_state::argument);
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                ++p_;
                                ++column_;
                                break;
                            case ')':
                            {
                                --paren_level;
                                push_token(token_type(end_function_arg), ec);
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    case path_state::argument:
                    {
                        push_token(argument_arg, ec);
                        state_stack_.pop_back();
                        break;
                    }
                    case path_state::unquoted_string: 
                        switch (*p_)
                        {
                            case '(':
                            case ')':
                            case ']':
                            case '[':
                            case '.':
                            case ',':
                            case ' ':case '\t':
                            case '\r':
                            case '\n':
                            case '!':
                            case '=':
                            case '<':
                            case '>':
                            case '~':
                            case '|':
                            case '&':
                            case '+':
                            case '-':
                            case '*':
                            case '/':
                            case '@':
                            case '$':
                            case '?':
                                state_stack_.pop_back(); // unquoted_string
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case path_state::rhs_expression: 
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '.':
                                state_stack_.emplace_back(path_state::recursive_descent_or_lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.emplace_back(path_state::bracket_specifier_or_union);
                                ++p_;
                                ++column_;
                                break;
                            case ')':
                            {
                                if (state_stack_.size() > 1 && (*(state_stack_.rbegin()+1) == path_state::argument))
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ++p_;
                                    ++column_;
                                    --paren_level;
                                    push_token(rparen_arg, ec);
                                }
                                break;
                            }
                            case '|':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                state_stack_.emplace_back(path_state::expect_or);
                                break;
                            case '&':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                state_stack_.emplace_back(path_state::expect_and);
                                break;
                            case '<':
                            case '>':
                            {
                                state_stack_.emplace_back(path_state::comparator_expression);
                                break;
                            }
                            case '=':
                            {
                                state_stack_.emplace_back(path_state::eq_or_regex);
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                state_stack_.emplace_back(path_state::cmp_ne);
                                break;
                            }
                            case '+':
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                push_token(token_type(resources.get_plus_operator()), ec);
                                ++p_;
                                ++column_;
                                break;
                            case '-':
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                push_token(token_type(resources.get_minus_operator()), ec);
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                push_token(token_type(resources.get_mult_operator()), ec);
                                ++p_;
                                ++column_;
                                break;
                            case '/':
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                push_token(token_type(resources.get_div_operator()), ec);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jsonpath_errc::expected_separator;
                                    return path_expression_type();
                                }
                        };
                        break;
                    case path_state::expect_or:
                    {
                        switch (*p_)
                        {
                            case '|':
                                push_token(token_type(resources.get_or_operator()), ec);
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_or;
                                return path_expression_type();
                        }
                        break;
                    }
                    case path_state::expect_and:
                    {
                        switch(*p_)
                        {
                            case '&':
                                push_token(token_type(resources.get_and_operator()), ec);
                                state_stack_.pop_back(); // expect_and
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_and;
                                return path_expression_type();
                        }
                        break;
                    }
                    case path_state::comparator_expression:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '<':
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::path_or_literal_or_function;
                                state_stack_.emplace_back(path_state::cmp_lt_or_lte);
                                break;
                            case '>':
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::path_or_literal_or_function;
                                state_stack_.emplace_back(path_state::cmp_gt_or_gte);
                                break;
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jsonpath_errc::syntax_error;
                                    return path_expression_type();
                                }
                                break;
                        }
                        break;
                    case path_state::eq_or_regex:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '=':
                            {
                                push_token(token_type(resources.get_eq_operator()), ec);
                                state_stack_.back() = path_state::path_or_literal_or_function;
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '~':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expect_regex);
                                break;
                            }
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jsonpath_errc::syntax_error;
                                    return path_expression_type();
                                }
                                break;
                        }
                        break;
                    case path_state::expect_regex: 
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '/':
                                state_stack_.back() = path_state::regex;
                                ++p_;
                                ++column_;
                                break;
                            default: 
                                ec = jsonpath_errc::expected_forward_slash;
                                return path_expression_type();
                        };
                        break;
                    case path_state::regex: 
                    {
                        switch (*p_)
                        {                   
                            case '/':
                                {
                                    std::regex::flag_type flags = std::regex_constants::ECMAScript; 
                                    if (p_+1  < end_input_ && *(p_+1) == 'i')
                                    {
                                        ++p_;
                                        ++column_;
                                        flags |= std::regex_constants::icase;
                                    }
                                    std::basic_regex<char_type> pattern(buffer, flags);
                                    push_token(resources.get_regex_operator(std::move(pattern)), ec);
                                    buffer.clear();
                                }
                                state_stack_.pop_back();
                                break;

                            default: 
                                buffer.push_back(*p_);
                                break;
                        }
                        ++p_;
                        ++column_;
                        break;
                    }
                    case path_state::cmp_lt_or_lte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token_type(resources.get_lte_operator()), ec);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token_type(resources.get_lt_operator()), ec);
                                state_stack_.pop_back();
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_gt_or_gte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token_type(resources.get_gte_operator()), ec);
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token_type(resources.get_gt_operator()), ec);
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_ne:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token_type(resources.get_ne_operator()), ec);
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_comparator;
                                return path_expression_type();
                        }
                        break;
                    }
                    case path_state::identifier:
                        switch (*p_)
                        {
                            case '\'':
                            case '\"':
                                ++p_;
                                ++column_;
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case path_state::single_quoted_string:
                        switch (*p_)
                        {
                            case '\'':
                                state_stack_.pop_back();
                                break;
                            case '\\':
                                state_stack_.emplace_back(path_state::quoted_string_escape_char);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case path_state::double_quoted_string: 
                        switch (*p_)
                        {
                            case '\"':
                                state_stack_.pop_back();
                                break;
                            case '\\':
                                state_stack_.emplace_back(path_state::quoted_string_escape_char);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case path_state::comma_or_right_bracket:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                state_stack_.back() = path_state::bracket_specifier_or_union;
                                ++p_;
                                ++column_;
                                break;
                            case ']':
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::expect_right_bracket:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']':
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::bracket_specifier_or_union:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '(':
                            {
                                push_token(token_type(begin_expression_arg), ec);
                                state_stack_.back() = path_state::expression;
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                ++paren_level;
                                push_token(lparen_arg, ec);
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '?':
                            {
                                push_token(token_type(begin_union_arg), ec);
                                state_stack_.back() = path_state::union_expression; // union
                                push_token(token_type(begin_filter_arg), ec);
                                state_stack_.emplace_back(path_state::filter);
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::path_or_literal_or_function);
                                ++p_;
                                ++column_;
                                break;
                            }
                            case ':': // slice_expression
                                state_stack_.back() = path_state::rhs_slice_expression_start ;
                                state_stack_.emplace_back(path_state::integer);
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                state_stack_.back() = path_state::wildcard_or_union;
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::single_quoted_name_or_union;
                                ++p_;
                                ++column_;
                                break;
                            case '\"':
                                state_stack_.back() = path_state::double_quoted_name_or_union;
                                ++p_;
                                ++column_;
                                break;
                            // integer
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = path_state::index_or_slice_or_union;
                                state_stack_.emplace_back(path_state::integer);
                                break;
                            default:
                                buffer.clear();
                                buffer.push_back(*p_);
                                state_stack_.back() = path_state::bracketed_unquoted_name_or_union;
                                ++p_;
                                ++column_;
                                break;
                        }
                        break;

                    case path_state::integer:
                        switch(*p_)
                        {
                            case '-':
                                buffer.push_back(*p_);
                                state_stack_.back() = path_state::digit;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::digit;
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
                    case path_state::index_or_slice_or_union:
                        switch(*p_)
                        {
                            case ']':
                            {
                                if (buffer.empty())
                                {
                                    ec = jsonpath_errc::invalid_number;
                                    return path_expression_type();
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jsonpath_errc::invalid_number;
                                        return path_expression_type();
                                    }
                                    push_token(token_type(jsoncons::make_unique<index_selector>(r.value())), ec);

                                    buffer.clear();
                                }
                                state_stack_.pop_back(); // bracket_specifier_or_union
                                ++p_;
                                ++column_;
                                break;
                            }
                            case ',':
                            {
                                push_token(token_type(begin_union_arg), ec);
                                if (buffer.empty())
                                {
                                    ec = jsonpath_errc::invalid_number;
                                    return path_expression_type();
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jsonpath_errc::invalid_number;
                                        return path_expression_type();
                                    }
                                    push_token(token_type(jsoncons::make_unique<index_selector>(r.value())), ec);

                                    buffer.clear();
                                }
                                push_token(token_type(separator_arg), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
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
                                        return path_expression_type();
                                    }
                                    slic.start_ = r.value();
                                    buffer.clear();
                                }
                                state_stack_.back() = path_state::rhs_slice_expression_start;
                                state_stack_.emplace_back(path_state::integer);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::index:
                        switch(*p_)
                        {
                            case ']':
                            case '.':
                            case ',':
                            {
                                if (buffer.empty())
                                {
                                    ec = jsonpath_errc::invalid_number;
                                    return path_expression_type();
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jsonpath_errc::invalid_number;
                                        return path_expression_type();
                                    }
                                    push_token(token_type(jsoncons::make_unique<index_selector>(r.value())), ec);

                                    buffer.clear();
                                }
                                state_stack_.pop_back(); // index
                                break;
                            }
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
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
                                return path_expression_type();
                            }
                            slic.stop_ = jsoncons::optional<int64_t>(r.value());
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token_type(jsoncons::make_unique<slice_selector>(slic)), ec);
                                slic = slice{};
                                state_stack_.pop_back(); // bracket_specifier2
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = path_state::rhs_slice_expression_stop;
                                state_stack_.emplace_back(path_state::integer);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
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
                                return path_expression_type();
                            }
                            if (r.value() == 0)
                            {
                                ec = jsonpath_errc::step_cannot_be_zero;
                                return path_expression_type();
                            }
                            slic.step_ = r.value();
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token_type(jsoncons::make_unique<slice_selector>(slic)), ec);
                                buffer.clear();
                                slic = slice{};
                                state_stack_.pop_back(); // rhs_slice_expression_stop
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    }

                    case path_state::bracketed_unquoted_name_or_union:
                        switch (*p_)
                        {
                            case ']': 
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '.':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case ',': 
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                push_token(token_type(separator_arg), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        }
                        break;
                    case path_state::union_expression:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '.':
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.emplace_back(path_state::bracket_specifier_or_union);
                                ++p_;
                                ++column_;
                                break;
                            case ',': 
                                push_token(token_type(separator_arg), ec);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            case ']': 
                                push_token(token_type(end_union_arg), ec);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::identifier_or_union:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']': 
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '.':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case ',': 
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                                push_token(token_type(separator_arg), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::wildcard_or_union:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']': 
                                push_token(token_type(jsoncons::make_unique<wildcard_selector>()), ec);
                                buffer.clear();
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '.':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<wildcard_selector>()), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<wildcard_selector>()), ec);
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            case ',': 
                                push_token(token_type(begin_union_arg), ec);
                                push_token(token_type(jsoncons::make_unique<wildcard_selector>()), ec);
                                push_token(token_type(separator_arg), ec);
                                buffer.clear();
                                state_stack_.back() = path_state::union_expression; // union
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    case path_state::single_quoted_name_or_union:
                        switch (*p_)
                        {
                            case '\'':
                                state_stack_.back() = path_state::identifier_or_union;
                                break;
                            case '\\':
                                state_stack_.emplace_back(path_state::quoted_string_escape_char);
                                break;
                            default:
                                buffer.push_back(*p_);
                                break;
                        };
                        ++p_;
                        ++column_;
                        break;
                    case path_state::double_quoted_name_or_union: 
                        switch (*p_)
                        {
                            case '\"':
                                state_stack_.back() = path_state::identifier_or_union;
                                break;
                            case '\\':
                                state_stack_.emplace_back(path_state::quoted_string_escape_char);
                                break;
                            default:
                                buffer.push_back(*p_);
                                break;
                        };
                        ++p_;
                        ++column_;
                        break;
                    case path_state::quoted_string_escape_char:
                        switch (*p_)
                        {
                            case '\"':
                                buffer.push_back('\"');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '\'':
                                buffer.push_back('\'');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '\\': 
                                buffer.push_back('\\');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '/':
                                buffer.push_back('/');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'b':
                                buffer.push_back('\b');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'f':
                                buffer.push_back('\f');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'n':
                                buffer.push_back('\n');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'r':
                                buffer.push_back('\r');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 't':
                                buffer.push_back('\t');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'u':
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::escape_u1;
                                break;
                            default:
                                ec = jsonpath_errc::illegal_escaped_character;
                                return path_expression_type();
                        }
                        break;
                    case path_state::escape_u1:
                        cp = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u2;
                        break;
                    case path_state::escape_u2:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u3;
                        break;
                    case path_state::escape_u3:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u4;
                        break;
                    case path_state::escape_u4:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        if (unicons::is_high_surrogate(cp))
                        {
                            ++p_;
                            ++column_;
                            state_stack_.back() = path_state::escape_expect_surrogate_pair1;
                        }
                        else
                        {
                            unicons::convert(&cp, &cp + 1, std::back_inserter(buffer));
                            ++p_;
                            ++column_;
                            state_stack_.pop_back();
                        }
                        break;
                    case path_state::escape_expect_surrogate_pair1:
                        switch (*p_)
                        {
                            case '\\': 
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::escape_expect_surrogate_pair2;
                                break;
                            default:
                                ec = jsonpath_errc::invalid_codepoint;
                                return path_expression_type();
                        }
                        break;
                    case path_state::escape_expect_surrogate_pair2:
                        switch (*p_)
                        {
                            case 'u': 
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::escape_u5;
                                break;
                            default:
                                ec = jsonpath_errc::invalid_codepoint;
                                return path_expression_type();
                        }
                        break;
                    case path_state::escape_u5:
                        cp2 = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u6;
                        break;
                    case path_state::escape_u6:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u7;
                        break;
                    case path_state::escape_u7:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u8;
                        break;
                    case path_state::escape_u8:
                    {
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return path_expression_type();
                        }
                        uint32_t codepoint = 0x10000 + ((cp & 0x3FF) << 10) + (cp2 & 0x3FF);
                        unicons::convert(&codepoint, &codepoint + 1, std::back_inserter(buffer));
                        state_stack_.pop_back();
                        ++p_;
                        ++column_;
                        break;
                    }
                    case path_state::filter:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                            case ']':
                            {
                                push_token(token_type(end_filter_arg), ec);
                                state_stack_.pop_back();
                                break;
                            }
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    }
                    case path_state::expression:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']':
                            {
                                push_token(token_type(end_expression_arg), ec);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jsonpath_errc::expected_right_bracket;
                                return path_expression_type();
                        }
                        break;
                    }
                    default:
                        ++p_;
                        ++column_;
                        break;
                }
            }

            if (state_stack_.size() >= 3 && state_stack_.back() == path_state::unquoted_string)
            {
                push_token(token_type(jsoncons::make_unique<identifier_selector>(buffer)), ec);
                state_stack_.pop_back(); // unquoted_string
                if (/*state_stack_.back() == path_state::identifier ||*/ state_stack_.back() == path_state::identifier_or_function_expr)
                {
                    buffer.clear();
                    state_stack_.pop_back(); // identifier
                }
            }

            if (state_stack_.size() > 2)
            {
                ec = jsonpath_errc::unexpected_end_of_input;
                return path_expression_type();
            }

            return path_expression_type(std::move(output_stack_));
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

        void unwind_rparen(std::error_code& ec)
        {
            auto it = operator_stack_.rbegin();
            while (it != operator_stack_.rend() && !it->is_lparen())
            {
                output_stack_.emplace_back(std::move(*it));
                ++it;
            }
            if (it == operator_stack_.rend())
            {
                ec = jsonpath_errc::unbalanced_parenthesis;
                return;
            }
            ++it;
            operator_stack_.erase(it.base(),operator_stack_.end());
        }

        void push_token(token_type&& tok, std::error_code& ec)
        {
            switch (tok.type())
            {
                case token_kind::begin_filter:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token_type(lparen_arg));
                    break;
                case token_kind::end_filter:
                {
                    unwind_rparen(ec);
                    if (ec)
                    {
                        return;
                    }
                    std::vector<token_type> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_kind::begin_filter)
                    {
                        toks.insert(toks.begin(), std::move(*it));
                        ++it;
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().selector_->add_selector(jsoncons::make_unique<filter_selector>(path_expression_type(std::move(toks))));
                    }
                    else
                    {
                        output_stack_.emplace_back(token_type(jsoncons::make_unique<filter_selector>(path_expression_type(std::move(toks)))));
                    }
                    break;
                }
                case token_kind::begin_expression:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token_type(lparen_arg));
                    break;
                case token_kind::end_expression:
                {
                    unwind_rparen(ec);
                    if (ec)
                    {
                        return;
                    }
                    std::vector<token_type> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_kind::begin_expression)
                    {
                        toks.insert(toks.begin(), std::move(*it));
                        ++it;
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().selector_->add_selector(jsoncons::make_unique<expression_selector>(path_expression_type(std::move(toks))));
                    }
                    else
                    {
                        output_stack_.emplace_back(token_type(jsoncons::make_unique<expression_selector>(path_expression_type(std::move(toks)))));
                    }
                    break;
                }
                case token_kind::selector:
                {
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().selector_->add_selector(std::move(tok.selector_));
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                }
                case token_kind::separator:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_kind::begin_union:
                    output_stack_.emplace_back(std::move(tok));
                    break;

                case token_kind::end_union:
                {
                    //unwind_rparen(ec);
                    //if (ec)
                    //{
                    //    return;
                    //}
                    std::vector<path_expression_type> expressions;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_kind::begin_union)
                    {
                        std::vector<token_type> toks;
                        do
                        {
                            toks.insert(toks.begin(), std::move(*it));
                            ++it;
                        } while (it != output_stack_.rend() && it->type() != token_kind::begin_union && it->type() != token_kind::separator);
                        if (it->type() == token_kind::separator)
                        {
                            ++it;
                        }
                        if (toks.front().type() != token_kind::literal)
                        {
                            toks.emplace(toks.begin(), current_node_arg);
                        }
                        expressions.emplace(expressions.begin(), path_expression_type(std::move(toks)));
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().selector_->add_selector(jsoncons::make_unique<union_selector>(std::move(expressions)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token_type(jsoncons::make_unique<union_selector>(std::move(expressions))));
                    }
                    break;
                }
                case token_kind::lparen:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
                case token_kind::rparen:
                {
                    unwind_rparen(ec);
                    break;
                }
                case token_kind::end_function:
                {
                    unwind_rparen(ec);
                    if (ec)
                    {
                        return;
                    }
                    std::vector<token_type> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_kind::begin_function)
                    {
                        toks.insert(toks.begin(), std::move(*it));
                        ++it;
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().selector_->add_selector(jsoncons::make_unique<function_expression>(path_expression_type(std::move(toks))));
                    }
                    else
                    {
                        output_stack_.emplace_back(token_type(jsoncons::make_unique<function_expression>(std::move(toks))));
                    }
                    break;
                }
                case token_kind::literal:
                    if (!output_stack_.empty() && (output_stack_.back().type() == token_kind::current_node || output_stack_.back().type() == token_kind::root_node))
                    {
                        output_stack_.back() = std::move(tok);
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                case token_kind::begin_function:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token_type(lparen_arg));
                    break;
                case token_kind::argument:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_kind::function:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
                case token_kind::root_node:
                case token_kind::current_node:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_kind::unary_operator:
                case token_kind::binary_operator:
                {
                    if (operator_stack_.empty() || operator_stack_.back().is_lparen())
                    {
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    else if (tok.precedence_level() < operator_stack_.back().precedence_level()
                             || (tok.precedence_level() == operator_stack_.back().precedence_level() && tok.is_right_associative()))
                    {
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    else
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend() && it->is_operator()
                               && (tok.precedence_level() > it->precedence_level()
                             || (tok.precedence_level() == it->precedence_level() && tok.is_right_associative())))
                        {
                            output_stack_.emplace_back(std::move(*it));
                            ++it;
                        }

                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    break;
                }
                default:
                    break;
            }
        }

        uint32_t append_to_codepoint(uint32_t cp, int c, std::error_code& ec)
        {
            cp *= 16;
            if (c >= '0'  &&  c <= '9')
            {
                cp += c - '0';
            }
            else if (c >= 'a'  &&  c <= 'f')
            {
                cp += c - 'a' + 10;
            }
            else if (c >= 'A'  &&  c <= 'F')
            {
                cp += c - 'A' + 10;
            }
            else
            {
                ec = jsonpath_errc::invalid_codepoint;
            }
            return cp;
        }
    };

    } // namespace detail

    template <class Json>
    class jsonpath_expression
    {
    public:
        using evaluator_t = typename jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json, const Json&>;
        using string_view_type = typename evaluator_t::string_view_type;
        using value_type = typename evaluator_t::value_type;
        using reference = typename evaluator_t::reference;
        using expression_t = typename evaluator_t::path_expression_type;
        using path_node_type = typename evaluator_t::path_node_type;
    private:
        jsoncons::jsonpath_new::detail::static_resources<value_type,reference> static_resources_;
        expression_t expr_;
    public:
        jsonpath_expression() = default;

        jsonpath_expression(jsoncons::jsonpath_new::detail::static_resources<value_type,reference>&& resources,
                            expression_t&& expr)
            : static_resources_(std::move(resources)), 
              expr_(std::move(expr))
        {
        }

        Json evaluate(reference root, result_flags flags = result_flags::value)
        {
            if ((flags & result_flags::value) == result_flags::value)
            {
                jsoncons::jsonpath_new::detail::dynamic_resources<Json> resources;
                return expr_.evaluate(resources, root, root, flags);
            }
            else if ((flags & result_flags::path) == result_flags::path)
            {
                jsoncons::jsonpath_new::detail::dynamic_resources<Json> resources;

                Json result(json_array_arg);
                auto callback = [&result](path_node_type& node)
                {
                    result.emplace_back(node.path);
                };
                expr_.evaluate(resources, root, root, callback, flags);
                return result;
            }
            else
            {
                return Json(json_array_arg);
            }
        }

        static jsonpath_expression compile(const string_view_type& path)
        {
            jsoncons::jsonpath_new::detail::static_resources<value_type,reference> resources;

            evaluator_t e;
            expression_t expr = e.compile(resources, path.data(), path.length());
            return jsonpath_expression(std::move(resources), std::move(expr));
        }

        static jsonpath_expression compile(const string_view_type& path,
                                           std::error_code& ec)
        {
            jsoncons::jsonpath_new::detail::static_resources<value_type,reference> resources;
            evaluator_t e;
            expression_t expr = e.compile(resources, path, ec);
            return jsonpath_expression(std::move(resources), std::move(expr));
        }
    };

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

    template<class Json>
    Json json_query(const Json& root, 
                    const typename Json::string_view_type& path, 
                    result_flags flags = result_flags::value)
    {
        auto expression = make_expression<Json>(path);
        return expression.evaluate(root, flags);
    }

/*
    template<class Json, class T>
    typename std::enable_if<!jsoncons::detail::is_function_object<T,Json>::value,void>::type
    json_replace(Json& root, const typename Json::string_view_type& path, T&& new_value)
    {
        jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,Json&> evaluator;
        jsoncons::jsonpath_new::detail::static_resources<value_type,reference> resources;
        evaluator.evaluate(resources, root, path);
        evaluator.replace(std::forward<T>(new_value));
    }

    template<class Json, class Op>
    typename std::enable_if<jsoncons::detail::is_function_object<Op,Json>::value,void>::type
    json_replace(Json& root, const typename Json::string_view_type& path, Op op)
    {
        jsoncons::jsonpath_new::detail::jsonpath_evaluator<Json,Json&> evaluator;
        jsoncons::jsonpath_new::detail::static_resources<value_type,reference> resources;
        evaluator.evaluate(resources, root, path);
        evaluator.replace_fn(op);
    }

*/

} // namespace jsonpath_new
} // namespace jsoncons

#endif
