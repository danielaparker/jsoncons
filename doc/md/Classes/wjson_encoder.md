    jsoncons::wjson_encoder

    typedef basic_json_encoder<wjson> wjson_encoder

The `wjson_encoder` class is an instantiation of the `basic_json_encoder` class template that uses `wjson` as the json type.

### Header

    #include <jsoncons/json.hpp>

### Interface

The interface is the same as [json_encoder](json_encoder), substituting wide character instantiations of classes - `std::wstring`, etc. - for narrow character ones.

