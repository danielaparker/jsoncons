### jsoncons::chars_format

```c++
#include <jsoncons/jsoncons_utilities.hpp>

#if !defined(JSONCONS_NO_TO_CHARS)
using chars_format = std::chars_format;
#else
enum class chars_format : uint8_t {fixed=1,scientific=2,hex=4,general=fixed|scientific};
#endif
```

A bitmask type used to specify floating-point formatting, typedefed to [std::chars_format](http://en.cppreference.com/w/cpp/utility/chars_format) if `JSONCONS_NO_TO_CHARS` is undefined. 

