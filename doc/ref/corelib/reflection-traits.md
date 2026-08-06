### Reflection traits

jsoncons supports two kinds of reflection traits

- json variant traits

- streaming traits

### json variant traits

jsoncons json variant traits define a compile time template based interface for conversion between a `basic_json` value
and a value of some other type. They are used in `basic_json` template constructors, `basic_json::is<T>`, `basic_json::as<T>`, 
and `basic_json::try_as<T>` with the requirement that type `T` have `json_traits` defined. 

#### jsoncons::reflect::json_traits 

Since 1.4.0. jsoncons introduces new trait definitions, [json_conv_traits](reflect/json_conv_traits.md), that support
non-throwing conversions and [uses-allocator construction](https://en.cppreference.com/w/cpp/memory/uses_allocator.html). 

`json_traits` defaults to the legacy [json_type_traits](json_type_traits/json_type_traits.md) if a type conversion is undefined.  

Since 1.9.0, `json_conv_traits` has been renamed to [json_traits](json_conv_traits](reflect/json_traits.md),
for backwards compatibility, the old name is aliased to the new name.

See [Eigen::Matrix example](reflect/Eigen-Matrix-example.md) for an example of specializing `json_traits` for an [Eigen matrix class](https://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html).

See [User-allocator construction example](reflect/uses-allocator-construction-example.md) for an example that illustrates uses-allocator construction for types with `json_traits` defined. 

#### Legacy jsoncons::json_type_traits

[json_type_traits](json_type_traits/json_type_traits.md) defines a compile time template based interface for conversion between a `basic_json` value
and a value of some other type. 

See [Eigen::Matrix example](json_type_traits/Eigen-Matrix-example.md) for an example of specializing `json_type_traits` for an [Eigen matrix class](https://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html).

### Streaming traits

jsoncons decode traits define a compile time template based interface for converting a stream of [staj_events](./staj_events.md) 
into a C++ data structure. They are used in the `decode_<format>` functions such as [decode_json](./decode_json.md). 

jsoncons encode traits define a compile time template based interface for converting 
a C++ data structure into a container of characters or an output stream. They are used in the 
`encode_<format>` functions such as [encode_json](./encode_json.md).

The jsoncons decode and encode traits fall back on the json variant traits if they have no
specialization for type `T`.

#### decode_traits

[decode_traits](./reflect/decode_traits.md)

#### encode_traits

[encode_traits](./reflect/encode_traits.md)

### Convenience macros

jsoncons includes some [convenience macros](reflect/reflect-traits-gen.md) for generating reflection traits classes.
Until 1.4.0, these macros generated [json_type_traits](json_type_traits/json_type_traits.md) class templates. Since 1.4.0, they
generate [json_traits](reflect/json_traits.md) class templates, as well as some additional traits that support streaming.

