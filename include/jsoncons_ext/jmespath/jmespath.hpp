// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JMESPATH_JMESPATH_HPP
#define JSONCONS_JMESPATH_JMESPATH_HPP

#include <string>
#include <vector>
#include <unordered_map> // std::unordered_map
#include <memory>
#include <type_traits> // std::is_const
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <functional> // 
#include <algorithm> // std::stable_sort
#include <cmath> // std::abs
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath_error.hpp>

namespace jsoncons { 
namespace jmespath {

    enum class token_type 
    {
        current_node,
        lparen,
        rparen,
        begin_multi_select_hash,
        end_multi_select_hash,
        begin_multi_select_list,
        end_multi_select_list,
        begin_filter,
        end_filter,
        pipe,
        separator,
        key,
        literal,
        expression,
        binary_operator,
        unary_operator,
        function,
        begin_function,
        end_function,
        argument,
        begin_expression_type,
        end_expression_type,
        end_of_expression
    };

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

    struct begin_expression_type_arg_t
    {
        explicit begin_expression_type_arg_t() = default;
    };
    constexpr begin_expression_type_arg_t begin_expression_type_arg{};

    struct end_expression_type_arg_t
    {
        explicit end_expression_type_arg_t() = default;
    };
    constexpr end_expression_type_arg_t end_expression_type_arg{};

    struct end_of_expression_arg_t
    {
        explicit end_of_expression_arg_t() = default;
    };
    constexpr end_of_expression_arg_t end_of_expression_arg{};

    struct separator_arg_t
    {
        explicit separator_arg_t() = default;
    };
    constexpr separator_arg_t separator_arg{};

    struct key_arg_t
    {
        explicit key_arg_t() = default;
    };
    constexpr key_arg_t key_arg{};

    struct lparen_arg_t
    {
        explicit lparen_arg_t() = default;
    };
    constexpr lparen_arg_t lparen_arg{};

    struct rparen_arg_t
    {
        explicit rparen_arg_t() = default;
    };
    constexpr rparen_arg_t rparen_arg{};

    struct begin_multi_select_hash_arg_t
    {
        explicit begin_multi_select_hash_arg_t() = default;
    };
    constexpr begin_multi_select_hash_arg_t begin_multi_select_hash_arg{};

    struct end_multi_select_hash_arg_t
    {
        explicit end_multi_select_hash_arg_t() = default;
    };
    constexpr end_multi_select_hash_arg_t end_multi_select_hash_arg{};

    struct begin_multi_select_list_arg_t
    {
        explicit begin_multi_select_list_arg_t() = default;
    };
    constexpr begin_multi_select_list_arg_t begin_multi_select_list_arg{};

    struct end_multi_select_list_arg_t
    {
        explicit end_multi_select_list_arg_t() = default;
    };
    constexpr end_multi_select_list_arg_t end_multi_select_list_arg{};

    struct begin_filter_arg_t
    {
        explicit begin_filter_arg_t() = default;
    };
    constexpr begin_filter_arg_t begin_filter_arg{};

    struct end_filter_arg_t
    {
        explicit end_filter_arg_t() = default;
    };
    constexpr end_filter_arg_t end_filter_arg{};

    struct pipe_arg_t
    {
        explicit pipe_arg_t() = default;
    };
    constexpr pipe_arg_t pipe_arg{};

    struct current_node_arg_t
    {
        explicit current_node_arg_t() = default;
    };
    constexpr current_node_arg_t current_node_arg{};

    struct end_function_arg_t
    {
        explicit end_function_arg_t() = default;
    };
    constexpr end_function_arg_t end_function_arg{};

    struct begin_function_arg_t
    {
        explicit begin_function_arg_t() = default;
    };
    constexpr begin_function_arg_t begin_function_arg{};

    struct argument_arg_t
    {
        explicit argument_arg_t() = default;
    };
    constexpr argument_arg_t argument_arg{};

    JSONCONS_STRING_LITERAL(sort_by,'s','o','r','t','-','b','y')

    struct slice
    {
        optional<int64_t> start_;
        optional<int64_t> stop_;
        int64_t step_;

        slice()
            : start_(), stop_(), step_(1)
        {
        }

        slice(const optional<int64_t>& start, const optional<int64_t>& end, int64_t step) 
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
        lhs_expression,
        rhs_expression,
        sub_expression,
        expression_type,
        comparator_expression,
        function_expression,
        argument,
        expression_or_expression_type,
        quoted_string,
        raw_string,
        raw_string_escape_char,
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
        literal,
        key_expr,
        val_expr,
        identifier_or_function_expr,
        unquoted_string,
        key_val_expr,
        number,
        digit,
        right_bracket_or_slice_expression,
        bracket_specifier,
        bracket_specifier_or_multi_select_list,
        filter,
        multi_select_list,
        multi_select_hash,
        rhs_slice_expression_start,
        rhs_slice_expression_stop,
        expect_right_bracket,
        expect_dot,
        expect_filter_right_bracket,
        expect_right_brace,
        expect_colon,
        expect_multi_select_list,
        cmp_lt_or_lte,
        cmp_eq,
        cmp_gt_or_gte,
        cmp_ne,
        cmp_lt_or_lte_old,
        cmp_eq_old,
        cmp_gt_or_gte_old,
        cmp_ne_old,
        expect_pipe_or_or,
        expect_and
    };

    template<class Json,
             class JsonReference>
    class jmespath_evaluator 
    {
    public:
        typedef typename Json::char_type char_type;
        typedef typename Json::char_traits_type char_traits_type;
        typedef std::basic_string<char_type,char_traits_type> string_type;
        typedef typename Json::string_view_type string_view_type;
        typedef JsonReference reference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        typedef typename Json::const_pointer const_pointer;

        // eval_context

        class eval_context
        {
            std::vector<std::unique_ptr<Json>> temp_storage_;

        public:
            reference number_type_name() 
            {
                static Json number_type_name(string_type({'n','u','m','b','e','r'}));

                return number_type_name;
            }

            reference boolean_type_name()
            {
                static Json boolean_type_name(string_type({'b','o','o','l','e','a','n'}));

                return boolean_type_name;
            }

            reference string_type_name()
            {
                static Json string_type_name(string_type({'s','t','r','i','n','g'}));

                return string_type_name;
            }

            reference object_type_name()
            {
                static Json object_type_name(string_type({'o','b','j','e','c','t'}));

                return object_type_name;
            }

            reference array_type_name()
            {
                static Json array_type_name(string_type({'a','r','r','a','y'}));

                return array_type_name;
            }

            reference null_type_name()
            {
                static Json null_type_name(string_type({'n','u','l','l'}));

                return null_type_name;
            }

            reference true_value() const
            {
                static const Json true_value(true, semantic_tag::none);
                return true_value;
            }

            reference false_value() const
            {
                static const Json false_value(false, semantic_tag::none);
                return false_value;
            }

            reference null_value() const
            {
                static const Json null_value(null_type(), semantic_tag::none);
                return null_value;
            }

            template <typename... Args>
            Json* create_json(Args&& ... args)
            {
                auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
                Json* ptr = temp.get();
                temp_storage_.emplace_back(std::move(temp));
                return ptr;
            }
        };

        static bool is_false(reference ref)
        {
            return (ref.is_array() && ref.empty()) ||
                   (ref.is_object() && ref.empty()) ||
                   (ref.is_string() && ref.as_string_view().size() == 0) ||
                   (ref.is_bool() && !ref.as_bool()) ||
                   ref.is_null();
        }

        static bool is_true(reference ref)
        {
            return !is_false(ref);
        }

        class unary_operator
        {
            std::size_t precedence_level_;
            bool is_right_associative_;
        public:
            unary_operator(std::size_t precedence_level, bool is_right_associative)
                : precedence_level_(precedence_level), is_right_associative_(is_right_associative)
            {
            }

            // virtual destructor not needed

            std::size_t precedence_level() const 
            {
                return precedence_level_;
            }
            bool is_right_associative() const
            {
                return is_right_associative_;
            }

            virtual reference evaluate(reference val, eval_context&, std::error_code& ec) const = 0;
        };

        class not_expression final : public unary_operator
        {
        public:
            not_expression()
                : unary_operator(1, true)
            {}

            reference evaluate(reference val, eval_context& context, std::error_code&) const override
            {
                return is_false(val) ? context.true_value() : context.false_value();
            }
        };

        class binary_operator
        {
            std::size_t precedence_level_;
            bool is_right_associative_;
        public:
            binary_operator(std::size_t precedence_level, bool is_right_associative = false)
                : precedence_level_(precedence_level), is_right_associative_(is_right_associative)
            {
            }

            // virtual destructor not needed

            std::size_t precedence_level() const 
            {
                return precedence_level_;
            }
            bool is_right_associative() const
            {
                return is_right_associative_;
            }

            virtual reference evaluate(reference lhs, reference rhs, eval_context&, std::error_code& ec) const = 0;

            virtual std::string to_string(size_t indent = 0) const
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("to_string not implemented\n");
                return s;
            }
        };

        // expression_base
        class expression_base
        {
            std::size_t precedence_level_;
            bool is_right_associative_;
            bool is_projection_;
        public:
            expression_base(std::size_t precedence_level, bool is_right_associative, bool is_projection)
                : precedence_level_(precedence_level), is_right_associative_(is_right_associative), is_projection_(is_projection)
            {
            }

            std::size_t precedence_level() const
            {
                return precedence_level_;
            }

            bool is_right_associative() const
            {
                return is_right_associative_;
            }

            bool is_projection() const 
            {
                return is_projection_;
            }

            virtual ~expression_base() = default;

            virtual reference evaluate(reference val, eval_context& context, std::error_code& ec) const = 0;

            virtual void add_expression(std::unique_ptr<expression_base>&& expressions) = 0;

            virtual std::string to_string(std::size_t = 0) const
            {
                return std::string("to_string not implemented");
            }
        };  

        // parameter

        enum class parameter_type{value, expression};

        struct parameter
        {
            parameter_type type_;

            union
            {
                expression_base* expression_;
                pointer value_;
            };

            parameter(const parameter& other) noexcept
                : type_(other.type_)
            {
                switch (type_)
                {
                    case parameter_type::expression:
                        expression_ = other.expression_;
                        break;
                    case parameter_type::value:
                        value_ = other.value_;
                        break;
                    default:
                        break;
                }
            }

            parameter(pointer value) noexcept
                : type_(parameter_type::value), value_(value)
            {
            }

            parameter(expression_base* expression) noexcept
                : type_(parameter_type::expression), expression_(expression)
            {
            }

            parameter& operator=(const parameter& other)
            {
                if (&other != this)
                {
                    type_ = other.type_;
                    switch (type_)
                    {
                        case parameter_type::expression:
                            expression_ = other.expression_;
                            break;
                        case parameter_type::value:
                            value_ = other.value_;
                            break;
                        default:
                            break;
                    }
                }
                return *this;
            }

            bool is_value() const
            {
                return type_ == parameter_type::value;
            }

            bool is_expression() const
            {
                return type_ == parameter_type::expression;
            }

            parameter_type type() const
            {
                return type_;
            }
        };

        // function_base
        class function_base
        {
            optional<std::size_t> arg_count_;
        public:
            function_base(optional<std::size_t> arg_count)
                : arg_count_(arg_count)
            {
            }

            optional<std::size_t> arg_count() const
            {
                return arg_count_;
            }

            virtual ~function_base() = default;

            virtual reference evaluate(std::vector<parameter>& args, eval_context&, std::error_code& ec) const = 0;

            virtual std::string to_string(std::size_t = 0) const
            {
                return std::string("to_string not implemented");
            }
        };  

        class abs_function : public function_base
        {
        public:
            abs_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto val = args[0].value_;
                switch (val->type())
                {
                    case json_type::uint64_value:
                        return *val;
                    case json_type::int64_value:
                    {
                        auto j = val->template as<int64_t>() >= 0 ? val : context.create_json(std::abs(val->template as<int64_t>()));
                        return *j;
                    }
                    case json_type::double_value:
                    {
                        auto j = val->template as<double>() >= 0 ? val : context.create_json(std::abs(val->template as<double>()));
                        return *j;
                    }
                    default:
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                }
            }
        };

        class avg_function : public function_base
        {
        public:
            avg_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->empty())
                {
                    return context.null_value();
                }

                double sum = 0;
                for (auto& j : ptr->array_range())
                {
                    if (!j.is_number())
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    sum += j.template as<double>();
                }

                return sum == 0 ? context.null_value() : *context.create_json(sum/ptr->size());
            }
        };

        class ceil_function : public function_base
        {
        public:
            ceil_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto val = args[0].value_;
                switch (val->type())
                {
                    case json_type::uint64_value:
                    case json_type::int64_value:
                    {
                        return *context.create_json(val->template as<double>());
                    }
                    case json_type::double_value:
                    {
                        return *context.create_json(std::ceil(val->template as<double>()));
                    }
                    default:
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                }
            }
        };

        class contains_function : public function_base
        {
        public:
            contains_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_value()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }


                auto ptr0 = args[0].value_;
                auto ptr1 = args[1].value_;

                switch (ptr0->type())
                {
                    case json_type::array_value:
                        for (auto& j : ptr0->array_range())
                        {
                            if (j == *ptr1)
                            {
                                return context.true_value();
                            }
                        }
                        return context.false_value();
                    case json_type::string_value:
                    {
                        if (!ptr1->is_string())
                        {
                            ec = jmespath_errc::invalid_type;
                            return context.null_value();
                        }
                        auto sv0 = ptr0->template as<string_view_type>();
                        auto sv1 = ptr1->template as<string_view_type>();
                        return sv0.find(sv1) != string_view_type::npos ? context.true_value() : context.false_value();
                    }
                    default:
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                }
            }
        };

        class ends_with_function : public function_base
        {
        public:
            ends_with_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_value()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr0 = args[0].value_;
                if (!ptr0->is_string())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr1 = args[1].value_;
                if (!ptr1->is_string())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto sv0 = ptr0->template as<string_view_type>();
                auto sv1 = ptr1->template as<string_view_type>();

                if (sv1.length() <= sv0.length() && sv1 == sv0.substr(sv0.length() - sv1.length()))
                {
                    return context.true_value();
                }
                else
                {
                    return context.false_value();
                }
            }
        };

        class floor_function : public function_base
        {
        public:
            floor_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto val = args[0].value_;
                switch (val->type())
                {
                    case json_type::uint64_value:
                    case json_type::int64_value:
                    {
                        return *context.create_json(val->template as<double>());
                    }
                    case json_type::double_value:
                    {
                        return *context.create_json(std::floor(val->template as<double>()));
                    }
                    default:
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                }
            }
        };

        class join_function : public function_base
        {
        public:
            join_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                auto ptr0 = args[0].value_;
                auto ptr1 = args[1].value_;

                if (!(args[0].is_value() && args[1].is_value()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                if (!ptr0->is_string())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (!ptr1->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                string_type sep = ptr0->template as<string_type>();
                string_type buf;
                for (auto& j : ptr1->array_range())
                {
                    if (!j.is_string())
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    if (!buf.empty())
                    {
                        buf.append(sep);
                    }
                    auto sv = j.template as<string_view_type>();
                    buf.append(sv.begin(), sv.end());
                }
                return *context.create_json(buf);
            }
        };

        class length_function : public function_base
        {
        public:
            length_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr0 = args[0].value_;

                switch (ptr0->type())
                {
                    case json_type::object_value:
                    case json_type::array_value:
                        return *context.create_json(ptr0->size());
                    case json_type::string_value:
                    {
                        auto sv0 = ptr0->template as<string_view_type>();
                        auto length = unicons::u32_length(sv0.begin(), sv0.end());
                        return *context.create_json(length);
                    }
                    default:
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                }
            }
        };

        class max_function : public function_base
        {
        public:
            max_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->empty())
                {
                    return context.null_value();
                }

                bool is_number = ptr->at(0).is_number();
                bool is_string = ptr->at(0).is_string();
                if (!is_number && !is_string)
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                std::size_t index = 0;
                for (std::size_t i = 1; i < ptr->size(); ++i)
                {
                    if (!(ptr->at(i).is_number() == is_number && ptr->at(i).is_string() == is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    if (ptr->at(i) > ptr->at(index))
                    {
                        index = i;
                    }
                }

                return ptr->at(index);
            }
        };

        class max_by_function : public function_base
        {
        public:
            max_by_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_expression()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->empty())
                {
                    return context.null_value();
                }

                auto& expr = args[1].expression_;

                std::error_code ec2;
                Json key1 = expr->evaluate(ptr->at(0), context, ec2); 

                bool is_number = key1.is_number();
                bool is_string = key1.is_string();
                if (!(is_number || is_string))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                std::size_t index = 0;
                for (std::size_t i = 1; i < ptr->size(); ++i)
                {
                    auto key2 = expr->evaluate(ptr->at(i), context, ec2); 
                    if (!(key2.is_number() == is_number && key2.is_string() == is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    if (key2 > key1)
                    {
                        key1 = key2;
                        index = i;
                    }
                }

                return ptr->at(index);
            }
        };

        class map_function : public function_base
        {
        public:
            map_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_expression() && args[1].is_value()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                auto& expr = args[0].expression_;

                auto ptr = args[1].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);

                for (auto& item : ptr->array_range())
                {
                    auto& j = expr->evaluate(item, context, ec);
                    if (ec)
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    result->push_back(j);
                }

                return *result;
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("map_function\n");
            }
        };

        class min_function : public function_base
        {
        public:
            min_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->empty())
                {
                    return context.null_value();
                }

                bool is_number = ptr->at(0).is_number();
                bool is_string = ptr->at(0).is_string();
                if (!is_number && !is_string)
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                std::size_t index = 0;
                for (std::size_t i = 1; i < ptr->size(); ++i)
                {
                    if (!(ptr->at(i).is_number() == is_number && ptr->at(i).is_string() == is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    if (ptr->at(i) < ptr->at(index))
                    {
                        index = i;
                    }
                }

                return ptr->at(index);
            }
        };

        class min_by_function : public function_base
        {
        public:
            min_by_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_expression()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->empty())
                {
                    return context.null_value();
                }

                auto& expr = args[1].expression_;

                std::error_code ec2;
                Json key1 = expr->evaluate(ptr->at(0), context, ec2); 

                bool is_number = key1.is_number();
                bool is_string = key1.is_string();
                if (!(is_number || is_string))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                std::size_t index = 0;
                for (std::size_t i = 1; i < ptr->size(); ++i)
                {
                    auto key2 = expr->evaluate(ptr->at(i), context, ec2); 
                    if (!(key2.is_number() == is_number && key2.is_string() == is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    if (key2 < key1)
                    {
                        key1 = key2;
                        index = i;
                    }
                }

                return ptr->at(index);
            }
        };

        class merge_function : public function_base
        {
        public:
            merge_function()
                : function_base(optional<std::size_t>())
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                if (args.empty())
                {
                    ec = jmespath_errc::invalid_arity;
                    return context.null_value();
                }

                for (auto& param : args)
                {
                    if (!param.is_value())
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                }

                auto ptr0 = args[0].value_;
                if (!ptr0->is_object())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (args.size() == 1)
                {
                    return *ptr0;
                }

                auto result = context.create_json(*ptr0);
                for (std::size_t i = 1; i < args.size(); ++i)
                {
                    auto ptr = args[i].value_;
                    if (!ptr->is_object())
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    for (auto& item : ptr->object_range())
                    {
                        result->insert_or_assign(item.key(),item.value());
                    }
                }

                return *result;
            }
        };

        class type_function : public function_base
        {
        public:
            type_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;

                switch (ptr->type())
                {
                    case json_type::int64_value:
                    case json_type::uint64_value:
                    case json_type::double_value:
                        return context.number_type_name();
                    case json_type::bool_value:
                        return context.boolean_type_name();
                    case json_type::string_value:
                        return context.string_type_name();
                    case json_type::object_value:
                        return context.object_type_name();
                    case json_type::array_value:
                        return context.array_type_name();
                    default:
                        return context.null_type_name();
                        break;

                }
            }
        };

        class sort_function : public function_base
        {
        public:
            sort_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->size() <= 1)
                {
                    return *ptr;
                }

                bool is_number = ptr->at(0).is_number();
                bool is_string = ptr->at(0).is_string();
                if (!is_number && !is_string)
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                for (std::size_t i = 1; i < ptr->size(); ++i)
                {
                    if (ptr->at(i).is_number() != is_number || ptr->at(i).is_string() != is_string)
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                }

                auto v = context.create_json(*ptr);
                std::stable_sort((v->array_range()).begin(), (v->array_range()).end());
                return *v;
            }
        };

        class sort_by_function : public function_base
        {
        public:
            sort_by_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_expression()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                if (ptr->size() <= 1)
                {
                    return *ptr;
                }

                auto& expr = args[1].expression_;

                auto v = context.create_json(*ptr);
                std::stable_sort((v->array_range()).begin(), (v->array_range()).end(),
                    [&expr,&context,&ec](reference lhs, reference rhs) -> bool
                {
                    std::error_code ec2;
                    reference key1 = expr->evaluate(lhs, context, ec2);
                    bool is_number = key1.is_number();
                    bool is_string = key1.is_string();
                    if (!(is_number || is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                    }

                    reference key2 = expr->evaluate(rhs, context, ec2);
                    if (!(key2.is_number() == is_number && key2.is_string() == is_string))
                    {
                        ec = jmespath_errc::invalid_type;
                    }
                    
                    return key1 < key2;
                });
                return ec ? context.null_value() : *v;
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("sort_by_function\n");
            }
        };

        class keys_function final : public function_base
        {
        public:
            keys_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_object())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);
                result->reserve(args.size());

                for (auto& item : ptr->object_range())
                {
                    result->emplace_back(item.key());
                }
                return *result;
            }
        };

        class values_function final : public function_base
        {
        public:
            values_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_object())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);
                result->reserve(args.size());

                for (auto& item : ptr->object_range())
                {
                    result->emplace_back(item.value());
                }
                return *result;
            }
        };

        class reverse_function final : public function_base
        {
        public:
            reverse_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                switch (ptr->type())
                {
                    case json_type::string_value:
                    {
                        string_view_type sv = ptr->as_string_view();
                        std::basic_string<char32_t> buf;
                        unicons::convert(sv.begin(), sv.end(), std::back_inserter(buf));
                        std::reverse(buf.begin(), buf.end());
                        string_type s;
                        unicons::convert(buf.begin(), buf.end(), std::back_inserter(s));
                        return *context.create_json(s);
                    }
                    case json_type::array_value:
                    {
                        auto result = context.create_json(*ptr);
                        std::reverse(result->array_range().begin(),result->array_range().end());
                        return *result;
                    }
                    default:
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                }
            }
        };

        class starts_with_function : public function_base
        {
        public:
            starts_with_function()
                : function_base(2)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!(args[0].is_value() && args[1].is_value()))
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr0 = args[0].value_;
                if (!ptr0->is_string())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr1 = args[1].value_;
                if (!ptr1->is_string())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto sv0 = ptr0->template as<string_view_type>();
                auto sv1 = ptr1->template as<string_view_type>();

                if (sv1.length() <= sv0.length() && sv1 == sv0.substr(0, sv1.length()))
                {
                    return context.true_value();
                }
                else
                {
                    return context.false_value();
                }
            }
        };

        class sum_function : public function_base
        {
        public:
            sum_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (!ptr->is_array())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }
                double sum = 0;
                for (auto& j : ptr->array_range())
                {
                    if (!j.is_number())
                    {
                        ec = jmespath_errc::invalid_type;
                        return context.null_value();
                    }
                    sum += j.template as<double>();
                }

                return *context.create_json(sum);
            }
        };

        class to_array_function final : public function_base
        {
        public:
            to_array_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                if (ptr->is_array())
                {
                    return *ptr;
                }
                else
                {
                    auto result = context.create_json(json_array_arg);
                    result->push_back(*ptr);
                    return *result;
                }
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("to_array_function\n");
            }
        };

        class to_number_function final : public function_base
        {
        public:
            to_number_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                switch (ptr->type())
                {
                    case json_type::int64_value:
                    case json_type::uint64_value:
                    case json_type::double_value:
                        return *ptr;
                    case json_type::string_value:
                    {
                        auto sv = ptr->as_string_view();
                        auto result1 = jsoncons::detail::to_integer<uint64_t>(sv.data(), sv.length());
                        if (result1)
                        {
                            return *context.create_json(result1.value());
                        }
                        auto result2 = jsoncons::detail::to_integer<int64_t>(sv.data(), sv.length());
                        if (result2)
                        {
                            return *context.create_json(result2.value());
                        }
                        jsoncons::detail::to_double_t to_double;
                        try
                        {
                            auto s = ptr->as_string();
                            double d = to_double(s.c_str(), s.length());
                            return *context.create_json(d);
                        }
                        catch (const std::exception&)
                        {
                            return context.null_value();
                        }
                    }
                    default:
                        return context.null_value();
                }
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("to_number_function\n");
            }
        };

        class to_string_function final : public function_base
        {
        public:
            to_string_function()
                : function_base(1)
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(args.size() == *this->arg_count());

                if (!args[0].is_value())
                {
                    ec = jmespath_errc::invalid_type;
                    return context.null_value();
                }

                auto ptr = args[0].value_;
                return *context.create_json(ptr->template as<string_type>());
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("to_string_function\n");
            }
        };

        class not_null_function final : public function_base
        {
        public:
            not_null_function()
                : function_base(optional<std::size_t>())
            {
            }

            reference evaluate(std::vector<parameter>& args, eval_context& context, std::error_code&) const override
            {
                for (auto& param : args)
                {
                    if (param.is_value() && !param.value_->is_null())
                    {
                        return *(param.value_);
                    }
                }
                return context.null_value();
            }

            std::string to_string(std::size_t = 0) const override
            {
                return std::string("to_string_function\n");
            }
        };

        // token

        class token
        {
        public:
            token_type type_;

            union
            {
                std::unique_ptr<expression_base> expression_;
                unary_operator* unary_operator_;
                binary_operator* binary_operator_;
                function_base* function_;
                Json value_;
                string_type key_;
            };
        public:

            token(current_node_arg_t) noexcept
                : type_(token_type::current_node)
            {
            }

            token(begin_function_arg_t) noexcept
                : type_(token_type::begin_function)
            {
            }

            token(end_function_arg_t) noexcept
                : type_(token_type::end_function)
            {
            }

            token(separator_arg_t) noexcept
                : type_(token_type::separator)
            {
            }

            token(lparen_arg_t) noexcept
                : type_(token_type::lparen)
            {
            }

            token(rparen_arg_t) noexcept
                : type_(token_type::rparen)
            {
            }

            token(end_of_expression_arg_t) noexcept
                : type_(token_type::end_of_expression)
            {
            }

            token(begin_multi_select_hash_arg_t) noexcept
                : type_(token_type::begin_multi_select_hash)
            {
            }

            token(end_multi_select_hash_arg_t) noexcept
                : type_(token_type::end_multi_select_hash)
            {
            }

            token(begin_multi_select_list_arg_t) noexcept
                : type_(token_type::begin_multi_select_list)
            {
            }

            token(end_multi_select_list_arg_t) noexcept
                : type_(token_type::end_multi_select_list)
            {
            }

            token(begin_filter_arg_t) noexcept
                : type_(token_type::begin_filter)
            {
            }

            token(end_filter_arg_t) noexcept
                : type_(token_type::end_filter)
            {
            }

            token(pipe_arg_t) noexcept
                : type_(token_type::pipe)
            {
            }

            token(key_arg_t, const string_type& key)
                : type_(token_type::key)
            {
                new (&key_) string_type(key);
            }

            token(std::unique_ptr<expression_base>&& expression)
                : type_(token_type::expression)
            {
                new (&expression_) std::unique_ptr<expression_base>(std::move(expression));
            }

            token(unary_operator* expression) noexcept
                : type_(token_type::unary_operator),
                  unary_operator_(expression)
            {
            }

            token(binary_operator* expression) noexcept
                : type_(token_type::binary_operator),
                  binary_operator_(expression)
            {
            }

            token(function_base* function) noexcept
                : type_(token_type::function),
                  function_(function)
            {
            }

            token(argument_arg_t) noexcept
                : type_(token_type::argument)
            {
            }

            token(begin_expression_type_arg_t) noexcept
                : type_(token_type::begin_expression_type)
            {
            }

            token(end_expression_type_arg_t) noexcept
                : type_(token_type::end_expression_type)
            {
            }

            token(literal_arg_t, Json&& value) noexcept
                : type_(token_type::literal), value_(std::move(value))
            {
            }

            token(token&& other) noexcept
            {
                construct(std::forward<token>(other));
            }

            token& operator=(token&& other)
            {
                if (&other != this)
                {
                    if (type_ == other.type_)
                    {
                        switch (type_)
                        {
                            case token_type::expression:
                                expression_ = std::move(other.expression_);
                                break;
                            case token_type::key:
                                key_ = std::move(other.key_);
                                break;
                            case token_type::unary_operator:
                                unary_operator_ = other.unary_operator_;
                                break;
                            case token_type::binary_operator:
                                binary_operator_ = other.binary_operator_;
                                break;
                            case token_type::function:
                                function_ = other.function_;
                                break;
                            case token_type::literal:
                                value_ = std::move(other.value_);
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        destroy();
                        construct(std::forward<token>(other));
                    }
                }
                return *this;
            }

            ~token() noexcept
            {
                destroy();
            }

            token_type type() const
            {
                return type_;
            }

            bool is_lparen() const
            {
                return type_ == token_type::lparen; 
            }

            bool is_lbrace() const
            {
                return type_ == token_type::begin_multi_select_hash; 
            }

            bool is_key() const
            {
                return type_ == token_type::key; 
            }

            bool is_rparen() const
            {
                return type_ == token_type::rparen; 
            }

            bool is_current_node() const
            {
                return type_ == token_type::current_node; 
            }

            bool is_projection() const
            {
                return type_ == token_type::expression && expression_->is_projection(); 
            }

            bool is_expression() const
            {
                return type_ == token_type::expression; 
            }

            bool is_operator() const
            {
                return type_ == token_type::unary_operator || 
                       type_ == token_type::binary_operator; 
            }

            std::size_t precedence_level() const
            {
                switch(type_)
                {
                    case token_type::unary_operator:
                        return unary_operator_->precedence_level();
                    case token_type::binary_operator:
                        return binary_operator_->precedence_level();
                    case token_type::expression:
                        return expression_->precedence_level();
                    default:
                        return 0;
                }
            }

            bool is_right_associative() const
            {
                switch(type_)
                {
                    case token_type::unary_operator:
                        return unary_operator_->is_right_associative();
                    case token_type::binary_operator:
                        return binary_operator_->is_right_associative();
                    case token_type::expression:
                        return expression_->is_right_associative();
                    default:
                        return false;
                }
            }

            void construct(token&& other)
            {
                type_ = other.type_;
                switch (type_)
                {
                    case token_type::expression:
                        new (&expression_) std::unique_ptr<expression_base>(std::move(other.expression_));
                        break;
                    case token_type::key:
                        new (&key_) string_type(std::move(other.key_));
                        break;
                    case token_type::unary_operator:
                        unary_operator_ = other.unary_operator_;
                        break;
                    case token_type::binary_operator:
                        binary_operator_ = other.binary_operator_;
                        break;
                    case token_type::function:
                        function_ = other.function_;
                        break;
                    case token_type::literal:
                        new (&value_) Json(std::move(other.value_));
                        break;
                    default:
                        break;
                }
            }

            void destroy() noexcept 
            {
                switch(type_)
                {
                    case token_type::expression:
                        expression_.~unique_ptr();
                        break;
                    case token_type::key:
                        key_.~basic_string();
                        break;
                    case token_type::literal:
                        value_.~Json();
                        break;
                    default:
                        break;
                }
            }

            std::string to_string(std::size_t indent = 0) const
            {
                switch(type_)
                {
                    case token_type::expression:
                        return expression_->to_string(indent);
                        break;
                    case token_type::unary_operator:
                        return std::string("unary_operator");
                        break;
                    case token_type::binary_operator:
                        return binary_operator_->to_string(indent);
                        break;
                    case token_type::current_node:
                        return std::string("current_node");
                        break;
                    case token_type::begin_function:
                        return std::string("begin_function");
                        break;
                    case token_type::end_function:
                        return std::string("end_function");
                        break;
                    case token_type::separator:
                        return std::string("separator");
                        break;
                    case token_type::literal:
                        return std::string("literal");
                        break;
                    case token_type::key:
                        return std::string("key") + key_;
                        break;
                    case token_type::begin_multi_select_hash:
                        return std::string("begin_multi_select_hash");
                        break;
                    case token_type::begin_multi_select_list:
                        return std::string("begin_multi_select_list");
                        break;
                    case token_type::begin_filter:
                        return std::string("begin_filter");
                        break;
                    case token_type::pipe:
                        return std::string("pipe");
                        break;
                    case token_type::lparen:
                        return std::string("lparen");
                        break;
                    case token_type::function:
                        return function_->to_string();
                    case token_type::argument:
                        return std::string("argument");
                        break;
                    case token_type::begin_expression_type:
                        return std::string("begin_expression_type");
                        break;
                    case token_type::end_expression_type:
                        return std::string("end_expression_type");
                        break;
                    default:
                        return std::string("default");
                        break;
                }
            }
        };

        static reference evaluate_tokens(reference doc, const std::vector<token>& output_stack, eval_context& context, std::error_code& ec)
        {
            pointer root_ptr = std::addressof(doc);
            std::vector<parameter> stack;
            std::vector<parameter> arg_stack;
            for (std::size_t i = 0; i < output_stack.size(); ++i)
            {
                auto& t = output_stack[i];
                switch (t.type())
                {
                    case token_type::literal:
                    {
                        stack.push_back(parameter(&t.value_));
                        break;
                    }
                    case token_type::begin_expression_type:
                    {
                        JSONCONS_ASSERT(i+1 < output_stack.size());
                        ++i;
                        JSONCONS_ASSERT(output_stack[i].is_expression());
                        JSONCONS_ASSERT(!stack.empty());
                        stack.pop_back();
                        stack.push_back(parameter(output_stack[i].expression_.get()));
                        break;
                    }
                    case token_type::pipe:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        root_ptr = stack.back().value_;
                        break;
                    }
                    case token_type::current_node:
                        stack.push_back(root_ptr);
                        break;
                    case token_type::expression:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        auto ptr = stack.back().value_;
                        stack.pop_back();
                        auto& ref = t.expression_->evaluate(*ptr, context, ec);
                        stack.push_back(std::addressof(ref));
                        break;
                    }
                    case token_type::unary_operator:
                    {
                        JSONCONS_ASSERT(stack.size() >= 1);
                        auto ptr = stack.back().value_;
                        stack.pop_back();
                        reference r = t.unary_operator_->evaluate(*ptr, context, ec);
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    case token_type::binary_operator:
                    {
                        JSONCONS_ASSERT(stack.size() >= 2);
                        auto rhs = stack.back().value_;
                        stack.pop_back();
                        auto lhs = stack.back().value_;
                        stack.pop_back();
                        reference r = t.binary_operator_->evaluate(*lhs,*rhs, context, ec);
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    case token_type::argument:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        arg_stack.push_back(std::move(stack.back()));
                        stack.pop_back();
                        break;
                    }
                    case token_type::function:
                    {
                        if (t.function_->arg_count() && *(t.function_->arg_count()) != arg_stack.size())
                        {
                            ec = jmespath_errc::invalid_arity;
                            return context.null_value();
                        }

                        reference r = t.function_->evaluate(arg_stack, context, ec);
                        if (ec)
                        {
                            return context.null_value();
                        }
                        arg_stack.clear();
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    default:
                        break;
                }
            }
            JSONCONS_ASSERT(stack.size() == 1);
            return *stack.back().value_;
        }

        // Implementations

        class or_operator final : public binary_operator
        {
        public:
            or_operator()
                : binary_operator(9)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override
            {
                if (lhs.is_null() && rhs.is_null())
                {
                    return context.null_value();
                }
                if (!is_false(lhs))
                {
                    return lhs;
                }
                else
                {
                    return rhs;
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("or_operator\n");
                return s;
            }
        };

        class and_operator final : public binary_operator
        {
        public:
            and_operator()
                : binary_operator(8)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context&, std::error_code&) const override
            {
                if (is_true(lhs))
                {
                    return rhs;
                }
                else
                {
                    return lhs;
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("and_operator\n");
                return s;
            }
        };

        class eq_operator final : public binary_operator
        {
        public:
            eq_operator()
                : binary_operator(6)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override 
            {
                return lhs == rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("eq_operator\n");
                return s;
            }
        };

        class ne_operator final : public binary_operator
        {
        public:
            ne_operator()
                : binary_operator(6)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override 
            {
                return lhs != rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("ne_operator\n");
                return s;
            }
        };

        class lt_operator final : public binary_operator
        {
        public:
            lt_operator()
                : binary_operator(5)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override 
            {
                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return context.null_value();
                }
                return lhs < rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("lt_operator\n");
                return s;
            }
        };

        class lte_operator final : public binary_operator
        {
        public:
            lte_operator()
                : binary_operator(5)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override 
            {
                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return context.null_value();
                }
                return lhs <= rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("lte_operator\n");
                return s;
            }
        };

        class gt_operator final : public binary_operator
        {
        public:
            gt_operator()
                : binary_operator(5)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override
            {
                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return context.null_value();
                }
                return lhs > rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("gt_operator\n");
                return s;
            }
        };

        class gte_operator final : public binary_operator
        {
        public:
            gte_operator()
                : binary_operator(5)
            {
            }

            reference evaluate(reference lhs, reference rhs, eval_context& context, std::error_code&) const override
            {
                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return context.null_value();
                }
                return lhs >= rhs ? context.true_value() : context.false_value();
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("gte_operator\n");
                return s;
            }
        };

        // selector_base
        class selector_base :  public expression_base
        {
        public:
            selector_base()
                : expression_base(1, false, false)
            {
            }

            void add_expression(std::unique_ptr<expression_base>&&) override
            {
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

            reference evaluate(reference val, eval_context& context, std::error_code&) const override
            {
                //std::cout << "(identifier_selector " << identifier_  << " ) " << pretty_print(val) << "\n";
                if (val.is_object() && val.contains(identifier_))
                {
                    return val.at(identifier_);
                }
                else 
                {
                    return context.null_value();
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("identifier_selector ");
                s.append(identifier_);
                return s;
            }
        };

        class current_node final : public selector_base
        {
        public:
            current_node()
            {
            }

            reference evaluate(reference val, eval_context&, std::error_code&) const override
            {
                return val;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("current_node ");
                return s;
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

            reference evaluate(reference val, eval_context& context, std::error_code&) const override
            {
                if (!val.is_array())
                {
                    return context.null_value();
                }
                int64_t slen = static_cast<int64_t>(val.size());
                if (index_ >= 0 && index_ < slen)
                {
                    std::size_t index = static_cast<std::size_t>(index_);
                    return val.at(index);
                }
                else if ((slen + index_) >= 0 && (slen+index_) < slen)
                {
                    std::size_t index = static_cast<std::size_t>(slen + index_);
                    return val.at(index);
                }
                else
                {
                    return context.null_value();
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("index_selector ");
                s.append(std::to_string(index_));
                return s;
            }
        };

        // projection_base
        class projection_base : public expression_base
        {
        protected:
            std::vector<std::unique_ptr<expression_base>> expressions_;
        public:
            projection_base(std::size_t precedence_level, bool is_right_associative = true)
                : expression_base(precedence_level, is_right_associative, true)
            {
            }

            void add_expression(std::unique_ptr<expression_base>&& expr) override
            {
                if (!expressions_.empty() && expressions_.back()->is_projection() && 
                    (expr->precedence_level() < expressions_.back()->precedence_level() ||
                     (expr->precedence_level() == expressions_.back()->precedence_level() && expr->is_right_associative())))
                {
                    expressions_.back()->add_expression(std::move(expr));
                }
                else
                {
                    expressions_.emplace_back(std::move(expr));
                }
            }

            reference apply_expressions(reference val, eval_context& context, std::error_code& ec) const
            {
                pointer ptr = std::addressof(val);
                for (auto& expression : expressions_)
                {
                    ptr = std::addressof(expression->evaluate(*ptr, context, ec));
                }
                return *ptr;
            }
        };

        class object_projection final : public projection_base
        {
        public:
            object_projection()
                : projection_base(11, true)
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (!val.is_object())
                {
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);
                for (auto item : val.object_range())
                {
                    if (!item.value().is_null())
                    {
                        auto j = this->apply_expressions(item.value(), context, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("object_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class list_projection final : public projection_base
        {
        public:
            list_projection()
                : projection_base(11, true)
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (!val.is_array())
                {
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);
                for (reference item : val.array_range())
                {
                    if (!item.is_null())
                    {
                        auto j = this->apply_expressions(item, context, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("list_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class slice_projection final : public projection_base
        {
            slice slice_;
        public:
            slice_projection(const slice& s)
                : projection_base(11, true), slice_(s)
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (!val.is_array())
                {
                    return context.null_value();
                }

                auto start = slice_.get_start(val.size());
                auto end = slice_.get_stop(val.size());
                auto step = slice_.step();

                if (step == 0)
                {
                    ec = jmespath_errc::step_cannot_be_zero;
                    return context.null_value();
                }

                auto result = context.create_json(json_array_arg);
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
                        auto j = this->apply_expressions(val.at(static_cast<std::size_t>(i)), context, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                else
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
                        auto j = this->apply_expressions(val.at(static_cast<std::size_t>(i)), context, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }

                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("slice_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class filter_expression final : public projection_base
        {
            std::vector<token> token_list_;
        public:
            filter_expression(std::vector<token>&& token_list)
                : projection_base(11, true), token_list_(std::move(token_list))
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (!val.is_array())
                {
                    return context.null_value();
                }
                auto result = context.create_json(json_array_arg);

                for (auto& item : val.array_range())
                {
                    reference j = evaluate_tokens(item, token_list_, context, ec);
                    if (is_true(j))
                    {
                        auto jj = this->apply_expressions(item, context, ec);
                        if (!jj.is_null())
                        {
                            result->push_back(jj);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("filter_expression\n");
                for (auto& item : token_list_)
                {
                    std::string sss = item.to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class flatten_projection final : public projection_base
        {
        public:
            flatten_projection()
                : projection_base(11, false)
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (!val.is_array())
                {
                    return context.null_value();
                }

                auto currentp = context.create_json(json_array_arg);
                for (reference item : val.array_range())
                {
                    if (item.is_array())
                    {
                        for (reference item_of_item : item.array_range())
                        {
                            currentp->push_back(item_of_item);
                        }
                    }
                    else
                    {
                        currentp->push_back(item);
                    }
                }
                auto result = context.create_json(json_array_arg);
                for (reference item : currentp->array_range())
                {
                    if (!item.is_null())
                    {
                        auto j = this->apply_expressions(item, context, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("flatten_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class multi_select_list final : public selector_base
        {
            std::vector<std::vector<token>> token_lists_;
        public:
            multi_select_list(std::vector<std::vector<token>>&& token_lists)
                : token_lists_(std::move(token_lists))
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (val.is_null())
                {
                    return val;
                }
                auto result = context.create_json(json_array_arg);
                result->reserve(token_lists_.size());

                for (auto& list : token_lists_)
                {
                    result->push_back(evaluate_tokens(val, list, context, ec));
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("multi_select_list\n");
                for (auto& list : token_lists_)
                {
                    for (auto& item : list)
                    {
                        std::string sss = item.to_string(indent+2);
                        s.insert(s.end(), sss.begin(), sss.end());
                        s.push_back('\n');
                    }
                    s.append("---\n");
                }
                return s;
            }
        };

        struct key_tokens
        {
            string_type key;
            std::vector<token> tokens;

            key_tokens(string_type&& key, std::vector<token>&& tokens) noexcept
                : key(std::move(key)), tokens(std::move(tokens))
            {
            }
        };

        class multi_select_hash final : public selector_base
        {
        public:
            std::vector<key_tokens> key_toks_;

            multi_select_hash(std::vector<key_tokens>&& key_toks)
                : key_toks_(std::move(key_toks))
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                if (val.is_null())
                {
                    return val;
                }
                auto resultp = context.create_json(json_object_arg);
                resultp->reserve(key_toks_.size());
                for (auto& item : key_toks_)
                {
                    resultp->try_emplace(item.key, evaluate_tokens(val, item.tokens, context, ec));
                }

                return *resultp;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("multi_select_list\n");
                /*for (auto& key_expr : key_toks_)
                {
                    for (std::size_t i = 0; i <= indent+2; ++i)
                    {
                        s.push_back(' ');
                    }
                    s.append(key_expr.key);
                    std::string sss = key_expr.expression->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }*/
                return s;
            }
        };

        class function_expression final : public selector_base
        {
        public:
            std::vector<token> toks_;

            function_expression(std::vector<token>&& toks)
                : toks_(std::move(toks))
            {
            }

            reference evaluate(reference val, eval_context& context, std::error_code& ec) const override
            {
                return evaluate_tokens(val, toks_, context, ec);
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("function_expression\n");
                for (auto& tok : toks_)
                {
                    for (std::size_t i = 0; i <= indent+2; ++i)
                    {
                        s.push_back(' ');
                    }
                    std::string sss = tok.to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class static_context
        {
            abs_function abs_func_;
            avg_function avg_func_;
            ceil_function ceil_func_;
            contains_function contains_func_;
            ends_with_function ends_with_func_;
            floor_function floor_func_;
            join_function join_func_;
            length_function length_func_;
            max_function max_func_;
            max_by_function max_by_func_;
            map_function map_func_;
            merge_function merge_func_;
            min_function min_func_;
            min_by_function min_by_func_;
            type_function type_func_;
            sort_function sort_func_;
            sort_by_function sort_by_func_;
            keys_function keys_func_;
            values_function values_func_;
            reverse_function reverse_func_;
            starts_with_function starts_with_func_;
            sum_function sum_func_;
            to_array_function to_array_func_;
            to_number_function to_number_func_;
            to_string_function to_string_func_;
            not_null_function not_null_func_;

            using function_dictionary = std::unordered_map<string_type,function_base*>;
            const function_dictionary functions_ =
            {
                {string_type{'a','b','s'}, &abs_func_},
                {string_type{'a','v','g'}, &avg_func_},
                {string_type{'c','e','i', 'l'}, &ceil_func_},
                {string_type{'c','o','n', 't', 'a', 'i', 'n', 's'}, &contains_func_},
                {string_type{'e','n','d', 's', '_', 'w', 'i', 't', 'h'}, &ends_with_func_},
                {string_type{'f','l','o', 'o', 'r'}, &floor_func_},
                {string_type{'j','o','i', 'n'}, &join_func_},
                {string_type{'l','e','n', 'g', 't', 'h'}, &length_func_},
                {string_type{'m','a','x'}, &max_func_},
                {string_type{'m','a','x','_','b','y'}, &max_by_func_},
                {string_type{'m','a','p'}, &map_func_},
                {string_type{'m','i','n'}, &min_func_},
                {string_type{'m','i','n','_','b','y'}, &min_by_func_},
                {string_type{'m','e','r', 'g', 'e'}, &merge_func_},
                {string_type{'t','y','p', 'e'}, &type_func_},
                {string_type{'s','o','r', 't'}, &sort_func_},
                {string_type{'s','o','r', 't','_','b','y'}, &sort_by_func_},
                {string_type{'k','e','y', 's'}, &keys_func_},
                {string_type{'v','a','l', 'u','e','s'}, &values_func_},
                {string_type{'r','e','v', 'e', 'r', 's','e'}, &reverse_func_},
                {string_type{'s','t','a', 'r','t','s','_','w','i','t','h'}, &starts_with_func_},
                {string_type{'s','u','m'}, &sum_func_},
                {string_type{'t','o','_','a','r','r','a','y',}, &to_array_func_},
                {string_type{'t','o','_', 'n', 'u', 'm','b','e','r'}, &to_number_func_},
                {string_type{'t','o','_', 's', 't', 'r','i','n','g'}, &to_string_func_},
                {string_type{'n','o','t', '_', 'n', 'u','l','l'}, &not_null_func_}
            };

            std::vector<std::unique_ptr<Json>> temp_storage_;

        public:

            function_base* get_function(const string_type& name, std::error_code& ec) const
            {
                auto it = functions_.find(name);
                if (it == functions_.end())
                {
                    ec = jmespath_errc::unknown_function;
                    return nullptr;
                }
                return it->second;
            }

            unary_operator* get_not_operator() const
            {
                static not_expression not_oper;

                return &not_oper;
            }

            binary_operator* get_or_operator() const
            {
                static or_operator or_oper;

                return &or_oper;
            }

            binary_operator* get_and_operator() const
            {
                static and_operator and_oper;

                return &and_oper;
            }

            binary_operator* get_eq_operator() const
            {
                static eq_operator eq_oper;
                return &eq_oper;
            }

            binary_operator* get_ne_operator() const
            {
                static ne_operator ne_oper;
                return &ne_oper;
            }

            binary_operator* get_lt_operator() const
            {
                static lt_operator lt_oper;
                return &lt_oper;
            }

            binary_operator* get_lte_operator() const
            {
                static lte_operator lte_oper;
                return &lte_oper;
            }

            binary_operator* get_gt_operator() const
            {
                static gt_operator gt_oper;
                return &gt_oper;
            }

            binary_operator* get_gte_operator() const
            {
                static gte_operator gte_oper;
                return &gte_oper;
            }
        };

        class jmespath_expression
        {
            static_context context_;
            std::vector<token> output_stack_;
        public:
            jmespath_expression()
            {
            }

            jmespath_expression(jmespath_expression&& expr)
                : context_(std::move(expr.context_)),
                  output_stack_(std::move(expr.output_stack_))
            {
            }

            jmespath_expression(static_context&& context,
                                std::vector<token>&& output_stack)
                : context_(std::move(context)), output_stack_(std::move(output_stack))
            {
            }

            Json evaluate(reference doc)
            {
                if (output_stack_.empty())
                {
                    return Json::null();
                }
                eval_context dynamic_storage;
                std::error_code ec;
                Json result = evaluate_tokens(doc, output_stack_, dynamic_storage, ec);
                if (ec)
                {
                    JSONCONS_THROW(jmespath_error(ec));
                }
                return result;
            }

            Json evaluate(reference doc, std::error_code& ec)
            {
                if (output_stack_.empty())
                {
                    return Json::null();
                }
                eval_context dynamic_storage;
                return evaluate_tokens(doc, output_stack_, dynamic_storage, ec);
            }

            static jmespath_expression compile(const string_view_type& expr)
            {
                jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
                std::error_code ec;
                jmespath_expression result = evaluator.compile(expr.data(), expr.size(), ec);
                if (ec)
                {
                    JSONCONS_THROW(jmespath_error(ec, evaluator.line(), evaluator.column()));
                }
                return result;
            }

            static jmespath_expression compile(const string_view_type& expr,
                                               std::error_code& ec)
            {
                jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
                return evaluator.compile(expr.data(), expr.size(), ec);
            }
        };
    private:
        std::size_t line_;
        std::size_t column_;
        const char_type* begin_input_;
        const char_type* end_input_;
        const char_type* p_;

        static_context context_;
        std::vector<path_state> state_stack_;

        std::vector<token> output_stack_;
        std::vector<token> operator_stack_;

    public:
        jmespath_evaluator()
            : line_(1), column_(1),
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

        jmespath_expression compile(const char_type* path, 
                                    std::size_t length,
                                    std::error_code& ec)
        {
            push_token(current_node_arg);
            state_stack_.emplace_back(path_state::start);

            string_type buffer;
            uint32_t cp = 0;
            uint32_t cp2 = 0;
     
            begin_input_ = path;
            end_input_ = path + length;
            p_ = begin_input_;

            slice slic{};
            int paren_level = 0;

            while (p_ < end_input_)
            {
                switch (state_stack_.back())
                {
                    case path_state::start: 
                    {
                        state_stack_.back() = path_state::rhs_expression;
                        state_stack_.emplace_back(path_state::lhs_expression);
                        break;
                    }
                    case path_state::rhs_expression:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '.': 
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::sub_expression);
                                break;
                            case '|':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                state_stack_.emplace_back(path_state::expect_pipe_or_or);
                                break;
                            case '&':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                state_stack_.emplace_back(path_state::expect_and);
                                break;
                            case '<':
                            case '>':
                            case '=':
                            {
                                state_stack_.emplace_back(path_state::comparator_expression);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                state_stack_.emplace_back(path_state::cmp_ne);
                                break;
                            }
                            case ')':
                            {
                                if (state_stack_.size() > 1 && (*(state_stack_.rbegin()+1) == path_state::argument || *(state_stack_.rbegin() + 1) == path_state::expression_type))
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ++p_;
                                    ++column_;
                                    --paren_level;
                                    push_token(rparen_arg);
                                }
                                break;
                            }
                            case '[':
                                state_stack_.emplace_back(path_state::bracket_specifier);
                                ++p_;
                                ++column_;
                                break;
                            //case '{':
                            //    state_stack_.emplace_back(path_state::lhs_expression);
                            //    break;
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jmespath_errc::syntax_error;
                                    return jmespath_expression();
                                }
                                break;
                        }
                        break;
                    case path_state::comparator_expression:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '<':
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::lhs_expression;
                                state_stack_.emplace_back(path_state::cmp_lt_or_lte);
                                break;
                            case '>':
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::lhs_expression;
                                state_stack_.emplace_back(path_state::cmp_gt_or_gte);
                                break;
                            case '=':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::lhs_expression;
                                state_stack_.emplace_back(path_state::cmp_eq);
                                break;
                            }
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jmespath_errc::syntax_error;
                                    return jmespath_expression();
                                }
                                break;
                        }
                        break;
                    case path_state::lhs_expression: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '\"':
                                state_stack_.back() = path_state::val_expr;
                                state_stack_.emplace_back(path_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::raw_string;
                                ++p_;
                                ++column_;
                                break;
                            case '`':
                                state_stack_.back() = path_state::literal;
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                push_token(begin_multi_select_hash_arg);
                                state_stack_.back() = path_state::multi_select_hash;
                                ++p_;
                                ++column_;
                                break;
                            case '*': // wildcard
                                push_token(token(jsoncons::make_unique<object_projection>()));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                ++paren_level;
                                push_token(lparen_arg);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                push_token(token(context_.get_not_operator()));
                                break;
                            }
                            case '@':
                                ++p_;
                                ++column_;
                                push_token(token(jsoncons::make_unique<current_node>()));
                                state_stack_.pop_back();
                                break;
                            case '[': 
                                state_stack_.back() = path_state::bracket_specifier_or_multi_select_list;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = path_state::identifier_or_function_expr;
                                    state_stack_.emplace_back(path_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_identifier;
                                    return jmespath_expression();
                                }
                                break;
                        };
                        break;
                    }
                    case path_state::sub_expression: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '\"':
                                state_stack_.back() = path_state::val_expr;
                                state_stack_.emplace_back(path_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                push_token(begin_multi_select_hash_arg);
                                state_stack_.back() = path_state::multi_select_hash;
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                push_token(token(jsoncons::make_unique<object_projection>()));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '[': 
                                state_stack_.back() = path_state::expect_multi_select_list;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = path_state::identifier_or_function_expr;
                                    state_stack_.emplace_back(path_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_identifier;
                                    return jmespath_expression();
                                }
                                break;
                        };
                        break;
                    }
                    case path_state::key_expr:
                        switch (*p_)
                        {
                            case '\"':
                                ++p_;
                                ++column_;
                                push_token(token(key_arg, buffer));
                                buffer.clear(); 
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token(key_arg, buffer));
                                buffer.clear(); 
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case path_state::val_expr:
                        switch (*p_)
                        {
                            case '\"':
                                ++p_;
                                ++column_;
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case path_state::expression_or_expression_type:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '&':
                                push_token(token(begin_expression_type_arg));
                                state_stack_.back() = path_state::expression_type;
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::argument;
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                        }
                        break;
                    case path_state::identifier_or_function_expr:
                        switch(*p_)
                        {
                            case '(':
                            {
                                auto f = context_.get_function(buffer, ec);
                                if (ec)
                                {
                                    return jmespath_expression();
                                }
                                buffer.clear();
                                ++paren_level;
                                push_token(token(begin_function_arg));
                                push_token(token(f));
                                state_stack_.back() = path_state::function_expression;
                                state_stack_.emplace_back(path_state::expression_or_expression_type);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            }
                        }
                        break;

                    case path_state::function_expression:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ',':
                                push_token(token(current_node_arg));
                                state_stack_.emplace_back(path_state::expression_or_expression_type);
                                ++p_;
                                ++column_;
                                break;
                            case ')':
                            {
                                --paren_level;
                                push_token(token(end_function_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                break;
                        }
                        break;

                    case path_state::argument:
                        push_token(argument_arg);
                        state_stack_.pop_back();
                        break;

                    case path_state::expression_type:
                        push_token(end_expression_type_arg);
                        push_token(argument_arg);
                        state_stack_.pop_back();
                        break;

                    case path_state::quoted_string: 
                        switch (*p_)
                        {
                            case '\"':
                                state_stack_.pop_back(); // quoted_string
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

                    case path_state::unquoted_string: 
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                state_stack_.pop_back(); // unquoted_string
                                advance_past_space_character(ec);
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
                                    state_stack_.pop_back(); // unquoted_string
                                }
                                break;
                        };
                        break;
                    case path_state::raw_string_escape_char:
                        switch (*p_)
                        {
                            case '\'':
                                buffer.push_back(*p_);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back('\\');
                                buffer.push_back(*p_);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                        }
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
                                ec = jmespath_errc::illegal_escaped_character;
                                return jmespath_expression();
                        }
                        break;
                    case path_state::escape_u1:
                        cp = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u2;
                        break;
                    case path_state::escape_u2:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u3;
                        break;
                    case path_state::escape_u3:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u4;
                        break;
                    case path_state::escape_u4:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
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
                                ec = jmespath_errc::invalid_codepoint;
                                return jmespath_expression();
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
                                ec = jmespath_errc::invalid_codepoint;
                                return jmespath_expression();
                        }
                        break;
                    case path_state::escape_u5:
                        cp2 = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u6;
                        break;
                    case path_state::escape_u6:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = path_state::escape_u7;
                        break;
                    case path_state::escape_u7:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return jmespath_expression();
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
                            return jmespath_expression();
                        }
                        uint32_t codepoint = 0x10000 + ((cp & 0x3FF) << 10) + (cp2 & 0x3FF);
                        unicons::convert(&codepoint, &codepoint + 1, std::back_inserter(buffer));
                        state_stack_.pop_back();
                        ++p_;
                        ++column_;
                        break;
                    }
                    case path_state::raw_string: 
                        switch (*p_)
                        {
                            case '\'':
                            {
                                push_token(token(literal_arg, Json(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); // raw_string
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
                                state_stack_.emplace_back(path_state::raw_string_escape_char);
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
                    case path_state::literal: 
                        switch (*p_)
                        {
                            case '`':
                            {
                                json_decoder<Json> decoder;
                                basic_json_reader<char_type,string_source<char_type>> reader(buffer, decoder);
                                std::error_code parse_ec;
                                reader.read(parse_ec);
                                if (parse_ec)
                                {
                                    ec = jmespath_errc::invalid_literal;
                                    return jmespath_expression();
                                }
                                auto j = decoder.get_result();

                                push_token(token(literal_arg, std::move(j)));
                                buffer.clear();
                                state_stack_.pop_back(); // json_value
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
                                if (p_+1 < end_input_)
                                {
                                    ++p_;
                                    ++column_;
                                    if (*p_ != '`')
                                    {
                                        buffer.push_back('\\');
                                    }
                                    buffer.push_back(*p_);
                                }
                                else
                                {
                                    ec = jmespath_errc::unexpected_end_of_input;
                                    return jmespath_expression();
                                }
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
                    case path_state::number:
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

                    case path_state::bracket_specifier:
                        switch(*p_)
                        {
                            case '*':
                                push_token(token(jsoncons::make_unique<list_projection>()));
                                state_stack_.back() = path_state::expect_right_bracket;
                                ++p_;
                                ++column_;
                                break;
                            case ']': // []
                                push_token(token(jsoncons::make_unique<flatten_projection>()));
                                state_stack_.pop_back(); // bracket_specifier
                                ++p_;
                                ++column_;
                                break;
                            case '?':
                                push_token(token(begin_filter_arg));
                                state_stack_.back() = path_state::filter;
                                state_stack_.emplace_back(path_state::rhs_expression);
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            case ':': // slice_expression
                                state_stack_.back() = path_state::rhs_slice_expression_start ;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            // number
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = path_state::right_bracket_or_slice_expression;
                                state_stack_.emplace_back(path_state::number);
                                break;
                            default:
                                ec = jmespath_errc::expected_index_expression;
                                return jmespath_expression();
                        }
                        break;
                    case path_state::bracket_specifier_or_multi_select_list:
                        switch(*p_)
                        {
                            case '*':
                                if (p_+1 >= end_input_)
                                {
                                    ec = jmespath_errc::unexpected_end_of_input;
                                    return jmespath_expression();
                                }
                                if (*(p_+1) == ']')
                                {
                                    state_stack_.back() = path_state::bracket_specifier;
                                }
                                else
                                {
                                    push_token(token(begin_multi_select_list_arg));
                                    state_stack_.back() = path_state::multi_select_list;
                                    state_stack_.emplace_back(path_state::lhs_expression);                                
                                }
                                break;
                            case ']': // []
                            case '?':
                            case ':': // slice_expression
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = path_state::bracket_specifier;
                                break;
                            default:
                                push_token(token(begin_multi_select_list_arg));
                                state_stack_.back() = path_state::multi_select_list;
                                state_stack_.emplace_back(path_state::lhs_expression);                                
                                break;
                        }
                        break;

                    case path_state::expect_multi_select_list:
                        switch(*p_)
                        {
                            case ']':
                            case '?':
                            case ':':
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                ec = jmespath_errc::expected_multi_select_list;
                                return jmespath_expression();
                            case '*':
                                push_token(token(jsoncons::make_unique<list_projection>()));
                                state_stack_.back() = path_state::expect_right_bracket;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(begin_multi_select_list_arg));
                                state_stack_.back() = path_state::multi_select_list;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                        }
                        break;

                    case path_state::multi_select_hash:
                        switch(*p_)
                        {
                            case '*':
                            case ']':
                            case '?':
                            case ':':
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                break;
                            default:
                                state_stack_.back() = path_state::key_val_expr;
                                break;
                        }
                        break;

                    case path_state::right_bracket_or_slice_expression:
                        switch(*p_)
                        {
                            case ']':
                            {
                                if (buffer.empty())
                                {
                                    push_token(token(jsoncons::make_unique<flatten_projection>()));
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jmespath_errc::invalid_number;
                                        return jmespath_expression();
                                    }
                                    push_token(token(jsoncons::make_unique<index_selector>(r.value())));

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
                                        ec = jmespath_errc::invalid_number;
                                        return jmespath_expression();
                                    }
                                    slic.start_ = r.value();
                                    buffer.clear();
                                }
                                state_stack_.back() = path_state::rhs_slice_expression_start;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    case path_state::rhs_slice_expression_start :
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jmespath_errc::invalid_number;
                                return jmespath_expression();
                            }
                            slic.stop_ = optional<int64_t>(r.value());
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token(jsoncons::make_unique<slice_projection>(slic)));
                                slic = slice{};
                                state_stack_.pop_back(); // bracket_specifier2
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = path_state::rhs_slice_expression_stop;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
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
                                ec = jmespath_errc::invalid_number;
                                return jmespath_expression();
                            }
                            if (r.value() == 0)
                            {
                                ec = jmespath_errc::step_cannot_be_zero;
                                return jmespath_expression();
                            }
                            slic.step_ = r.value();
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token(jsoncons::make_unique<slice_projection>(slic)));
                                buffer.clear();
                                slic = slice{};
                                state_stack_.pop_back(); // rhs_slice_expression_stop
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_right_bracket:
                    {
                        switch(*p_)
                        {
                            case ']':
                                state_stack_.pop_back(); // expect_right_bracket
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::key_val_expr: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '\"':
                                state_stack_.back() = path_state::expect_colon;
                                state_stack_.emplace_back(path_state::key_expr);
                                state_stack_.emplace_back(path_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::expect_colon;
                                state_stack_.emplace_back(path_state::raw_string);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = path_state::expect_colon;
                                    state_stack_.emplace_back(path_state::key_expr);
                                    state_stack_.emplace_back(path_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_key;
                                    return jmespath_expression();
                                }
                                break;
                        };
                        break;
                    }
                    case path_state::cmp_lt_or_lte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(context_.get_lte_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(context_.get_lt_operator()));
                                push_token(token(current_node_arg));
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
                                push_token(token(context_.get_gte_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(context_.get_gt_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_eq:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(context_.get_eq_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::cmp_ne:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(context_.get_ne_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::cmp_lt_or_lte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_eq_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::cmp_gt_or_gte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_ne_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_dot:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case '.':
                                state_stack_.pop_back(); // expect_dot
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_dot;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_pipe_or_or:
                    {
                        switch(*p_)
                        {
                            case '|':
                                push_token(token(context_.get_or_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(pipe_arg));
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case path_state::expect_and:
                    {
                        switch(*p_)
                        {
                            case '&':
                                push_token(token(context_.get_and_operator()));
                                push_token(token(current_node_arg));
                                state_stack_.pop_back(); // expect_and
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_and;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_filter_right_bracket:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ']':
                            {
                                state_stack_.pop_back();

                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::multi_select_list:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ',':
                                push_token(token(separator_arg));
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                            case '.':
                                state_stack_.emplace_back(path_state::sub_expression);
                                ++p_;
                                ++column_;
                                break;
                            case '|':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                state_stack_.emplace_back(path_state::expect_pipe_or_or);
                                break;
                            }
                            case ']':
                            {
                                push_token(token(end_multi_select_list_arg));
                                state_stack_.pop_back();

                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::filter:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ']':
                            {
                                push_token(token(end_filter_arg));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_right_brace:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ',':
                                push_token(token(separator_arg));
                                state_stack_.back() = path_state::key_val_expr; 
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                            case '{':
                                state_stack_.emplace_back(path_state::lhs_expression);
                                break;
                            case '.':
                                state_stack_.emplace_back(path_state::sub_expression);
                                ++p_;
                                ++column_;
                                break;
                            case '}':
                            {
                                state_stack_.pop_back();
                                push_token(end_multi_select_hash_arg);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_brace;
                                return jmespath_expression();
                        }
                        break;
                    }
                    case path_state::expect_colon:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character(ec);
                                break;
                            case ':':
                                state_stack_.back() = path_state::expect_right_brace;
                                state_stack_.emplace_back(path_state::lhs_expression);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_colon;
                                return jmespath_expression();
                        }
                        break;
                    }
                }
                
            }

            if (state_stack_.size() >= 3 && state_stack_.back() == path_state::unquoted_string)
            {
                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                state_stack_.pop_back(); // unquoted_string
                if (state_stack_.back() == path_state::val_expr || state_stack_.back() == path_state::identifier_or_function_expr)
                {
                    buffer.clear();
                    state_stack_.pop_back(); // val_expr
                }
            }
            if (state_stack_.size() >= 3 && state_stack_.back() == path_state::expect_dot)
            {
                state_stack_.pop_back(); // path_state::expect_dot
                if (state_stack_.back() == path_state::lhs_expression)
                {
                    state_stack_.pop_back(); // lhs_expression
                }
            }

            JSONCONS_ASSERT(state_stack_.size() == 1);
            JSONCONS_ASSERT(state_stack_.back() == path_state::rhs_expression);
            state_stack_.pop_back();

            push_token(end_of_expression_arg);

            //for (auto& t : output_stack_)
            //{
            //    std::cout << t.to_string() << std::endl;
            //}

            if (paren_level != 0)
            {
                ec = jmespath_errc::unbalanced_parentheses;
                return jmespath_expression();
            }

            return jmespath_expression(std::move(context_), std::move(output_stack_));
        }

        void advance_past_space_character(std::error_code& ec)
        {
            switch (*p_)
            {
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '\r':
                    if (p_+1 >= end_input_)
                    {
                        ec = jmespath_errc::unexpected_end_of_input;
                        return;
                    }
                    if (*(p_+1) == '\n')
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

        void unwind_rparen()
        {
            auto it = operator_stack_.rbegin();
            while (it != operator_stack_.rend() && !it->is_lparen())
            {
                output_stack_.emplace_back(std::move(*it));
                ++it;
            }
            if (it == operator_stack_.rend())
            {
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced parenthesis"));
            }
            ++it;
            operator_stack_.erase(it.base(),operator_stack_.end());
        }

        void push_token(token&& tok)
        {
            switch (tok.type())
            {
                case token_type::end_filter:
                {
                    unwind_rparen();
                    std::vector<token> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_filter)
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

                    if (toks.front().type() != token_type::literal)
                    {
                        toks.emplace(toks.begin(), current_node_arg);
                    }
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<filter_expression>(std::move(toks)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<filter_expression>(std::move(toks))));
                    }
                    break;
                }
                case token_type::end_multi_select_list:
                {
                    unwind_rparen();
                    std::vector<std::vector<token>> vals;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_list)
                    {
                        std::vector<token> toks;
                        do
                        {
                            toks.insert(toks.begin(), std::move(*it));
                            ++it;
                        } while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_list && it->type() != token_type::separator);
                        if (it->type() == token_type::separator)
                        {
                            ++it;
                        }
                        if (toks.front().type() != token_type::literal)
                        {
                            toks.emplace(toks.begin(), current_node_arg);
                        }
                        vals.insert(vals.begin(), std::move(toks));
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
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_list>(std::move(vals)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_list>(std::move(vals))));
                    }
                    break;
                }
                case token_type::end_multi_select_hash:
                {
                    unwind_rparen();
                    std::vector<key_tokens> key_toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_hash)
                    {
                        std::vector<token> toks;
                        do
                        {
                            toks.emplace(toks.begin(), std::move(*it));
                            ++it;
                        } while (it->type() != token_type::key);
                        JSONCONS_ASSERT(it->is_key());
                        auto key = std::move(it->key_);
                        ++it;
                        if (it->type() == token_type::separator)
                        {
                            ++it;
                        }
                        if (toks.front().type() != token_type::literal)
                        {
                            toks.emplace(toks.begin(), current_node_arg);
                        }
                        key_toks.emplace(key_toks.begin(), std::move(key), std::move(toks));
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
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_hash>(std::move(key_toks)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_hash>(std::move(key_toks))));
                    }
                    break;
                }
                case token_type::end_expression_type:
                {
                    std::vector<token> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_expression_type)
                    {
                        toks.insert(toks.begin(), std::move(*it));
                        ++it;
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    if (toks.front().type() != token_type::literal)
                    {
                        toks.emplace(toks.begin(), current_node_arg);
                    }
                    output_stack_.erase(it.base(),output_stack_.end());
                    output_stack_.emplace_back(token(jsoncons::make_unique<function_expression>(std::move(toks))));
                    break;
                }
                case token_type::literal:
                    if (!output_stack_.empty() && output_stack_.back().type() == token_type::current_node)
                    {
                        output_stack_.back() = std::move(tok);
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                case token_type::expression:
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(std::move(tok.expression_));
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                case token_type::rparen:
                    {
                        unwind_rparen();
                        break;
                    }
                case token_type::end_function:
                    {
                        unwind_rparen();
                        std::vector<token> toks;
                        auto it = output_stack_.rbegin();
                        while (it != output_stack_.rend() && it->type() != token_type::begin_function)
                        {
                            toks.insert(toks.begin(), std::move(*it));
                            ++it;
                        }
                        if (it == output_stack_.rend())
                        {
                            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                        }
                        ++it;
                        if (toks.front().type() != token_type::literal)
                        {
                            toks.emplace(toks.begin(), current_node_arg);
                        }
                        output_stack_.erase(it.base(),output_stack_.end());

                        if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                            (tok.precedence_level() < output_stack_.back().precedence_level() ||
                            (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                        {
                            output_stack_.back().expression_->add_expression(jsoncons::make_unique<function_expression>(std::move(toks)));
                        }
                        else
                        {
                            output_stack_.emplace_back(token(jsoncons::make_unique<function_expression>(std::move(toks))));
                        }
                        break;
                    }
                case token_type::end_of_expression:
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend())
                        {
                            output_stack_.emplace_back(std::move(*it));
                            ++it;
                        }
                        operator_stack_.clear();
                        break;
                    }
                case token_type::unary_operator:
                case token_type::binary_operator:
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
                case token_type::separator:
                {
                    unwind_rparen();
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                }
                case token_type::begin_filter:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::begin_multi_select_list:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::begin_multi_select_hash:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::begin_function:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::current_node:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_type::key:
                case token_type::pipe:
                case token_type::argument:
                case token_type::begin_expression_type:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_type::lparen:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
                case token_type::function:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
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
                ec = jmespath_errc::invalid_codepoint;
            }
            return cp;
        }
    };

    } // detail

    template <class Json>
    using jmespath_expression = typename jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&>::jmespath_expression;

    template<class Json>
    Json search(const Json& doc, const typename Json::string_view_type& path)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        std::error_code ec;
        auto expr = evaluator.compile(path.data(), path.size(), ec);
        if (ec)
        {
            JSONCONS_THROW(jmespath_error(ec, evaluator.line(), evaluator.column()));
        }
        auto result = expr.evaluate(doc, ec);
        if (ec)
        {
            JSONCONS_THROW(jmespath_error(ec));
        }
        return result;
    }

    template<class Json>
    Json search(const Json& doc, const typename Json::string_view_type& path, std::error_code& ec)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        auto expr = evaluator.compile(path.data(), path.size(), ec);
        if (ec)
        {
            return Json::null();
        }
        auto result = expr.evaluate(doc, ec);
        if (ec)
        {
            return Json::null();
        }
        return result;
    }

} // namespace jmespath
} // namespace jsoncons

#endif
