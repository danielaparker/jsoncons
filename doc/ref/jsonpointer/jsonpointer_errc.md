### jsoncons::jsonpointer::jsonpointer_errc

The constant integer values scoped by `jsonpointer_errc` define the values for jsonpointer error codes.

#### Header

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer_error_category.hpp>
```

### Member constants

constant                            |Description
------------------------------------|------------------------------
`expected_slash`                    |Expected /
`expected_digit_or_dash`            |Expected digit or '-'
`unexpected_leading_zero`           |Unexpected leading zero
`index_exceeds_array_size`          |Index exceeds array size
`expected_0_or_1`                   |Expected '0' or '1' after escape character '~'
`name_not_found`                    |Name not found
`key_already_exists`                |Key already exists
`expected_object_or_array`          |Expected object or array 
`end_of_input`                      |Unexpected end of input      




