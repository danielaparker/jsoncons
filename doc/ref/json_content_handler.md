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

    bool begin_object(semantic_tag_type tag=semantic_tag_type::none,
                      const serializing_context& context=null_serializing_context())
    bool begin_object(semantic_tag_type tag,
                      const serializing_context& context,
                      std::error_code& ec)
Indicates the begining of an object of indefinite length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter.
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool begin_object(size_t length, 
                      semantic_tag_type tag=semantic_tag_type::none,
                      const serializing_context& context=null_serializing_context())
    bool begin_object(size_t length, 
                      semantic_tag_type tag,
                      const serializing_context& context,
                      std::error_code& ec)
Indicates the begining of an object of known length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter.
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool end_object(const serializing_context& context = null_serializing_context())
    bool end_object(const serializing_context& context, std::error_code& ec)
Indicates the end of an object. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    bool begin_array(semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
Indicates the beginning of an indefinite length array. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const serializing_context& context=null_serializing_context())
    bool begin_array(semantic_tag_type tag, const serializing_context& context, std::error_code& ec)
Indicates the beginning of an array of known length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool end_array(const serializing_context& context=null_serializing_context())
    bool end_array(const serializing_context& context, std::error_code& ec)
Indicates the end of an array. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool name(const string_view_type& name, 
              const serializing_context& context=null_serializing_context())
    bool name(const string_view_type& name, 
              const serializing_context& context, 
              std::error_code& ec)
Writes the name part of a name-value pair inside an object. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter.  
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context());
    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec) 
Writes a string value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
    bool byte_string_value(const byte_string_view& b, 
                           byte_string_chars_format encoding_hint,
                           semantic_tag_type tag, 
                           const serializing_context& context,
                           std::error_code& ec)
Writes a byte string value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool byte_string_value(const uint8_t* p, size_t size, 
                           byte_string_chars_format encoding_hint = byte_string_chars_format::none,
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const serializing_context& context=null_serializing_context())
Writes a byte string value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool bignum_value(const string_view_type& s, 
                      const serializing_context& context=null_serializing_context()) 
    bool bignum_value(const string_view_type& s, 
                      const serializing_context& context, 
                      std::error_code& ec) 
Writes a bignum using the decimal string representation of a bignum. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool decimal_value(const string_view_type& s, 
                       const serializing_context& context=null_serializing_context()) 
    bool decimal_value(const string_view_type& s, 
                       const serializing_context& context, 
                       std::error_code& ec) 
Writes a decimal value using the decimal string representation. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool date_time_value(const string_view_type& s, 
                         const serializing_context& context=null_serializing_context()) 
    bool date_time_value(const string_view_type& s, 
                         const serializing_context& context, 
                         std::error_code& ec) 
Writes a date-time value using the string representation. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool epoch_time_value(int64_t val, 
                          const serializing_context& context=null_serializing_context()) 
    bool epoch_time_value(int64_t val, 
                          const serializing_context& context, 
                          std::error_code& ec) 
Writes an epoch time value using the integer representation. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool int64_value(int64_t value, 
                     semantic_tag_type tag = semantic_tag_type::none, 
                     const serializing_context& context=null_serializing_context());
    bool int64_value(int64_t value, 
                     semantic_tag_type tag, 
                     const serializing_context& context,
                     std::error_code& ec)
Writes a signed integer value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec)
Writes a non-negative integer value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool double_value(double value, 
                      const floating_point_options& fmt = floating_point_options(), 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const serializing_context& context=null_serializing_context())
    bool double_value(double value, 
                      const floating_point_options& fmt, 
                      semantic_tag_type tag, 
                      const serializing_context& context,
                      std::error_code& ec)
Writes a floating point value with default precision (`std::numeric_limits<double>::digits10`.) Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool bool_value(bool value, 
                    semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    bool bool_value(bool value, 
                    semantic_tag_type tag,
                    const serializing_context& context,
                    std::error_code& ec) 
Writes a boolean value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    bool null_value(semantic_tag_type tag = semantic_tag_type::none,
                    const serializing_context& context=null_serializing_context()) 
    bool null_value(semantic_tag_type tag,
                    const serializing_context& context,
                    std::error_code& ec) 
Writes a null value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    void flush()
Flushes whatever is buffered to the destination.

#### Private virtual consumer interface

    virtual bool do_begin_object(semantic_tag_type tag, 
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;
Handles the beginning of an object of indefinite length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_begin_object(size_t length,
                                 semantic_tag_type tag, 
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;
Handles the beginning of an object of known length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_end_object(const serializing_context& context, 
                               std::error_code& ec) = 0;
Handles the end of an object. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_begin_array(semantic_tag_type tag, 
                                const serializing_context& context, 
                                std::error_code& ec) = 0;
Handles the beginning of an array of indefinite length. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_begin_array(size_t length, 
                                const serializing_context& context, 
                                std::error_code& ec);
Handles the beginning of an array of known length. Defaults to calling `do_begin_array(semantic_tag_type, const serializing_context& context, std::error_code& ec)`. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_end_array(const serializing_context& context, 
                              std::error_code& ec) = 0;
Handles the end of an array. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_name(const string_view_type& name, 
                         const serializing_context& context, 
                         std::error_code& ec) = 0;
Handles the name part of a name-value pair inside an object. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter.  
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_string_value(const string_view_type& val, 
                                 semantic_tag_type tag,
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;
Handles a string value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_byte_string_value(const byte_string_view& b, 
                                      byte_string_chars_format encoding_hint,
                                      semantic_tag_type tag,
                                      const serializing_context& context, 
                                      std::error_code& ec) = 0;
Handles a byte string value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag_type tag, 
                                const serializing_context& context, 
                                std::error_code& ec) = 0;
Handles a signed integer value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag_type tag, 
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;
Handles a non-negative integer value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_double_value(double value, 
                                 const floating_point_options& fmt, 
                                 semantic_tag_type tag, 
                                 const serializing_context& context, 
                                 std::error_code& ec) = 0;
Handles a floating point value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_bool_value(bool value, 
                               semantic_tag_type tag, 
                               const serializing_context& context, 
                               std::error_code& ec) = 0;
Handles a boolean value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual bool do_null_value(semantic_tag_type tag, 
                               const serializing_context& context, 
                               std::error_code& ec) = 0;
Handles a null value. Contextual information including
line and column number is provided in the [context](serializing_context.md) parameter. 
Returns `true` if the producer should continue streaming events, `false` otherwise.

    virtual void do_flush() = 0;
Allows producers of json events to flush whatever they've buffered.

#### See also

- [semantic_tag_type](../semantic_tag_type.md)

