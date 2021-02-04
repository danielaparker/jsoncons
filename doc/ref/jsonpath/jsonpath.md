### jsonpath extension

The jsonpath extension implements [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/).
It provides functions for search and "search and replace" using JSONPath expressions.

Version 0.161.0 is a significant rewrite of the jsonpath extension. The functions `json_query` and
`json_replace`, however, are compatible with earlier versions.

### Classes
<table border="0">
  <tr>
    <td><a href="jsonpath_expression.md">jsonpath_expression</a></td>
    <td>Represents the compiled form of a JSONPath expression. (since 0.161.0)</td> 
  </tr>
</table>

### Functions

<table border="0">
  <tr>
    <td><a href="make_expression.md">make_expression</a></td>
    <td>Returns a compiled JSONPath expression for later evaluation. (since 0.161.0)</td> 
  </tr>
  <tr>
    <td><a href="json_query.md">json_query</a></td>
    <td>Searches for all values that match a JSONPath expression</td> 
  </tr>
  <tr>
    <td><a href="json_replace.md">json_replace</a></td>
    <td>Search and replace using JSONPath expressions.</td> 
  </tr>
  <tr>
    <td><a href="flatten.md">flatten<br>unflatten</a></td>
    <td>Flattens a json object or array.</td> 
  </tr>
</table>
    
### Stefan Goessner's JSONPath

[JSONPath](http://goessner.net/articles/JsonPath/), a loosely standardized syntax for querying JSON, is a creation of Stefan Goessner. 
JSONPath expressions refer to a JSON text in the same way as XPath expressions refer to an XML document. 

The [Jayway JSONPath Evaluator](https://jsonpath.herokuapp.com/) and [JSONPATH Expression Tester](https://jsonpath.curiousconcept.com/)
are good online evaluators for checking JSONPath expressions.

### jsoncons JSONPath

Go to [JSONPath Comparison](https://cburgmer.github.io/json-path-comparison/) to see how jsoncons JsonPath compares with other implementations.

#### Differences with Stefan Goessner's implementation

- Stefan Goessner's implemention returns `false` in case of no match, but in a note he suggests an alternative is to return an empty array. 
  The `jsoncons` implementation takes that alternative and returns an empty array in case of no match.
- Names in both the dot notation and the bracket notation may be unquoted (no spaces), single-quoted, or double-quoted.
- Wildcards are allowed in the dot notation
- Unions produce real unions with no duplicates instead of concatenated results
- Union of completely separate paths are allowed, e.g.

    $..[@.firstName,@.address.city]

#### Paths

JSONPath uses paths to select a set of nodes in a JSON value. Paths can use the dot-notation or the bracket-notation.

Select the first (indexed 0) book using the dot notation:

    $.books.0

or

    $.'books'.0

or

    $."books".0

The leading `$` represents the root JSON value. The jsoncons implementation
allows single and double quoted as well as
unquoted names. Unquoted names must not contain spaces, and spaces before
and after the name are ignored. 

Select the first (indexed 0) book using the bracket-notation: 

    $['store']['book'][0]

or

    $["store"]["book"][0]

Recursively select all book titles under `'$.store'`:

    $.'store'..'title'

Union of a subset of books identified by index:

    $.store[book[0],book[1],book[3]]

Union of the fourth book and all books with price > 10:

    $.store[book[3],book[?(@.price > 10)]]

JSONPath|       Description
--------|--------------------------------
`$`|    Represents the root JSON value
`@`|    Represents the current node in an expression.
`.` or `[]`|    Child operator
`..`    |Recursive descent. JSONPath borrows this syntax from [E4X](https://en.wikipedia.org/wiki/ECMAScript_for_XML).
`*` |   Wildcard. All objects/elements regardless their names.
`[]`    |Subscript operator. 
`[,]`   |Union.
`[start:end:step]`      |Array slice operator borrowed from ECMASCRIPT 4.
`()`    |Filter expression.
`?()`   |Applies a filter expression.

#### Slices

jsoncons jsonpath slices have the same semantics as Python slices
(including for negative steps since v0.153.3)

The syntax for a slice is
```
[start:stop:step]
```
Each component is optional.

- If `start` is omitted, it defaults to `0` if `step` is positive,
or the end of the array if `step` is negative.

- If `stop` is omitted, it defaults to the length of the array if `step` 
is positive, or the beginning of the array if `step` is negative.

- If `step` is omitted, it defaults to `1`.

Slice expression|       Description
--------|--------------------------------
`[start:stop]`  | Items `start` through `stop-1`
`[start:]`      | Items `start` to the end of the array
`[:stop]`       | Items from the beginning of the array through `stop-1`
`[:]`           | All items
`[start:stop:step]`|Items `start` up to but not including `stop`, by `step` 

A component `start`, `stop`, or `step` may be a negative number.

Example | Description
--------|------------
$[-1]    | Last item 
$[-2:]   | Last two items
$[:-2]   | All items except the last two
$[::-1]    | All items, reversed
$[1::-1]   | First two items, reversed
$[:-3:-1]  | Last two items, reversed
$[-3::-1]  | All items except the last two, reversed

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

The examples use the sample data file `books.json`, 

```json
{
    "books":
    [
        {
            "category": "fiction",
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "category": "fiction",
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "category": "fiction",
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "category": "memoir",
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
```

#### json_query function

```
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    auto result1 = jsonpath::json_query(data, "$.books[1,1,3].title");
    std::cout << "(1)\n" << pretty_print(result1) << "\n\n";

    auto result2 = jsonpath::json_query(data, "$.books[1,1,3].title",
                                        jsonpath::result_options::path);
    std::cout << "(2)\n" << pretty_print(result2) << "\n\n";

    auto result3 = jsonpath::json_query(data, "$.books[1,1,3].title",
                                        jsonpath::result_options::value | jsonpath::result_options::no_dups);
    std::cout << "(3)\n" << pretty_print(result3) << "\n\n";

    auto result4 = jsonpath::json_query(data, "$.books[1,1,3].title",
                                        jsonpath::result_options::path | jsonpath::result_options::no_dups);
    std::cout << "(4)\n" << pretty_print(result4) << "\n\n";
}
```
Output:
```
(1)
[
    "The Night Watch",
    "The Night Watch",
    "The Night Watch"
]

(2)
[
    "$['books'][1]['title']",
    "$['books'][1]['title']",
    "$['books'][3]['title']"
]

(3)
[
    "The Night Watch",
    "The Night Watch"
]

(4)
[
    "$['books'][1]['title']",
    "$['books'][3]['title']"
]
```

#### json_replace function

```
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    auto f = [](const json& price) {return std::round(price.as<double>() - 1.0);};
    jsonpath::json_replace(data, "$.books[*].price", f);

    std::cout << pretty_print(data) << "\n";
}
```
Output:
```
{
    "books": [
        {
            "author": "Haruki Murakami",
            "category": "fiction",
            "price": 22.0,
            "title": "A Wild Sheep Chase"
        },
        {
            "author": "Sergei Lukyanenko",
            "category": "fiction",
            "price": 23.0,
            "title": "The Night Watch"
        },
        {
            "author": "Graham Greene",
            "category": "fiction",
            "price": 21.0,
            "title": "The Comedians"
        },
        {
            "author": "Phillips, David Atlee",
            "category": "memoir",
            "title": "The Night Watch"
        }
    ]
}
```

#### jsonpath_expression function

A [jsoncons::jsonpath::jsonpath_expression](jsonpath_expression.md) 
represents the compiled form of a JMESPath expression. It allows you to 
evaluate a single compiled expression on multiple JSON documents.
A `jsonpath_expression` is immutable and thread-safe. 

```
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

int main()
{
    auto expr = jsonpath::make_expression<json>("$.books[1,1,3].title");

    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    json result1 = expr.evaluate(data);
    std::cout << "(1) " << pretty_print(result1) << "\n\n";

    json result2 = expr.evaluate(data, jsonpath::result_options::path);
    std::cout << "(2) " << pretty_print(result2) << "\n\n";

    json result3 = expr.evaluate(data, jsonpath::result_options::value | jsonpath::result_options::no_dups);
    std::cout << "(3) " << pretty_print(result3) << "\n\n";

    json result4 = expr.evaluate(data, jsonpath::result_options::path | jsonpath::result_options::no_dups);
    std::cout << "(4) " << pretty_print(result4) << "\n\n";
}
```
Output:
```
(1) [
    "The Night Watch",
    "The Night Watch",
    "The Night Watch"
]

(2) [
    "$['books'][1]['title']",
    "$['books'][1]['title']",
    "$['books'][3]['title']"
]

(3) [
    "The Night Watch",
    "The Night Watch"
]

(4) [
    "$['books'][1]['title']",
    "$['books'][3]['title']"
]
```

