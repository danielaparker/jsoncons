# jsoncons: a C++ library for json construction

jsoncons is a C++ library for the construction of [JavaScript Object Notation (JSON)](http://www.json.org). It supports parsing a JSON file or string into a `json` value, building a `json` value in C++ code, and serializing a `json` value to a file or string. It also provides an API for generating json read and write events in code, somewhat analogously to SAX processing in the XML world. Consult the wiki for the latest [documentation and tutorials](https://github.com/danielaparker/jsoncons/wiki) and [roadmap](https://github.com/danielaparker/jsoncons/wiki/Roadmap). 

jsoncons uses some features that are new to C++ 11, particularly [move semantics](http://thbecker.net/articles/rvalue_references/section_02.html) and the [AllocatorAwareContainer](http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer) concept. Through the use of conditional directives, however, it retains compatibility with VC++ 10 SP1 (note that SP1 is required for VC++ 10, it fixes compiler bugs with move semantics.) It has been tested with MS Visual C++ 10 SP1, MS Visual C++ 15, clang 3.3 and GCC 4.8. 

## Using the jsoncons library

The jsoncons library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries. 

To install the jsoncons library, download the zip file, extract the zipped files, look under `src` for the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

The jsoncons classes and functions are in namespace `jsoncons`. You need to include the header file
 
    #include "jsoncons/json.hpp"

and, for convenience, you can add a few using declarations

    using jsoncons::json;
    using jsoncons::json_deserializer;

### Reading JSON text from a file

Here is a sample file, `books.json`:

    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]

It consists of an array of book elements, each element is an object with members title, author, and price.

Read the JSON text into a `json` value,

    std::ifstream is("books.json");
    json books;
    is >> books;

Loop through the book array elements, using either a range-based for loop

    for (auto book : books.elements())
    {
        std::string author = book["author"].as<std::string>();
        std::string title = book["title"].as<std::string>();
        std::cout << author << ", " << title << std::endl;
    }

or a traditional for loop

    for (size_t i = 0; i < books.size(); ++i)
    {
        json& book = books[i];
        std::string author = book["author"].as<std::string>();
        std::string title = book["title"].as<std::string>();
        std::cout << author << ", " << title << std::endl;
    }

or begin-end iterators

    for (auto it = books.elements().begin(); 
         it != books.elements().end();
         ++it)
    {
        std::string author = (*it)["author"].as<std::string>();
        std::string title = (*it)["title"].as<std::string>();
        std::cout << author << ", " << title << std::endl;
    } 

The output is

    Haruki Murakami, Kafka on the Shore
    Charles Bukowski, Women: A Novel
    Ivan Passer, Cutter's Way

Note that the third book, Cutter's Way, is missing a price.

You have a choice of object member accessors:

    book["price"] will throw a `json_exception` if there is no price
    book.get("price",default_value) will return `default_value` if there is no price

So if you want to show "n/a" for the missing price, you can use this accessor

    std::string price = book.get("price","n/a").as<std::string>();

Or you can check if book has a member "price" with the method `count`, and output accordingly,

    if (book.count("price") > 0)
    {
        double price = book["price"].as<double>();
        std::cout << price;
    }
    else
    {
        std::cout << "n/a";
    }

### Constructing json values in C++

To construct an empty json object, use the default constructor:

     json image_sizing;

Serializing it to standard out

    std::cout << image_sizing << std::endl;

produces

    {}

Adding some members,

    image_sizing["resize_to_fit"] = true;  // a boolean 
    image_sizing["resize_unit"] =  "pixels";  // a string
    image_sizing["resize_what"] =  "long_edge";  // a string
    image_sizing["dimension1"] = 9.84;  // a double
    image_sizing["dimension2"] = json::null_type();  // a null

Serializing it, this time with pretty print,

 	std::cout << pretty_print(image_sizing) << std::endl;

produces

    {
        "dimension1":9.84,
        "dimension2":null,
        "resize_to_fit":true,
        "resize_unit":"pixels",
        "resize_what":"long_edge"
    }

To construct a json array, use the array type default constructor:

    json image_formats = json::array();

Adding some elements,

    image_formats.add("JPEG");
    image_formats.add("PSD");
    image_formats.add("TIFF");
    image_formats.add("DNG");

Combining the two

    json file_export;
    file_export["image_formats"] = std::move(image_formats);
    file_export["image_sizing"] = std::move(image_sizing);

and serializing

    std::cout << pretty_print(file_export) << std::endl;
produces

    {
        "image_formats":
        ["JPEG","PSD","TIFF","DNG"],
        "image_sizing":
        {
            "dimension1":9.84,
            "dimension2":null,
            "resize_to_fit":true,
            "resize_unit":"pixels",
            "resize_what":"long_edge"
        }
    }

### Converting CSV files to json

Here is a sample CSV file (tasks.csv):

    project_id, task_name, task_start, task_finish
    4001,task1,01/01/2003,01/31/2003
    4001,task2,02/01/2003,02/28/2003
    4001,task3,03/01/2003,03/31/2003
    4002,task1,04/01/2003,04/30/2003
    4002,task2,05/01/2003,

You can read the `CSV` file into a `json` value with the `csv_reader`.

    #include "jsoncons_ext/csv/csv_reader.hpp"

    using jsoncons::csv::csv_parameters;
    using jsoncons::csv::csv_reader;

    std::fstream is("tasks.csv");

    json_deserializer handler;

    csv_parameters params;
    params.assume_header(true);
    params.trim(true);
    params.ignore_empty_values(true);
    params.data_types("integer,string,string,string");

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;

The output is:

    [
        {
            "project_id":4001,
            "task_finish":"01/31/2003",
            "task_name":"task1",
            "task_start":"01/01/2003"
        },
        {
            "project_id":4001,
            "task_finish":"02/28/2003",
            "task_name":"task2",
            "task_start":"02/01/2003"
        },
        {
            "project_id":4001,
            "task_finish":"03/31/2003",
            "task_name":"task3",
            "task_start":"03/01/2003"
        },
        {
            "project_id":4002,
            "task_finish":"04/30/2003",
            "task_name":"task1",
            "task_start":"04/01/2003"
        },
        {
            "project_id":4002,
            "task_name":"task2",
            "task_start":"05/01/2003"
        }
    ]

There are a few things to note about the effect of the parameter settings.
- `assume_header` `true` tells the csv parser to parse the first line of the file for column names, which become object member names.
- `trim` `true` tells the parser to trim leading and trailing whitespace, in particular, to remove the leading whitespace in the column names.
- `ignore_empty_values` `true` causes the empty last value in the `task_finish` column to be omitted.
- The `data_types` setting specifies that column one ("project_id") contains integers and the remaining columns strings.

### Iterators

`jsoncons::json` supports iterators for accessing the members of json objects and the elements of json arrays.

An example of iterating over the name-value pairs of a json object:

    json person;
    person["first_name"] = "Jane";
    person["last_name"] = "Roe";
    person["events_attended"] = 10;
    person["accept_waiver_of_liability"] = true;

    for (auto it = person.members().begin(); it != person.members().end(); ++it)
    {
        std::cout << "name=" << it->name() 
                  << ", value=" << it->value().as<std::string>() << std::endl;
    }

An example of iterating over the elements of a json array:

    json cities= json::array();
    cities.add("Montreal");
    cities.add("Toronto");
    cities.add("Ottawa");
    cities.add("Vancouver");

    for (auto it = cities.elements().begin(); it != cities.elements().end(); ++it)
    {
        std::cout << it->as<std::string>() << std::endl;
    }

### jsonpath

[Stefan Goessner's JsonPath](http://goessner.net/articles/JsonPath/) is an XPATH inspired query language for selecting parts of a JSON structure.

Here is a sample JSON file (store.json):

    { "store": {
        "book": [ 
          { "category": "reference",
            "author": "Nigel Rees",
            "title": "Sayings of the Century",
            "price": 8.95
          },
          { "category": "fiction",
            "author": "Evelyn Waugh",
            "title": "Sword of Honour",
            "price": 12.99
          },
          { "category": "fiction",
            "author": "Herman Melville",
            "title": "Moby Dick",
            "isbn": "0-553-21311-3",
            "price": 8.99
          }
        ],
      }
    }

The following code returns all authors whose books are cheaper than $10. 
    
    #include "jsoncons_ext/jsonpath/json_query.hpp"

    using jsoncons::jsonpath::json_query;

    json root = json::parse_file("store.json");

    json result = json_query(root,"$.store.book[?(@.price < 10)].author");

    std::cout << pretty_print(result) << std::endl;

The result is

    ["Nigel Rees","Herman Melville"]

### About jsoncons::json

The json class is an instantiation of the `basic_json` class template that uses `std::string` as the string type
and `std::allocator<char>` as the allocator type,

    typedef basic_json<std::string,std::allocator<char>> json

The `jsoncons` library will always rebind the given allocator from the template parameter to internal data structures.

The library includes an instantiation for wide characters as well,

    typedef basic_json<std::wstring,std::allocator<wchar_t>> wjson

Note that the allocator type allows you to supply a custom allocator. For example, you can use the boost [fast_pool_allocator](http://www.boost.org/doc/libs/1_60_0/libs/pool/doc/html/boost/fast_pool_allocator.html):

    #include <boost/pool/pool_alloc.hpp>
    #include "jsoncons/json.hpp"

    typedef jsoncons::basic_json<std::string, boost::fast_pool_allocator<char>> myjson;

    myjson o;

    o.set("FirstName","Joe");
    o.set("LastName","Smith");

This results in a json value being constucted with all memory being allocated from the boost memory pool. (In this particular case there is no improvement in performance over `std::allocator`.)

Note that the underlying memory pool used by the this allocator is never freed. 

## Wide character support

jsoncons supports wide character strings and streams with `wjson` and `wjson_reader`. It supports `UTF16` encoding if `wchar_t` has size 2 (Windows) and `UTF32` encoding if `wchar_t` has size 4. You can construct a `wjson` value in exactly the same way as a `json` value, for instance:

    using jsoncons::wjson;

    wjson root;
    root[L"field1"] = L"test";
    root[L"field2"] = 3.9;
    root[L"field3"] = true;

    std::wcout << root << L"\n";

which prints

    {"field1":"test","field2":3.9,"field3":true}

### Type extensibility

In the json class, constructors, accessors and modifiers are templated, for example,

    template <typename T>
    explicit json(T val)

    template<typename T>
    bool is() const

    template<typename T>
    T as() const

    template <typename T>
    basic_json& operator=(T val)

    template <typename T>
    void add(T val)

The implementations of these functions and operators make use of the class template `json_type_traits`

    template <class JsonT, typename T>
    class json_type_traits
    {
    public:
        static bool is(const JsonT& rhs) noexcept;
        static T as(const JsonT& rhs);
        static void assign(JsonT& lhs, T rhs);
    };

This class template is extensible, you as a user can extend `json_type_traits` in the `jsoncons` namespace with your own types. You can, for example, extend `json_type_traits` to access and modify `json` structures with `boost::gregorian::date values`, and in your code, write

    json deal;
    deal["maturity"] = boost::gregorian::date(2015,1,1);
	
    json observation_dates = json::array();
    observation_dates.add(boost::gregorian::date(2013,10,21));
    observation_dates.add(boost::gregorian::date(2013,10,28));
    deal["observation_dates"] = std::move(observation_dates);
	
    boost::gregorian::date maturity = deal["maturity"].as<boost::gregorian::date>();
	
    std::cout << deal << std::endl;	

producing

    {
        "maturity":"2015-01-01",
        "observation_dates":
        ["2013-10-21","2013-10-28"]
    }

### json any

jsoncons provides a class `json::any` that can contain a value of any type as long as that type supports copy construction and 
assignment. This allows you to, for example, insert a boost matrix into a `json` object, and to retrieve it back cast to the appropriate type. You can do so by wrapping it in
a `json::any` value, like this:

    #include <boost/numeric/ublas/matrix.hpp>

    using boost::numeric::ublas::matrix;

    json val;

    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;

    val.set("A",json::any(A));

    matrix<double>& B = val["A"].any_cast<matrix<double>>();

By default, if you print `val` on a stream, 

    std::cout << pretty_print(val) << std::endl;

the template function

    template <typename CharT,class T> inline
    void serialize(basic_json_output_handler<CharT>& os, const T&)
    {
        os.value(null_type());
    }

gets called, and produces a `null` value for the matrix. You can however introduce a specialization of `serialize` for `boost::numeric::ublas::matrix` in the `jsoncons` namespace, to produce the output 

    {
        "A":
        [
            [1,2],
            [3,4]
        ]
    }

