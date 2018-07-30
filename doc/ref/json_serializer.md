### jsoncons::json_serializer

```c++
typedef basic_json_serializer<char> json_serializer
```
Implements [json_content_handler](json_content_handler.md) and supports compressed or pretty serialization. The `json_serializer` class is an instantiation of the `basic_json_serializer` class template that uses `char` as the character type. 

`json_serializer` is noncopyable and nonmoveable.

#### Header

    #include <jsoncons/json_serializer.hpp>

### Implemented interfaces

[json_content_handler](json_content_handler.md)

#### Constructors

    json_serializer(std::ostream& os)
Constructs a new serializer that writes to the specified output stream.
You must ensure that the output stream exists as long as does `json_serializer`, as `json_serializer` holds a pointer to but does not own this object.

    json_serializer(std::ostream& os, indenting line_indent)
Constructs a new serializer that writes to the output stream `os` with the specified [indenting](indenting.md).
You must ensure that the output stream exists as long as does `json_serializer`, as `json_serializer` holds a pointer to but does not own this object.

    json_serializer(std::ostream& os, const json_write_options& options)
Constructs a new serializer that writes to the specified output stream using the specified [json_write_options](json_write_options.md).
You must ensure that the output stream exists as long as does `json_serializer`, as `json_serializer` holds a pointer to but does not own this object.

    json_serializer(std::ostream& os, const json_write_options& options, indenting line_indent)
Constructs a new serializer that writes to output stream `os` using the specified [json_write_options](json_write_options.md)
and [indenting](indenting.md).
You must ensure that the output stream exists as long as does `json_serializer`, as `json_serializer` holds a pointer to but does not own this object.

#### Destructor

    virtual ~json_serializer()

### Examples

### Feeding json events directly to a `json_serializer`
```c++
#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json_serializer.hpp>

using namespace jsoncons;
using boost::numeric::ublas::matrix;

int main()
{
    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    json_serializing_options options;
    json_serializer os(std::cout, options, jsoncons::indenting::indent); 
    os.begin_array();
    for (size_t i = 0; i < A.size1(); ++i)
    {
        os.begin_array();
        for (size_t j = 0; j < A.size2(); ++j)
        {
            os.double_value(A(i, j));
        }
        os.end_array();
    }
    os.end_array();

    return 0;
}
```

Output:

```json
[
    [1,2],
    [3,4]
]
```
