### jsoncons::jsonpath::result_options

```c++
enum class result_options {
   value = 1,
   path = 2,
   no_dups = 4 | path,
   sort = 8 | path
};                            (until 0.164.0)

enum class result_options {
    value=0, 
    nodups=1, 
    sort=2, 
    path=4
};                            (since 0.164.0)
```

A [BitmaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType) 
used to specify result options for JSONPath queries. 

