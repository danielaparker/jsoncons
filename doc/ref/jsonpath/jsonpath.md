### jsonpath extension

The jsonpath extension implements [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/).  

It provides functions for search and "search and replace" using JSONPath expressions:

[json_query](json_query.md)

[json_replace](json_replace.md)

The [Jayway JSONPath Evaluator](https://jsonpath.herokuapp.com/) and [JSONPATH Expression Tester](https://jsonpath.curiousconcept.com/)
are good online evaluators for checking JSONPath expressions.
    
### Stefan Goessner's JSONPath

[JSONPath](http://goessner.net/articles/JsonPath/) is a creation of Stefan Goessner. JSONPath expressions refer to a JSON text in the same way as XPath expressions refer to an XML document. 

#### Paths

JSONPath uses paths to select a set of nodes in a JSON value. Paths can use the dot-notation

    $.store.book.0.title

or the bracket-notation 


    $[store][book][0][title]

or 

    $['store']['book'][0]['title']

or

    $["store"]["book"][0]["title"]

The leading `$` represents the root JSON value.

Stefan Goessner's original implementation supports unquoted or single quoted names inside of square brackets. Support for double quoted names inside of square brackets is a jsoncons extension.

JSONPath|       Description
--------|--------------------------------
`$`|    Represents the root JSON value
`@`|    Represents the current node being processed by a filter predicate.
`.` or `[]`|    Child operator
`..`    |Recursive descent. JSONPath borrows this syntax from [E4X](https://en.wikipedia.org/wiki/ECMAScript_for_XML).
`*` |   Wildcard. All objects/elements regardless their names.
`[]`    |Subscript operator. In JSON it is the native array operator.
`[,]`   |JSONPath allows alternate names or array indices as a set.
`[start:end:step]`      |Array slice operator borrowed from [ES4](http://wiki.ecmascript.org/doku.php?id=proposals:slice_syntax&s=array+slice).
`()`    |Filter expression.
`?()`   |Applies a filter expression.

#### Filter predicates

JSONPath uses filter predicates to restrict the set of nodes returned by a path.

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
8 |<code>&#124;&#124;</code> |Left 

#### Functions

Support for functions is a jsoncons extension.

Functions can be passed JSONPath paths and JSON expressions. 
Outside a filter predicate, functions can be passed paths that select from
the root JSON value `$`. Within a filter predicate, functions can be passed either a 
path that selects from the root JSON value `$`, or a path that selects from the current node `@`.

Function|Description|Result|Example
----------|--------|-------|---
`max(array)`|Returns the maximum value of an array of numbers|`double`|`max($.store.book[*].price)`
`min(array)`|Returns the minimum value of an array of numbers|`double`|`min($.store.book[*].price)`
`count(array)`|Returns the number of items in an array|`uint64_t`|`count($.store.book[*])`
`sum(array)`|Returns the sum value of an array of numbers|`double`|`$.store.book[?(@.price > sum($.store.book[*].price) / count($.store.book[*]))].title`
`avg(array)`|Returns the arithmetic average of each item of an array of numbers. If the input is an empty array, returns `null`.|`double`|`$.store.book[?(@.price > avg($.store.book[*].price))].title`
`prod(array)`|Returns the product of the elements in an array of numbers.|`double`|`$.store.book[?(479373 < prod($..price) && prod($..price) < 479374)].title`
`keys(object)`|Returns an array of keys.|`array of string`|`keys($.store.book[0])[*]`
`tokenize(string,pattern)`|Returns an array of strings formed by splitting the input string into an array of strings, separated by substrings that match the regular expression `pattern`.|`array of string`|`$.store.book[?(tokenize(@.author,'\\s+')[1] == 'Waugh')].title`

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

