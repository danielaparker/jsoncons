### jsoncons::semantic_tag

```c++
#include <jsoncons/tag_type.hpp>

enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined,
    datetime,
    seconds,                  // since 0.165.0
    epoch_second = seconds,   // (until 0.165.0, since deprecated)
    millis,                   // since 0.165.0
    epoch_milli = millis,     // (until 0.165.0, since deprecated)
    nanos,                    // since 0.165.0
    epoch_nano = nanos,       // (until 0.165.0, since deprecated)
    bigint,
    bigdec,
    float128,                 // since 0.165.0
    bigfloat,
    base16,
    base64,
    base64url,
    uri,
    clamped,
    multi_dim_row_major,
    multi_dim_column_major,
    ext,
    id,                       // since 0.165.0
    regex,                    // since 0.165.0
    code                      // since 0.165.0
};
```

