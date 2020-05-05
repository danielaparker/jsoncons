### jsoncons::jsonpointer::unflatten_method

```c++
#include <jsoncons/jsonpointer/jsonpointer.hpp>

enum class unflatten_method{preserve_arrays,object};
```
`unflatten_method` is used to specify how to unflatten a json 
object of JSON Pointer-value pairs. There is no unique solution.
An integer appearing in a path could be an array index or it could be an object key.

Value      |Definition
-----------|-----------
preserve_arrays|If there is a zero in a path, start a new array. If this leads to an error, back up and translate to an object.
object|Assume an integer appearing in a path is an object key

