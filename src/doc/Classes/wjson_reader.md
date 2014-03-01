    jsoncons::wjson_reader

### Header

    #include "jsoncons/json_reader.hpp"

### Constructors

    wjson_reader(std::wistream& is,
                json_input_handler& handler,
                error_handler& err_handler)
Constructs a `wjson_reader` that is associated with an input stream
`is` of JSON text, a [wjson_input_handler](basic_json_input_handler) handler that receives
notification of JSON events, and the specified [[error_handler]].

    wjson_reader(std::wistream& is,
                json_input_handler& handler)
Constructs a `wjson_reader` that is associated with an input stream
`is` of JSON text, a [wjson_input_handler](basic_json_input_handler) handler that receives
notification of JSON events, and a [[default_error_handler]]

### Member functions

    bool eof() const
Returns `true` when there is no more data to be read from the stream, `false` otherwise

    void read()
Reports JSON events to a [wjson_input_handler](basic_json_input_handler) handler, such as a [[json_deserializer]].

    size_t buffer_capacity() const

    void buffer_capacity(size_t buffer_capacity)

