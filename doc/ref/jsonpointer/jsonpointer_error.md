### jsoncons::jsonpointer::jsonpointer_error

#### Header

    #include <jsoncons/jsonpointer/jsonpointer.hpp>

### Base class

std::exception

<p>
<div class="mxgraph" style="max-width:100%;border:1px solid transparent;" data-mxgraph="{&quot;highlight&quot;:&quot;#0000ff&quot;,&quot;nav&quot;:true,&quot;resize&quot;:true,&quot;toolbar&quot;:&quot;zoom layers lightbox&quot;,&quot;edit&quot;:&quot;_blank&quot;,&quot;xml&quot;:&quot;&lt;mxfile modified=\&quot;2019-02-05T20:52:54.198Z\&quot; host=\&quot;www.draw.io\&quot; agent=\&quot;Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.81 Safari/537.36\&quot; etag=\&quot;A8xOiWqx-YAc1oq6qbsv\&quot; version=\&quot;10.1.8\&quot; type=\&quot;device\&quot;&gt;&lt;diagram id=\&quot;iK-0d-7Yl-5MlnfMu26I\&quot; name=\&quot;Page-1\&quot;&gt;7Vhtb5swEP41fOwEGNrsY5dknYZSrcukpJ8qC27gCTAzJpD9+pli85qUVo1Eq0aKFN9j39l+7rkLREPzqLhhOAlW1INQM3Wv0NBCM01DNw3xVSL7CkGWWQE+I55c1ABr8g+Up0Qz4kHaWcgpDTlJuqBL4xhc3sEwYzTvLvtNw+6uCfZhAKxdHA7RDfF4UKEzW2/wb0D8QO1s6HImwmqxBNIAezRvQWipoTmjlFejqJhDWJKneKn8vh6ZrQ/GIObPcVje/ipi524dmPO/Dt+sovvi+4WMssNhJi+ccrH+WnygcCHhhMby+HyvOGE0iz0ow+oa+pIHhMM6wW45mwsVCCzgUSgsQwyHx1R7AuNQtCB57BugEXC2F0vk7KVkUErIUIzmTUJq2oNWMiyJYakBv47c0CQGkqkXsHZ1nDWWxZxE8ABCe2xy5kz9rVE3G1A3IAli77qsXGG5IU5T4nZ5gYLwraSwHN+X40+2tBZFa2qxV0YsDr9VAUqj5VWajdujpfyO5iClGXNhXCMcMx/4eAWC1+lDw4y2MmYfSJjCGISYk123ex3KotzhByXiZrVgrK5eTNTTQXVt6dRuNr04n3u6m/XiVLQM4jxKqr70s1Tm71aB6dxu8U/wLzZ3DuwceKqtpfuUQ3S6+hwU4wG5HK1PZE1YnweZM99jffaKZ7Rgn9LMaMFenevz1fWJBir7k9I4KbcD9kZq0+79ds6mLk3rIGkPJ31OO2k3s6ZmzP6wzQwNm9lxTZ2b2aua2eVZZSMqm/SRFvVeH+vXyZfKzDJ6gYxT6UyYzf8A1fLm3xS0/A8=&lt;/diagram&gt;&lt;/mxfile&gt;&quot;}"></div>
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
