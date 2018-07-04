### jsonpath extension

The jsonpath extension implements [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/).  

It provides functions for search and "search and replace" using JSONPath expressions:

[json_query](json_query.md)

[json_replace](json_replace.md)

The [Jayway JSONPath Evaluator](https://jsonpath.herokuapp.com/) and [JSONPATH Expression Tester](https://jsonpath.curiousconcept.com/)
are good online evaluators for checking JSONPath expressions.
    
### Stefan Goessner's JSONPath

[JSONPath](http://goessner.net/articles/JsonPath/) is a creation of Stefan Goessner. JSONPath expressions refer to a JSON text in the same way as XPath expressions refer to an XML document. 

Stefan Goessner's javascript implemention returns `false` in case of no match, but in a note he suggests an alternative is to return an empty array. The `jsoncons` implementation takes that alternative and returns an empty array in case of no match.

Unlike XML, the root of a JSON text is usually an anonymous object or array, so JSONPath identifies the outermost level of the text with the symbol $.

JSONPath expressions can use the dot–notation

    $.store.book.0.title

or the bracket–notation 


    $[store][book][0][title]

or 

    $['store']['book'][0]['title']

or

    $["store"]["book"][0]["title"]


Note that Stefan Goessner's original implementation supports unquoted or single quoted names inside of square brackets, the jsoncons implementation in addition supports double quoted names.

JSONPath|       Description
--------|--------------------------------
`$`|    The root object or array
`@`|    the current object
`.` or `[]`|    Child operator
`..`    |Recursive descent. JSONPath borrows this syntax from [E4X](https://en.wikipedia.org/wiki/ECMAScript_for_XML).
`*` |   Wildcard. All objects/elements regardless their names.
`[]`    |Subscript operator. In JSON it is the native array operator.
`[,]`   |JSONPath allows alternate names or array indices as a set.
`[start:end:step]`      |Array slice operator borrowed from [ES4](http://wiki.ecmascript.org/doku.php?id=proposals:slice_syntax&s=array+slice).
`()`    |Filter expression.
`?()`   |Applies a filter expression.

### jsoncons filter expressions

[Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/) does not provide any specification for the allowable filter expressions, simply stating that expressions can be anything that the underlying script engine can handle. `jsoncons` expressions support the following comparision and arithmetic operators. 

Operator|       Description
--------|--------------------------------
`*`     |Left times right
`/`     |Left divided by right
`+`     |Left plus right
`-`     |Left minus right
`&&`    |Left and right
<code>&#124;&#124;</code>    |Left or right
`==`    |Left is equal to right 
`!=`    |Left is not equal to right
`<`     |Left is less than right
`<=`    |Left is less or equal to right
`>`     |Left is greater than right
`>=`    |Left is greater than or equal to right
'=~'    |Left matches regular expression [?(@.author =~ /Evelyn.*?/)]

Unary operators

Operator|       Description
--------|--------------------------------
`!`     |Not right
`-`     |Negates right

Operator precedence

Precedence|Operator|Associativity
----------|--------|-----------
1 |`!` unary `-`    |Right
2 |`=~`             |Left
3 |`*` `/`          |Left 
4 |`+` `-`          |Left 
5 |`<` `>` `<=` `>=`|Left 
6 |`==` `!=`        |Left 
7 |`&&`             |Left 
8 |<code>&#124;&#124;</code>             |Left 

#### Aggregate Functions

Functions can be called inside filter expressions. The input to an aggregate function is the value of a JSONPath expression.

Function|Description|Result|Example
----------|--------|-------|---
min()|Provides the minimum value of an array of numbers|double|$.store.book[?(@.price < max($.store.book[*].price))].title
max()|Provides the maximum value of an array of numbers|double|$.store.book[?(@.price > min($.store.book[*].price))].title

### Examples

The examples below use the JSON text from [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/) (booklist.json).

```json
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
```

JSONPath |Result|Notes
---------|--------------------------------------------------------|------
`$..book.length`        |The number of books|`length` is a property of an array
`$.store.*`                 |Everything in the store, including books and a bicycle.|`*` is a wildcard symbol
`$..author`                 |All authors|Recursive descent starting from root ($) to find all authors
`$.store.book[*].author`        |All authors of books in the store|
`$.store..price`                |The prices of everything in the store.|
`$..book[2]`                |The third book|Indices start at `0`
`$..book.2`                 |The third book|Using the dot notation
`$..book[(@.length-1)]`         |The last book in order.|Expressions (<expr>) can be used as an alternative to explicit names or indices
`$..book[-1:]`          |The last book in order.|A negative `start` becomes `start` + `length`. A missing `end` defaults to `length`.
`$..book[0,1]`            |The first two books
`$..book[:2]`               |All books from index 0 (inclusive) to index 2 (exclusive)|`start` defaults to 0
`$..book[1:2]`              |All books from index 1 (inclusive) to index 2 (exclusive)
`$..book[-2:]`  |Last two books|The start index `-2` becomes `-2+4=2`, `end` defaults to `length` (`4`).
`$..book[2:]`   |Last two books|`end` defaults to `length`
`$..book[?(@.isbn)]`        |All books that have isbn number
`$..book[?(@.price<10)]`        |All books that are cheaper than $10
`$..*`                  |Everything in the store.
`$.store.book[ ?((@.author == 'Nigel Rees')` || (@.author == 'Evelyn Waugh')) ].title`|The titles of all books by Nigel Rees and Evelyn Waugh
`$.store.book[?(@.author =~ /Evelyn.*?/)]`|All books whose author's name starts with Evelyn
`$.store.book[?((@.author =~ /evelyn.*?/i))]`|All books whose author's name starts with Evelyn, evelyn etc.|`i` indicates case insensitive
`$.store.book[?(!(@.author =~ /Evelyn.*?/))]`|All books whose author's name does not start with Evelyn
`$['store']['book']..['author','title']`|All authors and titles of books in the store
`$.store.book[?(@.price < max($.store.book[*].price))].title`|The titles of all books that are priced less than the most expensive book in the store
`$.store.book[?(@.price > min($.store.book[*].price))].title`|The titles of all books that are priced higher than the cheapest book in the store

