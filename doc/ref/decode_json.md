### jsoncons::decode_json

Converts a JSON formatted string to a C++ object. `decode_json` attempts to 
perform the conversion by streaming using `json_convert_traits`, and if
streaming is not supported, falls back to using `json_type_traits`. `decode_json` will 
work for all types that have `json_type_traits` defined.

#### Header

```c++
#include <jsoncons/json.hpp>

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s)
{
    return json_convert_traits<T>::decode(s);
}
```


