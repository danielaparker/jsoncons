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
- Supports [Stefan Goessner's JsonPath](http://goessner.net/articles/JsonPath/)

## What's new on master

- Supports [Stefan Goessner's JsonPath](http://goessner.net/articles/JsonPath/). See example below and [documentation](https://github.com/danielaparker/jsoncons/wiki/json_query).
- There are a few API changes, most retaining backwards compatability, refer to the [Changelog](https://github.com/danielaparker/jsoncons/blob/master/Changelog.md) for details.

## Benchmarks

[json_benchmarks](https://github.com/danielaparker/json_benchmarks) provides some measurements about how `jsoncons` compares to other `json` libraries.

## Using the jsoncons library

The jsoncons library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries. The accompanying test suite uses boost, but not the library itself.

To install the jsoncons library, download the zip file, unpack the release, under `src` find the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

For a quick guide, see the article [jsoncons: a C++ library for json construction](http://danielaparker.github.io/jsoncons).

## Examples

### json construction

    #include "jsoncons/json.hpp"

    // For convenience
    using jsoncons::json;

    // Construct a book object
    json book1;

    book1["category"] = "Fiction";
    book1["title"] = "A Wild Sheep Chase: A Novel";
    book1["author"] = "Haruki Murakami";
    book1["date"] = "2002-04-09";
    book1["price"] = 9.01;
    book1["isbn"] = "037571894X";  

    // Construct another using the member function set
    json book2;

    book2.set("category", "History");
    book2.set("title", "Charlie Wilson's War");
    book2.set("author", "George Crile");
    book2.set("date", "2007-11-06");
    book2.set("price", 10.50);
    book2.set("isbn", "0802143415");  

    // Use set again, but more efficiently
    json book3;

    // Reserve memory, to avoid reallocations
    book3.reserve(6);

    // Insert in name alphabetical order
    // Provide a hint to set where to insert the member
    json::object_iterator pos;
    pos = book3.set(book3.members().begin(),"author", "Haruki Murakami");
    pos = book3.set(pos, "category", "Fiction");
    pos = book3.set(pos, "date", "2006-01-03");
    pos = book3.set(pos, "isbn", "1400079276");  
    pos = book3.set(pos, "price", 13.45);
    pos = book3.set(pos, "title", "Kafka on the Shore");

    // Construct a fourth from a string
    json book4 = json::parse(R"(
    {
        "category" : "Fiction",
        "title" : "Pulp",
        "author" : "Charles Bukowski",
        "date" : "2004-07-08",
        "price" : 22.48,
        "isbn" : "1852272007"  
    }
    )");

    // Construct a booklist array
    json booklist = json::array();

    // For efficiency, reserve memory, to avoid reallocations
    booklist.reserve(4);

    // For efficency, tell jsoncons to move the contents 
    // of the four book objects into the array
    booklist.add(std::move(book1));    
    booklist.add(std::move(book2));    
    booklist.add(std::move(book3));

    // Add the last one to the front
    booklist.add(booklist.elements().begin(),std::move(book4));    

	// See what's left of book1, 2, 3 and 4 (expect nulls)
	std::cout << book1 << "," << book2 << "," << book3 << "," << book4 << std::endl;

    //Loop through the booklist elements using a range-based for loop    
    for(auto book : booklist.elements())
    {
		std::cout << book["title"].as<std::string>()
			<< ","
			<< book["price"].as<double>() << std::endl;
	}
	
	// Serialize the booklist to a file
	std::ofstream os("booklist.json");
	os << pretty_print(booklist);

The JSON output `booklist.json`

    [
        {
            "author":"Charles Bukowski",
            "category":"Fiction",
            "date":"2004-07-08",
            "isbn":"1852272007",
            "price":22.48,
            "title":"Pulp"
        },
        {
            "author":"Haruki Murakami",
            "category":"Fiction",
            "date":"2002-04-09",
            "isbn":"037571894X",
            "price":9.01,
            "title":"A Wild Sheep Chase: A Novel"
        },
        {
            "author":"George Crile",
            "category":"History",
            "date":"2007-11-06",
            "isbn":"0802143415",
            "price":10.5,
            "title":"Charlie Wilson's War"
        },
        {
            "author":"Haruki Murakami",
            "category":"Fiction",
            "date":"2006-01-03",
            "isbn":"1400079276",
            "price":13.45,
            "title":"Kafka on the Shore"
        }
    ]

### json query

    #include <fstream>
    #include "jsoncons/json.hpp"
    #include "jsoncons_ext/jsonpath/json_query.hpp"

    // For convenience
    using jsoncons::json;
    using jsoncons::jsonpath::json_query;

    // Deserialize the booklist
    std::ifstream is("booklist.json");
    json booklist;
    is >> booklist;

    // Use a JsonPath expression to find the authors of 
    // books that cost less than $12
    json result = json_query(booklist, "$[*][?(@.price < 12)].author");

    std::cout << pretty_print(result) << std::endl;

Result:

    ["Haruki Murakami","George Crile"]

## Once again, this time with wide characters

### wjson construction

    #include "jsoncons/json.hpp"

    // For convenience
    using jsoncons::wjson;

    // Construct a book object
    wjson book1;

    book1[L"category"] = L"Fiction";
    book1[L"title"] = L"A Wild Sheep Chase: A Novel";
    book1[L"author"] = L"Haruki Murakami";
    book1[L"date"] = L"2002-04-09";
    book1[L"price"] = 9.01;
    book1[L"isbn"] = L"037571894X";

    // Construct another using the member function set
    wjson book2;

    book2.set(L"category", L"History");
    book2.set(L"title", L"Charlie Wilson's War");
    book2.set(L"author", L"George Crile");
    book2.set(L"date", L"2007-11-06");
    book2.set(L"price", 10.50);
    book2.set(L"isbn", L"0802143415");

    // Use set again, but more efficiently
    wjson book3;

    // Reserve memory, to avoid reallocations
    book3.reserve(6);

    // Insert in name alphabetical order
    // Provide a hint where to insert the member
    wjson::object_iterator pos;
    pos = book3.set(book3.members().begin(), L"author", L"Haruki Murakami");
    pos = book3.set(pos, L"category", L"Fiction");
    pos = book3.set(pos, L"date", L"2006-01-03");
    pos = book3.set(pos, L"isbn", L"1400079276");
    pos = book3.set(pos, L"price", 13.45);
    pos = book3.set(pos, L"title", L"Kafka on the Shore");

    // Construct a fourth from a string
    wjson book4 = wjson::parse(LR"(
    {
        "category" : "Fiction",
        "title" : "Pulp",
        "author" : "Charles Bukowski",
        "date" : "2004-07-08",
        "price" : 22.48,
        "isbn" : "1852272007"  
    }
    )");

    // Construct a booklist array
    wjson booklist = wjson::array();

    // For efficiency, reserve memory, to avoid reallocations
    booklist.reserve(4);

    // For efficency, tell jsoncons to move the contents 
    // of the four book objects into the array
    booklist.add(std::move(book1));
    booklist.add(std::move(book2));
    booklist.add(std::move(book3));

    // Add the last one to the front
    booklist.add(booklist.elements().begin(),std::move(book4));    

    // See what's left of book1, 2, 3 and 4 (expect nulls)
    std::wcout << book1 << L"," << book2 << L"," << book3 << L"," << book4 << std::endl;

    //Loop through the booklist elements using a range-based for loop    
    for (auto book : booklist.elements())
    {
    	std::wcout << book[L"title"].as<std::wstring>()
        	   << L","
    		   << book[L"price"].as<double>() << std::endl;
    }

    // Serialize the booklist to a file
    std::wofstream os(L"booklist2.json");
    os << pretty_print(booklist);

### wjson query

    // Deserialize the booklist
    std::wifstream is("booklist2.json");
    wjson booklist;
    is >> booklist;

    // Use a JsonPath expression to find the authors of 
    // books that were published in 2004
    wjson result = json_query(booklist, L"$[*][?(@.date =~ /2004.*?/)].author");

    std::wcout << pretty_print(result) << std::endl;

Result:

    ["Charles Bukowski"]
       

## Acknowledgements

Special thanks to our [contributors](https://github.com/danielaparker/jsoncons/blob/master/acknowledgements.txt)

