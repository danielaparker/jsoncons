    #include "jsoncons/json.hpp"

    using jsoncons::json;

## Examining the type of a json value at runtime

### A json file

    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17,
            "reviews" :
            [
                {"rating" : "*****"},
                {"rating" : "*****"}
            ]
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00,
            "reviews" :
            [
                {"rating" : "*****"},
                {"rating" : "*"}
            ]
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer",
            "reviews" :
            [
                {"rating" : "****"}
            ]
        }
    ]
        
### Code to examine the properties of a json value at runtime:
        
    json val = json::parse_file("input/books.json");
    std::cout << std::boolalpha;
    std::cout << "Is this an object? " << val.is<json::object>() << ", or an array? " << val.is<json::array>() << std::endl;

    if (val.is<json::array>())
    {
        for (size_t i = 0; i < val.size(); ++i)
        {
            json& elem = val[i];
            std::cout << "Is element " << i << " an object? " << elem.is<json::object>() << std::endl;
            if (elem.is<json::object>())
            {
                for (auto it = elem.begin_members(); it != elem.end_members(); ++it)
                {
                    std::cout << "Is member " << it->name() << " a string? " << it->value().is<std::string>() << ", or a double? " << it->value().is<double>() << ", or perhaps an int? " << it->second.is<int>() << std::endl;

                }
            }
        }
    }
        
The output is
        
    Is this an object? false, or an array? true
    Is element 0 an object? true
    Is member author a string? true, or a double? false, or perhaps an int? false
    Is member price a string? false, or a double? true, or perhaps an int? false
    Is member reviews a string? false, or a double? false, or perhaps an int? false
    Is member title a string? true, or a double? false, or perhaps an int? false
    Is element 1 an object? true
    Is member author a string? true, or a double? false, or perhaps an int? false
    Is member price a string? false, or a double? true, or perhaps an int? false
    Is member reviews a string? false, or a double? false, or perhaps an int? false
    Is member title a string? true, or a double? false, or perhaps an int? false
    Is element 2 an object? true
    Is member author a string? true, or a double? false, or perhaps an int? false
    Is member reviews a string? false, or a double? false, or perhaps an int? false
    Is member title a string? true, or a double? false, or perhaps an int? false      
