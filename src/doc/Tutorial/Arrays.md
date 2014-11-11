    #include "jsoncons/json.hpp"

    using jsoncons::json;
    using jsoncons::pretty_print;

### Creating an array of elements 

    json cities(json::an_array);       // an empty array
    std::cout << cities << std::endl;  // output is "[]"

    cities.add("Toronto");  
    cities.add("Vancouver");
    cities.add(0,"Montreal");  // inserts "Montreal" at beginning of array

    std::cout << cities << std::endl;

The output is

    []
    ["Montreal","Toronto","Vancouver"]

### Creating an array of elements with reserved storage 

    json cities(json::an_array);  
    cities.reserve(10);  // storage is reserved
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    cities.add("Toronto");  
    cities.add("Vancouver");
    cities.add(0,"Montreal");
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    std::cout << cities << std::endl;

The output is

    capacity=10, size=0
    capacity=10, size=3
    ["Montreal","Toronto","Vancouver"]

### Accessing a `json` value as a `std::vector`

    std::string s = "{\"my-array\" : [1,2,3,4]}";
    json val = json::parse_string(s);
    std::vector<int> v = val["my-array"].as<std::vector<int>>();
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ",";
        }
        std::cout << v[i]; 
    }
    std::cout << std::endl;

The output is

    1,2,3,4

### Making an array of size 10 initialized with zeros

    json a = json::make_array<1>(10,0);
	a[1] = 1;
	a[2] = 2;
    std::cout << pretty_print(a) << std::endl;

The output is

    [0,1,2,0,0,0,0,0,0,0]

### Making a two dimensional array of size 3x4 initialized with zeros

    json a = json::make_array<2>(3,4,0);
    a[0][0] = "Tenor";
    a[0][1] = "ATM vol";
	a[0][2] = "25-d-MS";
    a[0][3] = "25-d-RR";
    a[1][0] = "1Y";
    a[1][1] = 0.20;
    a[1][2] = 0.009;
    a[1][3] = -0.006;
    a[2][0] = "2Y";
    a[2][1] = 0.18;
    a[2][2] = 0.009;
    a[2][3] = -0.005;

    std::cout << pretty_print(a) << std::endl;

The output is

    [
        ["Tenor","ATM vol","25-d-MS","25-d-RR"],
        ["1Y",0.2,0.009,-0.006],
        ["2Y",0.18,0.009,-0.005]
    ]

### Making a three dimensional array of size 4x3x2 initialized with zeros

    json a = json::make_array<3>(4,3,2,0);
    a[0][2][0] = 2;
	a[0][2][1] = 3;
    std::cout << pretty_print(a) << std::endl;

The output is

    [
        [
            [0,0],
            [0,0],
            [2,3]
        ],
        [
            [0,0],
            [0,0],
            [0,0]
        ],
        [
            [0,0],
            [0,0],
            [0,0]
        ],
        [
            [0,0],
            [0,0],
            [0,0]
        ]
    ]

