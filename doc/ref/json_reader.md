### jsoncons::json_reader

```c++
typedef basic_json_reader<char> json_reader
```
[json_reader](json_reader.md) uses the incremental parser [json_parser](json_parser.md) 
to read arbitrarily large files in chunks.
A `json_reader` can read a sequence of JSON texts from a stream, using `read_next()`,
which omits the check for unconsumed non-whitespace characters. 

`json_reader` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_reader.hpp>
```
#### Constructors

    json_reader(std::istream& is); // (1)

    json_reader(std::istream& is, 
                const json_read_options& options); // (2)

    json_reader(std::istream& is, 
                parse_error_handler& err_handler); // (3)

    json_reader(std::istream& is, 
                const json_read_options& options,
                parse_error_handler& err_handler); // (4)

    json_reader(std::istream& is, 
                basic_json_content_handler<CharT>& handler); // (5)

    json_reader(std::istream& is, 
                json_content_handler& handler,
                const json_read_options& options); // (6)

    json_reader(std::istream& is,
                json_content_handler& handler,
                parse_error_handler& err_handler); // (7)

    json_reader(std::istream& is,
                json_content_handler& handler, 
                const json_read_options& options,
                parse_error_handler& err_handler); // (8)


Constructors (1)-(4) use a default [json_content_handler](json_content_handler.md) that discards the JSON parse events, and are for validation only.

(1) Constructs a `json_reader` that reads from an input stream `is` of 
JSON text, uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(4) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Constructors (5)-(8) take a user supplied [json_content_handler](json_content_handler.md) that receives JSON parse events, such as a [json_decoder](json_decoder). 

(5) Constructs a `json_reader` that reads JSON text from an input stream `is`,
emits JSON parse events to the specified 
[json_content_handler](json_content_handler.md), and uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(6) Constructs a `json_reader` that reads JSON text from an input stream `is`,
emits JSON parse events to the specified [json_content_handler](json_content_handler.md) 
and uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(7) Constructs a `json_reader` that reads JSON text from an input stream `is`,
emits JSON parse events to the specified [json_content_handler](json_content_handler.md) 
and uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(8) Constructs a `json_reader` that reads JSON text from an input stream `is`,
emits JSON parse events to the specified [json_content_handler](json_content_handler.md) and
uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Note: It is the programmer's responsibility to ensure that `json_reader` does not outlive any input stream, content handler, and error handler passed in the constuctor.

#### Member functions

    bool eof() const
Returns `true` when there are no more JSON texts to be read from the stream, `false` otherwise

    void read()
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Throws if there are any unconsumed non-whitespace characters left in the input.
Throws [parse_error](parse_error.md) if parsing fails.

    void read(std::error_code& ec)
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if parsing fails or if there are any unconsumed non-whitespace characters left in the input.

    void read_next()
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Throws [parse_error](parse_error.md) if parsing fails.

    void read_next(std::error_code& ec)
Reads the next JSON text from the stream and reports JSON events to a [json_content_handler](json_content_handler.md), such as a [json_decoder](json_decoder.md).
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if parsing fails.

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.
if there are any unconsumed non-whitespace characters left in the input.

    void check_done(std::error_code& ec)
Throws if there are any unconsumed non-whitespace characters in the input.
Sets `ec` to a [json_parse_errc](jsoncons::json_parse_errc.md) if there are any unconsumed non-whitespace characters left in the input.

    size_t buffer_length() const

    void buffer_length(size_t length)

    size_t line_number() const

    size_t column_number() const

### Examples

#### Parsing JSON text with exceptions
```
std::string input = R"({"field1"{}})";    
std::istringstream is(input);

json_decoder<json> decoder;
json_reader reader(is,decoder);

try
{
    reader.read();
    json j = decoder.get_result();
}
catch (const parse_error& e)
{
    std::cout << e.what() << std::endl;
}

```
Output:
```
Expected name separator ':' at line 1 and column 10
```

#### Parsing JSON text with error codes
```
std::string input = R"({"field1":ru})";    
std::istringstream is(input);

json_decoder<json> decoder;
json_reader reader(is,decoder);

std::error_code ec;
reader.read(ec);

if (!ec)
{
    json j = decoder.get_result();   
}
else
{
    std::cerr << ec.message() 
              << " at line " << reader.line_number() 
              << " and column " << reader.column_number() << std::endl;
}
```
Output:
```
Expected value at line 1 and column 11
```

#### Reading a sequence of JSON texts from a stream

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
json_reader reader(is,decoder);

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
