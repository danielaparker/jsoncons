// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/subschema.hpp>
#include <jsoncons_ext/jsonschema/format_validator.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif
#include <jsoncons_ext/jsonschema/compilation_context.hpp>

namespace jsoncons {
namespace jsonschema {
    template <class Json>
    class abstract_keyword_validator_factory
    {
    public:
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        virtual ~abstract_keyword_validator_factory() = default;

        virtual validator_pointer make_keyword_validator(const Json& schema,
                                                         const compilation_context& context,
                                                         const std::vector<std::string>& keys) = 0;
        virtual validator_pointer make_required_validator(const compilation_context& context,
                                                          const std::vector<std::string>& items) = 0;

        virtual validator_pointer make_null_validator(const compilation_context& context) = 0;

        virtual validator_pointer make_true_validator(const compilation_context& context) = 0;

        virtual validator_pointer make_false_validator(const compilation_context& context) = 0;

        virtual validator_pointer make_object_validator(const Json& schema, 
                                                        const compilation_context& context) = 0;

        virtual validator_pointer make_array_validator(const Json& schema,
                                                       const compilation_context& context) = 0;

        virtual validator_pointer make_string_validator(const Json& schema,
                                                        const compilation_context& context) = 0;

        virtual validator_pointer make_boolean_validator(const compilation_context& context) = 0;

        virtual validator_pointer make_integer_validator(const Json& schema, 
                                                         const compilation_context& context, 
                                                         std::set<std::string>& keywords) = 0;

        virtual validator_pointer make_number_validator(const Json& schema, 
                                                        const compilation_context& context, 
                                                        std::set<std::string>& keywords) = 0;

        virtual validator_pointer make_not_validator(const Json& schema,
                                                     const compilation_context& context) = 0;

        virtual validator_pointer make_all_of_validator(const Json& schema,
                                                        const compilation_context& context) = 0;

        virtual validator_pointer make_any_of_validator(const Json& schema,
                                                        const compilation_context& context) = 0;

        virtual validator_pointer make_one_of_validator(const Json& schema,
                                                        const compilation_context& context) = 0;

        virtual validator_pointer make_type_validator(const Json& schema,
                                                      const compilation_context& context) = 0;
    };

    struct collecting_error_reporter : public error_reporter
    {
        std::vector<validation_output> errors;

    private:
        void do_error(const validation_output& o) override
        {
            errors.push_back(o);
        }
    };

    // string keyword_validator

    template <class Json>
    class string_validator : public keyword_validator<Json>
    {
        jsoncons::optional<std::size_t> max_length_;
        std::string max_length_location_;
        jsoncons::optional<std::size_t> min_length_;
        std::string min_length_location_;

    #if defined(JSONCONS_HAS_STD_REGEX)
        jsoncons::optional<std::regex> pattern_;
        std::string pattern_string_;
        std::string pattern_location_;
    #endif

        format_checker format_check_;
        std::string format_location_;

        jsoncons::optional<std::string> content_encoding_;
        std::string content_encoding_location_;
        jsoncons::optional<std::string> content_media_type_;
        std::string content_media_type_location_;

    public:
        string_validator(const Json& schema, const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), max_length_(), min_length_(), 
    #if defined(JSONCONS_HAS_STD_REGEX)
              pattern_(),
    #endif
              content_encoding_(), content_media_type_()
        {
            auto it = schema.find("maxLength");
            if (it != schema.object_range().end()) 
            {
                max_length_ = it->value().template as<std::size_t>();
                max_length_location_ = context.make_schema_path_with("maxLength");
            }

            it = schema.find("minLength");
            if (it != schema.object_range().end()) 
            {
                min_length_ = it->value().template as<std::size_t>();
                min_length_location_ = context.make_schema_path_with("minLength");
            }

            it = schema.find("contentEncoding");
            if (it != schema.object_range().end()) 
            {
                content_encoding_ = it->value().template as<std::string>();
                content_encoding_location_ = context.make_schema_path_with("contentEncoding");
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = schema.find("contentMediaType");
            if (it != schema.object_range().end()) 
            {
                content_media_type_ = it->value().template as<std::string>();
                content_media_type_location_ = context.make_schema_path_with("contentMediaType");
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = schema.find("pattern");
            if (it != schema.object_range().end()) 
            {
                pattern_string_ = it->value().template as<std::string>();
                pattern_ = std::regex(it->value().template as<std::string>(),std::regex::ECMAScript);
                pattern_location_ = context.make_schema_path_with("pattern");
            }
    #endif

            it = schema.find("format");
            if (it != schema.object_range().end()) 
            {
                format_location_ = context.make_schema_path_with("format");
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

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
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
                    if (retval.ec != jsoncons::conv_errc::success)
                    {
                        reporter.error(validation_output("contentEncoding", 
                                                         content_encoding_location_, 
                                                         instance_location.to_uri_fragment(), 
                                                         "Content is not a base64 string"));
                        if (reporter.fail_early())
                        {
                            return;
                        }
                    }
                }
                else if (!content_encoding_->empty())
                {
                    reporter.error(validation_output("contentEncoding", 
                                                     content_encoding_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     "unable to check for contentEncoding '" + *content_encoding_ + "'"));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
            else
            {
                content = instance.template as<std::string>();
            }

            if (content_media_type_) 
            {
                if (content_media_type_ == "application/Json")
                {
                    json_string_reader reader(content);
                    std::error_code ec;
                    reader.read(ec);

                    if (ec)
                    {
                        reporter.error(validation_output("contentMediaType", 
                                                         content_media_type_location_, 
                                                         instance_location.to_uri_fragment(), 
                                                         std::string("Content is not JSON: ") + ec.message()));
                    }
                }
            } 
            else if (instance.type() == json_type::byte_string_value) 
            {
                reporter.error(validation_output("contentMediaType", 
                                                 content_media_type_location_, 
                                                 instance_location.to_uri_fragment(), 
                                                 "Expected string, but is byte string"));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (instance.type() != json_type::string_value) 
            {
                return; 
            }

            if (min_length_) 
            {
                std::size_t length = unicode_traits::count_codepoints(content.data(), content.size());
                if (length < *min_length_) 
                {
                    reporter.error(validation_output("minLength", 
                                                     min_length_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     std::string("Expected minLength: ") + std::to_string(*min_length_)
                                              + ", actual: " + std::to_string(length)));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (max_length_) 
            {
                std::size_t length = unicode_traits::count_codepoints(content.data(), content.size());
                if (length > *max_length_)
                {
                    reporter.error(validation_output("maxLength", 
                                                     max_length_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     std::string("Expected maxLength: ") + std::to_string(*max_length_)
                        + ", actual: " + std::to_string(length)));
                    if (reporter.fail_early())
                    {
                        return;
                    }
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
                    reporter.error(validation_output("pattern", 
                                                     pattern_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     std::move(message)));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

    #endif

            if (format_check_ != nullptr) 
            {
                format_check_(format_location_, instance_location, content, reporter);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    template <class Json>
    class max_length_validator : public keyword_validator<Json>
    {
        std::size_t max_length_;
    public:
        max_length_validator(const compilation_context& context, std::size_t max_length)
            : keyword_validator<Json>(context.get_schema_path()), max_length_(max_length)
        {
        }

        static std::unique_ptr<max_length_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maxLength");
            if (!schema.is_number())
            {
                std::string message("maxLength must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<max_length_validator<Json>>(schema_path, value);
        }

    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter,
                         Json&) const override
        {
            std::size_t length = unicode_traits::count_codepoints(content.data(), content.size());
            if (length > max_length_)
            {
                reporter.error(validation_output("maxLength", 
                                                 max_length_location_, 
                                                 instance_location.to_uri_fragment(), 
                                                 std::string("Expected maxLength: ") + std::to_string(max_length_)
                    + ", actual: " + std::to_string(length)));
                if (reporter.fail_early())
                {
                    return;
                }
            }          
        }
    };

    // not_validator

    template <class Json>
    class not_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        validator_pointer rule_;

    public:
        not_validator(const std::string& schema_path,
            validator_pointer rule)
            : keyword_validator<Json>(schema_path), 
              rule_(rule)
        {
        }

        static std::unique_ptr<not_validator> compile(abstract_keyword_validator_factory<Json>* builder,
            const Json& schema, const compilation_context& context)
        {
            return jsoncons::make_unique<not_validator<Json>>(context.get_schema_path(), 
                builder->make_keyword_validator(schema, context, {"not"}));
        }

    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            collecting_error_reporter local_reporter;
            rule_->validate(instance, instance_location, local_reporter, patch);

            if (local_reporter.errors.empty())
            {
                reporter.error(validation_output("not", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Instance must not be valid against schema"));
            }
        }

        jsoncons::optional<Json> get_default_value(const jsonpointer::json_pointer& instance_location, 
                                                   const Json& instance, 
                                                   error_reporter& reporter) const override
        {
            return rule_->get_default_value(instance_location, instance, reporter);
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
                                const jsonpointer::json_pointer& instance_location, 
                                error_reporter& reporter, 
                                const collecting_error_reporter& local_reporter, 
                                std::size_t)
        {
            if (!local_reporter.errors.empty())
                reporter.error(validation_output("allOf", 
                                                 "",
                                                 instance_location.to_uri_fragment(), 
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

        static bool is_complete(const Json&, 
                                const jsonpointer::json_pointer&, 
                                error_reporter&, 
                                const collecting_error_reporter&, 
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
                                const jsonpointer::json_pointer& instance_location, 
                                error_reporter& reporter, 
                                const collecting_error_reporter&, 
                                std::size_t count)
        {
            if (count > 1)
            {
                std::string message(std::to_string(count));
                message.append(" subschemas matched, but exactly one is required to match");
                reporter.error(validation_output("oneOf", 
                                                 "", 
                                                 instance_location.to_uri_fragment(), 
                                                 std::move(message)));
            }
            return count > 1;
        }
    };

    template <class Json,class Criterion>
    class combining_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        std::vector<validator_pointer> subschemas_;

    public:
        combining_validator(abstract_keyword_validator_factory<Json>* builder,
                            const Json& schema,
                            const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path())
        {
            size_t c = 0;
            for (const auto& subsch : schema.array_range())
            {
                subschemas_.push_back(builder->make_keyword_validator(subsch, context, {Criterion::key(), std::to_string(c++)}));
            }

            // Validate value of allOf, anyOf, and oneOf "MUST be a non-empty array"
        }

    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            size_t count = 0;

            collecting_error_reporter local_reporter;
            for (auto& s : subschemas_) 
            {
                std::size_t mark = local_reporter.errors.size();
                s->validate(instance, instance_location, local_reporter, patch);
                if (mark == local_reporter.errors.size())
                    count++;

                if (Criterion::is_complete(instance, instance_location, reporter, local_reporter, count))
                    return;
            }

            if (count == 0)
            {
                reporter.error(validation_output("combined", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "No schema matched, but one of them is required to match", 
                                                 local_reporter.errors));
            }
        }
    };

    template <class Json,class T>
    class maximum_validator : public keyword_validator<Json>
    {
        T value_;

    public:
        maximum_validator(const std::string& schema_path, T value)
            : keyword_validator<Json>(schema_path), value_(value)
        {
        }

        static std::unique_ptr<maximum_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maximum");
            if (!schema.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<maximum_validator<Json,T>>(schema_path, value);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const 
        {
            T value = instance.template as<T>(); 
            if (value > value_)
            {
                reporter.error(validation_output("maximum", 
                    this->schema_path(), 
                    instance_location.to_uri_fragment(), 
                    instance.template as<std::string>() + " exceeds maximum of " + std::to_string(value)));
            }
        }
    };

    template <class Json,class T>
    class exclusive_maximum_validator : public keyword_validator<Json>
    {
        T value_;

    public:
        exclusive_maximum_validator(const std::string& schema_path, T value)
            : keyword_validator<Json>(schema_path), value_(value)
        {
        }

        static std::unique_ptr<exclusive_maximum_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<exclusive_maximum_validator<Json,T>>(schema_path, value);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const 
        {
            T value = instance.template as<T>(); 
            if (value >= value_)
            {
                reporter.error(validation_output("exclusiveMaximum", 
                    this->schema_path(), 
                    instance_location.to_uri_fragment(), 
                    instance.template as<std::string>() + " exceeds exclusiveMaximum of " + std::to_string(value_)));
            }
        }
    };

    template <class Json,class T>
    class minimum_validator : public keyword_validator<Json>
    {
        T value_;

    public:
        minimum_validator(const std::string& schema_path, T value)
            : keyword_validator<Json>(schema_path), value_(value)
        {
        }

        static std::unique_ptr<minimum_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("minimum");
            if (!schema.is_number())
            {
                std::string message("minimum must be a number");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<minimum_validator<Json,T>>(schema_path, value);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const 
        {
            T value = instance.template as<T>(); 
            if (value < value_)
            {
                reporter.error(validation_output("minimum", 
                    this->schema_path(), 
                    instance_location.to_uri_fragment(), 
                    instance.template as<std::string>() + " exceeds minimum of " + std::to_string(value_)));
            }
        }
    };

    template <class Json,class T>
    class exclusive_minimum_validator : public keyword_validator<Json>
    {
        T value_;

    public:
        exclusive_minimum_validator(const std::string& schema_path, T value)
            : keyword_validator<Json>(schema_path), value_(value)
        {
        }

        static std::unique_ptr<exclusive_minimum_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<exclusive_minimum_validator<Json,T>>(schema_path, value);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const 
        {
            T value = instance.template as<T>(); 
            if (value <= value_)
            {
                reporter.error(validation_output("exclusiveMinimum", 
                    this->schema_path(), 
                    instance_location.to_uri_fragment(), 
                    instance.template as<std::string>() + " exceeds exclusiveMinimum of " + std::to_string(value_)));
            }
        }
    };

    template <class Json,class T>
    class multiple_of_validator : public keyword_validator<Json>
    {
        double value_;

    public:
        multiple_of_validator(const std::string& schema_path, double value)
            : keyword_validator<Json>(schema_path), value_(value)
        {
        }

        static std::unique_ptr<multiple_of_validator> compile(Json schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("multipleOf");
            if (!schema.is_number())
            {
                std::string message("multipleOf must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<T>();
            return jsoncons::make_unique<multiple_of_validator<Json,T>>(context.get_schema_path(), value);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const 
        {
            T value = instance.template as<T>();
            if (value != 0) // Exclude zero
            {
                if (!is_multiple_of(value, value_))
                {
                    reporter.error(validation_output("multipleOf", 
                        this->schema_path(),
                        instance_location.to_uri_fragment(), 
                        instance.template as<std::string>() + " is not a multiple of " + std::to_string(value_)));
                }
            }
        }

        static bool is_multiple_of(T x, double multiple_of) 
        {
            double rem = std::remainder(x, multiple_of);
            double eps = std::nextafter(x, 0) - x;
            return std::fabs(rem) < std::fabs(eps);
        }
    };

    template <class Json>
    class integer_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        std::vector<validator_pointer> validators_;
    public:
        integer_validator(const std::string& schema_path, 
            const std::vector<validator_pointer>& validators)
            : keyword_validator<Json>(schema_path), validators_(validators)
        {
        }

        static std::unique_ptr<integer_validator> compile(const compilation_context& context,
            const std::vector<validator_pointer>& validators)
        {
            return jsoncons::make_unique<integer_validator<Json>>(context.get_schema_path(), validators);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const 
        {
            if (!(instance.template is_integer<int64_t>() || (instance.is_double() && static_cast<double>(instance.template as<int64_t>()) == instance.template as<double>())))
            {
                reporter.error(validation_output("integer", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Instance is not an integer"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
            for (const auto& validator : validators_)
            {
                validator->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    template <class Json>
    class number_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        std::vector<validator_pointer> validators_;
    public:
        number_validator(const std::string& schema_path, 
            const std::vector<validator_pointer>& validators)
            : keyword_validator<Json>(schema_path), validators_(validators)
        {
        }

        static std::unique_ptr<number_validator> compile(const compilation_context& context,
            const std::vector<validator_pointer>& validators)
        {
            return jsoncons::make_unique<number_validator<Json>>(context.get_schema_path(), validators);
        }

    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const 
        {
            if (!(instance.template is_integer<int64_t>() || instance.is_double()))
            {
                reporter.error(validation_output("number", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Instance is not a number"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
            for (const auto& validator : validators_)
            {
                validator->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // null

    template <class Json>
    class null_validator : public keyword_validator<Json>
    {
    public:
        null_validator(const std::string& schema_path)
            : keyword_validator<Json>(schema_path)
        {
        }

        static std::unique_ptr<null_validator> compile(const compilation_context& context)
        {
            return jsoncons::make_unique<null_validator<Json>>(context.get_schema_path());
        }
    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const override
        {
            if (!instance.is_null())
            {
                reporter.error(validation_output("null", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Expected to be null"));
            }
        }
    };

    template <class Json>
    class boolean_validator : public keyword_validator<Json>
    {
    public:
        boolean_validator(const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path())
        {
        }
    private:
        void do_validate(const Json&, 
                         const jsonpointer::json_pointer&, 
                         error_reporter&, 
                         Json&) const override
        {
        }

    };

    template <class Json>
    class true_validator : public keyword_validator<Json>
    {
    public:
        true_validator(const std::string& schema_path)
            : keyword_validator<Json>(schema_path)
        {
        }

        static std::unique_ptr<true_validator> compile(const compilation_context& context)
        {
            return jsoncons::make_unique<true_validator<Json>>(context.get_schema_path());
        }
    private:
        void do_validate(const Json&, 
                         const jsonpointer::json_pointer&, 
                         error_reporter&, 
                         Json&) const override
        {
        }
    };

    // false

    template <class Json>
    class false_validator : public keyword_validator<Json>
    {
    public:
        false_validator(const std::string& schema_path)
            : keyword_validator<Json>(schema_path)
        {
        }

        static std::unique_ptr<false_validator> compile(const compilation_context& context)
        {
            return jsoncons::make_unique<false_validator<Json>>(context.get_schema_path());
        }
    private:
        void do_validate(const Json&, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const override
        {
            reporter.error(validation_output("false", 
                                             this->schema_path(), 
                                             instance_location.to_uri_fragment(), 
                                             "False schema always fails"));
        }
    };

    template <class Json>
    class required_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        std::vector<std::string> items_;

    public:
        required_validator(const std::string& schema_path,
            const std::vector<std::string>& items)
            : keyword_validator<Json>(schema_path), items_(items)
        {
        }

        required_validator(const required_validator&) = delete;
        required_validator(required_validator&&) = default;
        required_validator& operator=(const required_validator&) = delete;
        required_validator& operator=(required_validator&&) = default;

        static std::unique_ptr<required_validator> compile(const compilation_context& context,
            const std::vector<std::string>& items)
        {
            return jsoncons::make_unique<required_validator<Json>>(context.get_schema_path(), items);
        }

    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json&) const override final
        {
            for (const auto& key : items_)
            {
                if (instance.find(key) == instance.object_range().end())
                {
                    reporter.error(validation_output("required", 
                                                     this->schema_path(), 
                                                     instance_location.to_uri_fragment(), 
                                                     "Required property \"" + key + "\" not found"));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
        }
    };

    template <class Json>
    class object_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        jsoncons::optional<std::size_t> max_properties_;
        std::string absolute_max_properties_location_;
        jsoncons::optional<std::size_t> min_properties_;
        std::string absolute_min_properties_location_;
        jsoncons::optional<required_validator<Json>> required_;

        std::map<std::string, validator_pointer> properties_;
    #if defined(JSONCONS_HAS_STD_REGEX)
        std::vector<std::pair<std::regex, validator_pointer>> pattern_properties_;
    #endif
        validator_pointer additional_properties_;

        std::map<std::string, validator_pointer> dependencies_;

        validator_pointer property_name_validator_;

    public:
        object_validator(abstract_keyword_validator_factory<Json>* builder,
                    const Json& schema,
                    const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), 
              max_properties_(), min_properties_(), 
              additional_properties_(nullptr),
              property_name_validator_(nullptr)
        {
            auto it = schema.find("maxProperties");
            if (it != schema.object_range().end()) 
            {
                max_properties_ = it->value().template as<std::size_t>();
                absolute_max_properties_location_ = context.make_schema_path_with("maxProperties");
            }

            it = schema.find("minProperties");
            if (it != schema.object_range().end()) 
            {
                min_properties_ = it->value().template as<std::size_t>();
                absolute_min_properties_location_ = context.make_schema_path_with("minProperties");
            }

            it = schema.find("required");
            if (it != schema.object_range().end()) 
            {
                auto location = context.make_schema_path_with("required");
                required_ = required_validator<Json>(location, 
                                                   it->value().template as<std::vector<std::string>>());
            }

            it = schema.find("properties");
            if (it != schema.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    properties_.emplace(
                        std::make_pair(
                            prop.key(),
                            builder->make_keyword_validator(prop.value(), context, {"properties", prop.key()})));
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = schema.find("patternProperties");
            if (it != schema.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    pattern_properties_.emplace_back(
                        std::make_pair(
                            std::regex(prop.key(), std::regex::ECMAScript),
                            builder->make_keyword_validator(prop.value(), context, {prop.key()})));
            }
    #endif

            it = schema.find("additionalProperties");
            if (it != schema.object_range().end()) 
            {
                additional_properties_ = builder->make_keyword_validator(it->value(), context, {"additionalProperties"});
            }

            it = schema.find("dependencies");
            if (it != schema.object_range().end()) 
            {
                for (const auto& dep : it->value().object_range())
                {
                    switch (dep.value().type()) 
                    {
                        case json_type::array_value:
                        {
                            auto location = context.make_schema_path_with("dependencies");
                            dependencies_.emplace(dep.key(),
                                builder->make_required_validator(compilation_context(std::vector<schema_location>{{location}}),
                                                                                 dep.value().template as<std::vector<std::string>>()));
                            break;
                        }
                        default:
                        {
                            dependencies_.emplace(dep.key(),
                                                  builder->make_keyword_validator(dep.value(), context, {"dependencies", dep.key()}));
                            break;
                        }
                    }
                }
            }

            auto property_names_it = schema.find("propertyNames");
            if (property_names_it != schema.object_range().end()) 
            {
                property_name_validator_ = builder->make_keyword_validator(property_names_it->value(), context, {"propertyNames"});
            }
        }
    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_properties_ && instance.size() > *max_properties_)
            {
                std::string message("Maximum properties: " + std::to_string(*max_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output("maxProperties", 
                                                 absolute_max_properties_location_, 
                                                 instance_location.to_uri_fragment(), 
                                                 std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (min_properties_ && instance.size() < *min_properties_)
            {
                std::string message("Minimum properties: " + std::to_string(*min_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output("minProperties", 
                                                 absolute_min_properties_location_, 
                                                 instance_location.to_uri_fragment(), 
                                                 std::move(message)));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (required_)
                required_->validate(instance, instance_location, reporter, patch);

            for (const auto& property : instance.object_range()) 
            {
                if (property_name_validator_)
                    property_name_validator_->validate(property.key(), instance_location, reporter, patch);

                bool a_prop_or_pattern_matched = false;
                auto properties_it = properties_.find(property.key());

                // check if it is in "properties"
                if (properties_it != properties_.end()) 
                {
                    a_prop_or_pattern_matched = true;
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= property.key();
                    properties_it->second->validate(property.value(), pointer, reporter, patch);
                }

    #if defined(JSONCONS_HAS_STD_REGEX)

                // check all matching "patternProperties"
                for (auto& schema_pp : pattern_properties_)
                    if (std::regex_search(property.key(), schema_pp.first)) 
                    {
                        a_prop_or_pattern_matched = true;
                        jsonpointer::json_pointer pointer(instance_location);
                        pointer /= property.key();
                        schema_pp.second->validate(property.value(), pointer, reporter, patch);
                    }
    #endif

                // finally, check "additionalProperties" 
                if (!a_prop_or_pattern_matched && additional_properties_) 
                {
                    collecting_error_reporter local_reporter;

                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= property.key();
                    additional_properties_->validate(property.value(), pointer, local_reporter, patch);
                    if (!local_reporter.errors.empty())
                    {
                        reporter.error(validation_output("additionalProperties", 
                                                         additional_properties_->schema_path(), 
                                                         instance_location.to_uri_fragment(), 
                                                         "Additional property \"" + property.key() + "\" found but was invalid."));
                        if (reporter.fail_early())
                        {
                            return;
                        }
                    }
                }
            }

            // reverse search
            for (auto const& prop : properties_) 
            {
                const auto finding = instance.find(prop.first);
                if (finding == instance.object_range().end()) 
                { 
                    // If property is not in instance
                    auto default_value = prop.second->get_default_value(instance_location, instance, reporter);
                    if (default_value) 
                    { 
                        // If default value is available, update patch
                        jsonpointer::json_pointer pointer(instance_location);
                        pointer /= prop.first;

                        update_patch(patch, pointer, std::move(*default_value));
                    }
                }
            }

            for (const auto& dep : dependencies_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) 
                {
                    // if dependency-property is present in instance
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= dep.first;
                    dep.second->validate(instance, pointer, reporter, patch); // validate
                }
            }
        }

        void update_patch(Json& patch, const jsonpointer::json_pointer& instance_location, Json&& default_value) const
        {
            Json j;
            j.try_emplace("op", "add"); 
            j.try_emplace("path", instance_location.to_uri_fragment()); 
            j.try_emplace("value", std::forward<Json>(default_value)); 
            patch.push_back(std::move(j));
        }
    };

    // array_validator

    template <class Json>
    class array_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        jsoncons::optional<std::size_t> max_items_;
        std::string absolute_max_items_location_;
        jsoncons::optional<std::size_t> min_items_;
        std::string absolute_min_items_location_;
        bool unique_items_ = false;
        validator_pointer items_validator_;
        std::vector<validator_pointer> item_validators_;
        validator_pointer additional_items_validator_;
        validator_pointer contains_validator_;

    public:
        array_validator(abstract_keyword_validator_factory<Json>* builder, 
                   const Json& schema, 
                   const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), 
              max_items_(), min_items_(), items_validator_(nullptr), additional_items_validator_(nullptr), contains_validator_(nullptr)
        {
            {
                auto it = schema.find("maxItems");
                if (it != schema.object_range().end()) 
                {
                    max_items_ = it->value().template as<std::size_t>();
                    absolute_max_items_location_ = context.make_schema_path_with("maxItems");
                }
            }

            {
                auto it = schema.find("minItems");
                if (it != schema.object_range().end()) 
                {
                    min_items_ = it->value().template as<std::size_t>();
                    absolute_min_items_location_ = context.make_schema_path_with("minItems");
                }
            }

            {
                auto it = schema.find("uniqueItems");
                if (it != schema.object_range().end()) 
                {
                    unique_items_ = it->value().template as<bool>();
                }
            }

            {
                auto it = schema.find("items");
                if (it != schema.object_range().end()) 
                {

                    if (it->value().type() == json_type::array_value) 
                    {
                        size_t c = 0;
                        for (const auto& subsch : it->value().array_range())
                            item_validators_.push_back(builder->make_keyword_validator(subsch, context, {"items", std::to_string(c++)}));

                        auto attr_add = schema.find("additionalItems");
                        if (attr_add != schema.object_range().end()) 
                        {
                            additional_items_validator_ = builder->make_keyword_validator(attr_add->value(), context, {"additionalItems"});
                        }

                    } 
                    else if (it->value().type() == json_type::object_value ||
                               it->value().type() == json_type::bool_value)
                    {
                        items_validator_ = builder->make_keyword_validator(it->value(), context, {"items"});
                    }

                }
            }

            {
                auto it = schema.find("contains");
                if (it != schema.object_range().end()) 
                {
                    contains_validator_ = builder->make_keyword_validator(it->value(), context, {"contains"});
                }
            }
        }
    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_items_)
            {
                if (instance.size() > *max_items_)
                {
                    std::string message("Expected maximum item count: " + std::to_string(*max_items_));
                    message.append(", found: " + std::to_string(instance.size()));
                    reporter.error(validation_output("maxItems", 
                                                     absolute_max_items_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     std::move(message)));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (min_items_)
            {
                if (instance.size() < *min_items_)
                {
                    std::string message("Expected minimum item count: " + std::to_string(*min_items_));
                    message.append(", found: " + std::to_string(instance.size()));
                    reporter.error(validation_output("minItems", 
                                                     absolute_min_items_location_, 
                                                     instance_location.to_uri_fragment(), 
                                                     std::move(message)));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (unique_items_) 
            {
                if (!array_has_unique_items(instance))
                {
                    reporter.error(validation_output("uniqueItems", 
                                                     this->schema_path(), 
                                                     instance_location.to_uri_fragment(), 
                                                     "Array items are not unique"));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            size_t index = 0;
            if (items_validator_)
            {
                for (const auto& i : instance.array_range()) 
                {
                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= index;
                    items_validator_->validate(i, pointer, reporter, patch);
                    index++;
                }
            }
            else 
            {
                auto validator_it = item_validators_.cbegin();
                for (const auto& item : instance.array_range()) 
                {
                    validator_pointer item_validator = nullptr;
                    if (validator_it != item_validators_.cend())
                    {
                        item_validator = *validator_it;
                        ++validator_it;
                    }
                    else if (additional_items_validator_ != nullptr)
                    {
                        item_validator = additional_items_validator_;
                    }
                    else
                        break;

                    jsonpointer::json_pointer pointer(instance_location);
                    pointer /= index;
                    item_validator->validate(item, pointer, reporter, patch);
                }
            }

            if (contains_validator_) 
            {
                bool contained = false;
                collecting_error_reporter local_reporter;
                for (const auto& item : instance.array_range()) 
                {
                    std::size_t mark = local_reporter.errors.size();
                    contains_validator_->validate(item, instance_location, local_reporter, patch);
                    if (mark == local_reporter.errors.size()) 
                    {
                        contained = true;
                        break;
                    }
                }
                if (!contained)
                {
                    reporter.error(validation_output("contains", 
                                                     this->schema_path(), 
                                                     instance_location.to_uri_fragment(), 
                                                     "Expected at least one array item to match \"contains\" schema", 
                                                     local_reporter.errors));
                    if (reporter.fail_early())
                    {
                        return;
                    }
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

    template <class Json>
    class conditional_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        validator_pointer if_validator_;
        validator_pointer then_validator_;
        validator_pointer else_validator_;

    public:
        conditional_validator(abstract_keyword_validator_factory<Json>* builder,
                         const Json& sch_if,
                         const Json& schema,
                         const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), if_validator_(nullptr), then_validator_(nullptr), else_validator_(nullptr)
        {
            auto then_it = schema.find("then");
            auto else_it = schema.find("else");

            if (then_it != schema.object_range().end() || else_it != schema.object_range().end()) 
            {
                if_validator_ = builder->make_keyword_validator(sch_if, context, {"if"});

                if (then_it != schema.object_range().end()) 
                {
                    then_validator_ = builder->make_keyword_validator(then_it->value(), context, {"then"});
                }

                if (else_it != schema.object_range().end()) 
                {
                    else_validator_ = builder->make_keyword_validator(else_it->value(), context, {"else"});
                }
            }
        }
    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            if (if_validator_) 
            {
                collecting_error_reporter local_reporter;

                if_validator_->validate(instance, instance_location, local_reporter, patch);
                if (local_reporter.errors.empty()) 
                {
                    if (then_validator_)
                        then_validator_->validate(instance, instance_location, reporter, patch);
                } 
                else 
                {
                    if (else_validator_)
                        else_validator_->validate(instance, instance_location, reporter, patch);
                }
            }
        }
    };

    // enum_validator

    template <class Json>
    class enum_validator : public keyword_validator<Json>
    {
        Json enum_validator_;

    public:
        enum_validator(const Json& schema,
                  const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), enum_validator_(schema)
        {
        }
    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter,
                         Json&) const final
        {
            bool in_range = false;
            for (const auto& item : enum_validator_.array_range())
            {
                if (item == instance) 
                {
                    in_range = true;
                    break;
                }
            }

            if (!in_range)
            {
                reporter.error(validation_output("enum", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 instance.template as<std::string>() + " is not a valid enum value"));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // const_keyword

    template <class Json>
    class const_keyword : public keyword_validator<Json>
    {
        Json const_validator_;

    public:
        const_keyword(const Json& schema, const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), const_validator_(schema)
        {
        }
    private:
        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter,
                         Json&) const final
        {
            if (const_validator_ != instance)
                reporter.error(validation_output("const", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Instance is not const"));
        }
    };

    template <class Json>
    class type_validator : public keyword_validator<Json>
    {
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        Json default_value_;
        std::vector<validator_pointer> type_mapping_;
        jsoncons::optional<enum_validator<Json>> enum_validator_;
        jsoncons::optional<const_keyword<Json>> const_validator_;
        std::vector<validator_pointer> combined_validators_;
        jsoncons::optional<conditional_validator<Json>> conditional_validator_;
        std::vector<std::string> expected_types_;

    public:
        type_validator(const type_validator&) = delete;
        type_validator& operator=(const type_validator&) = delete;
        type_validator(type_validator&&) = default;
        type_validator& operator=(type_validator&&) = default;

        type_validator(abstract_keyword_validator_factory<Json>* builder,
                     const Json& schema,
                     const compilation_context& context)
            : keyword_validator<Json>(context.get_schema_path()), default_value_(jsoncons::null_type()), 
              type_mapping_((uint8_t)(json_type::object_value)+1), 
              enum_validator_(), const_validator_()
        {
            std::set<std::string> known_keywords;

            auto it = schema.find("type");
            if (it == schema.object_range().end()) 
            {
                initialize_type_mapping(builder, "", schema, context, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        initialize_type_mapping(builder, type, schema, context, known_keywords);
                        expected_types_.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            initialize_type_mapping(builder, type, schema, context, known_keywords);
                            expected_types_.emplace_back(std::move(type));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            const auto default_it = schema.find("default");
            if (default_it != schema.object_range().end()) 
            {
                default_value_ = default_it->value();
            }

            it = schema.find("enum");
            if (it != schema.object_range().end()) 
            {
                enum_validator_ = enum_validator<Json >(it->value(), context);
            }

            it = schema.find("const");
            if (it != schema.object_range().end()) 
            {
                const_validator_ = const_keyword<Json>(it->value(), context);
            }

            it = schema.find("not");
            if (it != schema.object_range().end()) 
            {
                combined_validators_.push_back(builder->make_not_validator(it->value(), context));
            }

            it = schema.find("allOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators_.push_back(builder->make_all_of_validator(it->value(), context));
            }

            it = schema.find("anyOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators_.push_back(builder->make_any_of_validator(it->value(), context));
            }

            it = schema.find("oneOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators_.push_back(builder->make_one_of_validator(it->value(), context));
            }

            it = schema.find("if");
            if (it != schema.object_range().end()) 
            {
                conditional_validator_ = conditional_validator<Json>(builder, it->value(), schema, context);
            }
        }
    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const override final
        {
            auto type = type_mapping_[(uint8_t) instance.type()];

            if (type)
                type->validate(instance, instance_location, reporter, patch);
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

                reporter.error(validation_output("type", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 ss.str()));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (enum_validator_)
            { 
                enum_validator_->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            if (const_validator_)
            { 
                const_validator_->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            for (const auto& validator : combined_validators_)
            {
                validator->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }


            if (conditional_validator_)
            { 
                conditional_validator_->validate(instance, instance_location, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }

        jsoncons::optional<Json> get_default_value(const jsonpointer::json_pointer&, 
                                                   const Json&,
                                                   error_reporter&) const override
        {
            return default_value_;
        }

        void initialize_type_mapping(abstract_keyword_validator_factory<Json>* builder,
                                     const std::string& type,
                                     const Json& schema,
                                     const compilation_context& context,
                                     std::set<std::string>& keywords)
        {
            if (type == "null")
            {
                type_mapping_[(uint8_t)json_type::null_value] = builder->make_null_validator(context);
            }
            else if (type == "object")
            {
                type_mapping_[(uint8_t)json_type::object_value] = builder->make_object_validator(schema, context);
            }
            else if (type == "array")
            {
                type_mapping_[(uint8_t)json_type::array_value] = builder->make_array_validator(schema, context);
            }
            else if (type == "string")
            {
                type_mapping_[(uint8_t)json_type::string_value] = builder->make_string_validator(schema, context);
                // For binary types
                type_mapping_[(uint8_t) json_type::byte_string_value] = type_mapping_[(uint8_t) json_type::string_value];
            }
            else if (type == "boolean")
            {
                type_mapping_[(uint8_t)json_type::bool_value] = builder->make_boolean_validator(context);
            }
            else if (type == "integer")
            {
                type_mapping_[(uint8_t)json_type::int64_value] = builder->make_integer_validator(schema, context, keywords);
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::int64_value];
                type_mapping_[(uint8_t)json_type::double_value] = type_mapping_[(uint8_t)json_type::int64_value];
            }
            else if (type == "number")
            {
                type_mapping_[(uint8_t)json_type::double_value] = builder->make_number_validator(schema, context, keywords);
                type_mapping_[(uint8_t)json_type::int64_value] = type_mapping_[(uint8_t)json_type::double_value];
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::double_value];
            }
            else if (type.empty())
            {
                type_mapping_[(uint8_t)json_type::null_value] = builder->make_null_validator(context);
                type_mapping_[(uint8_t)json_type::object_value] = builder->make_object_validator(schema, context);
                type_mapping_[(uint8_t)json_type::array_value] = builder->make_array_validator(schema, context);
                type_mapping_[(uint8_t)json_type::string_value] = builder->make_string_validator(schema, context);
                // For binary types
                type_mapping_[(uint8_t) json_type::byte_string_value] = type_mapping_[(uint8_t) json_type::string_value];
                type_mapping_[(uint8_t)json_type::bool_value] = builder->make_boolean_validator(context);
                type_mapping_[(uint8_t)json_type::int64_value] = builder->make_integer_validator(schema, context, keywords);
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::int64_value];
                type_mapping_[(uint8_t)json_type::double_value] = type_mapping_[(uint8_t)json_type::int64_value];
                type_mapping_[(uint8_t)json_type::double_value] = builder->make_number_validator(schema, context, keywords);
                type_mapping_[(uint8_t)json_type::int64_value] = type_mapping_[(uint8_t)json_type::double_value];
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::double_value];
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_VALUE_RULES_HPP
