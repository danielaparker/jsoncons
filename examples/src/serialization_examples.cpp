// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"

using namespace jsoncons;

void serialization_example1()
{
    json val = json::parse(R"(
{
    "sfm_data_version": "0.2",
    "root_path": "D:\\Lagring\\Plugg\\Examensarbete\\Data\\images",
    "views": [],
    "intrinsics": [],
    "extrinsics": [
        {
            "key": 0,
            "value": {
                "rotation": [
                    [
                        0.89280214808572156,
                        0.35067276062587932,
                        -0.28272413998197254
                    ],
                    [
                        -0.090429686592667424,
                        0.75440463553446824,
                        0.65015084224113584
                    ],
                    [
                        0.44127859245183554,
                        -0.5548894131618759,
                        0.70524530697098287
                    ]
                ],
                "center": [
                    -0.60959634064871249,
                    0.24123645392011658,
                    0.57783384588917808
                ]
            }
        }
    ]
}   

)");

    std::cout << "Default pretty print" << std::endl;
    std::cout << pretty_print(val) << std::endl;


    std::cout << "block_style(\"[[\",block_options::next_line)" << std::endl;
    std::cout << "array_object_block_option(block_options::next_line)" << std::endl;
    output_format format1;
    format1.array_array_block_option(block_options::next_line)
           .array_object_block_option(block_options::next_line);
    std::cout << pretty_print(val,format1) << std::endl;
}

void serialization_example2()
{

    json val;

    val["verts"] = json::array{1, 2, 3};
    val["normals"] = json::array{1, 0, 1};
    val["uvs"] = json::array{0, 0, 1, 1};

    std::cout << "Default format" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "Array same line format" << std::endl;
    output_format format1;
    format1.array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val,format1) << std::endl;

    std::cout << "Object array next line format" << std::endl;
    output_format format2;
    format2.object_array_block_option(block_options::next_line);
    std::cout << pretty_print(val,format2) << std::endl;
}

void serialization_example3()
{

    json val = json::parse(R"({
       "verts" : [1, 2, 3],

       "normals" : [1, 0, 1],

       "uvs" : [ 0, 0, 1, 1 ]
    }
)");
    std::cout << "Default" << std::endl;
    std::cout << print(val) << std::endl;

    std::cout << "Default pretty print" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    output_format format1;
    format1.array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val,format1) << std::endl;

    output_format format;
    format.object_object_block_option(block_options::next_line);
    std::cout << pretty_print(val,format) << std::endl;

    json val2 = json::parse(R"(
    {
   "data":
   {
       "item": [[2],[4,5,2,3],[4],[4,5,2,3],[2],[4,5,3],[2],[4,3]],    //A two-dimensional array 
                                                                                           //blank line
       "id": [0,1,2,3,4,5,6,7]                                                   //A one-dimensional array 
   }
    }
)");

    std::cout << "Default" << std::endl;
    std::cout << pretty_print(val2) << std::endl;
 
    std::cout << "object_block_style(\"[[\",block_options::next_line)" << std::endl;
    output_format format2;
    format2.array_array_block_option(block_options::next_line);
    std::cout << pretty_print(val2,format2) << std::endl;

    std::cout << "object_block_style(\"[[\",block_options::same_line)" << std::endl;
    output_format format4;
    format4.array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val2, format4) << std::endl;

    std::cout << "array_block_style(\"[[\",block_options::same_line)" << std::endl;
    output_format format5;
    format5.array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val2, format5) << std::endl;

    json val3 = json::parse(R"(
    {
   "data":
   {
       "item": [[2]]    //A two-dimensional array 
   }
    }
)");
    std::cout << "block_style(\"[[\",block_options::next_line)" << std::endl;
    output_format format6;
    format6.array_array_block_option(block_options::next_line);
    std::cout << pretty_print(val3,format6) << std::endl;
}

void serialization_example4()
{
    json val;
    val["data"]["id"] = json::array{0,1,2,3,4,5,6,7};
    val["data"]["item"] = json::array{json::array{2},
                                      json::array{4,5,2,3},
                                      json::array{4},
                                      json::array{4,5,2,3},
                                      json::array{2},
                                      json::array{4,5,3},
                                      json::array{2},
                                      json::array{4,3}};

    std::cout << "Default array-array block format" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "Same line array-array block format" << std::endl;
    output_format format1;
    format1.array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val, format1) << std::endl;

    std::cout << "Next line object-array and same line array-array format" << std::endl;
    output_format format2;
    format2.object_array_block_option(block_options::next_line)
           .array_array_block_option(block_options::same_line);
    std::cout << pretty_print(val, format2) << std::endl;

    json val3 = json::parse(R"(
    {
   "data":
   {
       "item": [[2]]    //A two-dimensional array 
   }
    }
)");
    std::cout << "block_style(\"[[\",block_options::next_line)" << std::endl;
    output_format format6;
    format6.array_array_block_option(block_options::next_line);
    std::cout << pretty_print(val3,format6) << std::endl;
}

void serialization_examples()
{
    serialization_example1();
    serialization_example2();
    serialization_example3();
    serialization_example4();
}

