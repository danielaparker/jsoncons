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
#include <jsoncons_ext/jsonpath/path_expression.hpp>

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
    class base_selector : public jsonpath_selector<Json,JsonReference>
    {
        using supertype = jsonpath_selector<Json,JsonReference>;

        std::unique_ptr<supertype> tail_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        base_selector()
            : supertype(true, 11), tail_()
        {
        }

        base_selector(bool is_path, std::size_t precedence_level)
            : supertype(is_path, precedence_level), tail_()
        {
        }

        void append_selector(std::unique_ptr<supertype>&& expr) override
        {
            if (!tail_)
            {
                tail_ = std::move(expr);
            }
            else
            {
                tail_->append_selector(std::move(expr));
            }
        }

        void evaluate_tail(dynamic_resources<Json,JsonReference>& resources,
                           const path_node_type& path, 
                           reference root,
                           reference current,
                           std::vector<path_value_pair_type>& nodes,
                           node_kind& ndtype,
                           result_options options) const
        {
            if (!tail_)
            {
                nodes.emplace_back(std::addressof(path), std::addressof(current));
            }
            else
            {
                tail_->select(resources, path, root, current, nodes, ndtype, options);
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

    public:
        using value_type = Json;
        using reference = JsonReference;
        using pointer = typename supertype::pointer;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using string_view_type = basic_string_view<char_type>;
    private:
        string_type identifier_;
    public:

        identifier_selector(const string_view_type& identifier)
            : base_selector<Json,JsonReference>(), identifier_(identifier)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
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
                    this->evaluate_tail(resources, generate_path(resources, path, identifier_, options), 
                                        root, it->value(), nodes, ndtype, options);
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
                        this->evaluate_tail(resources, generate_path(resources, path, index, options), 
                                            root, current[index], nodes, ndtype, options);
                    }
                }
                else if (identifier_ == length_name && current.size() > 0)
                {
                    pointer ptr = resources.create_json(current.size());
                    this->evaluate_tail(resources, generate_path(resources, path, identifier_, options), 
                                            root, *ptr, nodes, ndtype, options);
                }
            }
            else if (current.is_string() && identifier_ == length_name)
            {
                string_view_type sv = current.as_string_view();
                std::size_t count = unicode_traits::count_codepoints(sv.data(), sv.size());
                pointer ptr = resources.create_json(count);
                this->evaluate_tail(resources, generate_path(resources, path, identifier_, options), 
                                        root, *ptr, nodes, ndtype, options);
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

        std::size_t id_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        root_selector(std::size_t id)
            : base_selector<Json,JsonReference>(), id_(id)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference,
                    std::vector<path_value_pair_type>& nodes,
                    node_kind& ndtype,
                    result_options options) const override
        {
            if (resources.is_cached(id_))
            {
                resources.retrieve_from_cache(id_, nodes, ndtype);
            }
            else
            {
                std::vector<path_value_pair_type> v;
                this->evaluate_tail(resources, path, 
                                    root, root, v, ndtype, options);
                resources.add_to_cache(id_, v, ndtype);
                for (auto&& item : v)
                {
                    nodes.push_back(std::move(item));
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
        using supertype::generate_path;

        current_node_selector()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "current_node_selector: " << current << "\n";
            ndtype = node_kind::single;
            this->evaluate_tail(resources, path, 
                                root, current, nodes, ndtype, options);
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
        using supertype::generate_path;

        index_selector(int64_t index)
            : base_selector<Json,JsonReference>(), index_(index)
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
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
                    this->evaluate_tail(resources, generate_path(resources, path, i, options), 
                                        root, current.at(i), nodes, ndtype, options);
                }
                else 
                {
                    int64_t index = slen + index_;
                    if (index >= 0 && index < slen)
                    {
                        std::size_t i = static_cast<std::size_t>(index);
                        this->evaluate_tail(resources, generate_path(resources, path, index, options), 
                                            root, current.at(i), nodes, ndtype, options);
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
        using supertype::generate_path;

        wildcard_selector()
            : base_selector<Json,JsonReference>()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
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
                    this->evaluate_tail(resources, generate_path(resources, path, i, options), root, current[i], nodes, tmptype, options);
                }
            }
            else if (current.is_object())
            {
                for (auto& item : current.object_range())
                {
                    this->evaluate_tail(resources, generate_path(resources, path, item.key(), options), root, item.value(), nodes, tmptype, options);
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
        using supertype::generate_path;

        recursive_selector()
            : base_selector<Json,JsonReference>()
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "wildcard_selector: " << current << "\n";
            if (current.is_array())
            {
                this->evaluate_tail(resources, path, root, current, nodes, ndtype, options);
                for (std::size_t i = 0; i < current.size(); ++i)
                {
                    select(resources, generate_path(resources, path, i, options), root, current[i], nodes, ndtype, options);
                }
            }
            else if (current.is_object())
            {
                this->evaluate_tail(resources, path, root, current, nodes, ndtype, options);
                for (auto& item : current.object_range())
                {
                    select(resources, generate_path(resources, path, item.key(), options), root, item.value(), nodes, ndtype, options);
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
    class union_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using path_expression_type = path_expression<Json, JsonReference>;
        using supertype::generate_path;
    private:
        std::vector<path_expression_type> expressions_;
    public:
        union_selector(std::vector<path_expression_type>&& expressions)
            : base_selector<Json,JsonReference>(), expressions_(std::move(expressions))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current, 
                    std::vector<path_value_pair_type>& nodes,
                    node_kind& ndtype,
                    result_options options) const override
        {
            //std::cout << "union_selector select current: " << current << "\n";
            ndtype = node_kind::multi;

            auto callback = [&](const path_node_type& p, reference v)
            {
                //std::cout << "union select callback: node: " << *node.ptr << "\n";
                this->evaluate_tail(resources, p, root, v, nodes, ndtype, options);
            };
            for (auto& expr : expressions_)
            {
                expr.evaluate(resources, path, root, current, callback, options);
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
            for (auto& expr : expressions_)
            {
                s.append(expr.to_string(level+1));
                //s.push_back('\n');
            }

            return s;
        }
    };

    template <class Json,class JsonReference>
    class filter_expression_selector final : public base_selector<Json,JsonReference>
    {
        using supertype = base_selector<Json,JsonReference>;

        expression_tree<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        filter_expression_selector(expression_tree<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current, 
                    std::vector<path_value_pair_type>& nodes,
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
                        this->evaluate_tail(resources, path, root, current[i], nodes, ndtype, options);
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
                        this->evaluate_tail(resources, path, root, member.value(), nodes, ndtype, options);
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

        expression_tree<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        index_expression_selector(expression_tree<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current, 
                    std::vector<path_value_pair_type>& nodes,
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
                    this->evaluate_tail(resources, path, root, current.at(start), nodes, ndtype, options);
                }
                else if (j.is_string() && current.is_object())
                {
                    this->evaluate_tail(resources, path, root, current.at(j.as_string_view()), nodes, ndtype, options);
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

        expression_tree<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;

        argument_expression(expression_tree<Json,JsonReference>&& expr)
            : expr_(std::move(expr))
        {
        }

        value_type evaluate_single(dynamic_resources<Json,JsonReference>& resources,
                                   const path_node_type&, 
                                   reference root,
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

        slice slice_;
    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        slice_selector(const slice& slic)
            : base_selector<Json,JsonReference>(), slice_(slic) 
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current,
                    std::vector<path_value_pair_type>& nodes,
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
                        this->evaluate_tail(resources, generate_path(resources, path, j, options), root, current[j], nodes, ndtype, options);
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
                            this->evaluate_tail(resources, generate_path(resources, path,j,options), root, current[j], nodes, ndtype, options);
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

        expression_tree<Json,JsonReference> expr_;

    public:
        using value_type = Json;
        using reference = JsonReference;
        using path_value_pair_type = typename supertype::path_value_pair_type;
        using path_node_type = typename supertype::path_node_type;
        using supertype::generate_path;

        function_selector(expression_tree<Json,JsonReference>&& expr)
            : base_selector<Json,JsonReference>(), expr_(std::move(expr))
        {
        }

        void select(dynamic_resources<Json,JsonReference>& resources,
                    const path_node_type& path, 
                    reference root,
                    reference current, 
                    std::vector<path_value_pair_type>& nodes,
                    node_kind& ndtype,
                    result_options options) const override
        {
            ndtype = node_kind::single;
            std::error_code ec;
            value_type ref = expr_.evaluate_single(resources, root, current, options, ec);
            if (!ec)
            {
                this->evaluate_tail(resources, path, root, *resources.create_json(std::move(ref)), nodes, ndtype, options);
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
