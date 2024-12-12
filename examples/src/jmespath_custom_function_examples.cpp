// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <chrono>
#include <thread>
#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
namespace jmespath = jsoncons::jmespath;

// When adding custom functions, they are generally placed in their own project's source code and namespace.
namespace myspace {

template <typename Json>
class my_custom_functions : public jmespath::custom_functions<Json>
{
    using reference = const Json&;
    using pointer = const Json*;

    static thread_local size_t current_index;
public:
    my_custom_functions()
    {
        this->register_function("current_date_time", // function name
            0,                                       // number of arguments   
            [](const jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                auto now = std::chrono::system_clock::now();
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
                return *resources.create_json(milliseconds.count());
            }
        );
        this->register_function("current_index", // function name
            0,                                   // number of arguments   
            [](const jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                auto result = resources.create_json(current_index);
                return *result;
            }
        );
        this->register_function("generate_array", // function name
            4,                                    // number of arguments   
            [](const jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                JSONCONS_ASSERT(4 == params.size());

                if (!(params[0].is_value() && params[2].is_expression()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                reference context = params[0].value();
                reference countValue = get_value(context, resources, params[1]);
                const auto& expr = params[2].expression();
                auto argDefault = params[3];

                if (!countValue.is_number())
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                auto result = resources.create_json(jsoncons::json_array_arg);
                int count = countValue.template as<int>();
                for (size_t i = 0; i < count; i++)
                {
                    current_index = i;
                    std::error_code ec2;

                    reference ele = expr.evaluate(context, resources, ec2); 

                    if (ele.is_null())
                    {
                        auto defaultVal = get_value(context, resources, argDefault);
                        result->emplace_back(defaultVal);
                    }
                    else
                    {
                        result->emplace_back(ele);
                        //result->emplace_back(jsoncons::json_const_pointer_arg, &ele); 
                        //result->emplace_back(*resources.create_json(deep_copy(ele)));
                    }
                }
                current_index = 0;

                return *result;
            }
        );
        this->register_function("add", // function name
            2,                         // number of arguments   
            [](jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                JSONCONS_ASSERT(2 == params.size());

                if (!(params[0].is_value() && params[1].is_value()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                reference arg0 = params[0].value();
                reference arg1 = params[1].value();
                if (!(arg0.is_number() && arg1.is_number()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                if (arg0.is<int64_t>() && arg1.is<int64_t>())
                {
                    int64_t v = arg0.template as<int64_t>() + arg1.template as<int64_t>();
                    return *resources.create_json(v);
                }
                else
                {
                    double v = arg0.template as<double>() + arg1.template as<double>();
                    return *resources.create_json(v);
                }
            }
        );
    }

    static reference get_value(reference context, jmespath::dynamic_resources<Json>& resources,
        const jmespath::parameter<Json>& param)
    {
        if (param.is_expression())
        {
            const auto& expr = param.expression();
            std::error_code ec;
            reference value = expr.evaluate(context, resources, ec);
            return value;
        }
        else
        {
            reference value = param.value();
            return value;
        }
    }
};

template <typename Json>
thread_local size_t my_custom_functions<Json>::current_index = 0;

} // namespace myspace

// for brevity
using json = jsoncons::json;
   
void jmespath_custom_function_example()
{
    std::string jtext = R"(
          {
            "devices": [
              {
                "position": 1,
                "id": "id-xxx",
                "state": 1
              },
              {
                "position": 5,
                "id": "id-yyy",
                "state": 1
              },
              {
                "position": 9,
                "id": "id-mmm",
                "state": 2
              }
            ]
          }        
    )";
  
    auto expr = jmespath::make_expression<json>("generate_array(devices, `16`, &[?position==add(current_index(), `1`)] | [0], &{id: '', state: `0`, position: add(current_index(), `1`)})",
        myspace::my_custom_functions<json>{});
  
    auto doc = json::parse(jtext);
  
    auto result = expr.evaluate(doc);
  
    std::cout << pretty_print(result) << "\n\n";
}

int main()
{
  std::cout << "\nJMESPath customer functions examples\n\n";

  jmespath_custom_function_example();

  std::cout << "\n";
}
