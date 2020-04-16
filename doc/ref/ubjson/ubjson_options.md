### jsoncons::ubjson::ubjson_options

```c++
#include <jsoncons_ext/ubjson/ubjson_options.hpp>
```

![ubjson_options](./diagrams/ubjson_options.png)

<br>

Specifies options for reading and writing CBOR.

#### Constructors

    ubjson_options()
Constructs a `ubjson_options` with default values. 

#### Modifiers

    void max_nesting_depth(int depth)
The maximum nesting depth allowed when decoding and encoding UBJSON. 
Default is 1024. Parsing can have an arbitrarily large depth
limited only by available memory. Serializing a [basic_json](../basic_json.md) to
UBJSON is limited by stack size.

