### jsoncons::toon::toon_options

```cpp
#include <jsoncons_ext/toon/toon_options.hpp>

class toon_options;
```

<br>

![toon_options](./diagrams/toon_options.png)

Specifies options for reading and writing [toon-format](https://github.com/toon-format/toon).

#### Constructors

    toon_options()
Constructs a `toon_options` with default values. 

#### Modifiers

    void delimiter(char value)
Delimiter character for arrays. Default is **','**.

    void indent(int value)
Number of spaces to indent each level. Default is **2**.

    void length_marker(jsoncons::optional<char> value)
Optional marker for prefixing array lengths. Default is none.

    void strict(bool value)
Strict mode for decode. Default is true.
