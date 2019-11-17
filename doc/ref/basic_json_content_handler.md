### jsoncons::basic_json_content_handler

```c++
#include <jsoncons/json_content_handler.hpp>

template <
    class CharT
> class basic_json_content_handler
```

Defines an interface for producing and consuming JSON events. 

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
json_content_handler    |`basic_json_content_handler<char>`
wjson_content_handler   |`basic_json_content_handler<wchar_t>`

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|CharT
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [std::string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if C++ 17 is detected or if `JSONCONS_HAS_STRING_VIEW` is defined, otherwise proxied.  

#### Public event producer interface

    bool begin_object(semantic_tag tag=semantic_tag::none,
                      const ser_context& context=null_ser_context()); // (1)

    bool begin_object(size_t length, 
                      semantic_tag tag=semantic_tag::none, 
                      const ser_context& context = null_ser_context()); // (2)

    bool end_object(const ser_context& context = null_ser_context()); // (3)

    bool begin_array(semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (4)

    bool begin_array(size_t length, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (5)

    bool end_array(const ser_context& context=null_ser_context()); // (6)

    bool name(const string_view_type& name, 
              const ser_context& context=null_ser_context()); // (7)

    bool null_value(semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (8) 

    bool bool_value(bool value, 
                    semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (9) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (10) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context()); // (11)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (12)

    bool int64_value(int64_t value, 
                     semantic_tag tag = semantic_tag::none, 
                     const ser_context& context=null_ser_context()); // (13)

    bool half_value(uint16_t value, 
                    semantic_tag tag = semantic_tag::none, 
                    const ser_context& context=null_ser_context()); // (14)

    bool double_value(double value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (15)

    bool begin_object(semantic_tag tag,
                      const ser_context& context,
                      std::error_code& ec); // (16)

    bool begin_object(size_t length, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (17)

    bool end_object(const ser_context& context, 
                    std::error_code& ec); // (18)

    bool begin_array(semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (19)

    bool begin_array(size_t length, 
                     semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (20)

    bool end_array(const ser_context& context, 
                   std::error_code& ec); // (21)

    bool name(const string_view_type& name, 
              const ser_context& context, 
              std::error_code& ec); // (22)

    bool null_value(semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (23) 

    bool bool_value(bool value, 
                    semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (24) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (25) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec); // (26)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (27)

    bool int64_value(int64_t value, 
                     semantic_tag tag, 
                     const ser_context& context,
                     std::error_code& ec); // (28)

    bool half_value(uint16_t value, 
                    semantic_tag tag, 
                    const ser_context& context,
                    std::error_code& ec); // (29)

    bool double_value(double value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (30)

(1) Indicates the begining of an object of indefinite length.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(2) Indicates the begining of an object of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(3) Indicates the end of an object.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(4) Indicates the beginning of an indefinite length array. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(5) Indicates the beginning of an array of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(6) Indicates the end of an array.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(7) Writes the name part of an object name-value pair.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(8) Writes a null value. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(9) Writes a boolean value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(10) Writes a text string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(11) Writes a byte string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(12) Writes a non-negative integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(13) Writes a signed integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(14) Writes a half precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(15) Writes a double precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(16)-(30) Same as (1)-(15), except sets `ec` and returns `false` on parse errors.

    void flush()
Flushes whatever is buffered to the destination.

#### Private event consumer interface

    virtual bool do_begin_object(semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0; // (1)

    virtual bool do_begin_object(size_t length, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec); // (2)

    virtual bool do_end_object(const ser_context& context, 
                               std::error_code& ec) = 0; // (3)

    virtual bool do_begin_array(semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (4)

    virtual bool do_begin_array(size_t length, 
                                semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec); // (5)

    virtual bool do_end_array(const ser_context& context, 
                              std::error_code& ec) = 0; // (6)

    virtual bool do_name(const string_view_type& name, 
                         const ser_context& context, 
                         std::error_code&) = 0; // (7)

    virtual bool do_null_value(semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) = 0; // (8)

    virtual bool do_bool_value(bool value, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code&) = 0; // (9)

    virtual bool do_string_value(const string_view_type& value, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0; // (10)

    virtual bool do_byte_string_value(const byte_string_view& value, 
                                      semantic_tag tag, 
                                      const ser_context& context,
                                      std::error_code& ec) = 0; // (11)

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag tag, 
                                 const ser_context& context,
                                 std::error_code& ec) = 0; // (12)

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag tag,
                                const ser_context& context,
                                std::error_code& ec) = 0; // (13)

    virtual bool do_half_value(uint16_t value, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec) = 0; // (14)

    virtual bool do_double_value(double value, 
                                 semantic_tag tag,
                                 const ser_context& context,
                                 std::error_code& ec) = 0; // (15)

(1) Handles the beginning of an object of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(2) Handles the beginning of an object of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(3) Handles the end of an object.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(4) Handles the beginning of an array of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(5) Handles the beginning of an array of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(6) Handles the end of an array.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(7) Handles the name part of an object name-value pair.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(8) Handles a null value.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(9) Handles a boolean value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(10) Handles a string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(11) Handles a byte string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(12) Handles a non-negative integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(13) Handles a signed integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(14) Handles a half precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(15) Handles a double precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

    virtual void do_flush() = 0;
Allows producers of json events to flush any buffered data.

#### Parameters

`tag` - a jsoncons semantic tag
`context` - parse context information including line and column number
`ec` - a parse error code

#### Exceptions

The overloads that do not take a `std::error_code&` parameter throw a
[ser_error](ser_error.md) on parse errors, constructed with the error code as the error code argument
and line and column from the `context`. 

The overloads that take a `std::error_code&` parameter set it to the error code and return `false` on parse errors.

### See also

- [semantic_tag](semantic_tag.md)

- [basic_default_json_content_handler](basic_default_json_content_handler.md)

- [basic_json_content_filter](basic_json_content_filter.md)

