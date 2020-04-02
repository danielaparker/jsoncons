### jsoncons::jsonpointer::unflatten_method

```c++
#include <jsoncons/jsonpointer/jsonpointer.hpp>

enum class unflatten_method{object=1,safe};
```
`unflatten_method` is used to specify how to unflatten a single depth 
object of JSON Pointer-value pairs. There is no unique solution.
An integer appearing in a path could be an index or it could be an object key.

Value      |Definition
-----------|-----------
unflatten_method()|If there is a zero in a path, it will start a new array.
object|Arrays will not be created when calling unflatten, rather, an integer appearing in a path is assumed to be an object key
safe|

