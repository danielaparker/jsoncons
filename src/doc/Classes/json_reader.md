    jsoncons::json_reader

    typedef basic_json_reader<char> json_reader

A `json_reader` can read a sequence of JSON texts from a stream.

### Header

    #include "jsoncons/json_reader.hpp"

### Constructors

    json_reader(std::istream& is,
                json_input_handler& handler,
                json_error_handler& err_handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives JSON events, and the specified [json_error_handler](json_error_handler).
You must ensure that the input stream, input handler, and error handler exist as long as does `json_reader`, as `json_reader` holds pointers to but does not own these objects.

    json_reader(std::istream& is,
                json_input_handler& handler)
Constructs a `json_reader` that is associated with an input stream `is` of JSON text, a [json_input_handler](json_input_handler) that receives JSON events, and a [default_error_handler](default_error_handler).
You must ensure that the input stream and input handler exist as long as does `json_reader`, as `json_reader` holds pointers to does not own these objects.

### Member functions

    bool eof() const
Returns `true` when there are no more JSON texts to be read from the stream, `false` otherwise

    void read_next()
Reads the next JSON text from the stream and reports JSON events to a [json_input_handler](json_input_handler), such as a [json_deserializer](json_deserializer).

    void check_done()
Throws if there are any unconsumed non-whitespace characters in the input.

    size_t buffer_capacity() const

    void buffer_capacity(size_t capacity)

    size_t max_depth() const
By default `jsoncons` can read a `JSON` text of arbitrarily large depth.

    void max_depth(size_t depth)

### Deprecated methods

    void read()
Use `read_next` instead. 

