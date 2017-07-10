```c++
jsoncons::json::emplace

template <class... Args>
pair<array_iterator, bool> emplace(Args&&... args);

template <class... Args>
object_iterator emplace(const_array_iterator pos, Args&&... args);
```

### Parameters

    pos
iterator before which the new json value will be constructed
    args
arguments to forward to the constructor of the json value
