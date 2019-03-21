### jsoncons::ubjson::basic_ubjson_serializer

```c++
template<
    class CharT,
    class Result>
> class basic_ubjson_serializer : public jsoncons::basic_json_content_handler<CharT>
```

`basic_ubjson_serializer` is noncopyable and nonmoveable.

#### Header

    #include <jsoncons_ext/ubjson/ubjson_serializer.hpp>

![ubjson_serializer](./diagrams/ubjson_serializer.png)

Four specializations for common character types and result types are defined:

Type                       |Definition
---------------------------|------------------------------
ubjson_serializer            |basic_ubjson_serializer<char,jsoncons::binary_stream_result>
ubjson_buffer_serializer     |basic_ubjson_serializer<char,jsoncons::binary_buffer_result>
wubjson_serializer           |basic_ubjson_serializer<wchar_t,jsoncons::binary_stream_result>
wubjson_buffer_serializer    |basic_ubjson_serializer<wchar_t,jsoncons::binary_buffer_result>

#### Member types

Type                       |Definition
---------------------------|------------------------------
char_type                  |CharT
result_type                |Result
string_view_type           |

#### Constructors

    explicit basic_ubjson_serializer(result_type result)
Constructs a new serializer that writes to the specified result.

#### Destructor

    virtual ~basic_ubjson_serializer()

### Inherited from [basic_json_content_handler](../json_content_handler.md)

#### Member functions

    bool begin_object(semantic_tag_type tag=semantic_tag_type::none,
                      const ser_context& context=null_ser_context()); 

    bool begin_object(size_t length, 
                      semantic_tag_type tag=semantic_tag_type::none,
                      const ser_context& context=null_ser_context()); 

    bool end_object(const ser_context& context = null_ser_context())

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const ser_context& context=null_ser_context()); 

    bool begin_array(semantic_tag_type tag=semantic_tag_type::none,
                     const ser_context& context=null_ser_context()); 

    bool end_array(const ser_context& context=null_ser_context()); 

    bool name(const string_view_type& name, 
              const ser_context& context=null_ser_context()); 

    bool string_value(const string_view_type& value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const ser_context& context=null_ser_context()); ;

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const ser_context& context=null_ser_context()); 

    bool byte_string_value(const uint8_t* p, size_t size, 
                           semantic_tag_type tag=semantic_tag_type::none, 
                           const ser_context& context=null_ser_context()); 

    bool big_integer_value(const string_view_type& s, 
                           const ser_context& context=null_ser_context());  

    bool big_decimal_value(const string_view_type& s, 
                           const ser_context& context=null_ser_context());  

    bool date_time_value(const string_view_type& s, 
                         const ser_context& context=null_ser_context());  

    bool timestamp_value(int64_t val, 
                         const ser_context& context=null_ser_context());  

    bool int64_value(int64_t value, 
                     semantic_tag_type tag = semantic_tag_type::none, 
                     const ser_context& context=null_ser_context()); ;

    bool uint64_value(uint64_t value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const ser_context& context=null_ser_context()); 

    bool double_value(double value, 
                      semantic_tag_type tag = semantic_tag_type::none, 
                      const ser_context& context=null_ser_context()); 

    bool bool_value(bool value, 
                    semantic_tag_type tag = semantic_tag_type::none,
                    const ser_context& context=null_ser_context());  

    bool null_value(semantic_tag_type tag = semantic_tag_type::none,
                    const ser_context& context=null_ser_context());  

    void flush()

### Examples


