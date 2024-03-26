// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_VALIDATION_MESSAGE_HPP
#define JSONCONS_JSONSCHEMA_VALIDATION_MESSAGE_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/json_schema.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#include <functional>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

    class validation_message 
    {
        std::string keyword_;
        jsonpointer::json_pointer eval_path_;
        uri schema_location_;
        jsonpointer::json_pointer instance_location_;
        std::string message_;
        std::vector<validation_message> details_;
    public:
        validation_message(std::string keyword,
            jsonpointer::json_pointer eval_path,
            uri schema_location,
            jsonpointer::json_pointer instance_location,
            std::string message)
            : keyword_(std::move(keyword)), 
              eval_path_(std::move(eval_path)),
              schema_location_(std::move(schema_location)),
              instance_location_(std::move(instance_location)),
              message_(std::move(message))
        {
        }

        validation_message(const std::string& keyword,
            const jsonpointer::json_pointer& eval_path,
            const uri& schema_location,
            const jsonpointer::json_pointer& instance_location,
            const std::string& message,
            const std::vector<validation_message>& details)
            : keyword_(keyword),
              eval_path_(eval_path),
              schema_location_(schema_location),
              instance_location_(instance_location), 
              message_(message),
              details_(details)
        {
        }

        const jsonpointer::json_pointer& instance_location() const
        {
            return instance_location_;
        }

        const std::string& message() const
        {
            return message_;
        }

        const jsonpointer::json_pointer& eval_path() const
        {
            return eval_path_;
        }

        const uri& schema_location() const
        {
            return schema_location_;
        }

        const std::string& keyword() const
        {
            return keyword_;
        }

        const std::vector<validation_message>& details() const
        {
            return details_;
        }
    };
    
    
    class validation_report
    {
        json_stream_encoder* encoder_ptr_;
    public:
        validation_report(json_stream_encoder& encoder)
            : encoder_ptr_(std::addressof(encoder))
        {
        }
    
        void operator()(const validation_message& message)
        {
            write_error(message);
        }
        
        void write_error(const validation_message& message)
        {
            encoder_ptr_->begin_object();

            encoder_ptr_->key("valid");
            encoder_ptr_->bool_value(false);

            encoder_ptr_->key("evaluationPath");
            encoder_ptr_->string_value(message.eval_path().string());

            encoder_ptr_->key("schemaLocation");
            encoder_ptr_->string_value(message.schema_location().string());

            encoder_ptr_->key("instanceLocation");
            encoder_ptr_->string_value(message.instance_location().string());

            encoder_ptr_->key("error");
            encoder_ptr_->string_value(message.message());

            if (!message.details().empty())
            {
                for (const auto& detail : message.details())
                {
                    write_error(detail);
                }
            }

            encoder_ptr_->end_object();
        }
    
        void begin_report()
        {
            encoder_ptr_->begin_array();
        }
    
        void end_report()
        {
            encoder_ptr_->end_array();
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP
