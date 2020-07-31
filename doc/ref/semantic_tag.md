### jsoncons::semantic_tag

```c++
#include <jsoncons/tag_type.hpp>

enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined,
    datetime,
    epoch_second, // since 0.155.0
    timestamp = epoch_second,  // deprecated (since 0.155.0)
    epoch_milli, // since 0.155.0
    epoch_nano, // since 0.155.0
    bigint,
    bigdec,
    bigfloat,
    base16,
    base64,
    base64url,
    uri,
    clamped,
    multi_dim_row_major,
    multi_dim_column_major,
    ext
};
```

