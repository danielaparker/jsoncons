### jsoncons::json_content_handler

```c++
typedef basic_json_content_handler<char> json_content_handler
```

Defines an interface for receiving JSON events. The `json_content_handler` class is an instantiation of the `basic_json_content_handler` class template that uses `char` as the character type. 

#### Header
```c++
#include <jsoncons/json_content_handler.hpp>
```
#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  

#### Public producer interface

    bool begin_object()
    bool begin_object(const serializing_context& context)
Indicates the begining of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool end_object()
    bool end_object(const serializing_context& context)
Indicates the end of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool begin_array()
    bool begin_array(const serializing_context& context)
Indicates the beginning of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool begin_array(size_t length)
    bool begin_array(size_t length, const serializing_context& context)
Indicates the beginning of an array of known length. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool end_array()
    bool end_array(const serializing_context& context)
Indicates the end of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool name(const string_view_type& name)
    bool name(const string_view_type& name, const serializing_context& context)
Writes the name part of a name-value pair inside an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.  
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool string_value(const string_view_type& value) 
    bool string_value(const string_view_type& value, const serializing_context& context) 
Writes a string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool byte_string_value(const uint8_t* data, size_t length) 
    bool byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) 
Writes a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool byte_string_value(const std::vector<uint8_t>& v) 
    bool byte_string_value(const std::vector<uint8_t>& v, const serializing_context& context) 
Writes a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool bignum_value(int signum, const uint8_t* data, size_t length) 
    bool bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) 
Writes a bignum using the sign-magnitude representation. 
The magnitude is an unsigned integer `n` encoded as a byte string data item in big-endian byte-order.
If the value of signum is 1, the value of the bignum is `n`. 
If the value of signum is -1, the value of the bignum is `-1 - n`. 
An empty list means a zero value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool bignum_value(const string_view_type& s) 
    bool bignum_value(const string_view_type& s, const serializing_context& context) 
Writes a bignum using the decimal string representation of a bignum. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool int64_value(int64_t value) 
    bool int64_value(int64_t value, const serializing_context& context)
Writes a signed integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool uint64_value(uint64_t value) 
    bool uint64_value(uint64_t value, const serializing_context& context)
Writes a non-negative integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool double_value(double value) 
    bool double_value(double value, const serializing_context& context)
Writes a floating point value with default precision (`std::numeric_limits<double>::digits10`.) Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool double_value(double value, uint8_t precision) 
    bool double_value(double value, uint8_t precision, const serializing_context& context)
Writes a floating point value with specified precision. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool double_value(double value, const floating_point_options& fmt) 
    bool double_value(double value, const floating_point_options& fmt, const serializing_context& context)
Writes a floating point value with specified precision. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool bool_value(bool value) 
    bool bool_value(bool value, const serializing_context& context) 
Writes a boolean value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool null_value() 
    bool null_value(const serializing_context& context) 
Writes a null value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    void flush()
Flushes whatever is buffered to the destination.

#### Private virtual consumer interface

    virtual bool do_begin_object(const serializing_context& context) = 0;
Handles the beginning of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_end_object(const serializing_context& context) = 0;
Handles the end of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_begin_array(const serializing_context& context) = 0;
Handles the beginning of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_begin_array(size_t length, const serializing_context& context);
Handles the beginning of an array of known length. Defaults to calling `do_begin_array(const serializing_context& context)`. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_end_array(const serializing_context& context) = 0;
Handles the end of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_name(const string_view_type& name, 
                         const serializing_context& context) = 0;
Handles the name part of a name-value pair inside an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.  
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_string_value(const string_view_type& val, 
                           const serializing_context& context) = 0;
Handles a string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) = 0;
Handles a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_bignum_value(const string_view_type& value, const serializing_context& context) = 0;
Handles a bignum using the sign-magnitude representation. 
The magnitude is an unsigned integer `n` encoded as a byte string data item in big-endian byte-order.
If the value of signum is 1, the value of the bignum is `n`. 
If the value of signum is -1, the value of the bignum is `-1 - n`. 
An empty list means a zero value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_int64_value(int64_t value, const serializing_context& context) = 0;
Handles a signed integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_uint64_value(uint64_t value, 
                             const serializing_context& context) = 0;
Handles a non-negative integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_double_value(double value, const floating_point_options& fmt, const serializing_context& context) = 0;
Handles a floating point value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_bool(bool value, const serializing_context& context) = 0;
Handles a boolean value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_null_value(const serializing_context& context) = 0;
Handles a null value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual void do_flush() = 0;
Flushes whatever is buffered to the destination.

