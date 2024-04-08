// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_COMMON_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/common/evaluation_context.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/validation_message.hpp>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

    // Interface for validation error handlers
    class error_reporter
    {
        bool fail_early_;
        std::size_t error_count_;
    public:
        error_reporter(bool fail_early = false)
            : fail_early_(fail_early), error_count_(0)
        {
        }

        virtual ~error_reporter() = default;

        void error(const validation_message& o)
        {
            ++error_count_;
            do_error(o);
        }

        std::size_t error_count() const
        {
            return error_count_;
        }

        bool fail_early() const
        {
            return fail_early_;
        }

    private:
        virtual void do_error(const validation_message& /* e */) = 0;
    };


    struct collecting_error_reporter : public error_reporter
    {
        std::vector<validation_message> errors;

    private:
        void do_error(const validation_message& o) final
        {
            errors.push_back(o);
        }
    };

    class range
    {
        std::size_t start_;
        std::size_t end_;
    public:
        range()
            : start_(0), end_(0)
        {
        }
    
        range(std::size_t start, std::size_t end)
            : start_(start), end_(end)
        {
        }
        
        std::size_t start() const
        {
            return start_;
        }
    
        std::size_t end() const
        {
            return end_;
        }
        
        bool contains(std::size_t index) const
        {
            return index >= start_ && index < end_; 
        }
    };
    
    class range_collection
    {
        std::vector<range> ranges_;
    public:    
        using const_iterator = std::vector<range>::const_iterator;
        using value_type = range;
        
        range_collection()
        {
        }
        
        std::size_t size() const
        {
            return ranges_.size();
        }
    
        range operator[](std::size_t index) const
        {
            return ranges_[index];
        }
        
        const_iterator begin() const
        {
            return ranges_.cbegin();
        }

        const_iterator end() const
        {
            return ranges_.cend();
        }
        
        void push_back(range index_range)
        {
            ranges_.push_back(index_range);
        }
        
        void append(const range_collection& collection)
        {
            std::size_t length = collection.size();
            for (std::size_t i = 0; i < length; ++i)
            {
                ranges_.push_back(collection[i]);
            }
        }
    
        bool contains(std::size_t index) const
        {
            bool found = false;
            std::size_t length = ranges_.size();
            for (std::size_t i = 0; i < length && !found; ++i)
            {
                if (ranges_[i].contains(index))
                {
                    found = true;
                }
            }
            return found;
        }
    };
    
    struct evaluation_results
    {
        std::unordered_set<std::string> evaluated_properties;
        range_collection evaluated_items;

        void merge(const evaluation_results& results)
        {
            for (auto&& name : results.evaluated_properties)
            {
                evaluated_properties.insert(name);
            }
            for (auto index_range : results.evaluated_items)
            {
                evaluated_items.push_back(index_range);
            }
        }
        void merge(std::unordered_set<std::string>&& properties)
        {
            for (auto&& name : properties)
            {
                evaluated_properties.insert(std::move(name));
            }
        }
        void merge(const range_collection& ranges)
        {
            for (auto index_range : ranges)
            {
                evaluated_items.push_back(index_range);
            }
        }
    };

    template <class Json>
    class schema_validator;

    template <class Json>
    class ref
    {
    public:
        virtual ~ref() = default;
        virtual void set_referred_schema(const schema_validator<Json>* target) = 0;
    };

    template <class Json>
    class validator_base 
    {
    public:
        virtual ~validator_base() = default;

        virtual const uri& schema_location() const = 0;

        void validate(const evaluation_context<Json>& context,
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const 
        {
            do_validate(context, instance, instance_location, results, reporter, patch);
        }

    private:
        virtual void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const = 0;
    };

    template <class Json>
    class keyword_validator : public validator_base<Json> 
    {
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        virtual const std::string& keyword_name() const = 0;

        virtual bool always_fails() const 
        {
            return false;
        }          

        virtual bool always_succeeds() const
        {
            return false;
        }
    };

    template <class Json>
    class keyword_validator_base : public keyword_validator<Json>
    {
        std::string keyword_name_;
        uri schema_location_;
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        keyword_validator_base(const std::string& keyword_name, const uri& schema_location)
            : keyword_name_(keyword_name), schema_location_(schema_location)
        {
        }

        keyword_validator_base(const keyword_validator_base&) = delete;
        keyword_validator_base(keyword_validator_base&&) = default;
        keyword_validator_base& operator=(const keyword_validator_base&) = delete;
        keyword_validator_base& operator=(keyword_validator_base&&) = default;

        const std::string& keyword_name() const override
        {
            return keyword_name_;
        }

        const uri& schema_location() const override
        {
            return schema_location_;
        }
    };

    template <class Json>
    class ref_validator : public keyword_validator_base<Json>, public virtual ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        const schema_validator<Json>* referred_schema_;

    public:
        ref_validator(const ref_validator& other)
            : keyword_validator_base<Json>(other.keyword_name(), other.schema_location()),
                  referred_schema_{other.referred_schema_}
        {
        }
        
        ref_validator(const uri& schema_location) 
            : keyword_validator_base<Json>("$ref", schema_location), referred_schema_{nullptr}
        {
            //std::cout << "ref_validator: " << this->schema_location().string() << "\n";
        }

        ref_validator(const uri& schema_location, const schema_validator<Json>* referred_schema)
            : keyword_validator_base<Json>("$ref", schema_location), referred_schema_(referred_schema) 
        {
            //std::cout << "ref_validator2: " << this->schema_location().string() << "\n";
        }

        const schema_validator<Json>* referred_schema() const {return referred_schema_;}
        
        void set_referred_schema(const schema_validator<Json>* target) final { referred_schema_ = target; }

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
            evaluation_context<Json> this_context(context, this->keyword_name());

            if (!referred_schema_)
            {
                reporter.error(validation_message(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_location(), 
                    instance_location, 
                    "Unresolved schema reference " + this->schema_location().string()));
                return;
            }

            referred_schema_->validate(this_context, instance, instance_location, results, reporter, patch);
        }
    };

    template <class Json>
    class keyword_base 
    {
        std::string keyword_name_;
        uri schema_location_;
    public:

        keyword_base(const std::string& keyword_name, const uri& schema_location)
            : keyword_name_(keyword_name), schema_location_(schema_location)
        {
        }

        virtual ~keyword_base() = default;

        keyword_base(const keyword_base&) = delete;
        keyword_base(keyword_base&&) = default;
        keyword_base& operator=(const keyword_base&) = delete;
        keyword_base& operator=(keyword_base&&) = default;

        const std::string& keyword_name() const 
        {
            return keyword_name_;
        }

        const uri& schema_location() const 
        {
            return schema_location_;
        }

    private:
    };

    template <class Json>
    class schema_validator : public validator_base<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

    public:
        schema_validator()
        {}

        virtual bool always_fails() const = 0;

        virtual bool always_succeeds() const = 0;
        
        virtual jsoncons::optional<Json> get_default_value() const = 0;

        virtual bool recursive_anchor() const = 0;

        virtual const jsoncons::optional<jsoncons::uri>& id() const = 0;

        virtual const schema_validator<Json>* get_schema_for_dynamic_anchor(const std::string& anchor) const = 0;

        virtual const jsoncons::optional<jsoncons::uri>& dynamic_anchor() const = 0;
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP
