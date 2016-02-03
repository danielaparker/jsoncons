    jsoncons::json_reader

    typedef basic_json_reader<char> json_reader

A `json_reader` can read a sequence of JSON texts from a stream.

### Header

    #include "jsoncons/json_reader.hpp"

### Constructors

    json_reader(std::istream& is,
                json_input_handler& handler,
                parse_error_handler& err_handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives JSON events, and the specified [parse_error_handler](parse_error_handler).
You must ensure that the input stream, input handler, and error handler exist as long as does `json_reader`, as `json_reader` holds pointers to but does not own these objects.

    json_reader(std::istream& is,
                json_input_handler& handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives JSON events, and a [default_parse_error_handler](default_parse_error_handler).
You must ensure that the input stream and input handler exist as long as does `json_reader`, as `json_reader` holds pointers to does not own these objects.

### Member functions

    bool eof() const
Returns `true` when there are no more JSON texts to be read from the stream, `false` otherwise

    void read_next()
Reads the next JSON text from the stream and reports JSON events to a [json_input_handler](json_input_handler), such as a [json_deserializer](json_deserializer).
Throws [parse_exception](parse_exception) if parsing fails.

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.
Throws [parse_exception](parse_exception) if parsing fails.

    size_t buffer_capacity() const

    void buffer_capacity(size_t capacity)

    size_t max_nesting_depth() const
By default `jsoncons` can read a `JSON` text of arbitrarily large depth.

    void max_nesting_depth(size_t depth)

### Deprecated methods

    void read()
Use `read_next` instead. 

    size_t max_depth() const
Use `max_nesting_depth()` instead

    void max_depth(size_t depth)
Use `max_nesting_depth(size_t depth)` instead

## Examples


### Reading a sequence of JSON texts from a stream

`jsoncons` supports reading a sequence of JSON texts, such as shown below (`json-texts.json`):

    {"a":1,"b":2,"c":3}
    {"a":4,"b":5,"c":6}
    {"a":7,"b":8,"c":9}

This is the code that reads them: 

    std::ifstream is("json-texts.json");
    if (!is.is_open())
    {
        throw std::runtime_error("Cannot open file");
    }

    json_deserializer handler;
    json_reader reader(is,handler);

    while (!reader.eof())
    {
        reader.read_next();
        if (!reader.eof())
        {
            json val = handler.get_result();
            std::cout << val << std::endl;
        }
    }

The output is

    {"a":1,"b":2,"c":3}
    {"a":4,"b":5,"c":6}
    {"a":7,"b":8,"c":9}

