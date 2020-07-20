### jsoncons::semantic_tag

```c++
#include <jsoncons/tag_type.hpp>

enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined,
    datetime,
    epoch_time, // since 0.155.0
    timestamp,  // deprecated (since 0.155.0)
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

