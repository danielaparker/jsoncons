    jsoncons::json_reader

### Header

    #include "jsoncons/json_reader.hpp"

### Constructors

    json_reader(json_input_handler& handler,
                error_handler& err_handler)
Constructs a `json_reader` that is associated with a [[json_input_handler]] handler that receives
notification of JSON events, and the specified [[error_handler]].

    json_reader(json_input_handler& handler)
Constructs a `json_reader` that is associated with a [[json_input_handler]] handler that receives
notification of JSON events, and a [[default_error_handler]]

### Member functions

    bool eof() const
Returns `true` when there is no more data to be read from the stream, `false` otherwise

    void read(std::istream& is)
Parses an input stream `is` of JSON text and reports JSON events to a [[json_input_handler]] handler, such as a [[json_deserializer]].

    size_t buffer_capacity() const

    void buffer_capacity(size_t buffer_capacity)

### Deprecated

    json_reader(std::istream& is,
                json_input_handler& handler,
                error_handler& err_handler)
Use the constructor that does not take `is` and pass `is` to `read` instead.

    json_reader(std::istream& is,
                json_input_handler& handler)
Use the constructor that does not take `is` and pass `is` to `read` instead.

    void read()
Use the constructor that does not take `is` and use the `read` method that does.

