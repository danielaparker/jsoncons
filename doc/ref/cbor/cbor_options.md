### jsoncons::cbor::cbor_options

Specifies options for encoding and decoding CBOR.

#### Header
```c++
#include <jsoncons/cbor/cbor_options.hpp>
```

![cbor_options](./diagrams/cbor_options.png)

#### Constructors

    cbor_options()
Constructs a `cbor_options` with default values. 

#### Modifiers

    cbor_options& pack_strings(bool value)

If set to `true`, then encode will store text strings and
byte strings once, and use references for repeated occurences
of the strings. In order to decode CBOR so encoded, the decoder
must understand the 
[stringref extension to CBOR](http://cbor.schmorp.de/stringref).

If set to `false` (the default), then encode
will encode strings the usual CBOR way. 

This option does not affect decode - jsoncons will always decode
string references if present.

### See also

[cbor_decode_options](cbor_decode_options.md)
[cbor_encode_options](cbor_encode_options.md)

