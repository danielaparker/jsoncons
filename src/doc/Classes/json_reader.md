    jsoncons::json_reader

    typedef basic_json_reader<char> json_reader

### Header

    #include "jsoncons/json_reader.hpp"

### Constructors

    json_reader(std::istream& is,
                json_input_handler& handler,
                parse_error_handler& err_handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives
JSON events, and the specified [parse_error_handler](parse_error_handler).
You must ensure that the input stream, input handler, and error handler exist as long as does `json_reader`, as `json_reader` holds pointers to but does not own these objects.

    json_reader(std::istream& is,
                json_input_handler& handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives
JSON events, and a [default_error_handler](default_error_handler).
You must ensure that the input stream and input handler exist as long as does `json_reader`, as `json_reader` holds pointers to does not own these objects.

### Member functions

    bool eof() const
Returns `true` when there is no more data to be read from the stream, `false` otherwise

    void read()
Reports JSON events to a [json_input_handler](json_input_handler), such as a [json_deserializer](json_deserializer).

    size_t buffer_capacity() const

    void buffer_capacity(size_t buffer_capacity)

