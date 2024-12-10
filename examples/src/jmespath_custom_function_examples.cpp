// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <chrono>
#include <thread>

#include <string>
#include <fstream>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

namespace myspace {
    using Json = jsoncons::json;
    using JsonReference = const Json&;
    using jmespath_errc = jsoncons::jmespath::jmespath_errc;
#define json_object_arg jsoncons::json_object_arg
#define json_array_arg jsoncons::json_array_arg
#define json_const_pointer_arg jsoncons::json_const_pointer_arg

    using function_base = jsoncons::jmespath::function_base<Json>;
    using dynamic_resources = jsoncons::jmespath::dynamic_resources<Json>;
    using static_resources = jsoncons::jmespath::detail::jmespath_evaluator<Json>::static_resources;
    using parameter = jsoncons::jmespath::parameter<Json>;
    using string_type = jsoncons::jmespath::detail::jmespath_evaluator<Json>::string_type;
    using customer_get_function = jsoncons::jmespath::detail::jmespath_evaluator<Json>::static_resources::customer_get_function;

    bool is_integer(JsonReference value)
    {
        if (value.is<int32_t>() || value.is<uint32_t>() || value.is<int64_t>() || value.is<uint64_t>())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    JsonReference get_value(JsonReference context, dynamic_resources& resources, const parameter& p)
    {
        if (p.is_expression())
        {
            const auto& expr = p.expression();
            std::error_code ec2;
            JsonReference value = expr.evaluate(context, resources, ec2);
            // if (value.is_object() || value.is_array())
            // {
            //     return *resources.create_json(deep_copy(value));
            // }
            // else
            // {
            //     return value;
            // }
            return value;
        }
        else
        {
            JsonReference value = p.value();
            return value;
        }
    }

    template <typename Json>
    class my_custom_functions : public jsoncons::jmespath::custom_functions<Json>
    {
    public:
        my_custom_functions()
        {
            this->register_function("current_date_time", // function name
                0,                                      // number of arguments   
                [](const jsoncons::span<const jsoncons::jmespath::parameter<Json>> params,
                    jsoncons::jmespath::dynamic_resources<Json>& resources,
                    std::error_code& ec) -> JsonReference
                {
                    auto now = std::chrono::system_clock::now();
                    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
                    return *resources.create_json(milliseconds.count());
                }
            );
            this->register_function("add", // function name
                2,                           // number of arguments   
                [](jsoncons::span<const jsoncons::jmespath::parameter<Json>> params,
                    jsoncons::jmespath::dynamic_resources<Json>& resources,
                    std::error_code& ec) -> JsonReference
                {
                    JSONCONS_ASSERT(2 == params.size());

                    if (!(params[0].is_value() && params[1].is_value()))
                    {
                        ec = jmespath_errc::invalid_argument;
                        return resources.null_value();
                    }

                    const auto arg0 = params[0].value();
                    const auto arg1 = params[1].value();
                    if (!(arg0.is_number() && arg1.is_number()))
                    {
                        ec = jmespath_errc::invalid_argument;
                        return resources.null_value();
                    }

                    if (is_integer(arg0) && is_integer(arg1))
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
    };

        // When adding custom functions, they are generally placed in their own project's source code and namespace.

        class current_date_time_function : public function_base
        {
        public:
            current_date_time_function() : function_base(0) {}
            JsonReference evaluate(const std::vector<parameter>& params, dynamic_resources& resources, std::error_code& ec) const override
            {
                auto now = std::chrono::system_clock::now();
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
                return *resources.create_json(milliseconds.count());
            }
            std::string to_string(std::size_t = 0) const override
            {
                return std::string("current_date_time_function\n");
            }
        };

        class current_index_function : public function_base
        {
        public:
            current_index_function() : function_base(0) {}
            JsonReference evaluate(const std::vector<parameter>& params, dynamic_resources& resources, std::error_code& ec) const override
            {
                size_t index = current_index_function::index;
                JsonReference result = *resources.create_json(index);
                return result;
            }
            std::string to_string(std::size_t = 0) const override
            {
                return std::string("current_index_function\n");
            }

            static thread_local size_t index;
        };

        thread_local size_t current_index_function::index = 0;

        /// @brief generate array，include 4 params：context value，array size (or &expression)，&generate expression，default value (or &expression)
        class generate_array_function : public function_base
        {
        public:
            generate_array_function() : function_base(4) {} // context, size (or &expression), &expression, default (or &expression)
            JsonReference evaluate(const std::vector<parameter>& params, dynamic_resources& resources, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(params.size() == *this->arity());

                if (!(params[0].is_value() && params[2].is_expression()))
                {
                    ec = jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                const auto context = params[0].value();
                const auto countValue = get_value(context, resources, params[1]);
                const auto& expr = params[2].expression();
                const auto& argDefault = params[3];

                if (!countValue.is_number())
                {
                    ec = jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                auto result = resources.create_json(json_array_arg);
                int count = countValue.template as<int>();
                for (size_t i = 0; i < count; i++)
                {
                    current_index_function::index = i;
                    std::error_code ec2;

                    auto ele = expr.evaluate(context, resources, ec2);

                    if (ele.is_null())
                    {
                        auto defaultVal = get_value(context, resources, argDefault);
                        result->emplace_back(defaultVal);
                    }
                    else
                    {
                        // result->emplace_back(ele); // ?: It may lead to an abnormal exit.
                        result->emplace_back(*resources.create_json(deep_copy(ele)));
                    }
                }
                current_index_function::index = 0;

                return *result;
            }
            std::string to_string(std::size_t = 0) const override
            {
                return std::string("generate_array_function\n");
            }
        };

        class add_function : public function_base
        {
        public:
            add_function() : function_base(2) {}
            JsonReference evaluate(const std::vector<parameter>& params, dynamic_resources& resources, std::error_code& ec) const override
            {
                JSONCONS_ASSERT(params.size() == *this->arity());

                if (!(params[0].is_value() && params[1].is_value()))
                {
                    ec = jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                const auto arg0 = params[0].value();
                const auto arg1 = params[1].value();
                if (!(arg0.is_number() && arg1.is_number()))
                {
                    ec = jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                if (is_integer(arg0) && is_integer(arg1))
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
            std::string to_string(std::size_t = 0) const override
            {
                return std::string("add_function\n");
            }
        };

        void init_customer_jmespath_functions()
        {
            customer_get_function cgf = [](const string_type& name) -> const function_base*
                {
                    static current_date_time_function current_date_time_func;
                    static current_index_function current_index_func;
                    static generate_array_function generate_array_func;
                    static add_function add_func;

                    static std::map<string_type, function_base*> functions = {
                        {"current_date_time", &current_date_time_func},
                        {"current_index", &current_index_func},
                        {"generate_array", &generate_array_func} /*,
                        {"add", &add_func}*/ };

                    auto it = functions.find(name);

                    if (it == functions.end())
                    {
                        return nullptr;
                    }
                    else
                    {
                        return it->second;
                    }
                };

            static_resources::get_or_set_customer_get_function(cgf, true);
        }

    }


// for brevity
using jsoncons::json;
namespace jmespath = jsoncons::jmespath;

void jmespath_customer_functions_example()
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

  auto expr = jmespath::jmespath_expression<json>::compile("generate_array(devices, `16`, &[?position==add(current_index(), `1`)] | [0], &{id: '', state: `0`, position: add(current_index(), `1`)})",
      myspace::my_custom_functions<json>{});

  json doc = json::parse(jtext);

  json result = expr.evaluate(doc);

  std::cout << pretty_print(result) << "\n\n";
}

int main()
{
  std::cout << "\nJMESPath customer functions examples\n\n";
  myspace::init_customer_jmespath_functions();

  jmespath_customer_functions_example();

  std::cout << "\n";
}
