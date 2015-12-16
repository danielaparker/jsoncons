# jsoncons: a C++ library for json construction

jsoncons is a C++ library for the construction of [JavaScript Object Notation (JSON)](http://www.json.org). It supports parsing a JSON file or string into a tree structured `json` value, building a `json` value in C++ code, and serializing a `json` value to a file or string. It also provides an event-based API for reading and writing JSON documents that are too large to fit into available memory, somewhat analogously to SAX processing in the XML world. Consult the wiki for the latest [documentation and tutorials](https://github.com/danielaparker/jsoncons/wiki) and [roadmap](https://github.com/danielaparker/jsoncons/wiki/Roadmap). 

jsoncons uses some features that are new to C++ 11, particularly move semantics, however, it has been written to be compatible with VC++ 10 SP1 (note that SP1 is required for VC++ 10, it fixes compiler bugs with move semantics.) It has been tested with MS Visual C++ 10 SP1, Intel C++ Studio XE 2013, clang 3.3 and GCC 4.8. 

The [code repository](https://github.com/danielaparker/jsoncons) and [releases](https://github.com/danielaparker/jsoncons/releases) are on github. It is distributed under the [Boost Software License](http://www.boost.org/users/license.html)

The library has a number of features, which are listed below:

- Uses the standard C++ input/output streams library
- Implements parsing and serializing JSON text in UTF-8 for narrow character strings and streams
- Supports UTF16 (UTF32) encodings with size 2 (size 4) wide characters
- Correctly handles surrogate pairs in \uXXXX escape sequences
- Supports event based JSON parsing and serializing with user defined input and output handlers
- Accepts and ignores single line comments that start with //, and multi line comments that start with /* and end with */
- Supports optional escaping of the solidus (/) character
- Supports Nan, Inf and -Inf replacement
- Supports reading a sequence of JSON texts from a stream
- Supports optional escaping of non-ascii UTF-8 octets
- Allows extensions to the types accepted by the json class accessors and modifiers
- Supports storing "any" values in a json object or array, with specialized serialization
- Supports reading (writing) JSON values from (to) CSV files
- Passes all tests from [JSON_checker](http://www.json.org/JSON_checker/) except `fail1.json`, which is allowed in [RFC7159](http://www.ietf.org/rfc/rfc7159.txt)
- Handles JSON texts of arbitrarily large depth of nesting, a limit can be set if desired

## What's new on master

- Supports [Stefan Goessner JsonPath](http://goessner.net/articles/JsonPath/).

## Using the jsoncons library

The jsoncons library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries. The accompanying test suite uses boost, but not the library itself.

To install the jsoncons library, download the zip file, unpack the release, under `src` find the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

The jsoncons classes and functions are in namespace `jsoncons`.  Namespace `jsoncons` includes a name for a null type defined as an empty struct:

	struct null_type{}

These include and using declarations are for the examples that appear below.

    #include "jsoncons/json.hpp"
    #include "jsoncons_ext/csv/json.hpp"

    using jsoncons::json;
    using jsoncons::json_exception;    
    using jsoncons::pretty_print;
    using jsoncons::json_deserializer;
    using jsoncons::csv::csv_parameters;
    using jsoncons::csv::csv_reader;

### Reading JSON text from a file

Here is a sample file (books.json):

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

You can read the JSON text into a `json` value like this

    json books = json::parse_file("books.json");

You can then loop through the books

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            double price = book["price"].as<double>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const json_exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

The output is

    Haruki Murakami, Kafka on the Shore, 25.17
    Charles Bukowski, Women: A Novel, 12
    Member price not found.

Note that the third book is missing a price, which causes an exception to be thrown.

You have a choice of accessors:

    book["price"] will throw if there is no price
    book.get("price") will return a json null value if there is no price
    book.get("price",default_value) will return default_value if there is no price

So if you want to show "n/a" for the missing price, you can use this accessor

    std::string price = book.get("price","n/a").as<std::string>();

and the output becomes

    Haruki Murakami, Kafka on the Shore, 25.17
    Charles Bukowski, Women: A Novel, 12.0
    Ivan Passer, Cutter's Way, n/a

Or you can check if book has a member "price" with the method `has_member`, and output accordingly,

    if (book.has_member("price"))
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

You can read the `CSV` file into a `json` value like this

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

### jsonpath

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
   
   using jsoncons::jsonpath::json_query;

   


### Iterators

`jsoncons::json` supports iterators for accessing the members of json objects and the elements of json arrays.

An example of iterating over the name-value pairs of a json object:

    json person;
    person["first_name"] = "Jane";
    person["last_name"] = "Roe";
    person["events_attended"] = 10;
    person["accept_waiver_of_liability"] = true;

    for (auto it = person.begin_members(); it != person.end_members(); ++it)
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

    for (auto it = cities.begin_elements(); it != cities.end_elements(); ++it)
    {
        std::cout << it->as<std::string>() << std::endl;
    }

### About jsoncons::json

The json class is an instantiation of the `basic_json` class template that uses `char` as the character type
and `std::allocator<void>` as the allocator type,

    typedef basic_json<char,std::allocator<void>> json

The library includes an instantiation for wide characters as well,

    typedef basic_json<wchar_t,std::allocator<void>> wjson

Note that the allocator type allows you to supply a custom allocator for dynamically allocated, 
fixed size small objects in the json container, the container rebinds it as necessay. 
For instance, you can use the boost `fast_pool_allocator`:

    #include <boost/pool/pool_alloc.hpp>
    #include "jsoncons/json.hpp"

    typedef jsoncons::basic_json<char, boost::fast_pool_allocator<void>> myjson;

    myjson o;

    o.set("FirstName","Joe");
    o.set("LastName","Smith");

This results in a json object instance being allocated from the boost memory pool. 
The allocator type is not used for structures including vectors and strings that use large or variable amounts of memory, 
these always use the default allocators.

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

    template <typename Char, typename Alloc, typename T>
    class json_type_traits
    {
    public:
        static bool is(const basic_json<Char,Alloc>&);
        static T as(const basic_json<Char,Alloc>& val);
        static void assign(basic_json<Char,Alloc>& lhs, T rhs);
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

    template <typename Char,class T> inline
    void serialize(basic_json_output_handler<Char>& os, const T&)
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


### Acknowledgements

Special thanks to our [contributors](https://github.com/danielaparker/jsoncons/blob/master/acknowledgements.txt)

