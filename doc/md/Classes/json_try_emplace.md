```c++
jsoncons::json::try_emplace

template <class T>
pair<object_iterator, bool> try_emplace(string_view_type key, T&& value);

template <class T>
object_iterator try_emplace(const_object_iterator hint, string_view_type key, T&& value);
```
