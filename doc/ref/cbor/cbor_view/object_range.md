### jsoncons::cbor::cbor_view::object_range

```c++
range<const_object_iterator> object_range() const;
```
Returns a "range" that supports a range-based for loop over the key-value pairs of a `cbor_view` object      
Throws `std::runtime_error` if not an object.

### Examples

#### Range-based for loop over key-value pairs of an object

```c++
// {\"foo\": [0, 1, 2],\"bar\": [3,4,5]}

std::vector<uint8_t> packed = {0xa2,0x63,'f','o','o',0x83,'\0','\1','\2',0x63,'b','a','r',0x83,'\3','\4','\5'};

cbor::cbor_view v{packed};
for (auto member : v.object_range())
{
    std::cout << member.key() << ":" << std::endl;
    for (auto element : member.value().array_range())
    {
        std::cout << element.as_integer() << std::endl;
    }
}
```
Output:
```
foo:
0
1
2
bar:
3
4
5
```

