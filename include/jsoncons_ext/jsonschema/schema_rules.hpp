// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_RULES_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_RULES_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/subschema.hpp>
#include <jsoncons_ext/jsonschema/format_checkers.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_builder
    {
    public:
        using schema_pointer = typename subschema<Json>::schema_pointer;

        virtual schema_pointer build(const Json& schema,
                                     const std::vector<std::string>& keys,
                                     const std::vector<uri_wrapper>& uris) = 0;
        virtual schema_pointer make_required_rule(const std::vector<std::string>& r) = 0;

        virtual schema_pointer make_null_rule() = 0;

        virtual schema_pointer make_true_rule() = 0;

        virtual schema_pointer make_false_rule() = 0;

        virtual schema_pointer make_object_rule(const Json& sch, 
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_array_rule(const Json& sch,
                                                 const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_string_rule(const Json& sch) = 0;

        virtual schema_pointer make_boolean_rule() = 0;

        virtual schema_pointer make_integer_rule(const Json& sch, std::set<std::string>& keywords) = 0;

        virtual schema_pointer make_number_rule(const Json& sch, std::set<std::string>& keywords) = 0;

        virtual schema_pointer make_not_rule(const Json& schema,
                                               const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_all_of_rule(const Json& schema,
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_any_of_rule(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_one_of_rule(const Json& schema,
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_type_rule(const Json& schema,
                                                const std::vector<uri_wrapper>& uris) = 0;
    };

    struct local_error_reporter : public error_reporter
    {
        std::vector<validation_error> errors;

        bool error_{false};
        std::string message_;

        operator bool() const { return error_; }

    private:
        void do_error(const validation_error& e) override
        {
            errors.push_back(e);
            if (*this)
                return;
            error_ = true;
            message_ = e.what();
        }
    };

    template <class Json>
    void update_patch(Json& patch, const jsoncons::jsonpointer::json_pointer& ptr, Json&& default_value)
    {
        Json j;
        j.try_emplace("op", "add"); 
        j.try_emplace("path", ptr.string()); 
        j.try_emplace("value", std::forward<Json>(default_value)); 

        patch.push_back(std::move(j));
    }

    // string rule

    template <class Json>
    void content_media_type_check(const jsoncons::jsonpointer::json_pointer& ptr, const Json&, 
                                  const std::string& content_media_type, const std::string& content,
                                  error_reporter& reporter)
    {
        if (content_media_type == "application/Json")
        {
            json_reader reader(content);
            std::error_code ec;
            reader.read(ec);

            if (ec)
            {
                reporter.error(validation_error(ptr.string(), std::string("Content is not JSON: ") + ec.message(), "contentMediaType"));
            }
        }
    }

    template <class Json>
    class string_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_length_;
        jsoncons::optional<std::size_t> min_length_;

    #if defined(JSONCONS_HAS_STD_REGEX)
        jsoncons::optional<std::regex> pattern_;
        std::string pattern_string_;
    #endif

        format_checker format_check_;

        jsoncons::optional<std::string> content_encoding_;
        jsoncons::optional<std::string> content_media_type_;

    public:
        string_rule(const Json& sch)
            : max_length_(), min_length_(), 
    #if defined(JSONCONS_HAS_STD_REGEX)
              pattern_(),
    #endif
              content_encoding_(), content_media_type_()
        {
            auto it = sch.find("maxLength");
            if (it != sch.object_range().end()) 
            {
                max_length_ = it->value().template as<std::size_t>();
            }

            it = sch.find("minLength");
            if (it != sch.object_range().end()) 
            {
                min_length_ = it->value().template as<std::size_t>();
            }

            it = sch.find("contentEncoding");
            if (it != sch.object_range().end()) 
            {
                content_encoding_ = it->value().template as<std::string>();
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = sch.find("contentMediaType");
            if (it != sch.object_range().end()) 
            {
                content_media_type_ = it->value().template as<std::string>();
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("pattern");
            if (it != sch.object_range().end()) 
            {
                pattern_string_ = it->value().template as<std::string>();
                pattern_ = std::regex(it->value().template as<std::string>(),std::regex::ECMAScript);
            }
    #endif

            it = sch.find("format");
            if (it != sch.object_range().end()) 
            {
                std::string format = it->value().template as<std::string>();
                if (format == "date-time")
                {
                    format_check_ = rfc3339_date_time_check;
                }
                else if (format == "date") 
                {
                    format_check_ = rfc3339_date_check;
                } 
                else if (format == "time") 
                {
                    format_check_ = rfc3339_time_check;
                } 
                else if (format == "email") 
                {
                    format_check_ = email_check;
                } 
                else if (format == "hostname") 
                {
                    format_check_ = hostname_check;
                } 
                else if (format == "ipv4") 
                {
                    format_check_ = ipv4_check;
                } 
                else if (format == "ipv6") 
                {
                    format_check_ = ipv6_check;
                } 
                else if (format == "regex") 
                {
                    format_check_ = regex_check;
                } 
                else
                {
                    // Not supported - ignore
                }
            }
        }

    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const override
        {
            std::string content;
            if (content_encoding_)
            {
                if (*content_encoding_ == "base64")
                {
                    auto s = instance.template as<jsoncons::string_view>();
                    auto retval = jsoncons::decode_base64(s.begin(), s.end(), content);
                    if (retval.ec != jsoncons::convert_errc::success)
                    {
                        reporter.error(validation_error(ptr.string(), "Content is not a base64 string", "contentEncoding"));
                    }
                }
                else if (!content_encoding_->empty())
                {
                    reporter.error(validation_error(ptr.string(), "unable to check for contentEncoding '" + *content_encoding_ + "'", "contentEncoding"));
                }
            }
            else
            {
                content = instance.template as<std::string>();
            }

            if (content_media_type_) 
            {
                content_media_type_check(ptr, instance, *content_media_type_, content, reporter);
            } 
            else if (instance.type() == json_type::byte_string_value) 
            {
                reporter.error(validation_error(ptr.string(), "Expected string, but is byte string", "contentMediaType"));
            }

            if (instance.type() != json_type::string_value) 
            {
                return; 
            }

            if (min_length_) 
            {
                std::size_t length = unicons::u32_length(content.begin(), content.end());
                if (length < *min_length_) 
                {
                    reporter.error(validation_error(ptr.string(), std::string("Expected minLength: ") + std::to_string(*min_length_)
                                              + ", actual: " + std::to_string(length), "minLength"));
                }
            }

            if (max_length_) 
            {
                std::size_t length = unicons::u32_length(content.begin(), content.end());
                if (length > *max_length_)
                {
                    reporter.error(validation_error(ptr.string(), std::string("Expected maxLength: ") + std::to_string(*max_length_)
                        + ", actual: " + std::to_string(length), "maxLength"));
                }
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            if (pattern_)
            {
                if (!std::regex_search(content, *pattern_))
                {
                    std::string message("String \"");
                    message.append(instance.template as<std::string>());
                    message.append("\" does not match pattern \"");
                    message.append(pattern_string_);
                    message.append("\"");
                    reporter.error(validation_error(ptr.string(), message, "pattern"));
                }
            }

    #endif

            if (format_check_ != nullptr) 
            {
                format_check_(ptr, content, reporter);
            }
        }
    };

    // not_rule

    template <class Json>
    class not_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        schema_pointer rule_;

    public:
        not_rule()
            : rule_(nullptr)
        {
        }

        not_rule(schema_builder<Json>* builder,
                 const Json& sch,
                 const std::vector<uri_wrapper>& uris)
        {
            rule_ = builder->build(sch, {"not"}, uris);
        }

    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            local_error_reporter local_reporter;
            rule_->validate(ptr, instance, local_reporter, patch);

            if (local_reporter.errors.empty())
                reporter.error(validation_error(ptr.string(), "Instance must not be valid against schema", "not"));
        }

        jsoncons::optional<Json> get_default_value(const jsoncons::jsonpointer::json_pointer& ptr, 
                                                   const Json& instance, 
                                                   error_reporter& reporter) const override
        {
            return rule_->get_default_value(ptr, instance, reporter);
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

        static bool is_complete(const Json&, 
                                const jsoncons::jsonpointer::json_pointer& ptr, 
                                error_reporter& reporter, 
                                const local_error_reporter& local_reporter, 
                                std::size_t)
        {
            if (local_reporter)
                reporter.error(validation_error(ptr.string(), "At least one subschema failed to match, but all are required to match. " + local_reporter.message_, "allOf"));
            return local_reporter;
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

        static bool is_complete(const Json&, 
                                const jsoncons::jsonpointer::json_pointer&, 
                                error_reporter&, 
                                const local_error_reporter&, 
                                std::size_t count)
        {
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

        static bool is_complete(const Json&, 
                                const jsoncons::jsonpointer::json_pointer& ptr, 
                                error_reporter& reporter, 
                                const local_error_reporter&, 
                                std::size_t count)
        {
            if (count > 1)
            {
                std::string message(std::to_string(count));
                message.append(" subschemas matched, but exactly one is required to match");
                reporter.error(validation_error(ptr.string(), message, "oneOf"));
            }
            return count > 1;
        }
    };

    template <class Json,class Criterion>
    class combining_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        std::vector<schema_pointer> subschemas_;

    public:
        combining_rule(schema_builder<Json>* builder,
                       const Json& sch,
                       const std::vector<uri_wrapper>& uris)
        {
            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                subschemas_.push_back(builder->build(subsch, {Criterion::key(), std::to_string(c++)}, uris));
            }

            // Validate value of allOf, anyOf, and oneOf "MUST be a non-empty array"
        }

    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            size_t count = 0;

            for (auto& s : subschemas_) 
            {
                local_error_reporter local_reporter;
                s->validate(ptr, instance, local_reporter, patch);
                if (!local_reporter)
                    count++;

                if (Criterion::is_complete(instance, ptr, reporter, local_reporter, count))
                    return;
            }

            if (count == 0)
            {
                reporter.error(validation_error(ptr.string(), "No subschema matched, but one of them is required to match", "combined"));
            }
        }
    };

    template <class Json,class T>
    class number_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        jsoncons::optional<T> maximum_;
        jsoncons::optional<T> minimum_;
        bool exclusive_maximum_;
        bool exclusive_minimum_;
        jsoncons::optional<double> multiple_of_;

    public:
        number_rule(const Json& sch, std::set<std::string>& keywords)
            : maximum_(), minimum_(),exclusive_maximum_(false), exclusive_minimum_(false), multiple_of_()
        {
            auto it = sch.find("maximum");
            if (it != sch.object_range().end()) 
            {
                maximum_ = it->value().template as<T>();
                keywords.insert("maximum");
            }

            it = sch.find("minimum");
            if (it != sch.object_range().end()) 
            {
                minimum_ = it->value().template as<T>();
                keywords.insert("minimum");
            }

            it = sch.find("exclusiveMaximum");
            if (it != sch.object_range().end()) 
            {
                exclusive_maximum_ = true;
                maximum_ = it->value().template as<T>();
                keywords.insert("exclusiveMaximum");
            }

            it = sch.find("exclusiveMinimum");
            if (it != sch.object_range().end()) 
            {
                minimum_ = it->value().template as<T>();
                exclusive_minimum_ = true;
                keywords.insert("exclusiveMinimum");
            }

            it = sch.find("multipleOf");
            if (it != sch.object_range().end()) 
            {
                multiple_of_ = it->value().template as<double>();
                keywords.insert("multipleOf");
            }
        }

    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json&) const override
        {
            T value = instance.template as<T>(); // conversion of Json to value_type
            if (Json(value) != instance)
            {
                reporter.error(validation_error(ptr.string(), "Instance is not a number", "number"));
            }

            if (multiple_of_ && value != 0) // zero is multiple of everything
                if (violates_multiple_of(value))
                {
                    reporter.error(validation_error(ptr.string(), instance.template as<std::string>() + " is not a multiple of " + std::to_string(*multiple_of_), "multipleOf"));
                }

            if (maximum_)
                if ((exclusive_maximum_ && value >= *maximum_) ||
                    value > *maximum_)
                    reporter.error(validation_error(ptr.string(), instance.template as<std::string>() + " exceeds maximum of " + std::to_string(*maximum_), "maximum"));

            if (minimum_)
                if ((exclusive_minimum_ && value <= *minimum_) ||
                    value < *minimum_)
                    reporter.error(validation_error(ptr.string(), instance.template as<std::string>() + " is below minimum of " + std::to_string(*minimum_), "minimum"));
        }

        // multipleOf - if the remainder of the division is 0 -> OK
        bool violates_multiple_of(T x) const
        {
            double res = std::remainder(x, *multiple_of_);
            double eps = std::nextafter(x, 0) - x;
            return std::fabs(res) > std::fabs(eps);
        }
    };

    // null_rule

    template <class Json>
    class null_rule : public subschema<Json>
    {
    public:
        using schema_pointer = typename subschema<Json>::schema_pointer;

        null_rule() = default;
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, const Json& instance, error_reporter& reporter, Json&) const override
        {
            if (!instance.is_null())
            {
                reporter.error(validation_error(ptr.string(), "Expected to be null", "null"));
            }
        }
    };

    template <class Json>
    class boolean_rule : public subschema<Json>
    {
    public:
        using schema_pointer = typename subschema<Json>::schema_pointer;

        boolean_rule() = default;
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer&, const Json&, error_reporter&, Json&) const override
        {
        }

    };

    template <class Json>
    class true_rule : public subschema<Json>
    {
    public:
        using schema_pointer = typename subschema<Json>::schema_pointer;

        true_rule() = default;
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer&, const Json&, error_reporter&, Json&) const override
        {
        }
    };

    template <class Json>
    class false_rule : public subschema<Json>
    {
    public:
        using schema_pointer = typename subschema<Json>::schema_pointer;

        false_rule() = default;
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, const Json&, error_reporter& reporter, Json&) const override
        {
            reporter.error(validation_error(ptr.string(), "False schema always fails", "false"));
        }
    };

    template <class Json>
    class required_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        const std::vector<std::string> required_;

    public:
        required_rule(const std::vector<std::string>& r)
            : required_(r) {}
    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, const Json& instance, error_reporter& reporter, Json&) const override final
        {
            for (const auto& key : required_)
            {
                if (instance.find(key) == instance.object_range().end())
                {
                    reporter.error(validation_error(ptr.string(), "Required key \"" + key + "\" not found", "required"));
                }
            }
        }
    };

    template <class Json>
    class object_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_properties_;
        jsoncons::optional<std::size_t> min_properties_;
        std::vector<std::string> required_;

        std::map<std::string, schema_pointer> properties_;
    #if defined(JSONCONS_HAS_STD_REGEX)
        std::vector<std::pair<std::regex, schema_pointer>> pattern_properties_;
    #endif
        schema_pointer additional_properties_;

        std::map<std::string, schema_pointer> dependencies_;

        schema_pointer property_names_;

    public:
        object_rule(schema_builder<Json>* builder,
                    const Json& sch,
                    const std::vector<uri_wrapper>& uris)
            : max_properties_(), min_properties_(), 
              additional_properties_(nullptr),
              property_names_(nullptr)
        {
            auto it = sch.find("maxProperties");
            if (it != sch.object_range().end()) 
            {
                max_properties_ = it->value().template as<std::size_t>();
            }

            it = sch.find("minProperties");
            if (it != sch.object_range().end()) 
            {
                min_properties_ = it->value().template as<std::size_t>();
            }

            it = sch.find("required");
            if (it != sch.object_range().end()) 
            {
                required_ = it->value().template as<std::vector<std::string>>();
            }

            it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    properties_.emplace(
                        std::make_pair(
                            prop.key(),
                            builder->build(prop.value(), {"properties", prop.key()}, uris)));
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("patternProperties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    pattern_properties_.push_back(
                        std::make_pair(
                            std::regex(prop.key(), std::regex::ECMAScript),
                            builder->build(prop.value(), {prop.key()}, uris)));
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                additional_properties_ = builder->build(it->value(), {"additionalProperties"}, uris);
            }

            it = sch.find("dependencies");
            if (it != sch.object_range().end()) 
            {
                for (const auto& dep : it->value().object_range())
                {
                    switch (dep.value().type()) 
                    {
                    case json_type::array_value:
                        dependencies_.emplace(dep.key(),
                                              builder->make_required_rule(
                                                  dep.value().template as<std::vector<std::string>>()));
                        break;
                    
                    default:
                        dependencies_.emplace(dep.key(),
                                              builder->build(dep.value(), {"dependencies", dep.key()}, uris));
                        break;
                    }
                }
            }

            auto property_names_it = sch.find("propertyNames");
            if (property_names_it != sch.object_range().end()) 
            {
                property_names_ = builder->build(property_names_it->value(), {"propertyNames"}, uris);
            }
        }
    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_properties_ && instance.size() > *max_properties_)
            {
                std::string message("Maximum properties: " + std::to_string(*max_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_error(ptr.string(), message, "maxProperties"));
            }

            if (min_properties_ && instance.size() < *min_properties_)
            {
                std::string message("Minimum properties: " + std::to_string(*min_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_error(ptr.string(), message, "minProperties"));
            }

            for (auto& key : required_)
                if (instance.find(key) == instance.object_range().end())
                    reporter.error(validation_error(ptr.string(), "Required key \"" + key + "\" not found", "required"));

            for (const auto& property : instance.object_range()) 
            {
                if (property_names_)
                    property_names_->validate(ptr, property.key(), reporter, patch);

                bool a_prop_or_pattern_matched = false;
                auto properties_it = properties_.find(property.key());

                // check if it is in "properties"
                if (properties_it != properties_.end()) 
                {
                    a_prop_or_pattern_matched = true;
                    properties_it->second->validate(ptr / property.key(), property.value(), reporter, patch);
                }

    #if defined(JSONCONS_HAS_STD_REGEX)

                // check all matching "patternProperties"
                for (auto& schema_pp : pattern_properties_)
                    if (std::regex_search(property.key(), schema_pp.first)) 
                    {
                        a_prop_or_pattern_matched = true;
                        schema_pp.second->validate(ptr / property.key(), property.value(), reporter, patch);
                    }
    #endif

                // finally, check "additionalProperties" 
                if (!a_prop_or_pattern_matched && additional_properties_) 
                {
                    local_error_reporter local_reporter;
                    additional_properties_->validate(ptr / property.key(), property.value(), local_reporter, patch);
                    if (local_reporter)
                        reporter.error(validation_error(ptr.string(), "Validation failed for additional property \"" + property.key() + "\". " + local_reporter.message_, "additionalProperties"));
                }
            }

            // reverse search
            for (auto const& prop : properties_) 
            {
                const auto finding = instance.find(prop.first);
                if (finding == instance.object_range().end()) 
                { 
                    // If property is not in instance
                    auto default_value = prop.second->get_default_value(ptr, instance, reporter);
                    if (default_value) 
                    { 
                        // If default value is available, update patch
                        update_patch(patch, ptr / prop.first, std::move(*default_value));
                    }
                }
            }

            for (const auto& dep : dependencies_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end())                                    // if dependency-property is present in instance
                    dep.second->validate(ptr / dep.first, instance, reporter, patch); // validate
            }
        }
    };

    template <class Json>
    class array_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_items_;
        jsoncons::optional<std::size_t> min_items_;
        bool unique_items_ = false;
        schema_pointer items_schema_;
        std::vector<schema_pointer> items_;
        schema_pointer additional_items_;
        schema_pointer contains_;

    public:
        array_rule(schema_builder<Json>* builder, 
                   const Json& sch, 
                   const std::vector<uri_wrapper>& uris)
            : max_items_(), min_items_(), items_schema_(nullptr), additional_items_(nullptr), contains_(nullptr)
        {
            {
                auto it = sch.find("maxItems");
                if (it != sch.object_range().end()) 
                {
                    max_items_ = it->value().template as<std::size_t>();
                }
            }

            {
                auto it = sch.find("minItems");
                if (it != sch.object_range().end()) 
                {
                    min_items_ = it->value().template as<std::size_t>();
                }
            }

            {
                auto it = sch.find("uniqueItems");
                if (it != sch.object_range().end()) 
                {
                    unique_items_ = it->value().template as<bool>();
                }
            }

            {
                auto it = sch.find("items");
                if (it != sch.object_range().end()) 
                {

                    if (it->value().type() == json_type::array_value) 
                    {
                        size_t c = 0;
                        for (const auto& subsch : it->value().array_range())
                            items_.push_back(builder->build(subsch, {"items", std::to_string(c++)}, uris));

                        auto attr_add = sch.find("additionalItems");
                        if (attr_add != sch.object_range().end()) 
                        {
                            additional_items_ = builder->build(attr_add->value(), {"additionalItems"}, uris);
                        }

                    } 
                    else if (it->value().type() == json_type::object_value ||
                               it->value().type() == json_type::bool_value)
                    {
                        items_schema_ = builder->build(it->value(), {"items"}, uris);
                    }

                }
            }

            {
                auto it = sch.find("contains");
                if (it != sch.object_range().end()) 
                {
                    contains_ = builder->build(it->value(), {"contains"}, uris);
                }
            }
        }
    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_items_ && instance.size() > *max_items_)
            {
                std::string message("Expected maximum item count: " + std::to_string(*max_items_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_error(ptr.string(), message, "maxItems"));
            }

            if (min_items_ && instance.size() < *min_items_)
            {
                std::string message("Expected minimum item count: " + std::to_string(*min_items_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_error(ptr.string(), message, "minItems"));
            }

            if (unique_items_) 
            {
                for (auto it = instance.object_range().cbegin(); it != instance.object_range().cend(); ++it) 
                {
                    auto v = std::find(it + 1, instance.object_range().end(), *it);
                    if (v != instance.object_range().end())
                        reporter.error(validation_error(ptr.string(), "Array items are not unique", "uniqueItems"));
                }
            }

            size_t index = 0;
            if (items_schema_)
            {
                for (const auto& i : instance.array_range()) 
                {
                    items_schema_->validate(ptr / index, i, reporter, patch);
                    index++;
                }
            }
            else 
            {
                auto item = items_.cbegin();
                for (const auto& i : instance.array_range()) 
                {
                    schema_pointer item_validator = nullptr;
                    if (item == items_.cend())
                        item_validator = additional_items_;
                    else 
                    {
                        item_validator = *item;
                        item++;
                    }

                    if (!item_validator)
                        break;

                    item_validator->validate(ptr / index, i, reporter, patch);
                }
            }

            if (contains_) 
            {
                bool contained = false;
                for (const auto& item : instance.array_range()) 
                {
                    local_error_reporter local_reporter;
                    contains_->validate(ptr, item, local_reporter, patch);
                    if (!local_reporter) 
                    {
                        contained = true;
                        break;
                    }
                }
                if (!contained)
                    reporter.error(validation_error(ptr.string(), "Expected at least one array item to match \"contains\" schema", "contains"));
            }
        }
    };

    template <class Json>
    class conditional_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        schema_pointer if_;
        schema_pointer then_;
        schema_pointer else_;

    public:
        conditional_rule(schema_builder<Json>* builder,
                         const Json& sch_if,
                         const Json& sch,
                         const std::vector<uri_wrapper>& uris)
            : if_(nullptr), then_(nullptr), else_(nullptr)
        {
            auto then_it = sch.find("then");
            auto else_it = sch.find("else");

            if (then_it != sch.object_range().end() || else_it != sch.object_range().end()) 
            {
                if_ = builder->build(sch_if, {"if"}, uris);

                if (then_it != sch.object_range().end()) 
                {
                    then_ = builder->build(then_it->value(), {"then"}, uris);
                }

                if (else_it != sch.object_range().end()) 
                {
                    else_ = builder->build(else_it->value(), {"else"}, uris);
                }
            }
        }
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            if (if_) 
            {
                local_error_reporter local_reporter;

                if_->validate(ptr, instance, local_reporter, patch);
                if (!local_reporter) 
                {
                    if (then_)
                        then_->validate(ptr, instance, reporter, patch);
                } 
                else 
                {
                    if (else_)
                        else_->validate(ptr, instance, reporter, patch);
                }
            }
        }
    };

    // enum_rule

    template <class Json>
    class enum_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        Json enum_;

    public:
        enum_rule(const Json& sch)
            : enum_(sch)
        {
        }
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const final
        {
            bool in_range = false;
            for (const auto& item : enum_.array_range())
            {
                if (item == instance) 
                {
                    in_range = true;
                    break;
                }
            }

            if (!in_range)
            {
                reporter.error(validation_error(ptr.string(), instance.template as<std::string>() + " is not a valid enum value", "enum"));
            }
        }
    };

    // const_rule

    template <class Json>
    class const_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        Json const_;

    public:
        const_rule(const Json& sch)
            : const_(sch)
        {
        }
    private:
        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const final
        {
            if (const_ != instance)
                reporter.error(validation_error(ptr.string(), "Instance is not const", "const"));
        }
    };

    template <class Json>
    class type_rule : public subschema<Json>
    {
        using schema_pointer = typename subschema<Json>::schema_pointer;

        Json default_value_;
        std::vector<schema_pointer> type_mapping_;
        jsoncons::optional<enum_rule<Json>> enum_;
        jsoncons::optional<const_rule<Json>> const_;
        std::vector<schema_pointer> combined_;
        jsoncons::optional<conditional_rule<Json>> conditional_;
        std::vector<std::string> expected_types_;

    public:
        type_rule(const type_rule&) = delete;
        type_rule& operator=(const type_rule&) = delete;
        type_rule(type_rule&&) = default;
        type_rule& operator=(type_rule&&) = default;

        type_rule(schema_builder<Json>* builder,
                  const Json& sch,
                  const std::vector<uri_wrapper>& uris)
            : default_value_(jsoncons::null_type()), 
              type_mapping_((uint8_t)(json_type::object_value)+1), 
              enum_(), const_()
        {
            std::set<std::string> known_keywords;

            auto it = sch.find("type");
            if (it == sch.object_range().end()) 
            {
                initialize_type_mapping(builder, "", sch, uris, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        initialize_type_mapping(builder, type, sch, uris, known_keywords);
                        expected_types_.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            initialize_type_mapping(builder, type, sch, uris, known_keywords);
                            expected_types_.emplace_back(std::move(type));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            const auto default_it = sch.find("default");
            if (default_it != sch.object_range().end()) 
            {
                default_value_ = default_it->value();
            }

            it = sch.find("enum");
            if (it != sch.object_range().end()) 
            {
                enum_ = enum_rule<Json >(it->value());
            }

            it = sch.find("const");
            if (it != sch.object_range().end()) 
            {
                const_ = const_rule<Json>(it->value());
            }

            it = sch.find("not");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_not_rule(it->value(), uris));
            }

            it = sch.find("allOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_all_of_rule(it->value(), uris));
            }

            it = sch.find("anyOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_any_of_rule(it->value(), uris));
            }

            it = sch.find("oneOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_one_of_rule(it->value(), uris));
            }

            it = sch.find("if");
            if (it != sch.object_range().end()) 
            {
                conditional_ = conditional_rule<Json>(builder, it->value(), sch, uris);
            }
        }
    private:

        void do_validate(const jsoncons::jsonpointer::json_pointer& ptr, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override final
        {
            std::string ptr_s = ptr.string();

            auto type = type_mapping_[(uint8_t) instance.type()];

            if (type)
                type->validate(ptr, instance, reporter, patch);
            else
            {
                std::ostringstream ss;
                ss << "Expected ";
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
                        ss << expected_types_[i];
                }
                ss << ", found " << instance.type();

                reporter.error(validation_error(ptr.string(), ss.str(), "type"));
            }

            if (enum_)
            { 
                enum_->validate(ptr, instance, reporter, patch);
            }

            if (const_)
            { 
                const_->validate(ptr, instance, reporter, patch);
            }

            for (const auto& l : combined_)
                l->validate(ptr, instance, reporter, patch);


            if (conditional_)
            { 
                conditional_->validate(ptr, instance, reporter, patch);
            }
        }

        jsoncons::optional<Json> get_default_value(const jsoncons::jsonpointer::json_pointer&, 
                                                   const Json&,
                                                   error_reporter&) const override
        {
            return default_value_;
        }

        void initialize_type_mapping(schema_builder<Json>* builder,
                                     const std::string& type,
                                     const Json& sch,
                                     const std::vector<uri_wrapper>& uris,
                                     std::set<std::string>& keywords)
        {
            if (type.empty() || type == "null")
            {
                type_mapping_[(uint8_t)json_type::null_value] = builder->make_null_rule();
            }
            if (type.empty() || type == "object")
            {
                type_mapping_[(uint8_t)json_type::object_value] = builder->make_object_rule(sch, uris);
            }
            if (type.empty() || type == "array")
            {
                type_mapping_[(uint8_t)json_type::array_value] = builder->make_array_rule(sch, uris);
            }
            if (type.empty() || type == "string")
            {
                type_mapping_[(uint8_t)json_type::string_value] = builder->make_string_rule(sch);
                // For binary types
                type_mapping_[(uint8_t) json_type::byte_string_value] = type_mapping_[(uint8_t) json_type::string_value];
            }
            if (type.empty() || type == "boolean")
            {
                type_mapping_[(uint8_t)json_type::bool_value] = builder->make_boolean_rule();
            }
            if (type.empty() || type == "integer")
            {
                type_mapping_[(uint8_t)json_type::int64_value] = builder->make_integer_rule(sch, keywords);
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::int64_value];
                type_mapping_[(uint8_t)json_type::double_value] = type_mapping_[(uint8_t)json_type::int64_value];
            }
            if (type.empty() || type == "number")
            {
                type_mapping_[(uint8_t)json_type::double_value] = builder->make_number_rule(sch, keywords);
                type_mapping_[(uint8_t)json_type::int64_value] = type_mapping_[(uint8_t)json_type::double_value];
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::double_value];
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_VALUE_RULES_HPP
