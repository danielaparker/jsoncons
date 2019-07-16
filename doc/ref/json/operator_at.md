### jsoncons::basic_json::operator[]

```c++
proxy_type operator[](const string_view_type& key); // (1)

const_reference operator[](const string_view_type& key) const; // (2)

reference operator[](size_t i); // (3)

const_reference operator[](size_t i) const; // (4)
```

(1) Returns a "reference-like" proxy object that can be used to access 
or assign to the underlying keyed value. Type `proxy_type` supports 
the `basic_json` interface and conversion to `basic_json&`. If accessing 
and the key exists, evaluates to a reference to the keyed value.If 
accessing and the key does not exist, `noexcept` accessors return a 
default value, e.g. the `is_xxx` functions return `false`, other 
accessors throw.
If assigning, inserts or updates with the new value. 
Throws `std::runtime_error` if not an object. 
If read, throws `std::out_of_range` if the object does not have a 
member with the specified key.  

(2) If `key` matches the key of a member in the basic_json object, returns a reference to the basic_json object, otherwise throws.
Throws `std::runtime_error` if not an object.
Throws `std::out_of_range` if the object does not have a member with the specified key.  

(3) Returns a reference to the value at index i in a `basic_json` object or array.
Throws `std::runtime_error` if not an object or array.

(4) Returns a `const_reference` to the value at index i in a `basic_json` object or array.
Throws `std::runtime_error` if not an object or array.

#### Notes

Unlike `std::map::operator[]`, a new element is never inserted into the container 
when this operator is used for reading but the key does not exist.

### Examples

#### Assigning to a proxy when the key does not exist

```c++
int main()
{
    json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};

    json color_spaces = json::array();
    color_spaces.push_back("sRGB");
    color_spaces.push_back("AdobeRGB");
    color_spaces.push_back("ProPhoto RGB");

    json export_settings;
    export_settings["File Format Options"]["Color Spaces"] = std::move(color_spaces);
    export_settings["File Format Options"]["Image Formats"] = std::move(image_formats);

    std::cout << pretty_print(export_settings) << "\n\n";
}
```
Output:
```
{
    "File Format Options": {
        "Color Spaces": ["sRGB", "AdobeRGB", "ProPhoto RGB"],
        "Image Formats": ["JPEG", "PSD", "TIFF", "DNG"]
    }
}
```
Note that if `file_export["File Format Options"]` doesn’t exist, the statement
```
file_export["File Format Options"]["Color Spaces"] = std::move(color_spaces)
```
creates "File Format Options" as an object and puts "Color Spaces" in it.

