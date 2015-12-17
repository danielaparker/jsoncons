    jsoncons::jsonpath::json_query

Returns a `json` array of `json` values extracted from a root `json` structure.

### Header

    #include "jsoncons/jsonpath/jsonpath.hpp"

    template<typename Char, class Alloc>
    basic_json<Char,Alloc> json_query(const basic_json<Char,Alloc>& root, 
                                      const std::basic_string<Char>& path);
    
### Stefan Goessner's JsonPath

JSONPath|	Description
--------|--------------------------------
`$`|	The root object
`@`|	the current object
`.` or `[]`|	Child operator
`..`	|Recursive descent. JSONPath borrows this syntax from E4X.
`*` |	Wildcard. All objects/elements regardless their names.
`[]`	|Subscript operator. In JSON it is the native array operator.
`[,]`	|JSONPath allows alternate names or array indices as a set.
`[start:end:step]`	|Array slice operator borrowed from [ES4](http://wiki.ecmascript.org/doku.php?id=proposals:slice_syntax&s=array+slice).
`?()`	|Applies a filter expression.
`()`	|Filter expression.

### Examples

The examples below use the JSON text from [Stefan Goessner's JsonPath](http://goessner.net/articles/JsonPath/) (store.json).

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
          },
          { "category": "fiction",
            "author": "J. R. R. Tolkien",
            "title": "The Lord of the Rings",
            "isbn": "0-395-19395-8",
            "price": 22.99
          }
        ],
        "bicycle": {
          "color": "red",
          "price": 19.95
        }
      }
    }

Our first example returns all authors whose books are cheaper than $10. 
    
    #include "jsoncons/json.hpp"
    #include "jsoncons_ext/jsonpath/jsonpath.hpp"

    using jsoncons::json;
    using jsoncons::pretty_print;
    using jsoncons::jsonpath::json_query;

    json root = json::parse_file("store.json");

    json result = json_query(root,"$.store.book[?(@.price < 10)].author");

    std::cout << pretty_print(result) << std::endl;

The result is

    ["Nigel Rees","Herman Melville"]

A list of sample JSON paths and results follows.

JSONPath |Result
---------|--------------------------------------------------------
`$.store.book[*].author`	|All authors of books in the store
`$..author`	            |All authors
`$.store.*`	            |Everything in the store, including books and a bicycle.
`$.store..price`	        |The prices of everything in the store.
`$..book[2]`	            |The third book
`$..book[-1:]`	        |The last book in order.
`$..book[0,1]`            |The first two books
`$..book[:2]`	            |All books from index 0 (inclusive) to index 2 (exclusive)
`$..book[1:2]`              |All books from index 1 (inclusive) to index 2 (exclusive)
`$..book[-2:]`	|Last two books
`$..book[2:]`	|Last two books
`$..book[?(@.isbn)]`	    |All books that have isbn number
`$..book[?(@.price<10)]`	|All books that are cheaper than $10
`$..*`	                |Everything in the store.
`$..book.length`	|The number of books
`$.store.book[ ?((@.author == 'Nigel Rees') || (@.author == 'Evelyn Waugh')) ].title`|The titles of all books by Nigel Rees and Evelyn Waugh

