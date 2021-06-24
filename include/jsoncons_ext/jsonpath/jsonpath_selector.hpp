// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_SELECTOR_HPP
#define JSONCONS_JSONPATH_JSONPATH_SELECTOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::is_const
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <regex>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath/expression.hpp>

namespace jsoncons { 
namespace jsonpath {
namespace detail {

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

    template <class Json,class JsonReference>
    struct path_generator
    {
        using char_type = typename Json::char_type;
        using path_node_type = path_node<char_type>;
        using string_type = std::basic_string<char_type>;

        static const path_node_type& generate(dynamic_resources<Json,JsonReference>& resources,
                                              const path_node_type& path, 
                                              std::size_t index, 
                                              result_options options) 
        {
            const result_options require_path = result_options::path | result_options::nodups | result_options::sort;
            if ((options & require_path) != result_options())
            {
                return *resources.new_path_node(&path, index);
            }
            else
            {
                return path;
            }
        }

        static const path_node_type& generate(dynamic_resources<Json,JsonReference>& resources,
                                              const path_node_type& path, 
                                              const string_type& identifier, 
                                              result_options options) 
        {
            const result_options require_path = result_options::path | result_options::nodups | result_options::sort;
            if ((options & require_path) != result_options())
            {
                return *resources.new_path_node(&path, identifier);
            }
            else
            {
                return path;
            }
        }
    };

    template <class Json,class JsonReference>
    class base_selector : public jsonpath_selector<Json,JsonReference>
    {
        using supertype = jsonpath_selector<Json,JsonReference>;

        supertype* tail_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using normalized_path_type = typename supertype::normalized_path_type;
        using node_accumulator_type = typename supertype::node_accumulator_type;
        using selector_type = typename supertype::selector_type;

        base_selector()
            : supertype(true, 11), tail_(nullptr)
        {
        }

        base_selector(bool is_path, std::size_t precedence_level)
            : supertype(is_path, precedence_level), tail_(nullptr)
        {
        }

        void append_selector(selector_type* expr) override
        {
            if (!tail_)
            {
                tail_ = expr;
            }
            else
            {
                tail_->append_selector(expr);
            }
        }

        void evaluate_tail(dynamic_resources<Json,JsonReference>& resources,
                           reference root,
                           const path_node_type& path, 
                           reference current,
                           node_accumulator_type& accumulator,
                           node_kind& ndtype,
                           result_options options) const
        {
            if (!tail_)
            {
                accumulator.accumulate(path, current);
            }
            else
            {
                tail_->select(resources, root, path, current, accumulator, ndtype, options);
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
            if (tail_)
            {
                s.append(tail_->to_string(level));
            }
            return s;
        }
    };

    template <class Json,class JsonReference>
    class identifier_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;
        using path_generator_type = path_generator<Json,JsonReference>;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename supertype::pointer;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using string_view_type = basic_string_view<char_type>;
        using node_accumulator_type = typename supertype::node_accumulator_type;
    private:
        string_type identifier_;
    public:

        identifier_selector(const string_view_type& identifier)
            : base_selector<Json,JsonReference>(), identifier_(identifier)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::string buf;
            //buf.append("identifier selector: ");
            //unicode_traits::convert(identifier_.data(),identifier_.size(),buf);

            static const char_type length_name[] = {'l', 'e', 'n', 'g', 't', 'h', 0};

            ndtype = node_kind::single;
            if (current.is_object())
            {
                auto it = current.find(identifier_);
                if (it != current.object_range().end())
                {
                    this->evaluate_tail(resources, root, 
                                        path_generator_type::generate(resources, path, identifier_, options),
                                        it->value(), accumulator, ndtype, options);
                }
            }
            else if (current.is_array())
            {
                int64_t n{0};
                auto r = jsoncons::detail::to_integer_decimal(identifier_.data(), identifier_.size(), n);
                if (r)
                {
                    std::size_t index = (n >= 0) ? static_cast<std::size_t>(n) : static_cast<std::size_t>(static_cast<int64_t>(current.size()) + n);
                    if (index < current.size())
                    {
                        this->evaluate_tail(resources, root, 
                                            path_generator_type::generate(resources, path, index, options),
                                            current[index], accumulator, ndtype, options);
                    }
                }
                else if (identifier_ == length_name && current.size() > 0)
                {
                    pointer ptr = resources.new_json(current.size());
                    this->evaluate_tail(resources, root, 
                                        path_generator_type::generate(resources, path, identifier_, options), 
                                        *ptr, 
                                        accumulator, ndtype, options);
                }
            }
            else if (current.is_string() && identifier_ == length_name)
            {
                string_view_type sv = current.as_string_view();
                std::size_t count = unicode_traits::count_codepoints(sv.data(), sv.size());
                pointer ptr = resources.new_json(count);
                this->evaluate_tail(resources, root, 
                                    path_generator_type::generate(resources, path, identifier_, options), 
                                    *ptr, accumulator, ndtype, options);
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
            s.append("identifier selector ");
            unicode_traits::convert(identifier_.data(),identifier_.size(),s);
            s.append(base_selector<Json,JsonReference>::to_string(level+1));
            //s.append("\n");

            return s;
        }
    };

    template <class Json,class JsonReference>
    class root_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;
        using path_generator_type = path_generator<Json,JsonReference>;

        std::size_t id_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        root_selector(std::size_t id)
            : base_selector<Json,JsonReference>(), id_(id)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            if (resources.is_cached(id_))
            {
                resources.retrieve_from_cache(id_, accumulator, ndtype);
            }
            else
            {
                path_stem_value_accumulator<Json,JsonReference> accum;

                this->evaluate_tail(resources, root, path, root, accum, ndtype, options);
                resources.add_to_cache(id_, std::move(accum.nodes), ndtype);
                resources.retrieve_from_cache(id_, accumulator, ndtype);
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
            s.append("root_selector ");
            s.append(base_selector<Json,JsonReference>::to_string(level+1));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class current_node_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        current_node_selector()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "current_node_selector: " << current << "\n";
            ndtype = node_kind::single;
            this->evaluate_tail(resources,  
                                root, path, current, accumulator, ndtype, options);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("current_node_selector");
            s.append(base_selector<Json,JsonReference>::to_string(level+1));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class index_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

        int64_t index_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        index_selector(int64_t index)
            : base_selector<Json,JsonReference>(), index_(index)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            ndtype = node_kind::single;
            if (current.is_array())
            {
                int64_t slen = static_cast<int64_t>(current.size());
                if (index_ >= 0 && index_ < slen)
                {
                    std::size_t i = static_cast<std::size_t>(index_);
                    this->evaluate_tail(resources, root, 
                                        path_generator_type::generate(resources, path, i, options), 
                                        current.at(i), accumulator, ndtype, options);
                }
                else 
                {
                    int64_t index = slen + index_;
                    if (index >= 0 && index < slen)
                    {
                        std::size_t i = static_cast<std::size_t>(index);
                        this->evaluate_tail(resources, root, 
                                            path_generator_type::generate(resources, path, index, options), 
                                            current.at(i), accumulator, ndtype, options);
                    }
                }
            }
        }
    };

    template <class Json,class JsonReference>
    class wildcard_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        wildcard_selector()
            : base_selector<Json,JsonReference>()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "wildcard_selector: " << current << "\n";
            ndtype = node_kind::multi; // always multi

            node_kind tmptype;
            if (current.is_array())
            {
                for (std::size_t i = 0; i < current.size(); ++i)
                {
                    this->evaluate_tail(resources, root, 
                                        path_generator_type::generate(resources, path, i, options), current[i], 
                                        accumulator, tmptype, options);
                }
            }
            else if (current.is_object())
            {
                for (auto& item : current.object_range())
                {
                    this->evaluate_tail(resources, root, 
                                        path_generator_type::generate(resources, path, item.key(), options), item.value(), accumulator, tmptype, options);
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
            s.append("wildcard selector");
            s.append(base_selector<Json,JsonReference>::to_string(level));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class recursive_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        recursive_selector()
            : base_selector<Json,JsonReference>()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "wildcard_selector: " << current << "\n";
            if (current.is_array())
            {
                this->evaluate_tail(resources, root, path, current, accumulator, ndtype, options);
                for (std::size_t i = 0; i < current.size(); ++i)
                {
                    select(resources, root, 
                           path_generator_type::generate(resources, path, i, options), current[i], accumulator, ndtype, options);
                }
            }
            else if (current.is_object())
            {
                this->evaluate_tail(resources, root, path, current, accumulator, ndtype, options);
                for (auto& item : current.object_range())
                {
                    select(resources, root, 
                           path_generator_type::generate(resources, path, item.key(), options), item.value(), accumulator, ndtype, options);
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
            s.append("wildcard selector");
            s.append(base_selector<Json,JsonReference>::to_string(level));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class union_selector final : public jsonpath_selector<Json,JsonReference>
    {
        using supertype = jsonpath_selector<Json,JsonReference>;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using normalized_path_type = typename supertype::normalized_path_type;
        using path_expression_type = path_expression<Json, JsonReference>;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;
        using selector_type = typename supertype::selector_type;
    private:
        std::vector<selector_type*> selectors_;
    public:
        union_selector(std::vector<selector_type*>&& selectors)
            : supertype(true, 11), selectors_(std::move(selectors))
        {
        }

        void append_selector(selector_type* tail) override
        {
            for (auto& selector : selectors_)
            {
                selector->append_selector(tail);
            }
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current, 
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "union_selector select current: " << current << "\n";
            ndtype = node_kind::multi;

            for (auto& selector : selectors_)
            {
                selector->select(resources, root, path, current, accumulator, ndtype, options);
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
            s.append("union selector ");
            for (auto& selector : selectors_)
            {
                s.append(selector->to_string(level+1));
                //s.push_back('\n');
            }

            return s;
        }
    };

    template <class Json,class JsonReference>
    class filter_expression_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

        expression<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        filter_expression_selector(expression<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current, 
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            if (current.is_array())
            {
                for (std::size_t i = 0; i < current.size(); ++i)
                {
                    std::error_code ec;
                    value_type r = expr_.evaluate_single(resources, root, current[i], options, ec);
                    bool t = ec ? false : detail::is_true(r);
                    if (t)
                    {
                        this->evaluate_tail(resources, root, path, current[i], accumulator, ndtype, options);
                    }
                }
            }
            else if (current.is_object())
            {
                for (auto& member : current.object_range())
                {
                    std::error_code ec;
                    value_type r = expr_.evaluate_single(resources, root, member.value(), options, ec);
                    bool t = ec ? false : detail::is_true(r);
                    if (t)
                    {
                        this->evaluate_tail(resources, root, path, member.value(), accumulator, ndtype, options);
                    }
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
            s.append("filter selector ");
            s.append(expr_.to_string(level+1));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class index_expression_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

        expression<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        index_expression_selector(expression<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current, 
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "index_expression_selector current: " << current << "\n";

            std::error_code ec;
            value_type j = expr_.evaluate_single(resources, root, current, options, ec);

            if (!ec)
            {
                if (j.template is<std::size_t>() && current.is_array())
                {
                    std::size_t start = j.template as<std::size_t>();
                    this->evaluate_tail(resources, root, path, current.at(start), accumulator, ndtype, options);
                }
                else if (j.is_string() && current.is_object())
                {
                    this->evaluate_tail(resources, root, path, current.at(j.as_string_view()), accumulator, ndtype, options);
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
            s.append("bracket expression selector ");
            s.append(expr_.to_string(level+1));
            s.append(base_selector<Json,JsonReference>::to_string(level+1));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class argument_expression final : public expression_base<Json,JsonReference>
    {
        using supertype = expression_base<Json,JsonReference>;

        expression<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using node_accumulator_type = node_accumulator<Json,JsonReference>;

        argument_expression(expression<Json,JsonReference>&& expr)
            : expr_(std::move(expr))
        {
        }

        value_type evaluate_single(dynamic_resources<Json,JsonReference>& resources,
                                   reference root,
                                   const path_node_type&, 
                                   reference current, 
                                   result_options options,
                                   std::error_code& ec) const override
        {
            value_type ref = expr_.evaluate_single(resources, root, current, options, ec);
            return ec ? Json::null() : ref; 
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("expression selector ");
            s.append(expr_.to_string(level+1));

            return s;
        }
    };

    template <class Json,class JsonReference>
    class slice_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;
        using path_generator_type = path_generator<Json, JsonReference>;

        slice slice_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        slice_selector(const slice& slic)
            : base_selector<Json,JsonReference>(), slice_(slic) 
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current,
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            ndtype = node_kind::multi;

            if (current.is_array())
            {
                auto start = slice_.get_start(current.size());
                auto end = slice_.get_stop(current.size());
                auto step = slice_.step();

                if (step > 0)
                {
                    if (start < 0)
                    {
                        start = 0;
                    }
                    if (end > static_cast<int64_t>(current.size()))
                    {
                        end = current.size();
                    }
                    for (int64_t i = start; i < end; i += step)
                    {
                        std::size_t j = static_cast<std::size_t>(i);
                        this->evaluate_tail(resources, root, 
                                            path_generator_type::generate(resources, path, j, options), 
                                            current[j], accumulator, ndtype, options);
                    }
                }
                else if (step < 0)
                {
                    if (start >= static_cast<int64_t>(current.size()))
                    {
                        start = static_cast<int64_t>(current.size()) - 1;
                    }
                    if (end < -1)
                    {
                        end = -1;
                    }
                    for (int64_t i = start; i > end; i += step)
                    {
                        std::size_t j = static_cast<std::size_t>(i);
                        if (j < current.size())
                        {
                            this->evaluate_tail(resources, root, 
                                                path_generator_type::generate(resources, path,j,options), current[j], accumulator, ndtype, options);
                        }
                    }
                }
            }
        }
    };

    template <class Json,class JsonReference>
    class function_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

        expression<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_generator_type = path_generator<Json,JsonReference>;
        using node_accumulator_type = typename supertype::node_accumulator_type;

        function_selector(expression<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    reference root,
                    const path_node_type& path, 
                    reference current, 
                    node_accumulator_type& accumulator,
                    node_kind& ndtype,
                    result_options options) const override
        {
            ndtype = node_kind::single;
            std::error_code ec;
            value_type ref = expr_.evaluate_single(resources, root, current, options, ec);
            if (!ec)
            {
                this->evaluate_tail(resources, root, path, *resources.new_json(std::move(ref)), accumulator, ndtype, options);
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
            s.append("function_selector ");
            s.append(expr_.to_string(level+1));

            return s;
        }
    };

} // namespace detail
} // namespace jsonpath
} // namespace jsoncons

#endif
