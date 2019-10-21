### jsoncons::basic_cbor_content_handler

```c++
#include <jsoncons/json_content_filter.hpp>

template <
    class Float128T
> class basic_cbor_content_handler
```

Defines an interface for producing and consuming CBOR events, including [typed arrays](https://tools.ietf.org/html/draft-ietf-cbor-array-tags-08). 

`basic_cbor_content_handler` is noncopyable and nonmoveable.

![basic_cbor_content_handler](./diagrams/basic_cbor_content_handler.png)

Typedef for content handler using only standard C++ floating point types:

Type                |Definition
--------------------|------------------------------
cbor_content_handler    |`basic_cbor_content_handler<void>`

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|char
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [std::string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if C++ 17 is detected or if `JSONCONS_HAS_STRING_VIEW` is defined, otherwise proxied.  
`float128_type`|Float128T

#### Public event producer interface

    bool typed_array(const uint8_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const uint16_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const uint32_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const uint64_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const int8_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const int16_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const int32_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const int64_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(half_arg_t, const uint16_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const float* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const double* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const float128_type* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context())

    bool typed_array(const uint8_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const uint16_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const uint32_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const uint64_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const int8_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const int16_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const int32_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const int64_t* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const float* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

    bool typed_array(const double* data, size_t size, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec)

#### Private event consumer interface

    virtual bool do_typed_array(const uint8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const uint16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const uint32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const uint64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const int8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const int16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const int32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const int64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(half_arg_t, const uint16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const float* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const double* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

    virtual bool do_typed_array(const float128_type* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0;

### See also

- [basic_json_content_handler](../basic_json_content_handler.md)

### Examples


