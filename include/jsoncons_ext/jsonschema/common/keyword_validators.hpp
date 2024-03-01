// Copyright 2013-2023 Daniel Parker
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
#include <jsoncons_ext/jsonschema/common/schema_validator.hpp>
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

    struct collecting_error_reporter : public error_reporter
    {
        std::vector<validation_output> errors;

    private:
        void do_error(const validation_output& o) final
        {
            errors.push_back(o);
        }
    };

    template <class Json>
    class ref_validator : public keyword_validator_base<Json>, public ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        const schema_validator<Json>* referred_schema_;

    public:
        ref_validator(const uri& schema_path) 
            : keyword_validator_base<Json>("$ref", schema_path), referred_schema_{nullptr}
        {
        }

        ref_validator(const uri& schema_path, const schema_validator<Json>* referred_schema)
            : keyword_validator_base<Json>("$ref", schema_path), referred_schema_(referred_schema) 
        {
        }

        void set_referred_schema(const schema_validator<Json>* target) final { referred_schema_ = target; }

        uri get_base_uri() const
        {
            return this->schema_path();
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (!referred_schema_)
            {
                reporter.error(validation_output(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            referred_schema_->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

    template <class Json>
    class recursive_ref_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

    public:
        recursive_ref_validator(const uri& schema_path) 
            : keyword_validator_base<Json>("$recursiveRef", schema_path)
        {}

        uri get_base_uri() const
        {
            return this->schema_path();
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            auto rit = eval_context.dynamic_scope().rbegin();
            auto rend = eval_context.dynamic_scope().rend();

            const schema_validator<Json>* schema_ptr = nullptr;

            while (rit != rend && schema_ptr == nullptr)
            {
                if ((*rit)->schema_path() == this->schema_path())
                {
                    schema_ptr = *rit; 
                }
                ++rit;
            }

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

            //std::cout << "recursive_ref_validator.do_validate " << "keywordLocation: << " << this->schema_path().string() << ", instanceLocation:" << instance_location.to_string() << "\n";

            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            if (schema_ptr == nullptr)
            {
                reporter.error(validation_output(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            schema_ptr->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

    template <class Json>
    class dynamic_ref_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        std::string value_;

    public:
        dynamic_ref_validator(const uri& schema_path, const std::string& value) 
            : keyword_validator_base<Json>("$dynamicRef", schema_path), value_(value)
        {}

        uri get_base_uri() const
        {
            return this->schema_path();
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            auto rit = eval_context.dynamic_scope().rbegin();
            auto rend = eval_context.dynamic_scope().rend();

            const schema_validator<Json>* schema_ptr = nullptr;

            while (rit != rend && schema_ptr == nullptr)
            {
                if ((*rit)->schema_path() == this->schema_path())
                {
                    schema_ptr = *rit; 
                }
                ++rit;
            }

            JSONCONS_ASSERT(schema_ptr != nullptr);

            if (schema_ptr->dynamic_anchor() && schema_ptr->dynamic_anchor()->value() == value_)
            {
                while (rit != rend)
                {
                    if ((*rit)->dynamic_anchor() && (*rit)->dynamic_anchor()->value() == value_)
                    {
                        schema_ptr = *rit; 
                    }
                    ++rit;
                }
            }

            //std::cout << "dynamic_ref_validator.do_validate " << "keywordLocation: << " << this->schema_path().string() << ", instanceLocation:" << instance_location.to_string() << "\n";

            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            if (schema_ptr == nullptr)
            {
                reporter.error(validation_output(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            schema_ptr->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

    // contentEncoding

    template <class Json>
    class content_encoding_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        std::string content_encoding_;

    public:
        content_encoding_validator(const uri& schema_path, const std::string& content_encoding)
            : keyword_validator_base<Json>("contentEncoding", schema_path), 
              content_encoding_(content_encoding)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (content_encoding_ == "base64")
            {
                auto s = instance.template as<jsoncons::string_view>();
                std::string content;
                auto retval = jsoncons::decode_base64(s.begin(), s.end(), content);
                if (retval.ec != jsoncons::conv_errc::success)
                {
                    reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(), 
                        instance_location.to_string(), 
                        "Content is not a base64 string"));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
            else if (!content_encoding_.empty())
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(),
                    instance_location.to_string(), 
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
        content_media_type_validator(const uri& schema_path, const std::string& content_media_type)
            : keyword_validator_base<Json>("contentMediaType", schema_path), 
              content_media_type_(content_media_type)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (content_media_type_ == "application/Json")
            {
                auto sv = instance.as_string_view();
                json_string_reader reader(sv);
                std::error_code ec;
                reader.read(ec);

                if (ec)
                {
                    reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(), 
                        instance_location.to_string(), 
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
        format_validator(const uri& schema_path, const format_checker& format_check)
            : keyword_validator_base<Json>("format", schema_path), format_check_(format_check)
        {

        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            if (format_check_ != nullptr) 
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());
                auto s = instance.template as<std::string>();

                format_check_(this_context.eval_path(), this->schema_path(), instance_location, s, reporter);
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
        pattern_validator(const uri& schema_path,
            const std::string& pattern_string, const std::regex& regex)
            : keyword_validator_base<Json>("pattern", schema_path), 
              pattern_string_(pattern_string), regex_(regex)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            auto s = instance.template as<std::string>();
            if (!std::regex_search(s, regex_))
            {
                std::string message("String \"");
                message.append(s);
                message.append("\" does not match pattern \"");
                message.append(pattern_string_);
                message.append("\"");
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(),
                    instance_location.to_string(), 
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
        pattern_validator(const uri& schema_path)
            : keyword_validator_base<Json>("pattern", schema_path)
        {
        }

    private:

        void do_validate(const Json&, 
            const jsonpointer::json_pointer&,
            std::unordered_set<std::string>&, 
            error_reporter&,
            Json&) const final
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
        max_length_validator(const uri& schema_path, std::size_t max_length)
            : keyword_validator_base<Json>("maxLength", schema_path), max_length_(max_length)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            auto sv = instance.as_string_view();
            std::size_t length = unicode_traits::count_codepoints(sv.data(), sv.size());
            if (length > max_length_)
            {
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(), 
                        instance_location.to_string(), 
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
        max_items_validator(const uri& schema_path, std::size_t max_items)
            : keyword_validator_base<Json>("maxItems", schema_path), max_items_(max_items)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (instance.size() > max_items_)
            {
                std::string message("Expected maximum item count: " + std::to_string(max_items_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(),
                        instance_location.to_string(), 
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
        min_items_validator(const uri& schema_path, std::size_t min_items)
            : keyword_validator_base<Json>("minItems", schema_path), min_items_(min_items)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (instance.size() < min_items_)
            {
                std::string message("Expected maximum item count: " + std::to_string(min_items_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(),
                        instance_location.to_string(), 
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
        schema_validator_type additional_items_validator_;
    public:
        items_array_validator(const uri& schema_path, 
            std::vector<schema_validator_type>&& item_validators,
            schema_validator_type&& additional_items_validator)
            : keyword_validator_base<Json>("items", schema_path), 
              item_validators_(std::move(item_validators)), 
              additional_items_validator_(std::move(additional_items_validator))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            size_t index = 0;
            auto validator_it = item_validators_.cbegin();

            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            for (const auto& item : instance.array_range()) 
            {
                jsonpointer::json_pointer pointer(instance_location);

                if (validator_it != item_validators_.cend())
                {
                    evaluation_context<Json> item_context{this_context, index};
                    pointer /= index++;
                    (*validator_it)->validate(item_context, item, pointer, evaluated_properties, reporter, patch);
                    ++validator_it;
                }
                else if (additional_items_validator_ != nullptr)
                {
                    pointer /= index++;
                    additional_items_validator_->validate(eval_context, item, pointer, evaluated_properties, reporter, patch);
                }
                else
                    break;

            }
        }
    };

    template <class Json>
    class items_object_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type items_validator_;
    public:
        items_object_validator(const uri& schema_path, 
            schema_validator_type&& items_validator)
            : keyword_validator_base<Json>("items", schema_path), 
              items_validator_(std::move(items_validator))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            size_t index = 0;
            if (items_validator_)
            {
                for (const auto& item : instance.array_range()) 
                {
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= index;
                    items_validator_->validate(this_context, item, pointer, evaluated_properties, reporter, patch);
                    index++;
                }
            }
        }
    };

    // items

    template <class Json>
    class prefix_items_validator : public keyword_validator_base<Json>
    {
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;

        std::vector<schema_validator_type> item_validators_;
        schema_validator_type additional_items_validator_;
    public:
        prefix_items_validator(const uri& schema_path, 
            std::vector<schema_validator_type>&& item_validators,
            schema_validator_type&& additional_items_validator)
            : keyword_validator_base<Json>("prefixItems", schema_path), 
              item_validators_(std::move(item_validators)), 
              additional_items_validator_(std::move(additional_items_validator))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter,
            Json& patch) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            size_t index = 0;
            auto validator_it = item_validators_.cbegin();

            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            for (const auto& item : instance.array_range()) 
            {
                jsonpointer::json_pointer pointer(instance_location);

                if (validator_it != item_validators_.cend())
                {
                    evaluation_context<Json> item_context{this_context, index};
                    pointer /= index++;
                    (*validator_it)->validate(item_context, item, pointer, evaluated_properties, reporter, patch);
                    ++validator_it;
                }
                else if (additional_items_validator_ != nullptr)
                {
                    pointer /= index++;
                    additional_items_validator_->validate(eval_context, item, pointer, evaluated_properties, reporter, patch);
                }
                else
                    break;

            }
        }
    };

    // uniqueItems

    template <class Json>
    class unique_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        bool are_unique_;
    public:
        unique_items_validator(const uri& schema_path, bool are_unique)
            : keyword_validator_base<Json>("uniqueItems", schema_path), are_unique_(are_unique)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_array())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (are_unique_ && !array_has_unique_items(instance))
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
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
        min_length_validator(const uri& schema_path, std::size_t min_length)
            : keyword_validator_base<Json>("minLength", schema_path), min_length_(min_length)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_string())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            auto sv = instance.as_string_view();
            std::size_t length = unicode_traits::count_codepoints(sv.data(), sv.size());
            if (length < min_length_) 
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
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
        not_validator(const uri& schema_path,
            schema_validator_type&& rule)
            : keyword_validator_base<Json>("not", schema_path), 
              rule_(std::move(rule))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            collecting_error_reporter local_reporter;
            rule_->validate(this_context, instance, instance_location, evaluated_properties, local_reporter, patch);

            if (local_reporter.errors.empty())
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Instance must not be valid against schema"));
            }
        }
    };

    template <class Json>
    struct all_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("allOf");
            return k;
        }

        static bool is_complete(const evaluation_context<Json>& eval_context, 
            const Json&, 
            const jsonpointer::json_pointer& instance_location, 
            error_reporter& reporter, 
            const collecting_error_reporter& local_reporter, 
            std::size_t)
        {
            if (!local_reporter.errors.empty())
                reporter.error(validation_output(key(),
                    eval_context.eval_path(), 
                    uri(""),
                    instance_location.to_string(), 
                    "At least one schema failed to match, but all are required to match. ", 
                    local_reporter.errors));
            return !local_reporter.errors.empty();
        }
    };

    template <class Json>
    struct any_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("anyOf");
            return k;
        }

        static bool is_complete(const evaluation_context<Json>&, 
            const Json&, 
            const jsonpointer::json_pointer&, 
            error_reporter&, 
            const collecting_error_reporter&, 
            std::size_t count)
        {
            //std::cout << "any_of_criterion is_complete\n";
            return count == 1;
        }
    };

    template <class Json>
    struct one_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("oneOf");
            return k;
        }

        static bool is_complete(const evaluation_context<Json>& eval_context, 
            const Json&, 
            const jsonpointer::json_pointer& instance_location, 
            error_reporter& reporter, 
            const collecting_error_reporter&, 
            std::size_t count)
        {
            if (count > 1)
            {
                std::string message(std::to_string(count));
                message.append(" subschemas matched, but exactly one is required to match");
                reporter.error(validation_output(key(),
                    eval_context.eval_path(), 
                    uri("XXX"), 
                    instance_location.to_string(), 
                    std::move(message)));
            }
            return count > 1;
        }
    };

    template <class Json,class Criterion>
    class combining_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::vector<schema_validator_type> validators_;

    public:
        combining_validator(const uri& schema_path,
             std::vector<schema_validator_type>&& validators)
            : keyword_validator_base<Json>(Criterion::key(), std::move(schema_path)),
              validators_(std::move(validators))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "combining_validator.do_validate " << "keywordLocation: << " << this->schema_path().string() << ", instanceLocation:" << instance_location.to_string() << "\n";

            size_t count = 0;

            collecting_error_reporter local_reporter;

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            bool is_complete = false;
            for (std::size_t i = 0; i < validators_.size(); ++i) 
            {
                evaluation_context<Json> item_context(this_context, i);

                std::size_t mark = local_reporter.errors.size();
                validators_[i]->validate(item_context, instance, instance_location, evaluated_properties, local_reporter, patch);
                if (!is_complete)
                {
                    if (mark == local_reporter.errors.size())
                        count++;
                    if (Criterion::is_complete(item_context, instance, instance_location, reporter, local_reporter, count))
                        is_complete = true;
                }
            }

            if (count == 0)
            {
                reporter.error(validation_output(Criterion::key(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "No schema matched, but one of them is required to match", 
                    local_reporter.errors));
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
        any_of_validator(const uri& schema_path,
             std::vector<schema_validator_type>&& validators)
            : keyword_validator_base<Json>("anyOf", schema_path),
              validators_(std::move(validators))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "any_of_validator.do_validate " << eval_context.eval_path().to_string() << ", " << instance << "\n";

            collecting_error_reporter local_reporter;

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            bool is_complete = false;
            for (std::size_t i = 0; i < validators_.size() && !is_complete; ++i) 
            {
                evaluation_context<Json> item_context(this_context, i);

                std::size_t mark = local_reporter.errors.size();
                validators_[i]->validate(item_context, instance, instance_location, evaluated_properties, local_reporter, patch);
                if (mark == local_reporter.errors.size())
                {
                    is_complete = true;
                }
                //std::cout << "is_complete: " << i << " " << is_complete << "\n";
            }

            if (!is_complete)
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "No schema matched, but one of them is required to match", 
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
        maximum_validator(const uri& schema_path, const Json& value)
            : keyword_validator_base<Json>("maximum", schema_path), value_(value),
              message_{" is greater than maximum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() > value_.template as<int64_t>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() > value_.template as<double>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
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
        exclusive_maximum_validator(const uri& schema_path, const Json& value)
            : keyword_validator_base<Json>("exclusiveMaximum", schema_path), value_(value),
              message_{" is no less than exclusiveMaximum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() >= value_.template as<int64_t>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() >= value_.template as<double>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
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
        minimum_validator(const uri& schema_path, const Json& value)
            : keyword_validator_base<Json>("minimum", schema_path), value_(value),
              message_{" is less than minimum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() < value_.template as<int64_t>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() < value_.template as<double>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
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
        exclusive_minimum_validator(const uri& schema_path, const Json& value)
            : keyword_validator_base<Json>("exclusiveMinimum", schema_path), value_(value),
              message_{" is no greater than exclusiveMinimum " + value.template as<std::string>()}
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            switch (instance.type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                {
                    if (instance.template as<int64_t>() <= value_.template as<int64_t>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
                            instance.template as<std::string>() + message_));
                    }
                    break;
                }
                case json_type::double_value:
                {
                    if (instance.template as<double>() <= value_.template as<double>())
                    {
                        reporter.error(validation_output(this->keyword_name(),
                            this_context.eval_path(), 
                            this->schema_path(), 
                            instance_location.to_string(), 
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
        multiple_of_validator(const uri& schema_path, double value)
            : keyword_validator_base<Json>("multipleOf", schema_path), value_(value)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final
        {
            if (!instance.is_number())
            {
                return;
            }
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            double value = instance.template as<double>();
            if (value != 0) // Exclude zero
            {
                if (!is_multiple_of(value, static_cast<double>(value_)))
                {
                    reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(),
                        instance_location.to_string(), 
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
        required_validator(const uri& schema_path,
            const std::vector<std::string>& items)
            : keyword_validator_base<Json>("required", schema_path), items_(items)
        {
        }

        required_validator(const required_validator&) = delete;
        required_validator(required_validator&&) = default;
        required_validator& operator=(const required_validator&) = delete;
        required_validator& operator=(required_validator&&) = default;

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& key : items_)
            {
                    if(instance.find(key) == instance.object_range().end())
                {
                        reporter.error(validation_output(this->keyword_name(),
                                                         this_context.eval_path(),
                                                         this->schema_path(),
                                                         instance_location.to_string(),
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
        max_properties_validator(const uri& schema_path, std::size_t max_properties)
            : keyword_validator_base<Json>("maxProperties", schema_path), max_properties_(max_properties)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_object())
            {
                if (instance.size() > max_properties_)
                {
                    return;
                }
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                std::string message("Maximum properties: " + std::to_string(max_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
                
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
        min_properties_validator(const uri& schema_path, std::size_t min_properties)
            : keyword_validator_base<Json>("minProperties", schema_path), min_properties_(min_properties)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (!instance.is_object())
            {
                return;
            }
            if (instance.size() < min_properties_)
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                std::string message("Maximum properties: " + std::to_string(min_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(),
                        this->schema_path(),
                        instance_location.to_string(),
                        std::move(message)));
                if (reporter.fail_early())
                {
                    return;
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
        unevaluated_properties_validator(const uri& schema_path,
            schema_validator_type&& validator
        )
            : keyword_validator_base<Json>("unevaluatedProperties", std::move(schema_path)), 
              validator_(std::move(validator))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "Evaluated properties\n";
            //for (const auto& s : evaluated_properties)
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
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                for (const auto& prop : instance.object_range()) 
                {
                    auto prop_it = evaluated_properties.find(prop.key());

                    // check if it is in "evaluated_properties"
                    if (prop_it == evaluated_properties.end()) 
                    {
                        
                        std::size_t error_count = reporter.error_count();
                        validator_->validate(this_context, prop.value() , instance_location, evaluated_properties, reporter, patch);
                        if (reporter.error_count() == error_count)
                        {
                            evaluated_properties.insert(prop.key());
                        }
                    }
                }
            }
        }
    };

    template <class Json>
    class conditional_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type if_validator_;
        schema_validator_type then_validator_;
        schema_validator_type else_validator_;

    public:
        conditional_validator(const uri& schema_path,
          schema_validator_type&& if_validator,
          schema_validator_type&& then_validator,
          schema_validator_type&& else_validator
        ) : keyword_validator_base<Json>("", std::move(schema_path)), 
              if_validator_(std::move(if_validator)), 
              then_validator_(std::move(then_validator)), 
              else_validator_(std::move(else_validator))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            if (if_validator_) 
            {
                collecting_error_reporter local_reporter;

                if_validator_->validate(this_context, instance, instance_location, evaluated_properties, local_reporter, patch);
                if (local_reporter.errors.empty()) 
                {
                    if (then_validator_)
                        then_validator_->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch);
                } 
                else 
                {
                    if (else_validator_)
                        else_validator_->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch);
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
        enum_validator(const uri& schema_path, const Json& sch)
            : keyword_validator_base<Json>("enum", schema_path), value_(sch)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

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
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
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
        const_validator(const uri& schema_path, const Json& sch)
            : keyword_validator_base<Json>("const", schema_path), value_(sch)
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter,
            Json&) const final
        {
            if (value_ != instance)
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
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

        type_validator(const uri& schema_path,
            std::vector<json_schema_type>&& expected_types)
            : keyword_validator_base<Json>("type", std::move(schema_path)),
              expected_types_(std::move(expected_types))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& /*valuated_properties*/,
            error_reporter& reporter, 
            Json& /*patch*/) const final
        {
            //std::cout << "type_validator.do_validate " << eval_context.eval_path().to_string() << instance << "\n";
            //for (auto& type : expected_types_ )
            //{
            //    std::cout << "    " << to_string(type) << "\n";
            //}

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

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
                                reporter.error(validation_output(this->keyword_name(),
                                    this_context.eval_path(), 
                                    this->schema_path(), 
                                    instance_location.to_string(), 
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
                ss << ", found " << instance.type();

                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    ss.str()));
                if (reporter.fail_early())
                {
                    return;
                }
            }
            //std::cout << "\n";
        }
    };

    template <class Json>
    class properties_validator : public keyword_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        std::map<std::string, schema_validator_type> properties_;
    public:
        properties_validator(const uri& schema_path,
            std::map<std::string, schema_validator_type>&& properties
        )
            : keyword_base<Json>("properties", std::move(schema_path)), 
              properties_(std::move(properties))
        {
        }

        void validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch,
            std::unordered_set<std::string>& all_properties) const 
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& prop : instance.object_range()) 
            {
                evaluation_context<Json> prop_context{this_context, prop.key()};

                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                auto properties_it = properties_.find(prop.key());

                // check if it is in "properties"
                if (properties_it != properties_.end()) 
                {
                    std::size_t error_count = reporter.error_count();
                    properties_it->second->validate(prop_context, prop.value() , pointer, evaluated_properties, reporter, patch);
                    all_properties.insert(prop.key());
                    if (reporter.error_count() == error_count)
                    {
                        evaluated_properties.insert(prop.key());
                    }
                }
            }
                // Any property that doesn't match any of the property names in the properties keyword is ignored by this keyword.

            // reverse search
            for (auto const& prop : properties_) 
            {
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
        }

    private:

        void update_patch(Json& patch, const jsonpointer::json_pointer& instance_location, Json&& default_value) const
        {
            Json j;
            j.try_emplace("op", "add"); 
            j.try_emplace("path", instance_location.to_string()); 
            j.try_emplace("value", std::forward<Json>(default_value)); 
            patch.push_back(std::move(j));
        }
    };

    template <class Json>
    class pattern_properties_validator : public keyword_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;
        std::vector<std::pair<std::regex, schema_validator_type>> pattern_properties_;

    public:
        pattern_properties_validator(const uri& schema_path,
            std::vector<std::pair<std::regex, schema_validator_type>>&& pattern_properties
        )
            : keyword_base<Json>("patternProperties", std::move(schema_path)), 
              pattern_properties_(std::move(pattern_properties))
        {
        }

#if defined(JSONCONS_HAS_STD_REGEX)
        void validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location, 
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch,
            std::unordered_set<std::string>& all_properties) const 
        {
            if (!instance.is_object())
            {
                return;
            }
            evaluation_context<Json> this_context(eval_context, this->keyword_name());
            for (const auto& prop : instance.object_range()) 
            {
                evaluation_context<Json> prop_context{this_context, prop.key()};
                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                // check all matching "patternProperties"
                for (auto& schema_pp : pattern_properties_)
                    if (std::regex_search(prop.key(), schema_pp.first)) 
                    {
                        all_properties.insert(prop.key());
                        std::size_t error_count = reporter.error_count();
                        schema_pp.second->validate(prop_context, prop.value() , pointer, evaluated_properties, reporter, patch);
                        if (reporter.error_count() == error_count)
                        {
                            evaluated_properties.insert(prop.key());
                        }
                    }
            }
#endif
        }

    private:
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
        additional_properties_validator(const uri& schema_path,
            std::unique_ptr<properties_validator<Json>>&& properties,
            std::unique_ptr<pattern_properties_validator<Json>>&& pattern_properties,
            schema_validator_type&& additional_properties
        )
            : keyword_validator_base<Json>("additionalProperties", std::move(schema_path)), 
              properties_(std::move(properties)),
              pattern_properties_(std::move(pattern_properties)),
              additional_properties_(std::move(additional_properties))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            std::unordered_set<std::string> all_properties;

            if (properties_)
            {
                properties_->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch, all_properties);
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (pattern_properties_)
            {
                pattern_properties_->validate(this_context, instance, instance_location, evaluated_properties, reporter, patch, all_properties);
                if (reporter.fail_early())
                {
                    return;
                }
            }

            for (const auto& prop : instance.object_range()) 
            {
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

                        additional_properties_->validate(this_context, prop.value() , pointer, evaluated_properties, local_reporter, patch);
                        if (!local_reporter.errors.empty())
                        {
                            reporter.error(validation_output(this->keyword_name(),
                                this_context.eval_path(), 
                                additional_properties_->schema_path().string(),
                                instance_location.to_string(), 
                                "Additional prop \"" + prop.key() + "\" found but was invalid."));
                            if (reporter.fail_early())
                            {
                                return;
                            }
                        }
                        else
                        {
                            evaluated_properties.insert(prop.key());
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
        dependent_required_validator(const uri& schema_path,
            std::map<std::string, keyword_validator_type>&& dependent_required
        )
            : keyword_validator_base<Json>("dependentRequired", std::move(schema_path)), 
              dependent_required_(std::move(dependent_required))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& dep : dependent_required_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, evaluated_properties, reporter, patch); // validate
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
        dependent_schemas_validator(const uri& schema_path,
            std::map<std::string, schema_validator_type>&& dependent_schemas
        )
            : keyword_validator_base<Json>("dependentSchemas", std::move(schema_path)), 
              dependent_schemas_(std::move(dependent_schemas))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& dep : dependent_schemas_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, evaluated_properties, reporter, patch); // validate
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
        property_names_validator(const uri& schema_path,
            schema_validator_type&& property_names_schema_validator
        )
            : keyword_validator_base<Json>("propertyNames", schema_path), 
                property_names_schema_validator_{ std::move(property_names_schema_validator) }
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& prop : instance.object_range()) 
            {
                jsonpointer::json_pointer pointer(instance_location);
                pointer /= prop.key();

                if (property_names_schema_validator_)
                {
                    property_names_schema_validator_->validate(this_context, prop.key() , instance_location, evaluated_properties, reporter, patch);
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
        dependencies_validator(const uri& schema_path,
            std::map<std::string, keyword_validator_type>&& dependent_required,
            std::map<std::string, schema_validator_type>&& dependent_schemas
        )
            : keyword_validator_base<Json>("dependencies", std::move(schema_path)), 
              dependent_required_(std::move(dependent_required)),
              dependent_schemas_(std::move(dependent_schemas))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            if (!instance.is_object())
            {
                return;
            }

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            for (const auto& dep : dependent_required_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-prop is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(this_context, instance, pointer, evaluated_properties, reporter, patch); // validate
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
                    dep.second->validate(this_context, instance, pointer, evaluated_properties, reporter, patch); // validate
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
        max_contains_keyword(const uri& schema_path, std::size_t max_value)
            : keyword_base<Json>("maxContains", schema_path), max_value_(max_value)
        {
        }

        void validate(const evaluation_context<Json>& eval_context, 
            const jsonpointer::json_pointer& instance_location,
            std::size_t count, 
            error_reporter& reporter) const 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (count > max_value_)
            {
                std::string message("Expected maxContains: " + std::to_string(max_value_));
                message.append(", actual: " + std::to_string(count));
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(),
                        instance_location.to_string(), 
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
        min_contains_keyword(const uri& schema_path, std::size_t min_value)
            : keyword_base<Json>("minContains", schema_path), min_value_(min_value)
        {
        }

        void validate(const evaluation_context<Json>& eval_context, 
            const jsonpointer::json_pointer& instance_location,
            std::size_t count, 
            error_reporter& reporter) const 
        {
            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            if (count < min_value_)
            {
                std::string message("Expected minContains: " + std::to_string(min_value_));
                message.append(", actual: " + std::to_string(count));
                reporter.error(validation_output(this->keyword_name(),
                        this_context.eval_path(), 
                        this->schema_path(),
                        instance_location.to_string(), 
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
        contains_validator(const uri& schema_path,
            schema_validator_type&& schema_validator)
            : keyword_validator_base<Json>("contains", std::move(schema_path)), 
              schema_validator_(std::move(schema_validator))
        {
        }

        contains_validator(const uri& schema_path,
            schema_validator_type&& schema_validator,
            std::unique_ptr<max_contains_keyword<Json>>&& max_contains,
            std::unique_ptr<min_contains_keyword<Json>>&& min_contains)
            : keyword_validator_base<Json>("contains", std::move(schema_path)), 
              schema_validator_(std::move(schema_validator)),
              max_contains_(std::move(max_contains)),
              min_contains_(std::move(min_contains))
        {
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
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

            evaluation_context<Json> this_context(eval_context, this->keyword_name());

            std::size_t contains_count = 0;
            collecting_error_reporter local_reporter;
            for (const auto& item : instance.array_range()) 
            {
                std::size_t mark = local_reporter.errors.size();
                schema_validator_->validate(this_context, item, instance_location, evaluated_properties, local_reporter, patch);
                if (mark == local_reporter.errors.size()) 
                {
                    ++contains_count;
                }
            }
            if (max_contains_ && min_contains_)
            {
                max_contains_->validate(this_context, instance_location, contains_count, reporter);
                min_contains_->validate(this_context, instance_location, contains_count, reporter);
            }
            else if (contains_count == 0)
            {
                reporter.error(validation_output(this->keyword_name(),
                    this_context.eval_path(), 
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Expected at least one array item to match \"contains\" schema", 
                    local_reporter.errors));
                if (reporter.fail_early())
                {
                    return;
                }
            }
            
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORDS_HPP
