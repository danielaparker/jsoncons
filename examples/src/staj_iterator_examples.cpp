// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_stream_reader.hpp>
#include <jsoncons/json.hpp>
#include <string>
#include <sstream>

using namespace jsoncons;

// Example JSON text
const std::string example = R"(
[ 
  { 
      "employeeNo" : "101",
      "name" : "Tommy Cochrane",
      "title" : "Supervisor"
  },
  { 
      "employeeNo" : "102",
      "name" : "Bill Skeleton",
      "title" : "Line manager"
  }
]
)";

void staj_array_iterator_example()
{
    std::istringstream is(example);

    json_stream_reader reader(is);

    staj_array_iterator<json> it(reader);

    for (const auto& j : it)
    {
        std::cout << pretty_print(j) << "\n";
    }
    std::cout << "\n\n";
}

struct employee
{
    std::string employeeNo;
    std::string name;
    std::string title;
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, employee>
    {
        static bool is(const Json& j) noexcept
        {
            return j.is_object() && j.contains("employeeNo") && j.contains("name") && j.contains("title");
        }
        static employee as(const Json& j)
        {
            employee val;
            val.employeeNo = j["employeeNo"].template as<std::string>();
            val.name = j["name"].template as<std::string>();
            val.title = j["title"].template as<std::string>();
            return val;
        }
        static Json to_json(const employee& val)
        {
            Json j;
            j["employeeNo"] = val.employeeNo;
            j["name"] = val.name;
            j["title"] = val.title;
            return j;
        }
    };
}

void staj_array_iterator_example2()
{
    std::istringstream is(example);

    json_stream_reader reader(is);

    staj_array_iterator<json,employee> it(reader);

    for (const auto& val : it)
    {
        std::cout << val.employeeNo << ", " << val.name << ", " << val.title << "\n";
    }
    std::cout << "\n\n";
}

void staj_iterator_examples()
{
    std::cout << "\nstaj_iterator examples\n\n";

    staj_array_iterator_example();

    staj_array_iterator_example2();

    std::cout << "\n";
}

