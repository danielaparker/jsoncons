### jsoncons::cbor::ondemand::access_result

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>

template <typename T> 
using access_result = jsoncons::expected<T,access_error>;
```

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`value_type`|`T`
`error_type`|[access_error](access_error.md)

