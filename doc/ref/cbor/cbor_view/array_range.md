### jsoncons::cbor::cbor_view::array_range

```c++
range<const_array_iterator> array_range() const;
```
Returns a "range" that supports a range-based for loop over the elements of a `cbor_view` array      
Throws `std::runtime_error` if not an array.

### Examples

#### Range-based for loop

```c++
// [\"Toronto\",\"Vancouver\",\"Montreal\"]

std::vector<uint8_t> packed = {0x83,0x67,'T','o','r','o','n','t','o',0x69,'V','a','n','c','o','u','v','e','r',0x68,'M','o','n','t','r','e','a','l'};

cbor::cbor_view v{packed};
for (auto element : v.array_range())
{
    std::cout << element.as_string() << std::endl;
}
```
Output:
```
Toronto
Vancouver 
Montreal
```


