### jsoncons::jsonpatch::patch

Patch a `json` document.

#### Header
```c++
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>

template <class Json>
std::tuple<jsonpatch_errc,typename Json::string_type> patch(Json& target, const Json& patch)
```

#### Return value

On success, returns a value-initialized [jsonpatch_errc](jsonpatch_errc.md) and an empty string value.  

On error, returns a [jsonpatch_errc](jsonpatch_errc.md) error code and the path that failed. 

### Examples


