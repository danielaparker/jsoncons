### Reflection traits

### Legacy json_type_traits

[json_type_traits](json_type_traits.md) defines a compile time template based interface for conversion between a `basic_json` value
and a value of some other type. 

jsoncons 1.4.0 introduces new trait definitions, [json_conv_traits](reflect/json_conv_traits.md), that support
non-throwing conversions and user supplied allocators. For backwards compatability, `json_conv_traits`
defaults to `json_type_traits` if a conversion is undefined.  

