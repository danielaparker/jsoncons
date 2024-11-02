### jsoncons::basic_json::operator[]

```cpp
proxy_type operator[](const string_view_type& key);             (1)  (until 0.179.0)
basic_json& operator[](const string_view_type& key);                 (since 0.179.0)

const_reference operator[](const string_view_type& key) const;  (2)

reference operator[](std::size_t i);                            (3)

const_reference operator[](std::size_t i) const;                (4)
```

(1) Unitl 0.179.0, returns a "reference-like" proxy object that can be used to access 
or assign to the underlying keyed value. 
  
Since 0.179, returns a reference to the value that is associated with `key`, 
performing an insertion if no such `key` exists.  

(2) If `key` matches the key of a member in the basic_json object, returns a reference to the basic_json object, otherwise throws.
Throws `std::domain_error` if not an object.
Throws `std::out_of_range` if the object does not have a member with the specified key.  

(3) Returns a reference to the value at index i in a `basic_json` object or array.
Throws `std::domain_error` if not an object or array.

(4) Returns a `const_reference` to the value at index i in a `basic_json` object or array.
Throws `std::domain_error` if not an object or array.

#### Notes

Unlike `std::map::operator[]`, a new element is never inserted into the container 
when this operator is used for reading but the key does not exist.

### Examples

#### Assigning to and` object when the key does not exist

```cpp
int main()
{
    json image_formats(json_array_arg, {"JPEG","PSD","TIFF","DNG"});

    json color_spaces(json_array_arg);
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

