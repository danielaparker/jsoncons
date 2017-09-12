### `jsoncons::json::dump`

```c++
template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const; // (1)

template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
          const serialization_options& options) const; // (2)

void dump(std::ostream& os) const; // (3)

void dump(std::ostream<CharT> os, const serialization_options& options) const; // (4)

void dump(std::ostream<CharT> os, const serialization_options& options, bool pprint) const; // (5)

void dump(basic_json_output_handler<char_type>& output_handler) const; // (6)

void dump_fragment(json_output_handler& handler) const; // (7)
```

(1) Inserts json value into string using default serialization_options.

(2) Inserts json value into string using specified [serialization_options](../serialization_options.md). 

(3) Inserts json value into stream with default serialization options. 

(4) Inserts json value into stream using specified [serialization_options](../serialization_options.md). 

(5) Inserts json value into stream using specified [serialization_options](../serialization_options.md) and pretty print flag. 

(6) Calls `begin_json()` on [output_handler](../json_output_handler.md), emits json value to the [output_handler](../json_output_handler.md), and calls `end_json()` on [output_handler](../json_output_handler.md). 

(7) Emits json value to the [output_handler](../json_output_handler.md) (does not call `begin_json()` or `end_json()`.)

### Examples



