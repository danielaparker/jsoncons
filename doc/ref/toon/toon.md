### toon-format extension

The toon extension implements encode to and decode from [toon-format](https://github.com/toon-format/toon).
You can parse into or serialize from the variant-like data structure, [basic_json](../corelib/basic_json.md).

[decode_toon, try_decode_toon](decode_toon.md)

[encode_toon](encode_toon.md)

[toon_options](toon_options.md)

### Compliance 

The jsoncons implementation passes all required toon-format [encode tests](https://github.com/toon-format/spec/tree/main/tests/fixtures/encode)
and  [decode tests](https://github.com/toon-format/spec/tree/main/tests/fixtures/decode) except for the 
[key-folding](https://github.com/toon-format/spec/blob/main/tests/fixtures/encode/key-folding.json) 
and [path-expansion](https://github.com/toon-format/spec/blob/main/tests/fixtures/decode/path-expansion.json) tests. The
encode option **keyFolding** and decode option **expandPaths** are not currently supported.

