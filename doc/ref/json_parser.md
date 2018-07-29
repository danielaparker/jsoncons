### jsoncons::json_parser

```c++
typedef basic_json_parser<char> json_parser
```
`json_parser` is an incremental json parser. It can be fed its input in chunks.

`json_parser` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_parser.hpp>
```
#### Constructors

    json_parser(); // (1)

    json_parser(const json_serializing_options& options); // (2)

    json_parser(parse_error_handler& err_handler); // (3)

    json_parser(const json_serializing_options& options, 
                parse_error_handler& err_handler); // (4)

    json_parser(json_content_handler& handler); // (5)

    json_parser(json_content_handler& handler,
                parse_error_handler& err_handler); // (6)

    json_parser(json_content_handler& handler,
                const json_serializing_options& options); // (7)

    json_parser(json_content_handler& handler, 
                const json_serializing_options& options,
                parse_error_handler& err_handler); // (8)

(4) Constructs a `json_parser` that is associated with a [json_content_handler](json_content_handler.md) that receives JSON events and a [default_parse_error_handler](default_parse_error_handler.md).
You must ensure that the input stream and input handler exist as long as does `json_parser`, as `json_parser` holds pointers to does not own these objects.

(6) Constructs a `json_parser` that is associated with a [json_content_handler](json_content_handler.md) that receives JSON events and the specified [parse_error_handler](parse_error_handler.md).
You must ensure that the input stream, input handler, and error handler exist as long as does `json_parser`, as `json_parser` holds pointers to but does not own these objects.

#### Member functions

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
Sets a `std::error_code` if parsing fails.

    void end_parse()
Called after there is no more input
Throws [parse_error](parse_error.md) if parsing fails.

    void end_parse(std::error_code& ec)
Called after there is no more input
Sets a `std::error_code` if parsing fails.

    void skip_bom()
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Throws [parse_error](parse_error.md) if parsing fails.

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.
Throws [parse_error](parse_error.md) if parsing fails.

    void check_done(std::error_code& ec)
Sets a `std::error_code` if there are any unconsumed non-whitespace characters in the input.

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
        parser.update("10",2);
        parser.parse_some();
        std::cout << "(1) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.update(".5",2);
        parser.parse_some();
        std::cout << "(2) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        parser.end_parse();
        std::cout << "(3) done: " << std::boolalpha << parser.done() << ", source_exhausted: " << parser.source_exhausted() << "\n\n";

        json j = decoder.get_result();
        std::cout << "(4) " << j << "\n";
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

(4) 10.5
```
