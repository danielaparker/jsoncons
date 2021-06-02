// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP
#define JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP

#include <string> // std::basic_string
#include <vector> // std::vector
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <limits> // std::numeric_limits
#include <set> // std::set
#include <utility> // std::move
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif
#include <jsoncons/json_type.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

namespace jsoncons { 
namespace jsonpath {

    struct reference_arg_t
    {
        explicit reference_arg_t() = default;
    };
    constexpr reference_arg_t reference_arg{};

    struct const_reference_arg_t
    {
        explicit const_reference_arg_t() = default;
    };
    constexpr const_reference_arg_t const_reference_arg{};

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

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

    struct begin_union_arg_t
    {
        explicit begin_union_arg_t() = default;
    };
    constexpr begin_union_arg_t begin_union_arg{};

    struct end_union_arg_t
    {
        explicit end_union_arg_t() = default;
    };
    constexpr end_union_arg_t end_union_arg{};

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

    struct begin_expression_arg_t
    {
        explicit begin_expression_arg_t() = default;
    };
    constexpr begin_expression_arg_t begin_expression_arg{};

    struct end_index_expression_arg_t
    {
        explicit end_index_expression_arg_t() = default;
    };
    constexpr end_index_expression_arg_t end_index_expression_arg{};

    struct end_argument_expression_arg_t
    {
        explicit end_argument_expression_arg_t() = default;
    };
    constexpr end_argument_expression_arg_t end_argument_expression_arg{};

    struct current_node_arg_t
    {
        explicit current_node_arg_t() = default;
    };
    constexpr current_node_arg_t current_node_arg{};

    struct root_node_arg_t
    {
        explicit root_node_arg_t() = default;
    };
    constexpr root_node_arg_t root_node_arg{};

    struct end_function_arg_t
    {
        explicit end_function_arg_t() = default;
    };
    constexpr end_function_arg_t end_function_arg{};

    struct argument_arg_t
    {
        explicit argument_arg_t() = default;
    };
    constexpr argument_arg_t argument_arg{};

    template <class CharT>
    class path_component
    {
        enum class component_kind {root,current,identifier,index};
    public:
        using string_type = std::basic_string<CharT>;
    private:

        component_kind kind_;
        string_type identifier_;
        std::size_t index_;
    public:
        path_component(root_node_arg_t)
            : kind_(component_kind::root)
        {
            identifier_.push_back('$');
        }
        path_component(current_node_arg_t)
            : kind_(component_kind::current)
        {
            identifier_.push_back('@');
        }

        path_component(const string_type& identifier)
            : kind_(component_kind::identifier), identifier_(identifier)
        {
        }

        path_component(std::size_t index)
            : kind_(component_kind::index), index_(index)
        {
        }

        path_component(const path_component&) = default;
        path_component(path_component&&) = default;
        path_component& operator=(const path_component&) = default;
        path_component& operator=(path_component&&) = default;

        bool is_identifier() const
        {
            return kind_ == component_kind::identifier || kind_ == component_kind::root || kind_ == component_kind::current;
        }

        bool is_index() const
        {
            return kind_ == component_kind::index;
        }

        const string_type& identifier() const
        {
            return identifier_;
        }

        std::size_t index() const
        {
            return index_;
        }

        bool operator==(const path_component& other) const
        {
            if (is_identifier() && other.is_identifier())
            {
                return identifier_ == other.identifier_;
            }
            else if (is_index() && other.is_index())
            {
                return index_ == other.index_;
            }
            else
            {
                return false;
            }
        }

        bool operator<(const path_component& other) const
        {
            if (is_identifier() && other.is_identifier())
            {
                return identifier_ < other.identifier_;
            }
            else if (is_index() && other.is_index())
            {
                return index_ < other.index_;
            }
            else
            {
                return is_index() ? true : false;
            }
        }

        void to_string(string_type& buffer) const
        {
            switch (kind_)
            {
                case component_kind::root:
                    buffer.push_back('$');
                    break;
                case component_kind::current:
                    buffer.push_back('@');
                    break;
                case component_kind::identifier:
                    buffer.push_back('[');
                    buffer.push_back('\'');
                    buffer.append(identifier_);
                    buffer.push_back('\'');
                    buffer.push_back(']');
                    break;
                case component_kind::index:
                    buffer.push_back('[');
                    jsoncons::detail::from_integer(index_,buffer);
                    buffer.push_back(']');
                    break;
            }
        }
    };

    template <class CharT>
    bool operator==(const path_component<CharT>& lhs,const path_component<CharT>& rhs)
    {
        return lhs.operator==(rhs);
    }

    template <class CharT>
    bool operator!=(const path_component<CharT>& lhs, const path_component<CharT>& rhs)
    {
        return !(lhs.operator==(rhs));
    }

    template <class CharT>
    bool operator<(const path_component<CharT>& lhs,const path_component<CharT>& rhs)
    {
        return lhs.operator<(rhs);
    }

    template <class CharT>
    std::basic_string<CharT> to_string(const std::vector<path_component<CharT>>& path)
    {
        std::basic_string<CharT> buffer;
        for (const auto& component : path)
        {
            component.to_string(buffer);
        }
        return buffer;
    }

    enum class result_options {value=0, nodups=1, sort=2, path=4};

    using result_type = result_options;

    inline result_options operator~(result_options a)
    {
        return static_cast<result_options>(~static_cast<unsigned int>(a));
    }

    inline result_options operator&(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
    }

    inline result_options operator^(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) ^ static_cast<unsigned int>(b));
    }

    inline result_options operator|(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    inline result_options operator&=(result_options& a, result_options b)
    {
        a = a & b;
        return a;
    }

    inline result_options operator^=(result_options& a, result_options b)
    {
        a = a ^ b;
        return a;
    }

    inline result_options operator|=(result_options& a, result_options b)
    {
        a = a | b;
        return a;
    }

    template <class Json>
    class parameter;

    template <class Json,class JsonReference>
    class value_or_pointer
    {
    public:
        friend class parameter<Json>;
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<reference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
    private:
        bool is_value_;
        union
        {
            value_type val_;
            pointer ptr_;
        };
    public:
        value_or_pointer(value_type&& val)
            : is_value_(true), val_(std::move(val))
        {
        }

        value_or_pointer(pointer ptr)
            : is_value_(false), ptr_(std::move(ptr))
        {
        }

        value_or_pointer(value_or_pointer&& other) noexcept
            : is_value_(other.is_value_)
        {
            if (is_value_)
            {
                new(&val_)value_type(std::move(other.val_));
            }
            else
            {
                ptr_ = other.ptr_;
            }
        }

        ~value_or_pointer() noexcept
        {
            if (is_value_)
            {
                val_.~value_type();
            }
        }

        reference value() 
        {
            return is_value_ ? val_ : *ptr_;
        }

        pointer ptr() 
        {
            return is_value_ ? &val_ : ptr_;
        }
    };

    template <class Json>
    class parameter
    {
        using value_type = Json;
        using reference = const Json&;
        using pointer = const Json*;
    private:
        value_or_pointer<Json,reference> data_;
    public:
        template <class JsonReference>
        parameter(value_or_pointer<Json,JsonReference>&& data) noexcept
            : data_(nullptr)
        {
            data_.is_value_ = data.is_value_;
            if (data.is_value_)
            {
                data_.val_ = std::move(data.val_);
            }
            else
            {
                data_.ptr_ = data.ptr_;
            }
        }

        parameter(const parameter& other) noexcept = default;

        parameter(parameter&& other) noexcept = default;

        parameter& operator=(const parameter& other) noexcept = default;

        parameter& operator=(parameter&& other) noexcept = default;

        const Json& value() const
        {
            return data_.is_value_ ? data_.val_ : *data_.ptr_;
        }
    };

    template <class Json>
    class custom_function
    {
    public:
        using value_type = Json;
        using char_type = typename Json::char_type;
        using parameter_type = parameter<Json>;
        using function_type = std::function<value_type(jsoncons::span<const parameter_type>, std::error_code& ec)>;
        using string_type = std::basic_string<char_type>;

        string_type function_name_;
        optional<std::size_t> arity_;
        function_type f_;

        custom_function(const string_type& function_name,
                        const optional<std::size_t>& arity,
                        const function_type& f)
            : function_name_(function_name),
              arity_(arity),
              f_(f)
        {
        }

        custom_function(string_type&& function_name,
                        optional<std::size_t>&& arity,
                        function_type&& f)
            : function_name_(std::move(function_name)),
              arity_(std::move(arity)),
              f_(std::move(f))
        {
        }

        custom_function(const custom_function&) = default;

        custom_function(custom_function&&) = default;

        const string_type& name() const 
        {
            return function_name_;
        }

        optional<std::size_t> arity() const 
        {
            return arity_;
        }

        const function_type& function() const 
        {
            return f_;
        }
    };

    template <class Json>
    class custom_functions
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using function_type = std::function<value_type(jsoncons::span<const parameter_type>, std::error_code& ec)>;
        using const_iterator = typename std::vector<custom_function<Json>>::const_iterator;

        std::vector<custom_function<Json>> functions_;
    public:
        void register_function(const string_type& name,
                               jsoncons::optional<std::size_t> arity,
                               const function_type& f)
        {
            functions_.emplace_back(name, arity, f);
        }

        const_iterator begin() const
        {
            return functions_.begin();
        }

        const_iterator end() const
        {
            return functions_.end();
        }
    };

namespace detail {

    enum class node_kind{unknown, single, multi};

    template <class Json,class JsonReference>
    class dynamic_resources;

    template <class Json,class JsonReference>
    struct unary_operator
    {
        std::size_t precedence_level_;
        bool is_right_associative_;

        unary_operator(std::size_t precedence_level,
                       bool is_right_associative)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
        {
        }

        virtual ~unary_operator() = default;

        std::size_t precedence_level() const 
        {
            return precedence_level_;
        }
        bool is_right_associative() const
        {
            return is_right_associative_;
        }

        virtual Json evaluate(JsonReference, 
                              std::error_code&) const = 0;
    };

    template <class Json>
    bool is_false(const Json& val)
    {
        return ((val.is_array() && val.empty()) ||
                 (val.is_object() && val.empty()) ||
                 (val.is_string() && val.as_string_view().empty()) ||
                 (val.is_bool() && !val.as_bool()) ||
                 (val.is_number() && (val == Json(0))) ||
                 val.is_null());
    }

    template <class Json>
    bool is_true(const Json& val)
    {
        return !is_false(val);
    }

    template <class Json,class JsonReference>
    class unary_not_operator final : public unary_operator<Json,JsonReference>
    {
    public:
        unary_not_operator()
            : unary_operator<Json,JsonReference>(1, true)
        {}

        Json evaluate(JsonReference val, 
                      std::error_code&) const override
        {
            return is_false(val) ? Json(true) : Json(false);
        }
    };

    template <class Json,class JsonReference>
    class unary_minus_operator final : public unary_operator<Json,JsonReference>
    {
    public:
        unary_minus_operator()
            : unary_operator<Json,JsonReference>(1, true)
        {}

        Json evaluate(JsonReference val, 
                      std::error_code&) const override
        {
            if (val.is_int64())
            {
                return Json(-val.template as<int64_t>());
            }
            else if (val.is_double())
            {
                return Json(-val.as_double());
            }
            else
            {
                return Json::null();
            }
        }
    };

    template <class Json,class JsonReference>
    class regex_operator final : public unary_operator<Json,JsonReference>
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        std::basic_regex<char_type> pattern_;
    public:
        regex_operator(std::basic_regex<char_type>&& pattern)
            : unary_operator<Json,JsonReference>(2, true),
              pattern_(std::move(pattern))
        {
        }

        regex_operator(regex_operator&&) = default;
        regex_operator& operator=(regex_operator&&) = default;

        Json evaluate(JsonReference val, 
                             std::error_code&) const override
        {
            if (!val.is_string())
            {
                return Json::null();
            }
            return std::regex_search(val.as_string(), pattern_) ? Json(true) : Json(false);
        }
    };

    template <class Json,class JsonReference>
    struct binary_operator
    {
        std::size_t precedence_level_;
        bool is_right_associative_;

        binary_operator(std::size_t precedence_level,
                        bool is_right_associative = false)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
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

        virtual Json evaluate(JsonReference, 
                             JsonReference, 

                             std::error_code&) const = 0;

        virtual std::string to_string(int = 0) const
        {
            return "binary operator";
        }

    protected:
        ~binary_operator() = default;
    };

    // Implementations

    template <class Json,class JsonReference>
    class or_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        or_operator()
            : binary_operator<Json,JsonReference>(9)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (lhs.is_null() && rhs.is_null())
            {
                return Json::null();
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
        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                //s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("or operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class and_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        and_operator()
            : binary_operator<Json,JsonReference>(8)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
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

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("and operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class eq_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        eq_operator()
            : binary_operator<Json,JsonReference>(6)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            return lhs == rhs ? Json(true) : Json(false);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class ne_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        ne_operator()
            : binary_operator<Json,JsonReference>(6)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            return lhs != rhs ? Json(true) : Json(false);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("not equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class lt_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        lt_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs < rhs ? Json(true) : Json(false);
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs < rhs ? Json(true) : Json(false);
            }
            return Json::null();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("less than operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class lte_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        lte_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs <= rhs ? Json(true) : Json(false);
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs <= rhs ? Json(true) : Json(false);
            }
            return Json::null();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("less than or equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class gt_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        gt_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            //std::cout << "operator> lhs: " << lhs << ", rhs: " << rhs << "\n";

            if (lhs.is_number() && rhs.is_number())
            {
                return lhs > rhs ? Json(true) : Json(false);
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs > rhs ? Json(true) : Json(false);
            }
            return Json::null();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("greater than operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class gte_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        gte_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs >= rhs ? Json(true) : Json(false);
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs >= rhs ? Json(true) : Json(false);
            }
            return Json::null();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("greater than or equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class plus_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        plus_operator()
            : binary_operator<Json,JsonReference>(4)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return Json::null();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return Json(((lhs.template as<int64_t>() + rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return Json((lhs.template as<uint64_t>() + rhs.template as<uint64_t>()));
            }
            else
            {
                return Json((lhs.as_double() + rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("plus operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class minus_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        minus_operator()
            : binary_operator<Json,JsonReference>(4)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return Json::null();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return Json(((lhs.template as<int64_t>() - rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return Json((lhs.template as<uint64_t>() - rhs.template as<uint64_t>()));
            }
            else
            {
                return Json((lhs.as_double() - rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("minus operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class mult_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        mult_operator()
            : binary_operator<Json,JsonReference>(3)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return Json::null();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return Json(((lhs.template as<int64_t>() * rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return Json((lhs.template as<uint64_t>() * rhs.template as<uint64_t>()));
            }
            else
            {
                return Json((lhs.as_double() * rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("multiply operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class div_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        div_operator()
            : binary_operator<Json,JsonReference>(3)
        {
        }

        Json evaluate(JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            //std::cout << "operator/ lhs: " << lhs << ", rhs: " << rhs << "\n";

            if (!(lhs.is_number() && rhs.is_number()))
            {
                return Json::null();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return Json(((lhs.template as<int64_t>() / rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return Json((lhs.template as<uint64_t>() / rhs.template as<uint64_t>()));
            }
            else
            {
                return Json((lhs.as_double() / rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("divide operator");
            return s;
        }
    };

    // function_base
    template <class Json>
    class function_base
    {
        jsoncons::optional<std::size_t> arg_count_;
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        function_base(jsoncons::optional<std::size_t> arg_count)
            : arg_count_(arg_count)
        {
        }

        virtual ~function_base() noexcept = default;

        jsoncons::optional<std::size_t> arity() const
        {
            return arg_count_;
        }

        virtual value_type evaluate(const std::vector<parameter_type>& args, 
                                    std::error_code& ec) const = 0;

        virtual std::string to_string(int level = 0) const
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("function");
            return s;
        }
    };  

    template <class Json>
    class decorator_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using string_view_type = typename Json::string_view_type;
        using function_type = std::function<value_type(jsoncons::span<const parameter_type>, std::error_code& ec)>;
    private:
        function_type f_;
    public:
        decorator_function(jsoncons::optional<std::size_t> arity,
            const function_type& f)
            : function_base<Json>(arity), f_(f)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args,
            std::error_code& ec) const override
        {
            return f_(args, ec);
        }
    };

    template <class Json>
    class contains_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using string_view_type = typename Json::string_view_type;

        contains_function()
            : function_base<Json>(2)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            auto arg1= args[1].value();

            switch (arg0.type())
            {
                case json_type::array_value:
                    for (auto& j : arg0.array_range())
                    {
                        if (j == arg1)
                        {
                            return value_type(true);
                        }
                    }
                    return value_type(false);
                case json_type::string_value:
                {
                    if (!arg1.is_string())
                    {
                        ec = jsonpath_errc::invalid_type;
                        return value_type::null();
                    }
                    auto sv0 = arg0.template as<string_view_type>();
                    auto sv1 = arg1.template as<string_view_type>();
                    return sv0.find(sv1) != string_view_type::npos ? value_type(true) : value_type(false);
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("contains function");
            return s;
        }
    };

    template <class Json>
    class ends_with_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using string_view_type = typename Json::string_view_type;

        ends_with_function()
            : function_base<Json>(2)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            auto arg1= args[1].value();
            if (!arg1.is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            auto sv0 = arg0.template as<string_view_type>();
            auto sv1 = arg1.template as<string_view_type>();

            if (sv1.length() <= sv0.length() && sv1 == sv0.substr(sv0.length() - sv1.length()))
            {
                return value_type(true);
            }
            else
            {
                return value_type(false);
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("ends_with function");
            return s;
        }
    };

    template <class Json>
    class starts_with_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using string_view_type = typename Json::string_view_type;

        starts_with_function()
            : function_base<Json>(2)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            auto arg1= args[1].value();
            if (!arg1.is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            auto sv0 = arg0.template as<string_view_type>();
            auto sv1 = arg1.template as<string_view_type>();

            if (sv1.length() <= sv0.length() && sv1 == sv0.substr(0, sv1.length()))
            {
                return value_type(true);
            }
            else
            {
                return value_type(false);
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("starts_with function");
            return s;
        }
    };

    template <class Json>
    class sum_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        sum_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_array())
            {
                //std::cout << "arg: " << arg0 << "\n";
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            //std::cout << "sum function arg: " << arg0 << "\n";

            double sum = 0;
            for (auto& j : arg0.array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
                sum += j.template as<double>();
            }

            return value_type(sum);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("sum function");
            return s;
        }
    };

#if defined(JSONCONS_HAS_STD_REGEX)

    template <class Json>
    class tokenize_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        tokenize_function()
            : function_base<Json>(2)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            if (!args[0].value().is_string() || !args[1].value().is_string())
            {
                //std::cout << "arg: " << arg0 << "\n";
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            auto arg0 = args[0].value().template as<string_type>();
            auto arg1 = args[1].value().template as<string_type>();

            std::regex::flag_type options = std::regex_constants::ECMAScript; 
            std::basic_regex<char_type> pieces_regex(arg1, options);

            std::regex_token_iterator<typename string_type::const_iterator> rit ( arg0.begin(), arg0.end(), pieces_regex, -1);
            std::regex_token_iterator<typename string_type::const_iterator> rend;

            value_type j(json_array_arg);
            while (rit != rend) 
            {
                j.emplace_back(rit->str());
                ++rit;
            }
            return j;
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("tokenize function");
            return s;
        }
    };

#endif // defined(JSONCONS_HAS_STD_REGEX)

    template <class Json>
    class ceil_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        ceil_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            switch (arg0.type())
            {
                case json_type::uint64_value:
                case json_type::int64_value:
                {
                    return value_type(arg0.template as<double>());
                }
                case json_type::double_value:
                {
                    return value_type(std::ceil(arg0.template as<double>()));
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("ceil function");
            return s;
        }
    };

    template <class Json>
    class floor_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        floor_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            switch (arg0.type())
            {
                case json_type::uint64_value:
                case json_type::int64_value:
                {
                    return value_type(arg0.template as<double>());
                }
                case json_type::double_value:
                {
                    return value_type(std::floor(arg0.template as<double>()));
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("floor function");
            return s;
        }
    };

    template <class Json>
    class to_number_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        to_number_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            switch (arg0.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                case json_type::double_value:
                    return arg0;
                case json_type::string_value:
                {
                    auto sv = arg0.as_string_view();
                    auto result1 = jsoncons::detail::to_integer<uint64_t>(sv.data(), sv.length());
                    if (result1)
                    {
                        return value_type(result1.value);
                    }
                    auto result2 = jsoncons::detail::to_integer<int64_t>(sv.data(), sv.length());
                    if (result2)
                    {
                        return value_type(result2.value);
                    }
                    jsoncons::detail::to_double_t to_double;
                    try
                    {
                        auto s = arg0.as_string();
                        double d = to_double(s.c_str(), s.length());
                        return value_type(d);
                    }
                    catch (const std::exception&)
                    {
                        return value_type::null();
                    }
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("to_number function");
            return s;
        }
    };

    template <class Json>
    class prod_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        prod_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_array() || arg0.empty())
            {
                //std::cout << "arg: " << arg0 << "\n";
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            double prod = 1;
            for (auto& j : arg0.array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
                prod *= j.template as<double>();
            }

            return value_type(prod);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("prod function");
            return s;
        }
    };

    template <class Json>
    class avg_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        avg_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_array())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            if (arg0.empty())
            {
                return value_type::null();
            }
            double sum = 0;
            for (auto& j : arg0.array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
                sum += j.template as<double>();
            }

            return value_type(sum / static_cast<double>(arg0.size()));
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("to_string function");
            return s;
        }
    };

    template <class Json>
    class min_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        min_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_array())
            {
                //std::cout << "arg: " << arg0 << "\n";
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            if (arg0.empty())
            {
                return value_type::null();
            }
            bool is_number = arg0.at(0).is_number();
            bool is_string = arg0.at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0.size(); ++i)
            {
                if (!(arg0.at(i).is_number() == is_number && arg0.at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
                if (arg0.at(i) < arg0.at(index))
                {
                    index = i;
                }
            }

            return arg0.at(index);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("min function");
            return s;
        }
    };

    template <class Json>
    class max_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        max_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_array())
            {
                //std::cout << "arg: " << arg0 << "\n";
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }
            if (arg0.empty())
            {
                return value_type::null();
            }

            bool is_number = arg0.at(0).is_number();
            bool is_string = arg0.at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0.size(); ++i)
            {
                if (!(arg0.at(i).is_number() == is_number && arg0.at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
                if (arg0.at(i) > arg0.at(index))
                {
                    index = i;
                }
            }

            return arg0.at(index);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("max function");
            return s;
        }
    };

    template <class Json>
    class abs_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;

        abs_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            switch (arg0.type())
            {
                case json_type::uint64_value:
                    return arg0;
                case json_type::int64_value:
                {
                    return arg0.template as<int64_t>() >= 0 ? arg0 : value_type(std::abs(arg0.template as<int64_t>()));
                }
                case json_type::double_value:
                {
                    return arg0.template as<double>() >= 0 ? arg0 : value_type(std::abs(arg0.template as<double>()));
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("abs function");
            return s;
        }
    };

    template <class Json>
    class length_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using string_view_type = typename Json::string_view_type;
        using parameter_type = parameter<Json>;

        length_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            //std::cout << "length function arg: " << arg0 << "\n";

            switch (arg0.type())
            {
                case json_type::object_value:
                case json_type::array_value:
                    return value_type(arg0.size());
                case json_type::string_value:
                {
                    auto sv0 = arg0.template as<string_view_type>();
                    auto length = unicode_traits::count_codepoints(sv0.data(), sv0.size());
                    return value_type(length);
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return value_type::null();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("length function");
            return s;
        }
    };

    template <class Json>
    class keys_function : public function_base<Json>
    {
    public:
        using value_type = Json;
        using parameter_type = parameter<Json>;
        using string_view_type = typename Json::string_view_type;

        keys_function()
            : function_base<Json>(1)
        {
        }

        value_type evaluate(const std::vector<parameter_type>& args, 
                            std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return value_type::null();
            }

            auto arg0= args[0].value();
            if (!arg0.is_object())
            {
                ec = jsonpath_errc::invalid_type;
                return value_type::null();
            }

            value_type result(json_array_arg);
            result.reserve(args.size());

            for (auto& item : arg0.object_range())
            {
                result.emplace_back(item.key());
            }
            return result;
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("keys function");
            return s;
        }
    };

    template <class Json, class JsonReference>
    struct static_resources
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using value_type = Json;
        using reference = JsonReference;
        using function_base_type = function_base<Json>;

        std::vector<std::unique_ptr<Json>> temp_json_values_;
        std::vector<std::unique_ptr<unary_operator<Json,JsonReference>>> unary_operators_;
        std::unordered_map<string_type,std::unique_ptr<function_base_type>> custom_functions_;

        static_resources()
        {
        }

        static_resources(const custom_functions<Json>& functions)
        {
            for (const auto& item : functions)
            {
                custom_functions_.emplace(item.name(),
                                          jsoncons::make_unique<decorator_function<Json>>(item.arity(),item.function()));
            }
        }

        static_resources(const static_resources&) = default;

        static_resources(static_resources&& other) noexcept 
            : temp_json_values_(std::move(other.temp_json_values_)),
              unary_operators_(std::move(other.unary_operators_)),
              custom_functions_(std::move(other.custom_functions_))
        {
        }

        const function_base_type* get_function(const string_type& name, std::error_code& ec) const
        {
            static abs_function<Json> abs_func;
            static contains_function<Json> contains_func;
            static starts_with_function<Json> starts_with_func;
            static ends_with_function<Json> ends_with_func;
            static ceil_function<Json> ceil_func;
            static floor_function<Json> floor_func;
            static to_number_function<Json> to_number_func;
            static sum_function<Json> sum_func;
            static prod_function<Json> prod_func;
            static avg_function<Json> avg_func;
            static min_function<Json> min_func;
            static max_function<Json> max_func;
            static length_function<Json> length_func;
            static keys_function<Json> keys_func;
#if defined(JSONCONS_HAS_STD_REGEX)
            static tokenize_function<Json> tokenize_func;
#endif

            static std::unordered_map<string_type,const function_base_type*> functions =
            {
                {string_type{'a','b','s'}, &abs_func},
                {string_type{'c','o','n','t','a','i','n','s'}, &contains_func},
                {string_type{'s','t','a','r','t','s','_','w','i','t','h'}, &starts_with_func},
                {string_type{'e','n','d','s','_','w','i','t','h'}, &ends_with_func},
                {string_type{'c','e','i','l'}, &ceil_func},
                {string_type{'f','l','o','o','r'}, &floor_func},
                {string_type{'t','o','_','n','u','m','b','e','r'}, &to_number_func},
                {string_type{'s','u','m'}, &sum_func},
                {string_type{'p','r','o', 'd'}, &prod_func},
                {string_type{'a','v','g'}, &avg_func},
                {string_type{'m','i','n'}, &min_func},
                {string_type{'m','a','x'}, &max_func},
                {string_type{'l','e','n','g','t','h'}, &length_func},
                {string_type{'k','e','y','s'}, &keys_func},
#if defined(JSONCONS_HAS_STD_REGEX)
                {string_type{'t','o','k','e','n','i','z','e'}, &tokenize_func},
#endif
                {string_type{'c','o','u','n','t'}, &length_func}
            };

            auto it = functions.find(name);
            if (it == functions.end())
            {
                auto it2 = custom_functions_.find(name);
                if (it2 == custom_functions_.end())
                {
                    ec = jsonpath_errc::unknown_function;
                    return nullptr;
                }
                else
                {
                    return it2->second.get();
                }
            }
            else
            {
                return it->second;
            }
        }

        const unary_operator<Json,JsonReference>* get_unary_not() const
        {
            static unary_not_operator<Json,JsonReference> oper;
            return &oper;
        }

        const unary_operator<Json,JsonReference>* get_unary_minus() const
        {
            static unary_minus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const unary_operator<Json,JsonReference>* get_regex_operator(std::basic_regex<char_type>&& pattern) 
        {
            unary_operators_.push_back(jsoncons::make_unique<regex_operator<Json,JsonReference>>(std::move(pattern)));
            return unary_operators_.back().get();
        }

        const binary_operator<Json,JsonReference>* get_or_operator() const
        {
            static or_operator<Json,JsonReference> oper;

            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_and_operator() const
        {
            static and_operator<Json,JsonReference> oper;

            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_eq_operator() const
        {
            static eq_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_ne_operator() const
        {
            static ne_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_lt_operator() const
        {
            static lt_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_lte_operator() const
        {
            static lte_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_gt_operator() const
        {
            static gt_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_gte_operator() const
        {
            static gte_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_plus_operator() const
        {
            static plus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_minus_operator() const
        {
            static minus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_mult_operator() const
        {
            static mult_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_div_operator() const
        {
            static div_operator<Json,JsonReference> oper;
            return &oper;
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    enum class token_kind
    {
        root_node,
        current_node,
        expression,
        lparen,
        rparen,
        begin_union,
        end_union,
        begin_filter,
        end_filter,
        begin_expression,
        end_index_expression,
        end_argument_expression,
        separator,
        literal,
        selector,
        function,
        end_function,
        argument,
        end_of_expression,
        unary_operator,
        binary_operator
    };

    inline
    std::string to_string(token_kind kind)
    {
        switch (kind)
        {
            case token_kind::root_node:
                return "root_node";
            case token_kind::current_node:
                return "current_node";
            case token_kind::lparen:
                return "lparen";
            case token_kind::rparen:
                return "rparen";
            case token_kind::begin_union:
                return "begin_union";
            case token_kind::end_union:
                return "end_union";
            case token_kind::begin_filter:
                return "begin_filter";
            case token_kind::end_filter:
                return "end_filter";
            case token_kind::begin_expression:
                return "begin_expression";
            case token_kind::end_index_expression:
                return "end_index_expression";
            case token_kind::end_argument_expression:
                return "end_argument_expression";
            case token_kind::separator:
                return "separator";
            case token_kind::literal:
                return "literal";
            case token_kind::selector:
                return "selector";
            case token_kind::function:
                return "function";
            case token_kind::end_function:
                return "end_function";
            case token_kind::argument:
                return "argument";
            case token_kind::end_of_expression:
                return "end_of_expression";
            case token_kind::unary_operator:
                return "unary_operator";
            case token_kind::binary_operator:
                return "binary_operator";
            default:
                return "";
        }
    }

    template <class Json,class JsonReference>
    struct path_node
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        using path_component_type = path_component<char_type>;

        std::vector<path_component_type> path;
        pointer ptr;

        path_node(const std::vector<path_component_type>& p, const pointer& valp)
            : path(p),ptr(valp)
        {
        }
        path_node(const pointer& valp)
            : ptr(valp)
        {
        }

        path_node(std::vector<path_component_type>&& p, pointer&& valp) noexcept
            : path(std::move(p)),ptr(valp)
        {
        }
        path_node(const path_node&) = default;

        path_node(path_node&& other) noexcept
            : path(std::move(other.path)), ptr(other.ptr)
        {

        }
        path_node& operator=(const path_node&) = default;

        path_node& operator=(path_node&& other) noexcept
        {
            path.swap(other.path);
            ptr = other.ptr;
            return *this;
        }
    };
 
    template <class Json,class JsonReference>
    struct path_node_less
    {
        bool operator()(const path_node<Json,JsonReference>& a,
                        const path_node<Json,JsonReference>& b) const noexcept
        {
            return a.path < b.path;
        }
    };

    template <class Json,class JsonReference>
    struct path_node_equal
    {
        bool operator()(const path_node<Json,JsonReference>& lhs,
                        const path_node<Json,JsonReference>& rhs) const noexcept
        {
            if (lhs.path.size() != rhs.path.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < lhs.path.size(); ++i)
            {
                if (lhs.path[i] != rhs.path[i])
                {
                    return false;
                }
            }
            return true;
        }
    };

    template <class Json, class JsonReference>
    class dynamic_resources
    {
        std::vector<std::unique_ptr<Json>> temp_json_values_;
        std::unordered_map<std::size_t,std::pair<std::vector<path_node<Json,JsonReference>>,node_kind>> cache_;
    public:

        bool is_cached(std::size_t id) const
        {
            return cache_.find(id) != cache_.end();
        }

        void add_to_cache(std::size_t id, const std::vector<path_node<Json,JsonReference>>& val, node_kind ndtype) 
        {
            cache_.emplace(id,std::make_pair(val,ndtype));
        }

        void retrieve_from_cache(std::size_t id, std::vector<path_node<Json,JsonReference>>& nodes, node_kind& ndtype) 
        {
            auto it = cache_.find(id);
            if (it != cache_.end())
            {
                for (auto& item : it->second.first)
                {
                    nodes.push_back(item);
                }
                ndtype = it->second.second;
            }
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    template <class Json,class JsonReference>
    struct node_less
    {
        bool operator()(const path_node<Json,JsonReference>& a, const path_node<Json,JsonReference>& b) const
        {
            return *(a.ptr) < *(b.ptr);
        }
    };

    template <class Json,class JsonReference>
    class selector_base
    {
        bool is_path_;
        std::size_t precedence_level_;

    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = jsoncons::basic_string_view<char_type, std::char_traits<char_type>>;
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        using path_node_type = path_node<Json,JsonReference>;
        using path_component_type = path_component<char_type>;

        selector_base(bool is_path,
                      std::size_t precedence_level = 0)
            : is_path_(is_path), 
              precedence_level_(precedence_level)
        {
        }

        virtual ~selector_base() noexcept = default;

        bool is_path() const 
        {
            return is_path_;
        }

        std::size_t precedence_level() const
        {
            return precedence_level_;
        }

        bool is_right_associative() const
        {
            return true;
        }

        static std::vector<path_component_type> generate_path(const std::vector<path_component_type>& path, 
                                                              std::size_t index, 
                                                              result_options options) 
        {
            static const result_options require_path = result_options::path | result_options::nodups | result_options::sort;
            std::vector<path_component_type> s(path);
            if ((options & require_path) != result_options())
            {
                s.emplace_back(index);
            }
            return s;
        }

        static std::vector<path_component_type> generate_path(const std::vector<path_component_type>& path, 
                                                              const string_type& identifier, 
                                                              result_options options) 
        {
            static const result_options require_path = result_options::path | result_options::nodups | result_options::sort;
            std::vector<path_component_type> s(path);
            if ((options & require_path) != result_options())
            {
                s.emplace_back(identifier);
            }
            return s;
        }

        virtual void select(dynamic_resources<Json,JsonReference>& resources,
                            const std::vector<path_component_type>& path, 
                            reference root,
                            reference val, 
                            std::vector<path_node_type>& nodes,
                            node_kind& ndtype,
                            result_options options) const = 0;

        virtual void append_selector(std::unique_ptr<selector_base>&&) 
        {
        }

        virtual std::string to_string(int = 0) const
        {
            return std::string();
        }
    };

    template <class Json, class JsonReference>
    class expression_base
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = jsoncons::basic_string_view<char_type, std::char_traits<char_type>>;
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        using path_node_type = path_node<Json,JsonReference>;
        using path_component_type = path_component<char_type>;

        virtual ~expression_base() noexcept = default;

        virtual value_type evaluate_single(dynamic_resources<Json,JsonReference>& resources,
                                           const std::vector<path_component_type>& path, 
                                           reference root,
                                           reference val, 
                                           result_options options,
                                           std::error_code& ec) const = 0;

        virtual std::string to_string(int level = 0) const = 0;
    };

    template <class Json,class JsonReference>
    class token
    {
    public:
        using selector_base_type = selector_base<Json,JsonReference>;
        using expression_base_type = expression_base<Json,JsonReference>;

        token_kind type_;

        union
        {
            std::unique_ptr<selector_base_type> selector_;
            std::unique_ptr<expression_base_type> expression_;
            const unary_operator<Json,JsonReference>* unary_operator_;
            const binary_operator<Json,JsonReference>* binary_operator_;
            const function_base<Json>* function_;
            Json value_;
        };
    public:

        token(const unary_operator<Json,JsonReference>* expr) noexcept
            : type_(token_kind::unary_operator),
              unary_operator_(expr)
        {
        }

        token(const binary_operator<Json,JsonReference>* expr) noexcept
            : type_(token_kind::binary_operator),
              binary_operator_(expr)
        {
        }

        token(current_node_arg_t) noexcept
            : type_(token_kind::current_node)
        {
        }

        token(root_node_arg_t) noexcept
            : type_(token_kind::root_node)
        {
        }

        token(end_function_arg_t) noexcept
            : type_(token_kind::end_function)
        {
        }

        token(separator_arg_t) noexcept
            : type_(token_kind::separator)
        {
        }

        token(lparen_arg_t) noexcept
            : type_(token_kind::lparen)
        {
        }

        token(rparen_arg_t) noexcept
            : type_(token_kind::rparen)
        {
        }

        token(end_of_expression_arg_t) noexcept
            : type_(token_kind::end_of_expression)
        {
        }

        token(begin_union_arg_t) noexcept
            : type_(token_kind::begin_union)
        {
        }

        token(end_union_arg_t) noexcept
            : type_(token_kind::end_union)
        {
        }

        token(begin_filter_arg_t) noexcept
            : type_(token_kind::begin_filter)
        {
        }

        token(end_filter_arg_t) noexcept
            : type_(token_kind::end_filter)
        {
        }

        token(begin_expression_arg_t) noexcept
            : type_(token_kind::begin_expression)
        {
        }

        token(end_index_expression_arg_t) noexcept
            : type_(token_kind::end_index_expression)
        {
        }

        token(end_argument_expression_arg_t) noexcept
            : type_(token_kind::end_argument_expression)
        {
        }

        token(std::unique_ptr<selector_base_type>&& expr)
            : type_(token_kind::selector)
        {
            new (&selector_) std::unique_ptr<selector_base_type>(std::move(expr));
        }

        token(std::unique_ptr<expression_base_type>&& expr)
            : type_(token_kind::expression)
        {
            new (&expression_) std::unique_ptr<expression_base_type>(std::move(expr));
        }

        token(const function_base<Json>* function) noexcept
            : type_(token_kind::function),
              function_(function)
        {
        }

        token(argument_arg_t) noexcept
            : type_(token_kind::argument)
        {
        }

        token(literal_arg_t, Json&& value) noexcept
            : type_(token_kind::literal), value_(std::move(value))
        {
        }

        token(token&& other) noexcept
        {
            construct(std::forward<token>(other));
        }

        const Json& get_value(const_reference_arg_t, dynamic_resources<Json,JsonReference>&) const
        {
            return value_;
        }

        Json& get_value(reference_arg_t, dynamic_resources<Json,JsonReference>& resources) const
        {
            return *resources.create_json(value_);
        }

        token& operator=(token&& other)
        {
            if (&other != this)
            {
                if (type_ == other.type_)
                {
                    switch (type_)
                    {
                        case token_kind::selector:
                            selector_ = std::move(other.selector_);
                            break;
                        case token_kind::expression:
                            expression_ = std::move(other.expression_);
                            break;
                        case token_kind::unary_operator:
                            unary_operator_ = other.unary_operator_;
                            break;
                        case token_kind::binary_operator:
                            binary_operator_ = other.binary_operator_;
                            break;
                        case token_kind::function:
                            function_ = other.function_;
                            break;
                        case token_kind::literal:
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

        token_kind type() const
        {
            return type_;
        }

        bool is_lparen() const
        {
            return type_ == token_kind::lparen; 
        }

        bool is_rparen() const
        {
            return type_ == token_kind::rparen; 
        }

        bool is_current_node() const
        {
            return type_ == token_kind::current_node; 
        }

        bool is_path() const
        {
            return type_ == token_kind::selector && selector_->is_path(); 
        }

        bool is_operator() const
        {
            return type_ == token_kind::unary_operator || 
                   type_ == token_kind::binary_operator; 
        }

        std::size_t precedence_level() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->precedence_level();
                case token_kind::unary_operator:
                    return unary_operator_->precedence_level();
                case token_kind::binary_operator:
                    return binary_operator_->precedence_level();
                default:
                    return 0;
            }
        }

        jsoncons::optional<std::size_t> arity() const
        {
            return type_ == token_kind::function ? function_->arity() : jsoncons::optional<std::size_t>();
        }

        bool is_right_associative() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->is_right_associative();
                case token_kind::unary_operator:
                    return unary_operator_->is_right_associative();
                case token_kind::binary_operator:
                    return binary_operator_->is_right_associative();
                default:
                    return false;
            }
        }

        void construct(token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
                case token_kind::selector:
                    new (&selector_) std::unique_ptr<selector_base_type>(std::move(other.selector_));
                    break;
                case token_kind::expression:
                    new (&expression_) std::unique_ptr<expression_base_type>(std::move(other.expression_));
                    break;
                case token_kind::unary_operator:
                    unary_operator_ = other.unary_operator_;
                    break;
                case token_kind::binary_operator:
                    binary_operator_ = other.binary_operator_;
                    break;
                case token_kind::function:
                    function_ = other.function_;
                    break;
                case token_kind::literal:
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
                case token_kind::selector:
                    selector_.~unique_ptr();
                    break;
                case token_kind::expression:
                    expression_.~unique_ptr();
                    break;
                case token_kind::literal:
                    value_.~Json();
                    break;
                default:
                    break;
            }
        }

        std::string to_string(int level = 0) const
        {
            std::string s;
            switch (type_)
            {
                case token_kind::root_node:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("root node");
                    break;
                case token_kind::current_node:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("current node");
                    break;
                case token_kind::argument:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("argument");
                    break;
                case token_kind::selector:
                    s.append(selector_->to_string(level));
                    break;
                case token_kind::expression:
                    s.append(expression_->to_string(level));
                    break;
                case token_kind::literal:
                {
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    auto sbuf = value_.to_string();
                    unicode_traits::convert(sbuf.data(), sbuf.size(), s);
                    break;
                }
                case token_kind::binary_operator:
                    s.append(binary_operator_->to_string(level));
                    break;
                case token_kind::function:
                    s.append(function_->to_string(level));
                    break;
                default:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("token kind: ");
                    s.append(jsoncons::jsonpath::detail::to_string(type_));
                    break;
            }
            //s.append("\n");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class path_expression
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = typename Json::string_view_type;
        using path_node_type = path_node<Json,JsonReference>;
        using path_node_less_type = path_node_less<Json,JsonReference>;
        using path_node_equal_type = path_node_equal<Json,JsonReference>;
        using value_type = Json;
        using reference = typename path_node_type::reference;
        using pointer = typename path_node_type::pointer;
        using token_type = token<Json,JsonReference>;
        using reference_arg_type = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,
            const_reference_arg_t,reference_arg_t>::type;
        using path_component_type = path_component<char_type>;
    private:
        std::unique_ptr<selector_base<Json,JsonReference>> selector_;
    public:

        path_expression()
        {
        }

        path_expression(path_expression&& expr)
            : selector_(std::move(expr.selector_))
        {
        }

        path_expression(std::unique_ptr<selector_base<Json,JsonReference>>&& selector)
            : selector_(std::move(selector))
        {
        }

        path_expression& operator=(path_expression&& expr) = default;

        Json evaluate(dynamic_resources<Json,JsonReference>& resources, 
                      const std::vector<path_component_type>& path, 
                      reference root,
                      reference instance,
                      result_options options) const
        {
            Json result(json_array_arg);

            if ((options & result_options::path) == result_options::path)
            {
                auto callback = [&result](const std::vector<path_component_type>& path, reference)
                {
                    result.emplace_back(jsoncons::jsonpath::to_string(path));
                };
                evaluate(resources, path, root, instance, callback, options);
            }
            else
            {
                auto callback = [&result](const std::vector<path_component_type>&, reference val)
                {
                    result.push_back(val);
                };
                evaluate(resources, path, root, instance, callback, options);
            }

            return result;
        }

        template <class Callback>
        typename std::enable_if<type_traits::is_binary_function_object<Callback,const std::vector<path_component_type>&,reference>::value,void>::type
        evaluate(dynamic_resources<Json,JsonReference>& resources, 
                 const std::vector<path_component_type>& ipath, 
                 reference root,
                 reference current, 
                 Callback callback,
                 result_options options) const
        {
            std::error_code ec;

            std::vector<path_component_type> path(ipath);

            std::vector<path_node_type> temp;
            node_kind ndtype = node_kind();
            selector_->select(resources, path, root, current, temp, ndtype, options);

            if (temp.size() > 1 && (options & result_options::sort) == result_options::sort)
            {
                std::sort(temp.begin(), temp.end(), path_node_less_type());
            }

            if (temp.size() > 1 && (options & result_options::nodups) == result_options::nodups)
            {
                if ((options & result_options::sort) == result_options::sort)
                {
                    auto last = std::unique(temp.begin(),temp.end(),path_node_equal_type());
                    temp.erase(last,temp.end());
                    for (auto& node : temp)
                    {
                        callback(node.path, *node.ptr);
                    }
                }
                else
                {
                    std::vector<path_node_type> index(temp);
                    std::sort(index.begin(), index.end(), path_node_less_type());
                    auto last = std::unique(index.begin(),index.end(),path_node_equal_type());
                    index.erase(last,index.end());

                    std::vector<path_node_type> temp2;
                    temp2.reserve(index.size());
                    for (auto&& node : temp)
                    {
                        auto it = std::lower_bound(index.begin(),index.end(),node, path_node_less_type());
                        if (it != index.end() && it->path == node.path) 
                        {
                            temp2.emplace_back(std::move(node));
                            index.erase(it);
                        }
                    }
                    for (auto& node : temp2)
                    {
                        callback(node.path, *node.ptr);
                    }
                }
            }
            else
            {
                for (auto& node : temp)
                {
                    callback(node.path, *node.ptr);
                }
            }
        }

        std::string to_string(int level) const
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("expression ");
            s.append(selector_->to_string(level+1));

            return s;

        }
    };

    template <class Json,class JsonReference>
    class expression_tree
    {
    public:
        using path_node_type = path_node<Json,JsonReference>;
        using value_type = Json;
        using reference = typename path_node_type::reference;
        using pointer = typename path_node_type::pointer;
        using const_pointer = const value_type*;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = typename Json::string_view_type;
        using path_node_less_type = path_node_less<Json,reference>;
        using path_node_equal_type = path_node_equal<Json,reference>;
        using parameter_type = parameter<Json>;
        using token_type = token<Json,reference>;
        using reference_arg_type = typename std::conditional<std::is_const<typename std::remove_reference<reference>::type>::value,
            const_reference_arg_t,reference_arg_t>::type;
        using path_component_type = path_component<char_type>;
        using stack_item_type = value_or_pointer<Json,JsonReference>;
    private:
        std::vector<token_type> token_list_;
    public:

        expression_tree()
        {
        }

        expression_tree(expression_tree&& expr)
            : token_list_(std::move(expr.token_list_))
        {
        }

        expression_tree(std::vector<token_type>&& token_stack)
            : token_list_(std::move(token_stack))
        {
        }

        expression_tree& operator=(expression_tree&& expr) = default;

        value_type evaluate_single(dynamic_resources<Json,reference>& resources, 
                                   reference root,
                                   reference current,
                                   result_options options,
                                   std::error_code& ec) const
        {
            std::vector<stack_item_type> stack;
            std::vector<parameter_type> arg_stack;
            std::vector<path_component_type> path = {path_component_type(current_node_arg)};

            //std::cout << "EVALUATE TOKENS\n";
            //for (auto& tok : token_list_)
            //{
            //    std::cout << tok.to_string() << "\n";
            //}
            //std::cout << "\n";

            if (!token_list_.empty())
            {
                for (auto& tok : token_list_)
                {
                    //std::cout << "Token: " << tok.to_string() << "\n";
                    switch (tok.type())
                    { 
                        case token_kind::literal:
                        {
                            stack.emplace_back(std::addressof(tok.get_value(reference_arg_type(), resources)));
                            break;
                        }
                        case token_kind::unary_operator:
                        {
                            JSONCONS_ASSERT(stack.size() >= 1);
                            auto item = std::move(stack.back());
                            stack.pop_back();

                            auto val = tok.unary_operator_->evaluate(item.value(), ec);
                            stack.emplace_back(std::move(val));
                            break;
                        }
                        case token_kind::binary_operator:
                        {
                            //std::cout << "binary operator: " << stack.size() << "\n";
                            JSONCONS_ASSERT(stack.size() >= 2);
                            auto rhs = std::move(stack.back());
                            //std::cout << "rhs: " << *rhs << "\n";
                            stack.pop_back();
                            auto lhs = std::move(stack.back());
                            //std::cout << "lhs: " << *lhs << "\n";
                            stack.pop_back();

                            auto val = tok.binary_operator_->evaluate(lhs.value(), rhs.value(), ec);
                            //std::cout << "Evaluate binary expression: " << r << "\n";
                            stack.emplace_back(std::move(val));
                            break;
                        }
                        case token_kind::root_node:
                            //std::cout << "root: " << root << "\n";
                            stack.emplace_back(std::addressof(root));
                            break;
                        case token_kind::current_node:
                            //std::cout << "current: " << current << "\n";
                            stack.emplace_back(std::addressof(current));
                            break;
                        case token_kind::argument:
                            JSONCONS_ASSERT(!stack.empty());
                            //std::cout << "argument stack items " << stack.size() << "\n";
                            //for (auto& item : stack)
                            //{
                            //    std::cout << *item.to_pointer(resources) << "\n";
                            //}
                            //std::cout << "\n";
                            arg_stack.emplace_back(std::move(stack.back()));
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";
                            stack.pop_back();
                            break;
                        case token_kind::function:
                        {
                            if (tok.function_->arity() && *(tok.function_->arity()) != arg_stack.size())
                            {
                                ec = jsonpath_errc::invalid_arity;
                                return Json::null();
                            }
                            //std::cout << "function arg stack:\n";
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";

                            value_type val = tok.function_->evaluate(arg_stack, ec);
                            if (ec)
                            {
                                return Json::null();
                            }
                            //std::cout << "function result: " << val << "\n";
                            arg_stack.clear();
                            stack.emplace_back(std::move(val));
                            break;
                        }
                        case token_kind::expression:
                        {
                            if (stack.empty())
                            {
                                stack.emplace_back(std::addressof(current));
                            }

                            auto item = std::move(stack.back());
                            stack.pop_back();
                            value_type val = tok.expression_->evaluate_single(resources, path, root, item.value(), options, ec);
                            //std::cout << "ref2: " << ref << "\n";
                            stack.emplace_back(std::move(val));
                            break;
                        }
                        case token_kind::selector:
                        {
                            if (stack.empty())
                            {
                                stack.emplace_back(std::addressof(current));
                            }

                            auto item = std::move(stack.back());
                            //for (auto& item : stack)
                            //{
                                //std::cout << "selector stack input:\n";
                                //switch (item.tag)
                                //{
                                //    case node_set_tag::single:
                                //        std::cout << "single: " << *(item.node.ptr) << "\n";
                                //        break;
                                //    case node_set_tag::multi:
                                //        for (auto& node : stack.back().ptr().nodes)
                                //        {
                                //            std::cout << "multi: " << *node.ptr << "\n";
                                //        }
                                //        break;
                                //    default:
                                //        break;
                            //}
                            //std::cout << "\n";
                            //}
                            //std::cout << "selector item: " << *ptr << "\n";
                            stack.pop_back();
                            std::vector<path_node_type> temp;
                            node_kind ndtype = node_kind();
                            tok.selector_->select(resources, path, root, item.value(), temp, ndtype, options);
                            
                            if ((options & result_options::sort) == result_options::sort)
                            {
                                std::sort(temp.begin(), temp.end(), path_node_less_type());
                            }

                            if ((options & result_options::nodups) == result_options::nodups)
                            {
                                if ((options & result_options::sort) == result_options::sort)
                                {
                                    auto last = std::unique(temp.begin(),temp.end(),path_node_equal_type());
                                    temp.erase(last,temp.end());
                                    stack.emplace_back(nodes_to_stack_item(temp, ndtype));
                                }
                                else
                                {
                                    std::vector<path_node_type> index(temp);
                                    std::sort(index.begin(), index.end(), path_node_less_type());
                                    auto last = std::unique(index.begin(),index.end(),path_node_equal_type());
                                    index.erase(last,index.end());

                                    std::vector<path_node_type> temp2;
                                    temp2.reserve(index.size());
                                    for (auto&& node : temp)
                                    {
                                        //std::cout << "node: " << node.path << ", " << *node.ptr << "\n";
                                        auto it = std::lower_bound(index.begin(),index.end(),node, path_node_less_type());

                                        if (it != index.end() && it->path == node.path) 
                                        {
                                            temp2.emplace_back(std::move(node));
                                            index.erase(it);
                                        }
                                    }
                                    stack.emplace_back(nodes_to_stack_item(temp2, ndtype));
                                }
                            }
                            else
                            {
                                //std::cout << "selector output " << temp.size() << "\n";
                                //for (auto& item : temp)
                                //{
                                //    std::cout << *item.ptr << "\n";
                                //}
                                //std::cout << "\n";
                                stack.emplace_back(nodes_to_stack_item(temp, ndtype));
                            }

                            
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            //if (stack.size() != 1)
            //{
            //    std::cout << "Stack size: " << stack.size() << "\n";
            //}
            return stack.empty() ? Json::null() : stack.back().value();
        }
 
        std::string to_string(int level) const
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("expression ");
            for (const auto& item : token_list_)
            {
                s.append(item.to_string(level+1));
            }

            return s;

        }
    private:
        static stack_item_type nodes_to_stack_item(std::vector<path_node_type>& nodes, node_kind tag)
        {
            if (nodes.empty())
            {
                return stack_item_type(Json(null_type()));
            }
            else if (nodes.size() == 1 && (tag == node_kind::single || tag == node_kind()))
            {
                return stack_item_type(nodes.back().ptr);
            }
            else
            {
                Json j(json_array_arg);
                j.reserve(nodes.size());
                for (auto& item : nodes)
                {
                    j.emplace_back(*item.ptr);
                }
                return stack_item_type(std::move(j));
            }
        }
    };

} // namespace detail
} // namespace jsonpath
} // namespace jsoncons

#endif // JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP
