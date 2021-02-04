### jsoncons::jsonpath::result_options

```c++
enum class result_options 
{
   value = 1,
   path = 2,
   no_dups = 4 | path,
   sort = 8 | path
};
```

A [BitmaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType) 
used to specify result options for JSONPath queries. 

