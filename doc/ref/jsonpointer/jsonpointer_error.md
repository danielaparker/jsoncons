### jsoncons::jsonpointer::jsonpointer_error

#### Header

    #include <jsoncons/jsonpointer/jsonpointer.hpp>

### Base class

std::exception

<p>
<div class="mxgraph" style="max-width:100%;border:1px solid transparent;" data-mxgraph="{&quot;highlight&quot;:&quot;#0000ff&quot;,&quot;nav&quot;:true,&quot;resize&quot;:true,&quot;toolbar&quot;:&quot;zoom layers lightbox&quot;,&quot;edit&quot;:&quot;_blank&quot;,&quot;xml&quot;:&quot;&lt;mxfile modified=\&quot;2019-02-05T19:18:17.776Z\&quot; host=\&quot;www.draw.io\&quot; agent=\&quot;Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.81 Safari/537.36\&quot; etag=\&quot;Cw4G6wvspfTD_vxoRfpU\&quot; version=\&quot;10.1.8\&quot; type=\&quot;device\&quot;&gt;&lt;diagram id=\&quot;iK-0d-7Yl-5MlnfMu26I\&quot; name=\&quot;Page-1\&quot;&gt;xVXbbtswDP0aP27wZVm6xy7JNqzosCED2j4VgsXZAmTJk+la2ddPqilf4gRZXzogQMQjkiIPj+Qo21T2s2F1eas5yCiNuY2ybZSmSZyu3J9HDj2SrQkojODkNAJ78QdCJKGt4NDMHFFriaKeg7lWCnKcYcwY3c3dfmk5P7VmBSyAfc7kEr0THMsevVrFI/4FRFGGk5OYdioWnAloSsZ1N4GyXZRtjNbYryq7AenJC7z0cZ/O7A6FGVD4LwG7bz+tuvmxL9PN7xu8u60e7Nc3lOWJyZYabtD5X7sf2BxqFFpR+XgInBjdKg4+bRxlH7tSIOxrlvvdzqnAYSVW0lmJW9IBYBDs2cqTgQ8nJNAVoDk4Fwp4RwyShN6T2Y3zGFgvJ7MIYYwkUAyJR5bcgoh6AWnr86SZVqGo4BGc9Mx/Jy75cJm59DWZu1owt+AIFL/299ZZuWRNI/I5LWAF3hODfv3g129XZG3tZGt7CIZyxd+HBN6YRHlzDHu2QlxfHPDFE3E0ANeAbk0OlzWDzBSAly7kcqCTia1ODCxgBiRD8TQv99QU6YTvWrhGzl20ZH2kg75LCpo+NUd5jmQ3aCzk6VlY5HmW1ND0KZU5c3wwe/fxs5Pt/gI=&lt;/diagram&gt;&lt;/mxfile&gt;&quot;}"></div>
<script type="text/javascript" src="https://www.draw.io/js/viewer.min.js"></script>
</p>

#### Constructors

    jsonpointer_error(std::error_code ec)

    jsonpointer_error(const jsonpointer_error& other)

#### Member functions

    const std::error_code code() const
Returns an error code for this exception

    const char* what() const
Returns an error message

### Example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;

int main()
{
    string s = "[1,2,3,4,]";
    try 
    {
        jsoncons::json val = jsoncons::json::parse(s);
    } 
    catch(const jsoncons::jsonpointer_error& e) 
    {
        std::cout << "Caught jsonpointer_error with category " 
                  << e.code().category().name() 
                          << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}
```

Output:
```
Caught jsonpointer_error with category jsoncons.jsonpointer, code 0x6 and message "Name not found"
```
