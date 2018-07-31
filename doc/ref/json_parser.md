### jsoncons::json_parser

```c++
typedef basic_json_parser<char> json_parser
```
`json_parser` is an incremental json parser. It can be fed its input in chunks.  

 A buffer of text is supplied to the parser with a call to `update(buffer)`. 
 If a subsequent call to `parse_some` reaches the end of the buffer in the middle of parsing, 
 say after digesting the sequence 'f', 'a', 'l', member function `done()` will return `false`, 
 and `source_exhausted()` will return `true`. Additional text can then be supplied to the parser, 
 `parse_some` called again, and parsing will continue from where it left off. `end_done` should be called
 when there is no more input to feed to the parser. Once the parser has read a complete JSON text, 
 `done()` will return `true`. `check_done` can be called to check if the input has any unconsumed non-whitespace characters,
 which would normally be considered an error.  

`json_parser` is used by [json_reader](json_reader.md) to read from a stream in chunks.

 `json_parser` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_parser.hpp>
```
#### Constructors

    json_parser(); // (1)

    json_parser(const json_read_options& options); // (2)

    json_parser(parse_error_handler& err_handler); // (3)

    json_parser(const json_read_options& options, 
                parse_error_handler& err_handler); // (4)

    json_parser(json_content_handler& handler); // (5)

    json_parser(json_content_handler& handler,
                const json_read_options& options); // (6)

    json_parser(json_content_handler& handler,
                parse_error_handler& err_handler); // (7)

    json_parser(json_content_handler& handler, 
                const json_read_options& options,
                parse_error_handler& err_handler); // (8)

Constructors (1)-(4) use a default [json_content_handler](json_content_handler.md) that discards the JSON parse events, and are for validation only.

(1) Constructs a `json_parser` that uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `json_parser` that uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `json_parser` that uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(4) Constructs a `json_parser` that uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Constructors (5)-(8) take a user supplied [json_content_handler](json_content_handler.md) that receives JSON parse events, such as a [json_decoder](json_decoder). 

(5) Constructs a `json_parser` that emits JSON parse events to the specified 
[json_content_handler](json_content_handler.md), and uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(6) Constructs a `json_parser` that emits JSON parse events to the specified [json_content_handler](json_content_handler.md) 
and uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(7) Constructs a `json_parser` that emits JSON parse events to the specified [json_content_handler](json_content_handler.md) 
and uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(8) Constructs a `json_parser` that emits JSON parse events to the specified [json_content_handler](json_content_handler.md) and
uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Note: It is the programmer's responsibility to ensure that `json_reader` does not outlive any content handler and error handler passed in the constuctor.

#### Member functions

    void update(const string_view_type& sv)
    void update(const char* data, size_t length)
Update the parser with a chunk of JSON

    bool done() const
Returns `true` when the parser has consumed a complete json text, `false` otherwise

    bool source_exhausted() const
Returns `true` if the input in the source buffer has been exhausted, `false` otherwise

    void parse_some()
Parses the source until a complete json text has been consumed or the source has been exhausted.
Throws [parse_error](parse_error.md) if parsing fails.

    void parse_some(std::error_code& ec)
Parses the source until a complete json text has been consumed or the source has been exhausted.
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if parsing fails.

    void end_parse()
Called after there is no more input.
Throws [parse_error](parse_error.md) if parsing fails.

    void end_parse(std::error_code& ec)
Called after there is no more input.
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if parsing fails.

    void skip_bom()
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Throws [parse_error](parse_error.md) if parsing fails.

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.
Throws [parse_error](parse_error.md) if parsing fails.

    void check_done(std::error_code& ec)
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if parsing fails.

    size_t reset() const
Resets the state of the parser to it's initial state

### Examples

#### Incremental parsing

```c++
int main()
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);
    try
    {
        parser.update("10");
        parser.parse_some();
        std::cout << "(1) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update(".5");
        parser.parse_some(); // This is the end, but the parser can't tell
        std::cout << "(2) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.end_parse(); // Indicates that this is the end
        std::cout << "(3) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.check_done(); // Checks if there are any unconsumed 
                             // non-whitespace characters in the input
        std::cout << "(4) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        json j = decoder.get_result();
        std::cout << "(5) " << j << "\n";
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```

Output:

```
(1) done: false, source_exhausted: true

(2) done: false, source_exhausted: true

(3) done: true, source_exhausted: true

(4) done: true, source_exhausted: true

(5) 10.5
```

#### Incremental parsing with unconsumed non-whitespace characters

```c++
int main()
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder);
    try
    {
        parser.update("[10");
        parser.parse_some();
        std::cout << "(1) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update(".5]{}");
        parser.parse_some(); // The parser reaches the end at ']'
        std::cout << "(2) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.end_parse(); // Indicates that this is the end
        std::cout << "(3) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.check_done(); // Checks if there are any unconsumed 
                             // non-whitespace characters in the input
                             // (there are)
    }
    catch (const parse_error& e)
    {
        std::cout << "(4) " << e.what() << std::endl;
    }
}
```

Output:

```
(1) done: false, source_exhausted: true

(2) done: true, source_exhausted: false

(3) done: true, source_exhausted: false

(4) Unexpected non-whitespace character after JSON text at line 1 and column 7
```

#### nan, inf, and -inf substitition

```c++
int main()
{
    std::string s = R"(
        {
           "A" : "NaN",
           "B" : "Infinity",
           "C" : "-Infinity"
        }
    )";

    json_serializing_options options; // Implements json_read_options
    options.nan_replacement("\"NaN\"")
           .pos_inf_replacement("\"Infinity\"")
           .neg_inf_replacement("\"-Infinity\"");

    jsoncons::json_decoder<json> decoder;
    json_parser parser(decoder, options);
    try
    {
        parser.update(s);
        parser.parse_some();
        parser.end_parse();
        parser.check_done();
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    json j = decoder.get_result(); // performs move
    if (j["A"].is<double>())
    {
        std::cout << "A: " << j["A"].as<double>() << std::endl;
    }
    if (j["B"].is<double>())
    {
        std::cout << "B: " << j["B"].as<double>() << std::endl;
    }
    if (j["C"].is<double>())
    {
        std::cout << "C: " << j["C"].as<double>() << std::endl;
    }
}
```

Output:

```
A: nan
B: inf
C: -inf
```


