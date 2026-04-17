### jsoncons::jsonschema::walk_result 

```cpp
enum class walk_result {
   advance,                 (until 1.7.0)
   abort
};
```

```cpp
enum class walk_state {
   advance,                 (since 1.7.0)
   abort
};

using walk_result = walk_state;  // for backwards compatibility
```

