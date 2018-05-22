## FAQ

### Deserialization

#### How can I parse JSON from a string?

```
std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

json j = json::parse(s);
```

or

```c++
using namespace jsoncons::literals;

json j = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30"          
}
)"_json;
```

#### How can I parse JSON from a file?

```
std::ifstream is("myfile.json");    

json j = json::parse(is);
```

#### How can I validate JSON without incurring parse exceptions?
```c++
std::string s = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" "2020-12-30"          
}
)";
std::stringstream is(s);

json_reader reader(is);

std::error_code ec;
reader.read(ec);
if (ec)
{
    std::cout << ec.message() 
              << " on line " << reader.line_number()
              << " and column " << reader.column_number()
              << std::endl;
}
```
Output:
```
Expected name separator ':' on line 4 and column 20
```

#### What if I want to allow comments? What if I don't?

jsoncons, by default, accepts and ignores C-style comments

```c++
std::string s = R"(
{
    // Single line comments
    /*
        Multi line comments 
    */
}
)";

// Default
json j = json::parse(s);
std::cout << "(1) " << j << std::endl;

// Strict
try
{
    strict_parse_error_handler err_handler;
    json j = json::parse(s, err_handler);
}
catch (const parse_error& e)
{
    std::cout << "(2) " << e.what() << std::endl;
}
```
Output:
```
(1) {}
(2) Illegal comment at line 3 and column 10
```

### Serialization

#### How can I serialize a json value to a string?

```
std::string s;

j.dump(s); // compressed

j.dump(s, indenting::indent); // pretty print
```

#### How can I serialize a json value to a stream?

```
j.dump(std::cout); // compressed

j.dump(std::cout, indenting::indent); // pretty print
```
or
```
std::cout << j << std::endl; // compressed

std::cout << pretty_print(j) << std::endl; // pretty_print
```

#### How can I iterate over a json array?

```c++
json book = json::array{1,2,3,4};

for (auto val : book.array_range())
{
    std::cout << val << std::endl;
}
```

#### How can I iterate over a json object?

```c++
json book = json::object{
    {"author", "Haruki Murakami"},
    {"title", "Kafka on the Shore"},
    {"price", 25.17}
};

for (const auto& kv : book.object_range())
{
    std::cout << kv.key() << "=" 
              << kv.value() << std::endl;
}
```

