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
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [std::string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if C++ 17 is detected or if `JSONCONS_HAS_STD_STRING_VIEW` is defined, otherwise proxied.  

#### Public event producer interface

    void flush(); // (1)

    bool begin_object(semantic_tag tag=semantic_tag::none,
                      const ser_context& context=null_ser_context()); // (2)

    bool begin_object(std::size_t length, 
                      semantic_tag tag=semantic_tag::none, 
                      const ser_context& context = null_ser_context()); // (3)

    bool end_object(const ser_context& context = null_ser_context()); // (4)

    bool begin_array(semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (5)

    bool begin_array(std::size_t length, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (6)

    bool end_array(const ser_context& context=null_ser_context()); // (7)

    bool key(const string_view_type& name, 
              const ser_context& context=null_ser_context()); // (8)

    bool null_value(semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (9) 

    bool bool_value(bool value, 
                    semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (10) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (11) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context()); // (12)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (13)

    bool int64_value(int64_t value, 
                     semantic_tag tag = semantic_tag::none, 
                     const ser_context& context=null_ser_context()); // (14)

    bool half_value(uint16_t value, 
                    semantic_tag tag = semantic_tag::none, 
                    const ser_context& context=null_ser_context()); // (15)

    bool double_value(double value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (16)

    bool begin_object(semantic_tag tag,
                      const ser_context& context,
                      std::error_code& ec); // (17)

    bool begin_object(std::size_t length, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (18)

    bool end_object(const ser_context& context, 
                    std::error_code& ec); // (19)

    bool begin_array(semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (20)

    bool begin_array(std::size_t length, 
                     semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (21)

    bool end_array(const ser_context& context, 
                   std::error_code& ec); // (22)

    bool key(const string_view_type& name, 
              const ser_context& context, 
              std::error_code& ec); // (23)

    bool null_value(semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (24) 

    bool bool_value(bool value, 
                    semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (25) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (26) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec); // (27)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (28)

    bool int64_value(int64_t value, 
                     semantic_tag tag, 
                     const ser_context& context,
                     std::error_code& ec); // (29)

    bool half_value(uint16_t value, 
                    semantic_tag tag, 
                    const ser_context& context,
                    std::error_code& ec); // (30)

    bool double_value(double value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (31)

    bool typed_array(const span<const uint8_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (32)

    bool typed_array(const span<const uint16_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (33)

    bool typed_array(const span<const uint32_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (34)

    bool typed_array(const span<const uint64_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (35)

    bool typed_array(const span<const int8_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (36)

    bool typed_array(const span<const int16_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (37)

    bool typed_array(const span<const int32_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (38)

    bool typed_array(const span<const int64_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (39)

    bool typed_array(half_arg_t, const span<const uint16_t>& data,
                     semantic_tag tag = semantic_tag::none,
                     const ser_context& context = null_ser_context()); // (40)

    bool typed_array(const span<const float>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (41)

    bool typed_array(const span<const double>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (42)

    bool typed_array(const span<const float128_type>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (43)

    bool begin_multi_dim(const span<const size_t>& shape,
                         semantic_tag tag,
                         const ser_context& context); // (44) 

    bool end_multi_dim(const ser_context& context=null_ser_context()); // (45) 

    bool typed_array(const span<const uint8_t>& v, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (46)

    bool typed_array(const span<const uint16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (47)

    bool typed_array(const span<const uint32_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (48)

    bool typed_array(const span<const uint64_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (49)

    bool typed_array(const span<const int8_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (50)

    bool typed_array(const span<const int16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (51)

    bool typed_array(const span<const int32_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (52)

    bool typed_array(const span<const int64_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (53)

    bool typed_array(half_arg_t arg, const span<const uint16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (54)

    bool typed_array(const span<const float>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (55)

    bool typed_array(const span<const double>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (56)

    bool begin_multi_dim(const span<const size_t>& shape,
                         semantic_tag tag,
                         const ser_context& context, 
                         std::error_code& ec); // (57)

    bool end_multi_dim(const ser_context& context,
                       std::error_code& ec); // (58) 

(1) Flushes whatever is buffered to the destination.

(2) Indicates the begining of an object of indefinite length.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(3) Indicates the begining of an object of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(4) Indicates the end of an object.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(5) Indicates the beginning of an indefinite length array. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(6) Indicates the beginning of an array of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(7) Indicates the end of an array.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(8) Writes the name part of an object name-value pair.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(9) Writes a null value. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(10) Writes a boolean value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(11) Writes a text string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(12) Writes a byte string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(13) Writes a non-negative integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(14) Writes a signed integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(15) Writes a half precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(16) Writes a double precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(17)-(31) Same as (2)-(16), except sets `ec` and returns `false` on parse errors.

#### Private event consumer interface

    virtual void do_flush() = 0; // (1)

    virtual bool do_begin_object(semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0; // (2)

    virtual bool do_begin_object(std::size_t length, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec); // (3)

    virtual bool do_end_object(const ser_context& context, 
                               std::error_code& ec) = 0; // (4)

    virtual bool do_begin_array(semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (5)

    virtual bool do_begin_array(std::size_t length, 
                                semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec); // (6)

    virtual bool do_end_array(const ser_context& context, 
                              std::error_code& ec) = 0; // (7)

    virtual bool do_key(const string_view_type& name, 
                         const ser_context& context, 
                         std::error_code&) = 0; // (8)

    virtual bool do_null(semantic_tag tag, 
                         const ser_context& context, 
                         std::error_code& ec) = 0; // (9)

    virtual bool do_bool(bool value, 
                         semantic_tag tag, 
                         const ser_context& context, 
                         std::error_code&) = 0; // (10)

    virtual bool do_string(const string_view_type& value, 
                           semantic_tag tag, 
                           const ser_context& context, 
                           std::error_code& ec) = 0; // (11)

    virtual bool do_byte_string(const byte_string_view& value, 
                                semantic_tag tag, 
                                const ser_context& context,
                                std::error_code& ec) = 0; // (12)

    virtual bool do_uint64(uint64_t value, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec) = 0; // (13)

    virtual bool do_int64(int64_t value, 
                          semantic_tag tag,
                          const ser_context& context,
                          std::error_code& ec) = 0; // (14)

    virtual bool do_half(uint16_t value, 
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec); // (15)

    virtual bool do_double(double value, 
                           semantic_tag tag,
                           const ser_context& context,
                           std::error_code& ec) = 0; // (16)

    virtual bool do_typed_array(const span<const uint8_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (17)

    virtual bool do_typed_array(const span<const uint16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (18)

    virtual bool do_typed_array(const span<const uint32_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (19)

    virtual bool do_typed_array(const span<const uint64_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (20)

    virtual bool do_typed_array(const span<const int8_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (21)

    virtual bool do_typed_array(const span<const int16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (22)

    virtual bool do_typed_array(const span<const int32_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (23)

    virtual bool do_typed_array(const span<const int64_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (24)

    virtual bool do_typed_array(half_arg_t, 
                                const span<const uint16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (25)

    virtual bool do_typed_array(const span<const float>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (26)

    virtual bool do_typed_array(const span<const double>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (27)

    virtual bool do_typed_array(const span<const float128_type>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec); // (28)

    virtual bool do_begin_multi_dim(const span<const size_t>& shape,
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec); // (29)

    virtual bool do_end_multi_dim(const ser_context& context,
                                  std::error_code& ec); // (30)

(1) Allows producers of json events to flush any buffered data.

(2) Handles the beginning of an object of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(3) Handles the beginning of an object of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(4) Handles the end of an object.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(5) Handles the beginning of an array of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(6) Handles the beginning of an array of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(7) Handles the end of an array.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(8) Handles the name part of an object name-value pair.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(9) Handles a null value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(10) Handles a boolean value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(11) Handles a string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(12) Handles a byte string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(13) Handles a non-negative integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(14) Handles a signed integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(15) Handles a half precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(16) Handles a double precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

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

- [basic_json_filter](basic_json_filter.md)

