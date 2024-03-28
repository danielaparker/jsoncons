// Copyright 2013-2024 Daniel Parker
// 
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_KEYWORD_VALIDATORS_HPP
#define JSONCONS_JSONSCHEMA_COMMON_KEYWORD_VALIDATORS_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/common/format_validator.hpp>
#include <jsoncons_ext/jsonschema/common/validator.hpp>
#include <jsoncons_ext/jsonschema/common/uri_wrapper.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#include <unordered_set>
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif

namespace jsoncons {
namespace jsonschema {

    template<class Json>
    class recursive_ref_validator : public keyword_validator_base<Json>, public virtual ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        const schema_validator<Json> *tentative_target_; 

    public:
        recursive_ref_validator(const uri& schema_location) 
            : keyword_validator_base<Json>("$recursiveRef", schema_location)
        {}

        uri get_base_uri() const
        {
            return this->schema_location();
        }

        void set_referred_schema(const schema_validator<Json>* target) final { tentative_target_ = target; }

    private:

        void do_validate(const evaluation_context<Json>& context, 
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            auto rit = context.dynamic_scope().rbegin();
            auto rend = context.dynamic_scope().rend();

            const schema_validator<Json>* schema_ptr = tentative_target_; 
 
            JSONCONS_ASSERT(schema_ptr != nullptr);

            if (schema_ptr->recursive_anchor())
            {
                while (rit != rend)
                {
                    if ((*rit)->recursive_anchor())
                    {
                        schema_ptr = *rit; 
                    }
                    ++rit;
                }
            }

            //std::cout << "recursive_ref_validator.do_validate " << "keywordLocation: << " << this->schema_location().string() << ", instanceLocation:" << instance_location.string() << "\n";

            evaluation_context<Json> this_context(context, this->keyword_name());
            if (schema_ptr == nullptr)
            {
                reporter.error(validation_message(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_location(), 
                    instance_location, 
                    "Unresolved schema reference " + this->schema_location().string()));
                return;
            }

            schema_ptr->validate(this_context, instance, instance_location, results, reporter, patch);
        }
    };

    template <class Json>
    class dynamic_ref_validator : public keyword_validator_base<Json>, public virtual ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        uri_wrapper value_;
        const schema_validator<Json>* tentative_target_;

    public:
        dynamic_ref_validator(const uri& schema_location, const uri_wrapper& value) 
            : keyword_validator_base<Json>("$dynamicRef", schema_location), value_(value)
        {
            //std::cout << "dynamic_ref_validator path: " << schema_location.string() << ", value: " << value.string() << "\n";
        }

        void set_referred_schema(const schema_validator<Json>* target) final { tentative_target_ = target; }

        const jsoncons::uri& value() const { return value_.uri(); }

        uri get_base_uri() const
        {
            return this->schema_location();
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            //std::cout << "dynamic_ref_validator [" << context.eval_path().string() << "," << this->schema_location().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";

            auto rit = context.dynamic_scope().rbegin();
            auto rend = context.dynamic_scope().rend();

            //std::cout << "dynamic_ref_validator::do_validate " << this->value().string() << "\n";

            const schema_validator<Json> *schema_ptr = tentative_target_;

            evaluation_context<Json> this_context(context, this->keyword_name());
            JSONCONS_ASSERT(schema_ptr != nullptr);

            if (value_.has_plain_name_fragment() && schema_ptr->dynamic_anchor())
            {
                while (rit != rend)
                {
                    auto p = (*rit)->get_schema_for_dynamic_anchor(schema_ptr->dynamic_anchor()->fragment()); 
                    //std::cout << "  (2) [" << (*rit)->schema_location().string() << "] " << ((*rit)->dynamic_anchor() ? (*rit)->dynamic_anchor()->value().string() : "") << "\n";

                    if (p != nullptr) 
                    {
                        //std::cout << "Match found " << p->schema_location().string() << "\n";
                        schema_ptr = p;
                    }

                    ++rit;
                }
            }
            
            //assert(schema_ptr != tentative_target_);

            //std::cout << "dynamic_ref_validator.do_validate " << "keywordLocation: << " << this->schema_location().string() << ", instanceLocation:" << instance_location.string() << "\n";

            schema_ptr->validate(this_context, instance, instance_location, results, reporter, patch);
        }
    };

    // contentEncoding

    template <class Json>
    class content_encoding_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::string content_encoding_;

    public:
        content_encoding_validator(const uri& schema_location, const std::string& content_encoding)
            : keyword_validator_base<Json>("contentEncoding", schema_location), 
              content_encoding_(content_encoding)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (content_encoding_ == "base64")
            {
                auto s = instance.template as<jsoncons::string_view>();
                std::string content;
                auto retval = jsoncons::decode_base64(s.begin(), s.end(), content);
                if (retval.ec != jsoncons::conv_errc::success)
                {
                    reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(), 
                        instance_location, 
                        "Content is not a base64 string"));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
            else if (!content_encoding_.empty())
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(),
                    instance_location, 
                    "unable to check for contentEncoding '" + content_encoding_ + "'"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // contentMediaType

    template <class Json>
    class content_media_type_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::string content_media_type_;

    public:
        content_media_type_validator(const uri& schema_location, const std::string& content_media_type)
            : keyword_validator_base<Json>("contentMediaType", schema_location), 
              content_media_type_(content_media_type)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (content_media_type_ == "application/json")
            {
                auto sv = instance.as_string_view();
                json_string_reader reader(sv);
                std::error_code ec;
                reader.read(ec);

                if (ec)
                {
                    reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(), 
                        instance_location, 
                        std::string("Content is not JSON: ") + ec.message()));
                }
            }
        }
    };

    // format 

    template <class Json>
    class format_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        format_checker format_check_;

    public:
        format_validator(const uri& schema_location, const format_checker& format_check)
            : keyword_validator_base<Json>("format", schema_location), format_check_(format_check)
        {

        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            if (format_check_ != nullptr) 
            {
                evaluation_context<Json> this_context(context, this->keyword_name());
                auto s = instance.template as<std::string>();

                format_check_(this_context.eval_path(), this->schema_location(), instance_location, s, reporter);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // pattern 

#if defined(JSONCONS_HAS_STD_REGEX)
    template <class Json>
    class pattern_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::string pattern_string_;
        std::regex regex_;

    public:
        pattern_validator(const uri& schema_location,
            const std::string& pattern_string, const std::regex& regex)
            : keyword_validator_base<Json>("pattern", schema_location), 
              pattern_string_(pattern_string), regex_(regex)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            auto s = instance.template as<std::string>();
            if (!std::regex_search(s, regex_))
            {
                std::string message("String \"");
                message.append(s);
                message.append("\" does not match pattern \"");
                message.append(pattern_string_);
                message.append("\"");
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(),
                    instance_location, 
                    std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };
#else
    template <class Json>
    class pattern_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

    public:
        pattern_validator(const uri& schema_location)
            : keyword_validator_base<Json>("pattern", schema_location)
        {
        }

    private:

        void do_validate(const Json&, 
            const jsonpointer::json_pointer&,
            evaluation_results& /*results*/, 
            error_reporter&,
            Json& /*patch*/) const final
        {
        }
    };
#endif

    // maxLength

    template <class Json>
    class max_length_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t max_length_;
    public:
        max_length_validator(const uri& schema_location, std::size_t max_length)
            : keyword_validator_base<Json>("maxLength", schema_location), max_length_(max_length)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            auto sv = instance.as_string_view();
            std::size_t length = unicode_traits::count_codepoints(sv.data(), sv.size());
            if (length > max_length_)
            {
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(), 
                        instance_location, 
                        std::string("Expected maxLength: ") + std::to_string(max_length_)
                    + ", actual: " + std::to_string(length)));
                if (reporter.fail_early())
                {
                    return;
                }
            }          
        }
    };

    // maxItems

    template <class Json>
    class max_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t max_items_;
    public:
        max_items_validator(const uri& schema_location, std::size_t max_items)
            : keyword_validator_base<Json>("maxItems", schema_location), max_items_(max_items)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (instance.size() > max_items_)
            {
                std::string message("Maximum number of items is " + std::to_string(max_items_));
                message.append(" but found: " + std::to_string(instance.size()));
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(),
                        instance_location, 
                        std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
            }          
        }
    };

    // minItems

    template <class Json>
    class min_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::size_t min_items_;
    public:
        min_items_validator(const uri& schema_location, std::size_t min_items)
            : keyword_validator_base<Json>("minItems", schema_location), min_items_(min_items)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (instance.size() < min_items_)
            {
                std::string message("Minimum number of items is " + std::to_string(min_items_));
                message.append(" but found: " + std::to_string(instance.size()));
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(),
                        instance_location, 
                        std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
            }          
        }
    };

    // items

    template <class Json>
    class items_array_validator : public keyword_validator_base<Json>
    {
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::vector<schema_validator_type> item_validators_;
        schema_validator_type additional_items_val_;
    public:
        items_array_validator(const uri& schema_location, 
            std::vector<schema_validator_type>&& item_validators,
            schema_validator_type&& additional_items_val)
            : keyword_validator_base<Json>("items", schema_location), 
              item_validators_(std::move(item_validators)), 
              additional_items_val_(std::move(additional_items_val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            size_t index = 0;
            auto validator_it = item_validators_.cbegin();

            evaluation_context<Json> this_context(context, this->keyword_name());
            
            for (const auto& item : instance.array_range()) 
            {
                jsonpointer::json_pointer pointer(instance_location);

                if (validator_it != item_validators_.cend())
                {
                    evaluation_context<Json> item_context{this_context, index, evaluation_flags{}};
                    pointer /= index;
                    std::size_t errors = reporter.error_count();
                    (*validator_it)->validate(item_context, item, pointer, results, reporter, patch);
                    if (context.require_evaluated_items() && errors == reporter.error_count())
                    {
                        results.evaluated_items.insert(index);
                    }
                    ++validator_it;
                    ++index;
                }
                else if (additional_items_val_ != nullptr)
                {
                    pointer /= index;
                    std::size_t errors = reporter.error_count();
                    additional_items_val_->validate(context, item, pointer, results, reporter, patch);
                    if (context.require_evaluated_items() && errors == reporter.error_count())
                    {
                        results.evaluated_items.insert(index);
                    }
                    ++index;
                }
                else
                    break;

            }
        }
    };

    // 202012
    template <class Json>
    class items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type items_val_;
    public:
        items_validator(const uri& schema_location, 
            schema_validator_type&& items_val)
            : keyword_validator_base<Json>("items", schema_location), 
              items_val_(std::move(items_val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (items_val_)
            {
                size_t index = 0;
                for (const auto& item : instance.array_range()) 
                {
                    evaluation_context<Json> item_context{this_context, index, evaluation_flags{}};
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= index;
                    std::size_t errors = reporter.error_count();
                    items_val_->validate(item_context, item, pointer, results, reporter, patch);
                    if (context.require_evaluated_items() && errors == reporter.error_count())
                    {
                        results.evaluated_items.insert(index);
                    }
                    ++index;
                }
            }
        }
    };

    // items

    // uniqueItems

    template <class Json>
    class unique_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        bool are_unique_;
    public:
        unique_items_validator(const uri& schema_location, bool are_unique)
            : keyword_validator_base<Json>("uniqueItems", schema_location), are_unique_(are_unique)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            if (are_unique_ && !array_has_unique_items(instance))
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "Array items are not unique"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }

        static bool array_has_unique_items(const Json& a) 
        {
            for (auto it = a.array_range().begin(); it != a.array_range().end(); ++it) 
            {
                for (auto jt = it+1; jt != a.array_range().end(); ++jt) 
                {
                    if (*it == *jt) 
                    {
                        return false; // contains duplicates 
                    }
                }
            }
            return true; // elements are unique
        }
    };

    // minLength

    template <class Json>
    class min_length_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t min_length_;

    public:
        min_length_validator(const uri& schema_location, std::size_t min_length)
            : keyword_validator_base<Json>("minLength", schema_location), min_length_(min_length)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            auto sv = instance.as_string_view();
            std::size_t length = unicode_traits::count_codepoints(sv.data(), sv.size());
            if (length < min_length_) 
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    std::string("Expected minLength: ") + std::to_string(min_length_)
                                          + ", actual: " + std::to_string(length)));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // not

    template <class Json>
    class not_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type rule_;

    public:
        not_validator(const uri& schema_location,
            schema_validator_type&& rule)
            : keyword_validator_base<Json>("not", schema_location), 
              rule_(std::move(rule))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            evaluation_results local_results;
            collecting_error_reporter local_reporter;
            rule_->validate(this_context, instance, instance_location, local_results, local_reporter, patch);

            if (local_reporter.errors.empty())
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "Instance must not be valid against schema"));
            }
            else
            {
                results.merge(local_results);
            }
        }
    };

    template <class Json>
    class any_of_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::vector<schema_validator_type> validators_;

    public:
        any_of_validator(const uri& schema_location,
             std::vector<schema_validator_type>&& validators)
            : keyword_validator_base<Json>("anyOf", schema_location),
              validators_(std::move(validators))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "any_of_validator.do_validate " << context.eval_path().string() << ", " << instance << "\n";

            collecting_error_reporter local_reporter;

            evaluation_context<Json> this_context(context, this->keyword_name());

            evaluation_results local_results1;
            std::size_t count = 0;
            for (std::size_t i = 0; i < validators_.size(); ++i) 
            {
                evaluation_results local_results2;
                evaluation_context<Json> item_context(this_context, i);

                std::size_t errors = local_reporter.errors.size();
                validators_[i]->validate(item_context, instance, instance_location, local_results2, local_reporter, patch);
                if (errors == local_reporter.errors.size())
                {
                    local_results1.merge(local_results2);
                    ++count;
                }
                //std::cout << "success: " << i << " " << success << "\n";
            }

            if (count > 0)
            {
                results.merge(local_results1);
            }
            else 
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "No schema matched, but at least one of them is required to match", 
                    local_reporter.errors));
            }
        }
    };

    template <class Json>
    class one_of_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::vector<schema_validator_type> validators_;

    public:
        one_of_validator(const uri& schema_location,
             std::vector<schema_validator_type>&& validators)
            : keyword_validator_base<Json>("oneOf", schema_location),
              validators_(std::move(validators))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "any_of_validator.do_validate " << context.eval_path().string() << ", " << instance << "\n";

            collecting_error_reporter local_reporter;

            evaluation_context<Json> this_context(context, this->keyword_name());

            evaluation_results local_results1;
            std::size_t count = 0;
            for (std::size_t i = 0; i < validators_.size(); ++i) 
            {
                evaluation_results local_results2;
                evaluation_context<Json> item_context(this_context, i);

                std::size_t errors = local_reporter.errors.size();
                validators_[i]->validate(item_context, instance, instance_location, local_results2, local_reporter, patch);
                if (errors == local_reporter.errors.size())
                {
                    local_results1.merge(local_results2);
                    ++count;
                }
                //std::cout << "success: " << i << " " << success << "\n";
            }

            if (count == 1)
            {
                results.merge(local_results1);
            }
            else 
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "No schema matched, but exactly one of them is required to match", 
                    local_reporter.errors));
            }
        }
    };

    template <class Json>
    class all_of_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::vector<schema_validator_type> validators_;

    public:
        all_of_validator(const uri& schema_location,
             std::vector<schema_validator_type>&& validators)
            : keyword_validator_base<Json>("allOf", schema_location),
              validators_(std::move(validators))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << this->keyword_name() << " [" << context.eval_path().string() << ", " << this->schema_location().string() << "]\n";

            evaluation_results local_results1;
            collecting_error_reporter local_reporter;

            evaluation_context<Json> this_context(context, this->keyword_name());

            std::size_t count = 0;
            for (std::size_t i = 0; i < validators_.size(); ++i) 
            {
                evaluation_results local_results2;
                evaluation_context<Json> item_context(this_context, i);

                std::size_t errors = local_reporter.errors.size();
                validators_[i]->validate(item_context, instance, instance_location, local_results2, local_reporter, patch);
                //std::cout << "local_results2:\n";
                //for (const auto& s : local_results2.evaluated_items)
                //{
                //    std::cout << "    " << s << "\n";
                //}
                if (errors == local_reporter.errors.size())
                {
                    local_results1.merge(local_results2);
                    ++count;
                }
                //std::cout << "success: " << i << " " << success << "\n";
            }

            //std::cout << "local_results1:\n";
            //for (const auto& s : local_results1.evaluated_items)
            //{
            //    std::cout << "    " << s << "\n";
            //}

            if (count == validators_.size())
            {
                results.merge(local_results1);
            }
            else 
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "No schema matched, but all of them are required to match", 
                    local_reporter.errors));
            }
        }
    };

    template <class Json>
    class maximum_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;
        std::string message_;

    public:
        maximum_validator(const uri& schema_location, const Json& value)
            : keyword_validator_base<Json>("maximum", schema_location), value_(value),
              message_{" is greater than maximum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final 
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() > value_.template as<int64_t>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() > value_.template as<double>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    template <class Json>
    class exclusive_maximum_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;
        std::string message_;

    public:
        exclusive_maximum_validator(const uri& schema_location, const Json& value)
            : keyword_validator_base<Json>("exclusiveMaximum", schema_location), value_(value),
              message_{" is no less than exclusiveMaximum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final 
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() >= value_.template as<int64_t>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() >= value_.template as<double>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    template <class Json>
    class minimum_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;
        std::string message_;

    public:
        minimum_validator(const uri& schema_location, const Json& value)
            : keyword_validator_base<Json>("minimum", schema_location), value_(value),
              message_{" is less than minimum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() < value_.template as<int64_t>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() < value_.template as<double>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    template <class Json>
    class exclusive_minimum_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;
        std::string message_;

    public:
        exclusive_minimum_validator(const uri& schema_location, const Json& value)
            : keyword_validator_base<Json>("exclusiveMinimum", schema_location), value_(value),
              message_{" is no greater than exclusiveMinimum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final 
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() <= value_.template as<int64_t>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() <= value_.template as<double>())
                    {
                        reporter.error(validation_message(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_location(), 
                            instance_location, 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    template <class Json>
    class multiple_of_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        double value_;

    public:
        multiple_of_validator(const uri& schema_location, double value)
            : keyword_validator_base<Json>("multipleOf", schema_location), value_(value)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final
        {
            if (!instance.is_number())
            {
                return;
            }
            evaluation_context<Json> this_context(context, this->keyword_name());

            double value = instance.template as<double>();
            if (value != 0) // Exclude zero
            {
                if (!is_multiple_of(value, static_cast<double>(value_)))
                {
                    reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(),
                        instance_location, 
                        instance.template as<std::string>() + " is not a multiple of " + std::to_string(value_)));
                }
            }
        }

        static bool is_multiple_of(double x, double multiple_of) 
        {
            double rem = std::remainder(x, multiple_of);
            double eps = std::nextafter(x, 0) - x;
            return std::fabs(rem) < std::fabs(eps);
        }
    };

    template <class Json>
    class required_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::vector<std::string> items_;

    public:
        required_validator(const uri& schema_location,
            const std::vector<std::string>& items)
            : keyword_validator_base<Json>("required", schema_location), items_(items)
        {
        }

        required_validator(const required_validator&) = delete;
        required_validator(required_validator&&) = default;
        required_validator& operator=(const required_validator&) = delete;
        required_validator& operator=(required_validator&&) = default;

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& key : items_)
            {
                if(instance.find(key) == instance.object_range().end())
                {
                        reporter.error(validation_message(this->keyword_name(),
                                                         this_context.eval_path(),
                                                         this->schema_location(),
                                                         instance_location,
                                                         "Required property \"" + key + "\" not found"));
                        if(reporter.fail_early())
                    {
                            return;
                    }
                }
            }
            
        }
    };

    // maxProperties

    template <class Json>
    class max_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t max_properties_;
    public:
        max_properties_validator(const uri& schema_location, std::size_t max_properties)
            : keyword_validator_base<Json>("maxProperties", schema_location), max_properties_(max_properties)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_object())
            {
                return;
            }
            
            if (instance.size() > max_properties_)
            {
                evaluation_context<Json> this_context(context, this->keyword_name());

                std::string message("Maximum properties: " + std::to_string(max_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    std::move(message)));
            }           
        }
    };

    // minProperties

    template <class Json>
    class min_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t min_properties_;
    public:
        min_properties_validator(const uri& schema_location, std::size_t min_properties)
            : keyword_validator_base<Json>("minProperties", schema_location), min_properties_(min_properties)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (!instance.is_object())
            {
                return;
            }
            if (instance.size() < min_properties_)
            {
                evaluation_context<Json> this_context(context, this->keyword_name());

                std::string message("Maximum properties: " + std::to_string(min_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(),
                        this->schema_location(),
                        instance_location,
                        std::move(message)));
            }
            
        }
    };

    template <class Json>
    class conditional_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type if_val_;
        schema_validator_type then_val_;
        schema_validator_type else_val_;

    public:
        conditional_validator(const uri& schema_location,
          schema_validator_type&& if_val,
          schema_validator_type&& then_val,
          schema_validator_type&& else_val
        ) : keyword_validator_base<Json>("", std::move(schema_location)), 
              if_val_(std::move(if_val)), 
              then_val_(std::move(then_val)), 
              else_val_(std::move(else_val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            evaluation_context<Json> this_context(context, this->keyword_name());
            if (if_val_) 
            {
                collecting_error_reporter local_reporter;

                if_val_->validate(this_context, instance, instance_location, results, local_reporter, patch);
                if (local_reporter.errors.empty()) 
                {
                    if (then_val_)
                        then_val_->validate(this_context, instance, instance_location, results, reporter, patch);
                } 
                else 
                {
                    if (else_val_)
                        else_val_->validate(this_context, instance, instance_location, results, reporter, patch);
                }
            }
        }
    };

    // enum_validator

    template <class Json>
    class enum_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;

    public:
        enum_validator(const uri& schema_location, const Json& sch)
            : keyword_validator_base<Json>("enum", schema_location), value_(sch)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            bool in_range = false;
            for (const auto& item : value_.array_range())
            {
                if (item == instance) 
                {
                    in_range = true;
                    break;
                }
            }

            if (!in_range)
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    instance.template as<std::string>() + " is not a valid enum value"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // const_validator

    template <class Json>
    class const_validator : public keyword_validator_base<Json>
    {        
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        Json value_;

    public:
        const_validator(const uri& schema_location, const Json& sch)
            : keyword_validator_base<Json>("const", schema_location), value_(sch)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter,
            Json& /*patch*/) const final
        {
            if (value_ != instance)
            {
                evaluation_context<Json> this_context(context, this->keyword_name());

                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "Instance is not const"));
            }
        }
    };

    enum class json_schema_type{null,object,array,string,boolean,integer,number};

    inline
    std::string to_string(json_schema_type type)
    {
        switch (type)
        {
            case json_schema_type::null:
                return "null";
            case json_schema_type::object:
                return "object";
            case json_schema_type::array:
                return "array";
            case json_schema_type::string:  // OK
                return "string";
            case json_schema_type::boolean: // OK
                return "boolean";
            case json_schema_type::integer:
                return "integer";
            case json_schema_type::number:
                return "number";
            default:
                return "unknown";
        }

    }

    template <class Json>
    class type_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::vector<json_schema_type> expected_types_;

    public:
        type_validator(const type_validator&) = delete;
        type_validator& operator=(const type_validator&) = delete;
        type_validator(type_validator&&) = default;
        type_validator& operator=(type_validator&&) = default;

        type_validator(const uri& schema_location,
            std::vector<json_schema_type>&& expected_types)
            : keyword_validator_base<Json>("type", std::move(schema_location)),
              expected_types_(std::move(expected_types))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/,
            error_reporter& reporter, 
            Json& /*patch*/) const final
        {
            //std::cout << "type_validator.do_validate " << context.eval_path().string() << instance << "\n";
            //for (auto& type : expected_types_ )
            //{
            //    std::cout << "    " << to_string(type) << "\n";
            //}

            evaluation_context<Json> this_context(context, this->keyword_name());

            bool is_type_found = false;

            auto end = expected_types_.end();
            for (auto it = expected_types_.begin(); it != end && !is_type_found; ++it)
            {
                switch (*it)
                {
                    case json_schema_type::null:
                        if (instance.is_null()) // OK
                        {
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::object:
                        if (instance.is_object()) // OK
                        {
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::array:
                        if (instance.is_array())  // OK
                        {
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::string:  // OK
                        if (instance.is_string())
                        {
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::boolean: // OK
                        if (instance.is_bool())
                        {
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::integer:
                        if (instance.is_number())
                        {
                            if (!(instance.template is_integer<int64_t>() || (instance.is_double() && static_cast<double>(instance.template as<int64_t>()) == instance.template as<double>())))
                            {
                                reporter.error(validation_message(this->keyword_name(),
                                    this_context.eval_path(), 
                                    this->schema_location(), 
                                    instance_location, 
                                    "Instance is not an integer"));
                                if (reporter.fail_early())
                                {
                                    return;
                                }
                            }
                            is_type_found = true;
                        }
                        break;
                    case json_schema_type::number:
                        if (instance.is_number())
                        {
                            is_type_found = true;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!is_type_found)
            {
                std::ostringstream ss;
                ss << "Expected " << expected_types_.size() << " ";
                for (std::size_t i = 0; i < expected_types_.size(); ++i)
                {
                        if (i > 0)
                        { 
                            ss << ", ";
                            if (i+1 == expected_types_.size())
                            { 
                                ss << "or ";
                            }
                        }
                        ss << to_string(expected_types_[i]);
                        //std::cout << ", " << i << ". expected " << to_string(expected_types_[i]);
                }
                ss << ", found " << to_schema_type(instance.type());

                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    ss.str()));
                if (reporter.fail_early())
                {
                    return;
                }
            }
            //std::cout << "\n";
        }
        
        std::string to_schema_type(json_type type) const
        {
            switch (type)
            {
                case json_type::null_value:
                {
                    return "null";
                }
                case json_type::bool_value:
                {
                    return "boolean";
                }
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    return "integer";
                }
                case json_type::half_value:
                case json_type::double_value:
                {
                    return "number";
                }
                case json_type::string_value:
                {
                    return "string";
                }
                case json_type::array_value:
                {
                    return "array";
                }
                case json_type::object_value:
                {
                    return "object";
                }
                default:
                {
                    return "unsupported type";
                }
            }
        }
    };

    template <class Json>
    class properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::map<std::string, schema_validator_type> properties_;
    public:
        properties_validator(const uri& schema_location,
            std::map<std::string, schema_validator_type>&& properties
        )
            : keyword_validator_base<Json>("properties", std::move(schema_location)),
              properties_(std::move(properties))
        {
        }

        void validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch,
            std::unordered_set<std::string>& all_properties) const 
        {
            //std::cout << "properties_validator begin[" << context.eval_path().string() << "," << this->schema_location().string() << "]\n";
            if (!instance.is_object())
            {
                return;
            }

            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& prop : instance.object_range()) 
            {
                evaluation_context<Json> prop_context{this_context, prop.key(), evaluation_flags{}};

                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                auto properties_it = properties_.find(prop.key());

                // check if it is in "properties"
                if (properties_it != properties_.end()) 
                {
                    std::size_t errors = reporter.error_count();
                    properties_it->second->validate(prop_context, prop.value() , pointer, results, reporter, patch);
                    all_properties.insert(prop.key());
                    if (context.require_evaluated_properties() && errors == reporter.error_count())
                    {
                        results.evaluated_properties.insert(prop.key());
                    }
                }
            }
                // Any property that doesn't match any of the property names in the properties keyword is ignored by this keyword.

            // reverse search
            for (auto const& prop : properties_) 
            {
                //std::cout << "   prop:" << prop.first << "\n";
                const auto finding = instance.find(prop.first);
                if (finding == instance.object_range().end()) 
                { 
                    // If prop is not in instance
                    auto default_value = prop.second->get_default_value();
                    if (default_value) 
                    { 
                        // If default value is available, update patch
                        jsonpointer::json_pointer pointer(instance_location);
                        pointer /= prop.first;

                        update_patch(patch, pointer, std::move(*default_value));
                    }
                }
            }
            //std::cout << "properties_validator end[" << context.eval_path().string() << "," << this->schema_location().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            std::unordered_set<std::string> all_properties;
            validate(context, instance, instance_location, results, reporter, patch, all_properties);
        }

        void update_patch(Json& patch, const jsonpointer::json_pointer& instance_location, Json&& default_value) const
        {
            Json j;
            j.try_emplace("op", "add"); 
            j.try_emplace("path", instance_location.string()); 
            j.try_emplace("value", std::forward<Json>(default_value)); 
            patch.push_back(std::move(j));
        }
    };

    template <class Json>
    class pattern_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        std::vector<std::pair<std::regex, schema_validator_type>> pattern_properties_;

    public:
        pattern_properties_validator(const uri& schema_location,
            std::vector<std::pair<std::regex, schema_validator_type>>&& pattern_properties
        )
            : keyword_validator_base<Json>("patternProperties", std::move(schema_location)),
              pattern_properties_(std::move(pattern_properties))
        {
        }

#if defined(JSONCONS_HAS_STD_REGEX)
        void validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location, 
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch,
            std::unordered_set<std::string>& all_properties) const 
        {
            if (!instance.is_object())
            {
                return;
            }
            evaluation_context<Json> this_context(context, this->keyword_name());
            for (const auto& prop : instance.object_range()) 
            {
                evaluation_context<Json> prop_context{this_context, prop.key(), evaluation_flags{}};
                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                // check all matching "patternProperties"
                for (auto& schema_pp : pattern_properties_)
                    if (std::regex_search(prop.key(), schema_pp.first)) 
                    {
                        all_properties.insert(prop.key());
                        std::size_t errors = reporter.error_count();
                        schema_pp.second->validate(prop_context, prop.value() , pointer, results, reporter, patch);
                        if (context.require_evaluated_properties() && errors == reporter.error_count())
                        {
                            results.evaluated_properties.insert(prop.key());
                        }
                    }
            }
#endif
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            std::unordered_set<std::string> all_properties;
            validate(context, instance, instance_location, results, reporter, patch, all_properties);
        }
    };

    template <class Json>
    class additional_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::unique_ptr<properties_validator<Json>> properties_; 
        std::unique_ptr<pattern_properties_validator<Json>> pattern_properties_;
        schema_validator_type additional_properties_;

    public:
        additional_properties_validator(const uri& schema_location,
            std::unique_ptr<properties_validator<Json>>&& properties,
            std::unique_ptr<pattern_properties_validator<Json>>&& pattern_properties,
            schema_validator_type&& additional_properties
        )
            : keyword_validator_base<Json>("additionalProperties", std::move(schema_location)), 
              properties_(std::move(properties)),
              pattern_properties_(std::move(pattern_properties)),
              additional_properties_(std::move(additional_properties))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            std::unordered_set<std::string> all_properties;

            if (properties_)
            {
                properties_->validate(this_context, instance, instance_location, results, reporter, patch, all_properties);
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (pattern_properties_)
            {
                pattern_properties_->validate(this_context, instance, instance_location, results, reporter, patch, all_properties);
                if (reporter.fail_early())
                {
                    return;
                }
            }

            for (const auto& prop : instance.object_range()) 
            {
                evaluation_context<Json> prop_context{this_context, prop.key(), evaluation_flags{}};

                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                auto properties_it = all_properties.find(prop.key());

                // check if it is in "properties"
                if (properties_it == all_properties.end()) 
                {
                    // finally, check "additionalProperties" 
                    //std::cout << "additional_properties_validator a_prop_or_pattern_matched " << a_prop_or_pattern_matched << ", " << bool(additional_properties_);
                    if (additional_properties_) 
                    {
                        //std::cout << " !!!additionalProperties!!!";
                        collecting_error_reporter local_reporter;

                        additional_properties_->validate(prop_context, prop.value() , pointer, results, local_reporter, patch);
                        if (!local_reporter.errors.empty())
                        {
                            reporter.error(validation_message(this->keyword_name(),
                                this_context.eval_path(), 
                                additional_properties_->schema_location().string(),
                                instance_location, 
                                "Additional prop \"" + prop.key() + "\" found but was invalid."));
                            if (reporter.fail_early())
                            {
                                return;
                            }
                        }
                        else if (context.require_evaluated_properties())
                        {
                            results.evaluated_properties.insert(prop.key());
                        }
                    }
                }
                //std::cout << "\n";
            }
        }
    };

    template <class Json>
    class dependent_required_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        std::map<std::string, keyword_validator_type> dependent_required_;

    public:
        dependent_required_validator(const uri& schema_location,
            std::map<std::string, keyword_validator_type>&& dependent_required
        )
            : keyword_validator_base<Json>("dependentRequired", std::move(schema_location)), 
              dependent_required_(std::move(dependent_required))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& dep : dependent_required_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, results, reporter, patch); // validate
                }
            }
        }
    };

    template <class Json>
    class dependent_schemas_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::map<std::string, schema_validator_type> dependent_schemas_;

    public:
        dependent_schemas_validator(const uri& schema_location,
            std::map<std::string, schema_validator_type>&& dependent_schemas
        )
            : keyword_validator_base<Json>("dependentSchemas", std::move(schema_location)), 
              dependent_schemas_(std::move(dependent_schemas))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& dep : dependent_schemas_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, results, reporter, patch); // validate
                }
            }
        }
    };

    template <class Json>
    class property_names_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type property_names_schema_validator_;

    public:
        property_names_validator(const uri& schema_location,
            schema_validator_type&& property_names_schema_validator
        )
            : keyword_validator_base<Json>("propertyNames", schema_location), 
                property_names_schema_validator_{ std::move(property_names_schema_validator) }
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& prop : instance.object_range()) 
            {
                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                if (property_names_schema_validator_)
                {
                    property_names_schema_validator_->validate(this_context, prop.key() , instance_location, results, reporter, patch);
                }
                // Any property that doesn't match any of the property names in the properties keyword is ignored by this keyword.
            }
        }
    };

    template <class Json>
    class dependencies_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::map<std::string, keyword_validator_type> dependent_required_;
        std::map<std::string, schema_validator_type> dependent_schemas_;

    public:
        dependencies_validator(const uri& schema_location,
            std::map<std::string, keyword_validator_type>&& dependent_required,
            std::map<std::string, schema_validator_type>&& dependent_schemas
        )
            : keyword_validator_base<Json>("dependencies", std::move(schema_location)), 
              dependent_required_(std::move(dependent_required)),
              dependent_schemas_(std::move(dependent_schemas))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            for (const auto& dep : dependent_required_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, results, reporter, patch); // validate
                }
            }

            for (const auto& dep : dependent_schemas_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, results, reporter, patch); // validate
                }
            }
        }
    };

    template <class Json>
    class max_contains_keyword : public keyword_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::size_t max_value_;
    public:
        max_contains_keyword(const uri& schema_location, std::size_t max_value)
            : keyword_base<Json>("maxContains", schema_location), max_value_(max_value)
        {
        }

        void validate(const evaluation_context<Json>& context, 
            const jsonpointer::json_pointer& instance_location,
            std::size_t count, 
            error_reporter& reporter) const 
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            if (count > max_value_)
            {
                std::string message("A schema can match a contains constraint at most " + std::to_string(max_value_) + " times");
                message.append(" but it matched " + std::to_string(count) + " times.");
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(),
                        instance_location, 
                        std::move(message)));
            }
        }
    };

    // minItems

    template <class Json>
    class min_contains_keyword : public keyword_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::size_t min_value_;
    public:
        min_contains_keyword(const uri& schema_location, std::size_t min_value)
            : keyword_base<Json>("minContains", schema_location), min_value_(min_value)
        {
        }

        void validate(const evaluation_context<Json>& context, 
            const jsonpointer::json_pointer& instance_location,
            std::size_t count, 
            error_reporter& reporter) const 
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            if (count < min_value_)
            {
                std::string message("A schema must match a contains constraint at least " + std::to_string(min_value_) + " times");
                message.append(" but it matched " + std::to_string(count) + " times.");
                reporter.error(validation_message(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_location(),
                        instance_location, 
                        std::move(message)));
            }
        }
    };

    template <class Json>
    class contains_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type schema_validator_;
        std::unique_ptr<max_contains_keyword<Json>> max_contains_;
        std::unique_ptr<min_contains_keyword<Json>> min_contains_;

    public:
        contains_validator(const uri& schema_location,
            schema_validator_type&& schema_validator)
            : keyword_validator_base<Json>("contains", std::move(schema_location)), 
              schema_validator_(std::move(schema_validator))
        {
        }

        contains_validator(const uri& schema_location,
            schema_validator_type&& schema_validator,
            std::unique_ptr<max_contains_keyword<Json>>&& max_contains,
            std::unique_ptr<min_contains_keyword<Json>>&& min_contains)
            : keyword_validator_base<Json>("contains", std::move(schema_location)), 
              schema_validator_(std::move(schema_validator)),
              max_contains_(std::move(max_contains)),
              min_contains_(std::move(min_contains))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            if (!schema_validator_) 
            {
                return;
            }

            evaluation_context<Json> this_context(context, this->keyword_name());

            std::size_t contains_count = 0;
            collecting_error_reporter local_reporter;

            std::size_t index = 0;
            for (const auto& item : instance.array_range()) 
            {
                std::size_t errors = local_reporter.errors.size();
                schema_validator_->validate(this_context, item, instance_location, results, local_reporter, patch);
                if (errors == local_reporter.errors.size())
                {
                    if (context.require_evaluated_items())
                    {
                        results.evaluated_items.insert(index);
                    }
                    ++contains_count;
                }
                ++index;
            }
            
            if (max_contains_ || min_contains_)
            {
                if (max_contains_)
                {
                    max_contains_->validate(this_context, instance_location, contains_count, reporter);
                }
                if (min_contains_)
                {
                    min_contains_->validate(this_context, instance_location, contains_count, reporter);
                }
            }
            else if (contains_count == 0)
            {
                reporter.error(validation_message(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_location(), 
                    instance_location, 
                    "Expected at least one array item to match \"contains\" schema", 
                    local_reporter.errors));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    template <class Json>
    class prefix_items_validator : public keyword_validator_base<Json>
    {
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::vector<schema_validator_type> prefix_items_validators_;
        schema_validator_type items_val_;
    public:
        prefix_items_validator(const uri& schema_location, 
            std::vector<schema_validator_type>&& item_validators,
            schema_validator_type&& additional_items_val)
            : keyword_validator_base<Json>("prefixItems", schema_location), 
              prefix_items_validators_(std::move(item_validators)), 
              items_val_(std::move(additional_items_val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }
        
            size_t index = 0;
            auto it = instance.array_range().cbegin();
            auto end_it = instance.array_range().cend();
        
            evaluation_context<Json> this_context(context, this->keyword_name());
            for (const auto& val : prefix_items_validators_) 
            {
                if (it == end_it)
                {
                    break;
                }
                jsonpointer::json_pointer pointer(instance_location);
                pointer /= index;
        
                evaluation_context<Json> item_context{this_context, index, evaluation_flags{}};
                std::size_t errors = reporter.error_count();
                val->validate(item_context, *it, pointer, results, reporter, patch);
                if (context.require_evaluated_items() && errors == reporter.error_count())
                {
                    results.evaluated_items.insert(index);
                }
                ++it;
                ++index;
            }
            if (it != end_it && items_val_)
            {
                if (items_val_->always_fails())
                {
                    evaluation_context<Json> item_context{this_context, index, evaluation_flags{}};
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= index;
                    reporter.error(validation_message(this->keyword_name(),
                        item_context.eval_path(), 
                        this->schema_location(), 
                        pointer,
                        "Extra item at index '" + std::to_string(index) + "' but the schema does not allow extra items."));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
                else
                {
                    while (it != end_it)
                    {
                        evaluation_context<Json> item_context{this_context, index, evaluation_flags{}};
                        jsonpointer::json_pointer pointer(instance_location);
                        pointer /= index;
                        if (items_val_)
                        {
                            std::size_t errors = reporter.error_count();
                            items_val_->validate(item_context, *it, pointer, results, reporter, patch);
                            if (context.require_evaluated_items() && errors == reporter.error_count())
                            {
                                results.evaluated_items.insert(index);
                            }
                        }
                        ++it;
                        ++index;
                    }
                }
            }
        }
    };

    template <class Json>
    class unevaluated_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type validator_;

    public:
        unevaluated_properties_validator(const uri& schema_location,
            schema_validator_type&& val
        )
            : keyword_validator_base<Json>("unevaluatedProperties", std::move(schema_location)), 
              validator_(std::move(val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "unevaluated_properties_validator [" << context.eval_path().string() << "," << this->schema_location().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results.evaluated_properties_)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
            if (!instance.is_object())
            {
                return;
            }

            if (validator_)
            {
                evaluation_context<Json> this_context(context, this->keyword_name());

                for (const auto& prop : instance.object_range()) 
                {
                    auto prop_it = results.evaluated_properties.find(prop.key());

                    // check if it is in "results"
                    if (prop_it == results.evaluated_properties.end()) 
                    {
                        //std::cout << "Not in evaluated properties: " << prop.key() << "\n";
                        std::size_t error_count = reporter.error_count();
                        validator_->validate(this_context, prop.value() , instance_location, results, reporter, patch);
                        if (reporter.error_count() == error_count)
                        {
                            results.evaluated_properties.insert(prop.key());
                        }
                    }
                }
            }
        }
    };

    template <class Json>
    class unevaluated_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type validator_;

    public:
        unevaluated_items_validator(const uri& schema_location,
            schema_validator_type&& val
        )
            : keyword_validator_base<Json>("unevaluatedProperties", std::move(schema_location)), 
              validator_(std::move(val))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << this->keyword_name() << " [" << context.eval_path().string() << ", " << this->schema_location().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results.evaluated_items)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
            if (!instance.is_array())
            {
                return;
            }

            if (validator_)
            {
                evaluation_context<Json> this_context(context, this->keyword_name());

                std::size_t index = 0;
                for (const auto& item : instance.array_range()) 
                {
                    auto item_it = results.evaluated_items.find(index);

                    // check if it is in "results"
                    if (item_it == results.evaluated_items.end()) 
                    {
                        //std::cout << "Not in evaluated properties: " << item.key() << "\n";
                        std::size_t error_count = reporter.error_count();
                        validator_->validate(this_context, item, instance_location, results, reporter, patch);
                        if (reporter.error_count() == error_count)
                        {
                            results.evaluated_items.insert(index);
                        }
                    }
                    ++index;
                }
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORDS_HPP
