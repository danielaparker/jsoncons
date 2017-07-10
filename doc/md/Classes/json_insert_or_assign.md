```
jsoncons::json::insert_or_assign
```

```
template <class T>
pair<object_iterator, bool> insert_or_assign(string_view_type key, T&& value);

template <class T>
object_iterator insert_or_assign(const_object_iterator hint, string_view_type key, T&& value);
```
