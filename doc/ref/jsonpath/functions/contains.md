### contains

```
boolean contains(array|string subject, any search)
```

If subject is an array, returns true if the array contains an item that is equal to 
the provided search value, false otherwise.

If subject is a string, returns true if the string contains a substring that is equal to
the provided search value, false otherwise.

Returns null if 

- the provided subject is not an array or string, or
- the provided subject is a string but the provided search value is not a string.

### Examples


