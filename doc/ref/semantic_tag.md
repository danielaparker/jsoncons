### jsoncons::semantic_tag

__`jsoncons/tag_type.hpp`__

```c++
enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined,
    datetime,
    timestamp,
    bigint,
    bigdec,
    bigfloat,
    base16,
    base64,
    base64url,
    uri,
    clamped,
    multi_dim_row_major,
    multi_dim_column_major
};
```

