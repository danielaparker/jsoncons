### Custom Error Messages (since 1.2.0)

jsoncons supports an `errorMessage` keyword that allows schema authors to provide
custom error messages. To enable this feature, you need to provide an `expression_options`
argument with `enable_custom_error_message` set to `true` when calling `make_json_schema`. 

The `errorMessage` keyword can be set to either

- An object that maps message keys to custom messages, or
- A string that represents a custom message. 
 
An example of an object that maps message keys to custom messages is

```json
{
  "maxItems" : "At most 3 numbers are allowed in 'foo'",
  "type" : "Only numbers are allowed in 'foo'",
  "format.date": "Date format must be YYYY-MM-DD"
}
```
Message keys are JSON Schema keywords, except for `format`,
where they have the form 

    format.<format value> 

### Example

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <iostream>

namespace jsonschema = jsoncons::jsonschema;

int main()
{
    std::string schema_str = R"(
{
    "type": "object",
    "properties": {
        "date": {
          "type": "string",
          "format": "date"
        },
        "foo": {
            "type": "array",
            "maxItems": 3,
            "items" : {
                "type" : "number"
            },
            "errorMessage" : {
                "maxItems" : "At most 3 numbers are allowed in 'foo'",
                "type" : "Only numbers are allowed in 'foo'"
            }
        },
        "bar": {
            "type": "string",
            "errorMessage" : "Type of `bar` must be string"    
        }
    },
    "errorMessage": {
        "format.date": "Date format must be YYYY-MM-DD"
    }
}
    )";

    auto options = jsonschema::evaluation_options{}
        .enable_custom_error_message(true)
        .require_format_validation(true);

    auto schema = jsoncons::json::parse(schema_str);
    auto compiled = jsonschema::make_json_schema<jsoncons::json>(schema, options);

    std::string data_str = R"(
{
    "foo": [1, 2, "three"],
    "bar": 123,        
    "date": "05-13-1955"
}        
    )";

    auto data = jsoncons::json::parse(data_str);

    std::vector<std::string> messages;
    auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            messages.push_back(msg.message());
            return jsonschema::walk_result::advance;
        };
    compiled.validate(data, reporter);

    for (const auto& msg : messages)
    {
        std::cout << msg << "\n";
    }
}
```

Output:

```
Type of `bar` must be string
Date format must be YYYY-MM-DD
Only numbers are allowed in 'foo'
```


