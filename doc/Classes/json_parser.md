```c++
jsoncons::json_parser

typedef basic_json_parser<char> json_parser
```
`json_parser` is an incremental json parser.

`json_parser` is noncopyable and nonmoveable.

### Header
```c++
#include <jsoncons/json_parser.hpp>
```
### Constructors

    json_parser(std::istream& is,
                json_input_handler& handler,
                parse_error_handler& err_handler)
Constructs a `json_parser` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler.md) that receives JSON events, and the specified [parse_error_handler](parse_error_handler.md).
You must ensure that the input stream, input handler, and error handler exist as long as does `json_parser`, as `json_parser` holds pointers to but does not own these objects.

    json_parser(std::istream& is,
                json_input_handler& handler)
Constructs a `json_parser` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler.md) that receives JSON events, and a [default_parse_error_handler](default_parse_error_handler.md).
You must ensure that the input stream and input handler exist as long as does `json_parser`, as `json_parser` holds pointers to does not own these objects.

### Member functions

    bool done() const
Returns `true` when the parser has consumed a complete json text, `false` otherwise

    bool source_exhausted() const
Returns `true` if the input in the source buffer has been exhausted, `false` otherwise

    void parse()
Parses the source until a complete json text has been consumed or the source has been exhausted.
Throws [parse_error](parse_error.md) if parsing fails.

    void parse(std::error_code& ec)
Parses the source until a complete json text has been consumed or the source has been exhausted.
Sets a `std::error_code` if parsing fails.

    void skip_bom()
Reads the next JSON text from the stream and reports JSON events to a [json_input_handler](json_input_handler.md), such as a [json_decoder](json_decoder.md).
Throws [parse_error](parse_error.md) if parsing fails.

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.
Throws [parse_error](parse_error.md) if parsing fails.

    size_t max_nesting_depth() const
By default `jsoncons` can read a `JSON` text of arbitrarily large depth.

    void max_nesting_depth(size_t depth)

## Examples


### Reading a sequence of JSON texts from a stream

`jsoncons` supports reading a sequence of JSON texts, such as shown below (`json-texts.json`):
```json
{"a":1,"b":2,"c":3}
{"a":4,"b":5,"c":6}
{"a":7,"b":8,"c":9}
```
This is the code that reads them: 
```c++
std::ifstream is("json-texts.json");
if (!is.is_open())
{
    throw std::runtime_error("Cannot open file");
}

json_decoder<json> decoder;
json_parser reader(is,decoder);

while (!reader.eof())
{
    reader.read_next();
    if (!reader.eof())
    {
        json val = decoder.get_result();
        std::cout << val << std::endl;
    }
}
```
Output:
```json
{"a":1,"b":2,"c":3}
{"a":4,"b":5,"c":6}
{"a":7,"b":8,"c":9}
```
