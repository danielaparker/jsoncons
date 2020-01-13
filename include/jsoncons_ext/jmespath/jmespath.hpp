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

struct slice
{
    int64_t start_;
    optional<int64_t> end_;
    int64_t step_;

    slice()
        : start_(0), end_(), step_(1)
    {
    }

    slice(int64_t start, const optional<int64_t>& end, int64_t step) 
        : start_(start), end_(end), step_(step)
    {
    }

    slice(const slice& other)
        : start_(other.start_), end_(other.end_), step_(other.step_)
    {
    }

    slice& operator=(const slice&) = default;

    int64_t get_start(std::size_t size) const
    {
        return start_ >= 0 ? start_ : (static_cast<int64_t>(size) - start_);
    }

    int64_t get_end(std::size_t size) const
    {
        if (end_)
        {
            auto len = end_.value() >= 0 ? end_.value() : (static_cast<int64_t>(size) - end_.value());
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

    quoted_string,
    raw_string,
    json_value,
    unquoted_string,
    expression1,
    expression2,
    expression3,
    identifier_expression1,
    index_expression1,
    number,
    digit,
    bracket_specifier9,
    bracket_specifier,
    bracket_specifier2,
    bracket_specifier3,
    bracket_specifier4,
    expect_dot,
    expect_right_bracket,
    comparator_expression,
    comparator,
    cmp_lt_or_lte,
    cmp_eq,
    cmp_gt_or_gte,
    cmp_ne,
    multiselect_list
};

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

    class temp_storage
    {
        std::vector<std::unique_ptr<Json>> temp_storage_;
    public:

        template <typename... Args>
        Json* operator()(Args&& ... args)
        {
            auto temp = make_unique_ptr<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_storage_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    class selector_base
    {
    public:
        virtual ~selector_base()
        {
        }

        virtual void add_selector(std::unique_ptr<selector_base>&&) = 0;

        virtual reference select(reference val, temp_storage&) = 0;
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

        reference select(reference val, temp_storage& make_temp) override
        {
            pointer ptr = std::addressof(val);
            for (auto& selector : selectors_)
            {
                ptr = std::addressof(selector->select(*ptr, make_temp));
            }
            return *ptr;
        }
    };

    class list_projection_selector final : public selector_base
    {
    public:
        std::unique_ptr<selector_base> lhs_selector_;
        std::vector<std::unique_ptr<selector_base>> rhs_selectors_;

        list_projection_selector(std::unique_ptr<selector_base>&& lhs_selector)
        {
            lhs_selector_ = std::move(lhs_selector);
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            rhs_selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};
            auto j = lhs_selector_->select(val, make_temp);
            if (!j.is_array())
            {
                return null;
            }

            auto resultp = make_temp(json_array_arg);
            for (reference item : j.array_range())
            {
                pointer ptr = std::addressof(item);
                for (auto& selector : rhs_selectors_)
                {
                    ptr = std::addressof(selector->select(*ptr, make_temp));
                }
                if (!ptr->is_null())
                {
                    resultp->push_back(*ptr);
                }
            }
            return *resultp;
        }
    };

    class flatten_selector final : public selector_base
    {
    public:
        std::unique_ptr<selector_base> lhs_selector_;
        std::vector<std::unique_ptr<selector_base>> rhs_selectors_;

        flatten_selector(std::unique_ptr<selector_base>&& lhs_selector)
        {
            lhs_selector_ = std::move(lhs_selector);
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            rhs_selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};
            auto j = lhs_selector_->select(val, make_temp);
            if (!j.is_array())
            {
                return null;
            }

            auto currentp = make_temp(json_array_arg);
            for (reference item : j.array_range())
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

            auto resultp = make_temp(json_array_arg);
            for (reference item : currentp->array_range())
            {
                pointer ptr = std::addressof(item);

                for (auto& selector : rhs_selectors_)
                {
                    ptr = std::addressof(selector->select(*ptr, make_temp));
                }
                if (!ptr->is_null())
                {
                    resultp->push_back(*ptr);
                }
            }
            return *resultp;
        }
    };

    class object_projection_selector final : public selector_base
    {
    public:
        std::unique_ptr<selector_base> lhs_selector_;
        std::vector<std::unique_ptr<selector_base>> rhs_selectors_;

        object_projection_selector(std::unique_ptr<selector_base>&& lhs_selector)
        {
            lhs_selector_ = std::move(lhs_selector);
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            rhs_selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};
            auto j = lhs_selector_->select(val, make_temp);
            if (!j.is_object())
            {
                return null;
            }

            auto resultp = make_temp(json_array_arg);
            for (auto& item : j.object_range())
            {
                pointer ptr = std::addressof(item.value());
                for (auto& selector : rhs_selectors_)
                {
                    ptr = std::addressof(selector->select(*ptr, make_temp));
                }
                if (!ptr->is_null())
                {
                    resultp->push_back(*ptr);
                }
            }
            return *resultp;
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

        void add_selector(std::unique_ptr<selector_base>&&) override
        {
            // Error
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};

            if (val.is_object() && val.contains(identifier_))
            {
                return val.at(identifier_);
            }
            else if (val.is_array())
            {
                auto resultp = make_temp(json_array_arg);
                for (const auto& item : val.array_range())
                {
                    if (item.is_object() && item.contains(identifier_))
                    {
                        resultp->push_back(item.at(identifier_));
                    }
                }
                return *resultp;
            }
            return null;
        }
    };

    class json_value_selector final : public selector_base
    {
    private:
        Json j_;
    public:
        json_value_selector(const Json& j)
            : j_(j)
        {
        }
        json_value_selector(Json&& j)
            : j_(std::move(j))
        {
        }

        void add_selector(std::unique_ptr<selector_base>&&) override
        {
            // noop
        }

        reference select(reference, temp_storage&) override
        {
            return j_;
        }
    };

    class index_selector final : public selector_base
    {
    private:
        int64_t index_;
    public:
        index_selector(int64_t index)
            : index_(index)
        {
        }

        void add_selector(std::unique_ptr<selector_base>&&) 
        {
            // noop
        }

        reference select(reference val, temp_storage&) override
        {
            static Json null{null_type()};

            if (val.is_array())
            {
                int64_t slen = static_cast<int64_t>(val.size());
                if (index_ >= 0 && index_ < slen)
                {
                    std::size_t index = static_cast<std::size_t>(index_);
                    return val.at(index);
                }
                else if (index_ < 0 && (slen+index_) < slen)
                {
                    std::size_t index = static_cast<std::size_t>(slen + index_);
                    return val.at(index);
                }
                else
                {
                    return null;
                }
            }
            return null;
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

        void add_selector(std::unique_ptr<selector_base>&&) override
        {
            // Error
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};

            if (val.is_array())
            {
                auto resultp = make_temp(json_array_arg);

                auto start = slice_.get_start(val.size());
                auto end = slice_.get_end(val.size());
                auto step = slice_.step();
                if (step >= 0)
                {
                    for (int64_t j = start; j < end; j += step)
                    {
                        resultp->emplace_back(val[static_cast<std::size_t>(j)]);
                    }
                    return *resultp; 
                }
                else
                {
                    for (int64_t j = end-1; j >= start; j += step)
                    {
                        resultp->emplace_back(val[static_cast<std::size_t>(j)]);
                    }
                    return *resultp; 
                }
            }
            else
            {
                return null;
            }
        }
    };

    struct cmp_eq
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return lhs == rhs ? true : false;
        }
    };

    struct cmp_lt
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return (lhs.is_number() && rhs.is_number()) ? (lhs < rhs? true : false) : optional<bool>();
        }
    };

    struct cmp_lte
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return (lhs.is_number() && rhs.is_number()) ? (lhs <= rhs? true : false) : optional<bool>();
        }
    };

    struct cmp_gt
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return (lhs.is_number() && rhs.is_number()) ? (lhs > rhs? true : false) : optional<bool>();
        }
    };

    struct cmp_gte
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return (lhs.is_number() && rhs.is_number()) ? (lhs >= rhs? true : false) : optional<bool>();
        }
    };

    struct cmp_ne
    {
        optional<bool> operator()(const Json& lhs, const Json& rhs)
        {
            return lhs != rhs ? true : false;
        }
    };

    template <typename Comparator>
    class filter_selector final : public selector_base
    {
    public:
        std::unique_ptr<selector_base> lhs_selector_;
        std::vector<std::unique_ptr<selector_base>> rhs_selectors_;
        Comparator cmp_;

        filter_selector(std::unique_ptr<selector_base>&& lhs_selector)
        {
            lhs_selector_ = std::move(lhs_selector);
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            rhs_selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};

            if (val.is_array())
            {
                auto resultp = make_temp(json_array_arg);
                for (auto& item : val.array_range())
                {
                    auto lhs = lhs_selector_->select(item, make_temp);

                    pointer rhsp = std::addressof(item);
                    for (auto& selector : rhs_selectors_)
                    {
                        rhsp = std::addressof(selector->select(*rhsp, make_temp));
                    }
                    auto r = cmp_(lhs,*rhsp);
                    if (r && r.value())
                    {
                        resultp->emplace_back(item);
                    }
                }
                return *resultp;
            }

            return null;
        }
    };

    class multiselect_list_selector final : public selector_base
    {
    public:
        std::vector<std::unique_ptr<selector_base>> selectors_;

        multiselect_list_selector()
        {
        }

        void add_selector(std::unique_ptr<selector_base>&& rhs_selectors) 
        {
            selectors_.emplace_back(std::move(rhs_selectors));
        }

        reference select(reference val, temp_storage& make_temp) override
        {
            static Json null{null_type()};
            if (!val.is_array())
            {
                return null;
            }

            auto resultp = make_temp(json_array_arg);
            for (reference item : val.array_range())
            {
                for (auto& selector : selectors_)
                {
                    auto ptr = std::addressof(selector->select(item, make_temp));
                    if (!ptr->is_null())
                    {
                        resultp->push_back(*ptr);
                    }
                }
            }
            return *resultp;
        }
    };

    std::size_t line_;
    std::size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;

    std::vector<path_state> state_stack_;
    std::vector<std::unique_ptr<selector_base>> selector_stack_;
    temp_storage temp_factory_;

public:
    jmespath_evaluator()
        : line_(1), column_(1),
          begin_input_(nullptr), end_input_(nullptr),
          p_(nullptr)
    {
        selector_stack_.emplace_back(make_unique_ptr<sub_expression_selector>());
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

        slice a_slice;

        while (p_ < end_input_)
        {
            switch (state_stack_.back())
            {
                case path_state::start: 
                {
                    state_stack_.back() = path_state::expression1;
                    break;
                }
                case path_state::expression1: 
                {
                    switch (*p_)
                    {
                        case '\"':
                            state_stack_.emplace_back(path_state::quoted_string);
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.emplace_back(path_state::raw_string);
                            ++p_;
                            ++column_;
                            break;
                        case '`':
                            state_stack_.emplace_back(path_state::json_value);
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.back() = path_state::index_expression1;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            ++p_;
                            ++column_;
                            break;
                        case '*':
                            selector_stack_.back() = make_unique_ptr<object_projection_selector>(std::move(selector_stack_.back()));
                            state_stack_.emplace_back(path_state::expect_dot);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                state_stack_.back() = path_state::identifier_expression1;
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
                            selector_stack_.back()->add_selector(make_unique_ptr<identifier_selector>(buffer));
                            buffer.clear();
                            state_stack_.pop_back(); // unquoted_string
                            advance_past_space_character();
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
                                selector_stack_.back()->add_selector(make_unique_ptr<identifier_selector>(buffer));
                                state_stack_.pop_back(); // unquoted_string
                                buffer.clear();
                            }
                            break;
                    };
                    break;
                case path_state::raw_string: 
                    switch (*p_)
                    {
                        case '\'':
                        {
                            selector_stack_.back()->add_selector(make_unique_ptr<json_value_selector>(Json(buffer)));
                            buffer.clear();
                            state_stack_.pop_back(); // raw_string
                            ++p_;
                            ++column_;
                            break;
                        }
                        case '\\':
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
                case path_state::json_value: 
                    switch (*p_)
                    {
                        case '\'':
                        {
                            auto j = Json::parse(buffer);
                            selector_stack_.back()->add_selector(make_unique_ptr<json_value_selector>(std::move(j)));
                            buffer.clear();
                            state_stack_.pop_back(); // raw_string
                            break;
                        }
                        case '\\':
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
                case path_state::index_expression1:
                    switch(*p_)
                    {
                        case '.':
                            ++p_;
                            ++column_;
                            state_stack_.back() = path_state::expression1;
                            break;
                        case '[':
                            state_stack_.back() = path_state::expression1;
                            break;
                        default:
                            ec = jmespath_errc::expected_index;
                            return result;
                    }
                    break;
                case path_state::identifier_expression1:
                    switch(*p_)
                    {
                        case '.':
                            ++p_;
                            ++column_;
                            state_stack_.back() = path_state::expression1;
                            break;
                        case '[':
                            state_stack_.back() = path_state::expression1;
                            break;
                        default:
                            ec = jmespath_errc::expected_index;
                            return result;
                    }
                    break;

                case path_state::bracket_specifier:
                    switch(*p_)
                    {
                        case '*':
                            selector_stack_.back() = make_unique_ptr<list_projection_selector>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::bracket_specifier4;
                            ++p_;
                            ++column_;
                            break;
                        case ']':
                            selector_stack_.back() = make_unique_ptr<flatten_selector>(std::move(selector_stack_.back()));
                            state_stack_.pop_back(); // bracket_specifier
                            ++p_;
                            ++column_;
                            break;
                        case '?':
                            selector_stack_.emplace_back(make_unique_ptr<sub_expression_selector>());
                            state_stack_.back() = path_state::expression2;
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                            state_stack_.back() = path_state::bracket_specifier2;
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                            state_stack_.back() = path_state::bracket_specifier9;
                            state_stack_.emplace_back(path_state::number);
                            break;
                        default:
                            selector_stack_.emplace_back(make_unique_ptr<multiselect_list_selector>());
                            state_stack_.back() = path_state::bracket_specifier9;
                            state_stack_.emplace_back(path_state::expression3);
                            break;
                    }
                    break;

                case path_state::bracket_specifier9:
                    switch(*p_)
                    {
                        case ']':
                        {
                            if (buffer.empty())
                            {
                                selector_stack_.back() = make_unique_ptr<flatten_selector>(std::move(selector_stack_.back()));
                            }
                            else
                            {
                                auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                if (!r)
                                {
                                    ec = jmespath_errc::invalid_number;
                                    return result;
                                }
                                selector_stack_.back()->add_selector(make_unique_ptr<index_selector>(r.value()));
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
                                    return result;
                                }
                                a_slice.start_ = r.value();
                                buffer.clear();
                            }
                            state_stack_.back() = path_state::bracket_specifier2;
                            state_stack_.emplace_back(path_state::number);
                            ++p_;
                            ++column_;
                            break;
                        }
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
                        a_slice.end_ = optional<int64_t>(r.value());
                    }
                    switch(*p_)
                    {
                        case ']':
                            selector_stack_.back()->add_selector(make_unique_ptr<slice_selector>(a_slice));
                            buffer.clear();
                            a_slice = slice();
                            state_stack_.pop_back(); // bracket_specifier2
                            ++p_;
                            ++column_;
                            break;
                        case ':':
                            state_stack_.back() = path_state::bracket_specifier3;
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
                        a_slice.step_ = r.value();
                        buffer.clear();
                    }
                    switch(*p_)
                    {
                        case ']':
                            selector_stack_.back()->add_selector(make_unique_ptr<slice_selector>(a_slice));
                            buffer.clear();
                            a_slice = slice();
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
                case path_state::expression2: 
                {
                    switch (*p_)
                    {
                        case '\"':
                            state_stack_.back() = path_state::comparator;
                            state_stack_.emplace_back(path_state::quoted_string);
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.back() = path_state::comparator;
                            state_stack_.emplace_back(path_state::raw_string);
                            ++p_;
                            ++column_;
                            break;
                        case '`':
                            state_stack_.back() = path_state::comparator;
                            state_stack_.emplace_back(path_state::json_value);
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.back() = path_state::comparator;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            ++p_;
                            ++column_;
                            break;
                        case '*':
                            state_stack_.back() = path_state::comparator;
                            selector_stack_.back() = make_unique_ptr<object_projection_selector>(std::move(selector_stack_.back()));
                            state_stack_.emplace_back(path_state::expect_dot);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                state_stack_.back() = path_state::comparator;
                                state_stack_.emplace_back(path_state::unquoted_string);
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                            }
                            else
                            {
                                selector_stack_.back()->add_selector(make_unique_ptr<identifier_selector>(buffer));
                                state_stack_.pop_back(); // expression2
                                buffer.clear();
                            }
                            break;
                    };
                    break;
                }
                case path_state::expression3: 
                {
                    switch (*p_)
                    {
                        case '\"':
                            state_stack_.back() = path_state::expect_right_bracket;
                            state_stack_.emplace_back(path_state::quoted_string);
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            state_stack_.back() = path_state::expect_right_bracket;
                            state_stack_.emplace_back(path_state::raw_string);
                            ++p_;
                            ++column_;
                            break;
                        case '`':
                            state_stack_.back() = path_state::expect_right_bracket;
                            state_stack_.emplace_back(path_state::json_value);
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            state_stack_.back() = path_state::expect_right_bracket;
                            state_stack_.emplace_back(path_state::bracket_specifier);
                            ++p_;
                            ++column_;
                            break;
                        case '*':
                            state_stack_.back() = path_state::expect_right_bracket;
                            selector_stack_.back() = make_unique_ptr<object_projection_selector>(std::move(selector_stack_.back()));
                            state_stack_.emplace_back(path_state::expect_dot);
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                            {
                                state_stack_.back() = path_state::expect_right_bracket;
                                state_stack_.emplace_back(path_state::unquoted_string);
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                            }
                            else
                            {
                                selector_stack_.back()->add_selector(make_unique_ptr<identifier_selector>(buffer));
                                state_stack_.pop_back(); // expression3
                                buffer.clear();
                            }
                            break;
                    };
                    break;
                }
                case path_state::comparator:
                {
                    switch(*p_)
                    {
                        case '<':
                            state_stack_.back() = path_state::cmp_lt_or_lte;
                            ++p_;
                            ++column_;
                            break;
                        case '=':
                            state_stack_.back() = path_state::cmp_eq;
                            ++p_;
                            ++column_;
                            break;
                        case '>':
                            state_stack_.back() = path_state::cmp_gt_or_gte;
                            ++p_;
                            ++column_;
                            break;
                        case '!':
                            state_stack_.back() = path_state::cmp_ne;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_comparator;
                            return result;
                    }
                    break;
                }
                case path_state::cmp_lt_or_lte:
                {
                    switch(*p_)
                    {
                        case '=':
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_lte>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_lt>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            break;
                    }
                    break;
                }
                case path_state::cmp_eq:
                {
                    switch(*p_)
                    {
                        case '=':
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_eq>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_comparator;
                            return result;
                    }
                    break;
                }
                case path_state::cmp_gt_or_gte:
                {
                    switch(*p_)
                    {
                        case '=':
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_gte>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_gt>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            break;
                    }
                    break;
                }
                case path_state::cmp_ne:
                {
                    switch(*p_)
                    {
                        case '=':
                            selector_stack_.back() = make_unique_ptr<filter_selector<cmp_ne>>(std::move(selector_stack_.back()));
                            state_stack_.back() = path_state::expression3;
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_comparator;
                            return result;
                    }
                    break;
                }
                case path_state::expect_dot:
                {
                    switch(*p_)
                    {
                        case '.':
                            state_stack_.pop_back(); // expect_dot
                            ++p_;
                            ++column_;
                            break;
                        default:
                            ec = jmespath_errc::expected_dot;
                            return result;
                    }
                    break;
                }
                case path_state::expect_right_bracket:
                {
                    switch(*p_)
                    {
                        case ',':
                            state_stack_.back() = path_state::expression3; 
                            ++p_;
                            ++column_;
                            break;
                        case '.':
                            state_stack_.back() = path_state::expression3; 
                            ++p_;
                            ++column_;
                            break;
                        case ']':
                        {
                            state_stack_.pop_back();

                            auto tmp = std::move(selector_stack_.back());
                            selector_stack_.pop_back();

                            auto p = make_unique_ptr<sub_expression_selector>();
                            p->add_selector(std::move(selector_stack_.back()));
                            p->add_selector(std::move(tmp));
                            selector_stack_.back() = std::move(p);
                            ++p_;
                            ++column_;
                            break;
                        }
                        default:
                            ec = jmespath_errc::expected_right_bracket;
                            return result;
                    }
                    break;
                }
            }
        }

        switch (state_stack_.back())
        {
            case path_state::unquoted_string: 
            {
                selector_stack_.back()->add_selector(make_unique_ptr<identifier_selector>(buffer));
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
        JSONCONS_ASSERT(state_stack_.back() == path_state::expression1 ||
                        state_stack_.back() == path_state::index_expression1 ||
                        state_stack_.back() == path_state::identifier_expression1);
        state_stack_.pop_back();

        reference r = selector_stack_.back()->select(root, temp_factory_); 
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
