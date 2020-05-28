### jsoncons::basic_json::array_range

```c++
range<array_iterator,const_array_iterator> array_range();
range<const_array_iterator,const_array_iterator> array_range() const;
```
Returns a [range](range.md) that supports a range-based for loop over the elements of a `json` array      
Throws `std::domain_error` if not an array.

### Examples

#### Range-based for loop

```c++
json j(json_array_arg);
j.push_back("Montreal");
j.push_back("Toronto");
j.push_back("Vancouver");

for (const auto& val : j.array_range())
{
    std::cout << val.as<std::string>() << std::endl;
}
```
Output:
```json
Montreal
Toronto
Vancouver 
```

#### Array iterator
```c++
json j(json_array_arg, {"Montreal", "Toronto", "Vancouver"});

for (auto it = j.array_range().begin(); it != j.array_range().end(); ++it)
{
    std::cout << it->as<std::string>() << std::endl;
}
```
Output:
```json
Montreal
Toronto
Vancouver 
```

#### Reverse array iterator
```c++
json j(json_array_arg, {"Montreal", "Toronto", "Vancouver"});

for (auto it = j.array_range().rbegin(); it != j.array_range().rend(); ++it)
{
    std::cout << it->as<std::string>() << std::endl;
}
```
Output:
```json
Vancouver
Toronto
Montreal
```


