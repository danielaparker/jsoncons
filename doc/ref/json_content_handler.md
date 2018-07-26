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

#### Public interface methods

    void begin_document()
Sends a notification of the beginning of a sequence of JSON events. 

    void end_document()
Sends a notification of the end of a sequence of JSON events. 

    void begin_object()
    void begin_object(const serializing_context& context)
Sends a notification of the begining of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.

    void end_object()
    void end_object(const serializing_context& context)
Sends a notification of the end of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void begin_array()
    void begin_array(const serializing_context& context)
Sends a notification of the beginning of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void begin_array(size_t length)
    void begin_array(size_t length, const serializing_context& context)
Sends a notification of the beginning of an array of known length. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void end_array()
    void end_array(const serializing_context& context)
Sends a notification of the end of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void name(const string_view_type& name)
    void name(const string_view_type& name, const serializing_context& context)
Sends the name part of a name-value pair inside an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.  

    void string_value(const string_view_type& value) 
    void string_value(const string_view_type& value, const serializing_context& context) 
Sends a string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void byte_string_value(const uint8_t* data, size_t length) 
    void byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) 
Sends a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void byte_string_value(const std::vector<uint8_t>& v) 
    void byte_string_value(const std::vector<uint8_t>& v, const serializing_context& context) 
Sends a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void bignum_value(int signum, const uint8_t* data, size_t length) 
    void bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) 
Sends a bignum using the sign-magnitude representation. 
The magnitude is an unsigned integer `n` encoded as a byte string data item in big-endian byte-order.
If the value of signum is 1, the value of the bignum is `n`. 
If the value of signum is -1, the value of the bignum is `-1 - n`. 
An empty list means a zero value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void bignum_value(const string_view_type& s) 
    void bignum_value(const string_view_type& s, const serializing_context& context) 
Sends a bignum using the decimal string representation of a bignum. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void integer_value(int64_t value) 
    void integer_value(int64_t value, const serializing_context& context)
Sends a signed integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void uinteger_value(uint64_t value) 
    void uinteger_value(uint64_t value, const serializing_context& context)
Sends a non-negative integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void double_value(double value) 
    void double_value(double value, const serializing_context& context)
Sends a floating point value with default precision (`std::numeric_limits<double>::digits10`.) Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void double_value(double value, uint8_t precision) 
    void double_value(double value, uint8_t precision, const serializing_context& context)
Sends a floating point value with specified precision. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void double_value(double value, const floating_point_options& fmt) 
    void double_value(double value, const floating_point_options& fmt, const serializing_context& context)
Sends a floating point value with specified precision. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void bool_value(bool value) 
    void bool_value(bool value, const serializing_context& context) 
Sends a boolean value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    void null_value() 
    void null_value(const serializing_context& context) 
Sends a null value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

#### Private virtual implementation methods

    virtual void do_begin_document() = 0;
Receives a notification of the beginning of a sequence of JSON events.

    virtual void do_end_document() = 0;
Receives a notification of the end of a sequence of JSON events.

    virtual void do_begin_object(const serializing_context& context) = 0;
Receives a notification of the beginning of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_end_object(const serializing_context& context) = 0;
Receives a notification of the end of an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_begin_array(const serializing_context& context) = 0;
Receives a notification of the beginning of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_begin_array(size_t length, const serializing_context& context);
Receives a notification of the beginning of an array of known length. Defaults to calling `do_begin_array(const serializing_context& context)`. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_end_array(const serializing_context& context) = 0;
Receives a notification of the end of an array. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_name(const string_view_type& name, 
                         const serializing_context& context) = 0;
Receives the name part of a name-value pair inside an object. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter.  

    virtual void do_string_value(const string_view_type& val, 
                                 const serializing_context& context) = 0;
Receives a string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) = 0;
Receives a byte string value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) = 0;
Receives a bignum using the sign-magnitude representation. 
The magnitude is an unsigned integer `n` encoded as a byte string data item in big-endian byte-order.
If the value of signum is 1, the value of the bignum is `n`. 
If the value of signum is -1, the value of the bignum is `-1 - n`. 
An empty list means a zero value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_integer_value(int64_t value, const serializing_context& context) = 0;
Receives a signed integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_uinteger_value(uint64_t value, 
                                   const serializing_context& context) = 0;
Receives a non-negative integer value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_double_value(double value, const floating_point_options& fmt, const serializing_context& context) = 0;
Receives a floating point value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_bool_value(bool value, const serializing_context& context) = 0;
Receives a boolean value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

    virtual void do_null_value(const serializing_context& context) = 0;
Receives a null value. Contextual information including
line and column information is provided in the [context](serializing_context.md) parameter. 

