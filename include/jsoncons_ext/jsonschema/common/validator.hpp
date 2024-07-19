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
    
    enum class walk_result {advance, abort};

    template <typename Json>
    struct json_schema_traits    
    {
        using walk_reporter_type = std::function<walk_result(const std::string& keyword,
            const Json& schema, const uri& schema_location,
            const Json& instance, const jsonpointer::json_pointer& instance_location)>;      
    };

    // Interface for validation error handlers
    class error_reporter
    {
        std::size_t error_count_;
    public:
        error_reporter()
            : error_count_(0)
        {
        }

        virtual ~error_reporter() = default;

        walk_result error(const validation_message& msg)
        {
            ++error_count_;
            return do_error(msg);
        }

        std::size_t error_count() const
        {
            return error_count_;
        }

    private:
        virtual walk_result do_error(const validation_message& /* e */) = 0;
    };

    struct collecting_error_listener : public error_reporter
    {
        std::vector<validation_message> errors;

    private:
        walk_result do_error(const validation_message& msg) final
        {
            errors.push_back(msg);
            return walk_result::advance;
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
        range_collection(const range_collection& other) = default;
        range_collection(range_collection&& other) = default;

        range_collection& operator=(const range_collection& other) = default;
        range_collection& operator=(range_collection&& other) = default;
        
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
        
        void insert(range index_range)
        {
            ranges_.push_back(index_range);
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
                evaluated_items.insert(index_range);
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
                evaluated_items.insert(index_range);
            }
        }
    };

    template <typename Json>
    class schema_validator;

    template <typename Json>
    class ref
    {
    public:
        virtual ~ref() = default;
        virtual void set_referred_schema(const schema_validator<Json>* target) = 0;
    };

    template <typename Json>
    class validator_base 
    {
    public:
        using walk_reporter_type = typename json_schema_traits<Json>::walk_reporter_type;

        virtual ~validator_base() = default;

        virtual const uri& schema_location() const = 0;

        walk_result validate(const evaluation_context<Json>& context,
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const 
        {
            return do_validate(context, instance, instance_location, results, reporter, patch);
        }

        walk_result walk(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location, const walk_reporter_type& reporter) const 
        {
            return do_walk(context, instance, instance_location, reporter);
        }

    private:
        virtual walk_result do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const = 0;

        virtual walk_result do_walk(const evaluation_context<Json>& /*context*/, const Json& /*instance*/, 
            const jsonpointer::json_pointer& /*instance_location*/, const walk_reporter_type& /*reporter*/) const = 0;
   };

    template <typename Json>
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

    template <typename Json>
    class keyword_validator_base : public keyword_validator<Json>
    {
        using walk_reporter_type = typename json_schema_traits<Json>::walk_reporter_type;

        std::string keyword_name_;
        const Json* schema_ptr_;
        uri schema_location_;
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        keyword_validator_base(const std::string& keyword_name, const Json& schema, const uri& schema_location)
            : keyword_name_(keyword_name), schema_ptr_(std::addressof(schema)), schema_location_(schema_location)
        {
        }

        keyword_validator_base(const keyword_validator_base&) = delete;
        keyword_validator_base(keyword_validator_base&&) = default;
        keyword_validator_base& operator=(const keyword_validator_base&) = delete;
        keyword_validator_base& operator=(keyword_validator_base&&) = default;

        const std::string& keyword_name() const final
        {
            return keyword_name_;
        }
        
        const Json& schema() const
        {
            JSONCONS_ASSERT(schema_ptr_ != nullptr);
            return *schema_ptr_;
        }           

        const uri& schema_location() const final
        {
            return schema_location_;
        }
    };

    template <typename Json>
    class ref_validator : public keyword_validator_base<Json>, public virtual ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;
        using walk_reporter_type = typename json_schema_traits<Json>::walk_reporter_type;

        const schema_validator<Json>* referred_schema_;

    public:
        ref_validator(const Json& schema, const ref_validator& other)
            : keyword_validator_base<Json>(other.keyword_name(), schema, other.schema_location()),
                  referred_schema_{other.referred_schema_}
        {
        }
        
        ref_validator(const Json& schema, const uri& schema_location) 
            : keyword_validator_base<Json>("$ref", schema, schema_location), referred_schema_{nullptr}
        {
            //std::cout << "ref_validator: " << this->schema_location().string() << "\n";
        }

        ref_validator(const Json& schema, const uri& schema_location, const schema_validator<Json>* referred_schema)
            : keyword_validator_base<Json>("$ref", schema, schema_location), referred_schema_(referred_schema)
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

        walk_result do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            if (!referred_schema_)
            {
                return reporter.error(validation_message(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_location(), 
                    instance_location, 
                    "Unresolved schema reference " + this->schema_location().string()));
            }

            return referred_schema_->validate(this_context, instance, instance_location, results, reporter, patch);
        }

        walk_result do_walk(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location, const walk_reporter_type& reporter) const final 
        {
            if (!referred_schema_)
            {
                return walk_result::advance;
            }
            evaluation_context<Json> this_context(context, this->keyword_name());
            return referred_schema_->walk(this_context, instance, instance_location, reporter);           
        }
    };

    template <typename Json>
    class keyword_base 
    {
        using walk_reporter_type = typename json_schema_traits<Json>::walk_reporter_type;

        std::string keyword_name_;
        const Json* schema_ptr_;
        uri schema_location_;
    public:

        keyword_base(const std::string& keyword_name, const Json& schema, const uri& schema_location)
            : keyword_name_(keyword_name), schema_ptr_(std::addressof(schema)), schema_location_(schema_location)
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

        const Json& schema() const
        {
            return *schema_ptr_;
        }

        const uri& schema_location() const 
        {
            return schema_location_;
        }

        walk_result walk(const evaluation_context<Json>& /*context*/, const Json& instance,
            const jsonpointer::json_pointer& instance_location, const walk_reporter_type& reporter) const
        {
            return reporter(this->keyword_name(), this->schema(), this->schema_location(), instance, instance_location);
        }
    };

    template <typename Json>
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
