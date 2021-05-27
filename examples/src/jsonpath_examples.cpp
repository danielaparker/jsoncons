// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <fstream>
#include <cmath>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

// for brevity
using jsoncons::json; 
namespace jsonpath = jsoncons::jsonpath;

namespace {

    void json_query_examples() 
    {
        std::ifstream is("./input/store.json");
        json booklist = json::parse(is);

        // The authors of books that are cheaper than $10
        json result1 = jsonpath::json_query(booklist, "$.store.book[?(@.price < 10)].author");
        std::cout << "(1) " << result1 << "\n";

        // The number of books
        json result2 = jsonpath::json_query(booklist, "length($..book)");
        std::cout << "(2) " << result2 << "\n";

        // The third book
        json result3 = jsonpath::json_query(booklist, "$..book[2]");
        std::cout << "(3)\n" << pretty_print(result3) << "\n";

        // All books whose author's name starts with Evelyn
        json result4 = jsonpath::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
        std::cout << "(4)\n" << pretty_print(result4) << "\n";

        // The titles of all books that have isbn number
        json result5 = jsonpath::json_query(booklist, "$..book[?(@.isbn)].title");
        std::cout << "(5) " << result5 << "\n";

        // All authors and titles of books
        json result6 = jsonpath::json_query(booklist, "$['store']['book']..['author','title']");
        std::cout << "(6)\n" << pretty_print(result6) << "\n";

        // Union of two ranges of book titles
        json result7 = jsonpath::json_query(booklist, "$..book[1:2,2:4].title");
        std::cout << "(7)\n" << pretty_print(result7) << "\n";

        // Union of a subset of book titles identified by index
        json result8 = jsonpath::json_query(booklist, "$.store[@.book[0].title,@.book[1].title,@.book[3].title]");
        std::cout << "(8)\n" << pretty_print(result8) << "\n";

        // Union of third book title and all book titles with price > 10
        json result9 = jsonpath::json_query(booklist, "$.store[@.book[3].title,@.book[?(@.price > 10)].title]");
        std::cout << "(9)\n" << pretty_print(result9) << "\n";

        // Intersection of book titles with category fiction and price < 15
        json result10 = jsonpath::json_query(booklist, "$.store.book[?(@.category == 'fiction')][?(@.price < 15)].title");
        std::cout << "(10)\n" << pretty_print(result10) << "\n";

        // Normalized path expressions
        json result11 = jsonpath::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]", jsonpath::result_options::path);
        std::cout << "(11)\n" << pretty_print(result11) << "\n";

        // All titles whose author's second name is 'Waugh'
        json result12 = jsonpath::json_query(booklist,"$.store.book[?(tokenize(@.author,'\\\\s+')[1] == 'Waugh')].title");
        std::cout << "(12)\n" << result12 << "\n";

        // All keys in the second book
        json result13 = jsonpath::json_query(booklist,"keys($.store.book[1])");
        std::cout << "(13)\n" << result13 << "\n";

        // 
        json result14 = jsonpath::json_query(booklist,"$.store.book[?(ceil(@.price) == 9)]");
        std::cout << "(14)\n" << result14 << "\n";

        // 
        json result15 = jsonpath::json_query(booklist,"$.store.book[?(ceil(@.price*100) == 895)]");
        std::cout << "(15)\n" << result15 << "\n";

        // 
        json result16 = jsonpath::json_query(booklist,"$.store.book[?(floor(@.price) == 8)]");
        std::cout << "(16)\n" << result16 << "\n";

        // 
        json result17 = jsonpath::json_query(booklist,"$.store.book[?(floor(@.price*100) == 895)]");
        std::cout << "(17)\n" << result17 << "\n";

        json result18 = jsonpath::json_query(booklist,"floor($.store.book[0].price*100)");
        std::cout << "(18)\n" << result18.to_string() << "\n";
    }

    void function_tokenize_example() 
    {
        std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami"
        },
        {
            "title" : "Almost Transparent Blue",
            "author" : "Ryu Murakami"
        },
        {
            "title" : "The Quiet American",
            "author" : "Graham Greene"
        }
    ]
}
        )";

        json j = json::parse(data);

        // All titles whose author's last name is 'Murakami'
        std::string expr = R"($.books[?(tokenize(@.author,'\\s+')[-1] == 'Murakami')].title)";

        json result = jsonpath::json_query(j, expr);
        std::cout << pretty_print(result) << "\n\n";
    }

    void function_sum_example() 
    {
        std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
        )";

        json j = json::parse(data);

        // All titles whose price is greater than the average price
        std::string expr = R"($.books[?(@.price > sum($.books[*].price)/length($.books[*].price))].title)";

        json result = jsonpath::json_query(j, expr);
        std::cout << result << "\n\n";
    }

    void function_avg_example() 
    {
        std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
        )";

        json j = json::parse(data);

        // All titles whose price is greater than the average price
        std::string expr = R"($.books[?(@.price > avg($.books[*].price))].title)";

        json result = jsonpath::json_query(j, expr);
        std::cout << result << "\n\n";
    }

    void function_floor_example() 
    {
        std::string data = R"(
        [
          {
            "number" : 8.95
          },
          {
            "number" : -8.95
          }
        ]        
        )";

        json j = json::parse(data);

        json result1 = jsonpath::json_query(j, "$[?(floor(@.number*100) == 895)]");
        std::cout << "(1) " << result1 << "\n\n";
        json result2 = jsonpath::json_query(j, "$[?(floor(@.number*100) == 894)]");
        std::cout << "(2) " << result2 << "\n\n";
        json result3 = jsonpath::json_query(j, "$[?(floor(@.number*100) == -895)]");
        std::cout << "(3) " << result3 << "\n\n";
    }

    void function_ceil_example() 
    {
        std::string data = R"(
        {
            "books":
            [
                {
                    "title" : "A Wild Sheep Chase",
                    "author" : "Haruki Murakami",
                    "price" : 22.72
                },
                {
                    "title" : "The Night Watch",
                    "author" : "Sergei Lukyanenko",
                    "price" : 23.58
                }            
            ]
        }
        )";

        json j = json::parse(data);

        json result1 = jsonpath::json_query(j, "$.books[?(ceil(@.price) == 23.0)]");
        std::cout << "(1) " << result1 << "\n\n";
        json result2 = jsonpath::json_query(j, "$.books[?(ceil(@.price*100) == 2358.0)]");
        std::cout << "(2) " << result2 << "\n\n";
    }

    void function_keys_example() 
    {
        std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
        )";

        json j = json::parse(data);

        // All books that don't have a price
        std::string expr = "$.books[?(!contains(keys(@),'price'))]";

        json result = jsonpath::json_query(j, expr);
        std::cout << result << "\n\n";
    }

    void function_length_example() 
    {
        std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
        )";

        json j = json::parse(data);

        json result1 = jsonpath::json_query(j, "length($.books[*])");
        std::cout << "(1) " << result1 << "\n\n";

        json result2 = jsonpath::json_query(j, "length($.books[*].price)");
        std::cout << "(2) "  << result2 << "\n\n";
    }

    void json_replace_example1()
    { 
        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        jsonpath::json_replace(data,"$.books[?(@.title == 'A Wild Sheep Chase')].price",20.0);
        std::cout << pretty_print(data) << "\n\n";
    }

    void json_replace_example2()
    {
        json j;
        try
        {
            j = json::parse(R"(
    {"store":
    {"book": [
    {"category": "reference",
    "author": "Margaret Weis",
    "title": "Dragonlance Series",
    "price": 31.96}, 
    {"category": "reference",
    "author": "Brent Weeks",
    "title": "Night Angel Trilogy",
    "price": 14.70
    }]}}
    )");
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

        std::cout << ("1\n") << pretty_print(j) << "\n";

        jsonpath::json_replace(j,"$..book[?(@.price==31.96)].price", 30.9);

        std::cout << ("2\n") << pretty_print(j) << "\n\n";
    }

    void json_replace_example3()
    {
        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        auto f = [](const std::string& /*path*/, json& price) 
        {
            price = std::round(price.as<double>() - 1.0);
        };

        // make a discount on all books
        jsonpath::json_replace(data, "$.books[*].price", f);
        std::cout << pretty_print(data) << "\n\n";
    }

    void json_replace_example4()
    {
        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        auto f = [](const std::string& /*path*/, json& book) 
        {
            if (book.at("category") == "memoir" && !book.contains("price"))
            {
                book.try_emplace("price",140.0);
            }
        };

        jsonpath::json_replace(data, "$.books[*]", f);
        std::cout << pretty_print(data) << "\n\n";
    }

    void jsonpath_complex_examples()
    {
        const json j = json::parse(R"(
        [
          {
            "root": {
              "id" : 10,
              "second": [
                {
                     "names": [
                       2
                  ],
                  "complex": [
                    {
                      "names": [
                        1
                      ],
                      "panels": [
                        {
                          "result": [
                            1
                          ]
                        },
                        {
                          "result": [
                            1,
                            2,
                            3,
                            4
                          ]
                        },
                        {
                          "result": [
                            1
                          ]
                        }
                      ]
                    }
                  ]
                }
              ]
            }
          },
          {
            "root": {
              "id" : 20,
              "second": [
                {
                  "names": [
                    2
                  ],
                  "complex": [
                    {
                      "names": [
                        1
                      ],
                      "panels": [
                        {
                          "result": [
                            1
                          ]
                        },
                        {
                          "result": [
                            3,
                            4,
                            5,
                            6
                          ]
                        },
                        {
                          "result": [
                            1
                          ]
                        }
                      ]
                    }
                  ]
                }
              ]
            }
          }
        ]
        )");

        // Find all arrays of elements where length(@.result) is 4
        json result1 = jsonpath::json_query(j,"$..[?(length(@.result) == 4)].result");
        std::cout << "(1) " << result1 << "\n";

        // Find array of elements that has id 10 and length(@.result) is 4
        json result2 = jsonpath::json_query(j,"$..[?(@.id == 10)]..[?(length(@.result) == 4)].result");
        std::cout << "(2) " << result2 << "\n";

        // Find all arrays of elements where length(@.result) is 4 and that have value 3 
        json result3 = jsonpath::json_query(j,"$..[?(length(@.result) == 4 && (@.result[0] == 3 || @.result[1] == 3 || @.result[2] == 3 || @.result[3] == 3))].result");
        std::cout << "(3) " << result3 << "\n";
    }

    void jsonpath_union()
    {
        const json root = json::parse(R"(
    {
      "firstName": "John",
      "lastName" : "doe",
      "age"      : 26,
      "address"  : {
        "streetAddress": "naist street",
        "city"         : "Nara",
        "postalCode"   : "630-0192"
      },
      "phoneNumbers": [
        {
          "type"  : "iPhone",
          "number": "0123-4567-8888"
        },
        {
          "type"  : "home",
          "number": "0123-4567-8910"
        }
      ]
    }    )");

        std::string path = "$..[@.firstName,@.address.city]";
        json result = jsonpath::json_query(root,path);

        std::cout << result << "\n";
    }

    void flatten_and_unflatten()
    {
        json input = json::parse(R"(
        {
           "application": "hiking",
           "reputons": [
               {
                   "rater": "HikingAsylum",
                   "assertion": "advanced",
                   "rated": "Marilyn C",
                   "rating": 0.90
                },
                {
                   "rater": "HikingAsylum",
                   "assertion": "intermediate",
                   "rated": "Hongmin",
                   "rating": 0.75
                }    
            ]
        }
        )");

        json result = jsonpath::flatten(input);

        std::cout << pretty_print(result) << "\n";

        json original = jsonpath::unflatten(result);
        assert(original == input);
    }

    void more_json_query_examples()
    {
        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        auto result1 = jsonpath::json_query(data, "$.books[1,1,3].title");
        std::cout << "(1)\n" << pretty_print(result1) << "\n\n";

        auto result2 = jsonpath::json_query(data, "$.books[1,1,3].title",
                                            jsonpath::result_options::path);
        std::cout << "(2)\n" << pretty_print(result2) << "\n\n";

        auto result3 = jsonpath::json_query(data, "$.books[1,1,3].title", 
                                            jsonpath::result_options::nodups);
        std::cout << "(3)\n" << pretty_print(result3) << "\n\n";

        auto result4 = jsonpath::json_query(data, "$.books[1,1,3].title",
                                            jsonpath::result_options::path | jsonpath::result_options::nodups);
        std::cout << "(4)\n" << pretty_print(result4) << "\n\n";
    }

    void make_expression_examples()
    {
        auto expr = jsonpath::make_expression<json>("$.books[1,1,3].title");

        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        json result1 = expr.evaluate(data);
        std::cout << "(1) " << pretty_print(result1) << "\n\n";

        json result2 = expr.evaluate(data, jsonpath::result_options::path);
        std::cout << "(2) " << pretty_print(result2) << "\n\n";

        json result3 = expr.evaluate(data, jsonpath::result_options::nodups);
        std::cout << "(3) " << pretty_print(result3) << "\n\n";

        json result4 = expr.evaluate(data, jsonpath::result_options::path | jsonpath::result_options::nodups);
        std::cout << "(4) " << pretty_print(result4) << "\n\n";
    }

    void more_make_expression_example()
    {
        auto expr = jsonpath::make_expression<json>("$.books[?(@.price > avg($.books[*].price))].title");

        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        json result = expr.evaluate(data);
        std::cout << pretty_print(result) << "\n\n";
    }

    void make_expression_with_callback_example()
    {
        auto expr = jsonpath::make_expression<json>("$.books[?(@.price >= 22.0)]");

        std::ifstream is("./input/books.json");
        json data = json::parse(is);

        auto callback = [](const std::string& path, const json& val)
        {
            std::cout << path << ": " << val << "\n";
        };
        expr.evaluate(data, callback, jsonpath::result_options::path);
    }

    void json_query_with_callback_example()
    {
        std::ifstream is("./input/books.json");
        json data = json::parse(is);
        std::string path = "$.books[?(@.price >= 22.0)]";

        auto callback = [](const std::string& path, const json& val)
        {
            std::cout << path << ": " << val << "\n";
        };
        jsonpath::json_query(data, path, callback, jsonpath::result_options::path);
    }

    void json_query_with_options_example()
    {
        std::string s = "[1,2,3,4,5]";
        json data = json::parse(s);
        std::string path = "$[4,1,1]";

        auto result1 = jsonpath::json_query(data, path);
        std::cout << "(1) " << result1 << "\n\n";

        auto result2 = jsonpath::json_query(data, path, jsonpath::result_options::path);
        std::cout << "(2) " << result2 << "\n\n";

        auto result3 = jsonpath::json_query(data, path, jsonpath::result_options::sort);
        std::cout << "(3) " << result3 << "\n\n";

        auto result4 = jsonpath::json_query(data, path, 
                                            jsonpath::result_options::sort | jsonpath::result_options::path);
        std::cout << "(4) " << result4 << "\n\n";

        auto result5 = jsonpath::json_query(data, path, jsonpath::result_options::nodups);
        std::cout << "(5) " << result5 << "\n\n";

        auto result6 = jsonpath::json_query(data, path, jsonpath::result_options::nodups | jsonpath::result_options::path);
        std::cout << "(6) " << result6 << "\n\n";

        auto result7 = jsonpath::json_query(data, path, jsonpath::result_options::nodups | jsonpath::result_options::sort);
        std::cout << "(7) " << result7 << "\n\n";

        auto result8 = jsonpath::json_query(data, path, 
                                            jsonpath::result_options::nodups | 
                                            jsonpath::result_options::sort |
                                            jsonpath::result_options::path);
        std::cout << "(8) " << result8 << "\n\n";
    }

    void search_for_and_replace_a_value()
    {
        std::string data = R"(
          { "books": [ 
              { "author": "Nigel Rees",
                "title": "Sayings of the Century",
                "isbn": "0048080489",
                "price": 8.95
              },
              { "author": "Evelyn Waugh",
                "title": "Sword of Honour",
                "isbn": "0141193557",
                "price": 12.99
              },
              { "author": "Herman Melville",
                "title": "Moby Dick",
                "isbn": "0553213113",
                "price": 8.99
              }
            ]
          }
        )";

        json j = json::parse(data);

        // Change the price of "Moby Dick" from $8.99 to $10
        jsonpath::json_replace(j,"$.books[?(@.isbn == '0553213113')].price",10.0);

        // Increase the price of "Sayings of the Century" by $1
        auto f = [](const std::string& /*path*/, json& value) 
        {
            value = value.as<double>() + 1.0;
        };
        jsonpath::json_replace(j, "$.books[?(@.isbn == '0048080489')].price", f); // (since 0.161.0)

        std::cout << pretty_print(j) << std::endl;
    }

    void union_example() 
    {
        std::ifstream is("./input/store.json");
        json store = json::parse(is);

        std::string path = "$.store.book[0:2,-1,?(@.author=='Herman Melville')].title";
        auto result1 = jsonpath::json_query(store, path);
        std::cout << "(1) " << result1 << "\n\n";

        auto result2 = jsonpath::json_query(store, path, jsonpath::result_options::path);
        std::cout << "(2) " << result2 << "\n\n";
    }

    template <class Json>
    class my_custom_functions : public jsonpath::custom_functions<Json>
    {
    public:
        my_custom_functions()
        {
            this->register_function("divide", // function name
                 2,                           // number of arguments   
                 [](jsoncons::span<const jsonpath::parameter<Json>> params, 
                    std::error_code& ec) -> Json 
                 {
                   const Json& arg0 = params[0].value();    
                   const Json& arg1 = params[1].value();    

                   if (!(arg0.is_number() && arg1.is_number())) 
                   {
                       ec = jsonpath::jsonpath_errc::invalid_type; 
                       return Json::null();
                   }
                   return Json(arg0.as<double>() / arg1.as<double>());
                 }
            );
        }
    };

    void custom_functions1()
    {
        my_custom_functions<json> funcs;

        json root = json::parse(R"([{"foo": 60, "bar": 10},{"foo": 60, "bar": 5}])");
        std::cout << pretty_print(root) << "\n\n";

        auto expr = jsonpath::make_expression<json>("$[?(divide(@.foo, @.bar) == 6)]", funcs);
        json result = expr.evaluate(root);

        std::cout << pretty_print(result) << "\n\n";
    }

    void custom_functions2()
    {
        my_custom_functions<json> funcs;

        json root = json::parse(R"([{"foo": 60, "bar": 10},{"foo": 60, "bar": 5}])");
        std::cout << pretty_print(root) << "\n\n";

        json result = jsonpath::json_query(root, "$[?(divide(@.foo, @.bar) == 6)]", jsonpath::result_options(), funcs);

        std::cout << pretty_print(result) << "\n\n";
    }

} // namespace

void jsonpath_examples()
{
    std::cout << "\nJsonPath examples\n\n";

    jsonpath_complex_examples();
    jsonpath_union();
    flatten_and_unflatten();
    more_json_query_examples();
    make_expression_examples();
    more_make_expression_example();
    json_query_with_options_example();
    make_expression_with_callback_example();
    json_query_with_callback_example();
    json_replace_example2();
    json_replace_example3();
    json_replace_example1();
    json_replace_example4();

    function_tokenize_example();
    function_sum_example();
    function_avg_example();
    function_length_example();
    function_keys_example();
    search_for_and_replace_a_value();

    custom_functions1();
    custom_functions2();

    json_query_examples();
    function_floor_example();
    function_ceil_example();

    union_example();
    std::cout << "\n";
}

