// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>

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


    std::cout << "array_array_split_lines(line_split_kind::new_line)" << std::endl;
    std::cout << "array_object_split_lines(line_split_kind::new_line)" << std::endl;

    serialization_options options;

    options.array_array_split_lines(line_split_kind::new_line)
          .array_object_split_lines(line_split_kind::new_line);
    std::cout << pretty_print(val,options) << std::endl;
}

void serialization_example2()
{

    json val;

    val["verts"] = json::array{1, 2, 3};
    val["normals"] = json::array{1, 0, 1};
    val["uvs"] = json::array{0, 0, 1, 1};

    std::cout << "Default object-array same line options" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::same_line)" << std::endl;
    serialization_options options1;
    options1.object_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val,options1) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    serialization_options options2 ;
    options2 .object_array_split_lines(line_split_kind::new_line);
    std::cout << pretty_print(val,options2 ) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::multi_line)" << std::endl;
    serialization_options options3;
    options3.object_array_split_lines(line_split_kind::multi_line);
    std::cout << pretty_print(val,options3) << std::endl;
}

void serialization_example3()
{
    {
        json val = json::parse(R"(
        [
            {"first-name" : "John",
             "last-name" : "Doe"},
            {"first-name" : "Jane",
             "last-name" : "Doe"}
        ]
        )");

        serialization_options options;
        options.array_object_split_lines(line_split_kind::same_line);
        std::cout << "array_object_split_lines(line_split_kind::same_line)" << std::endl;
        std::cout << pretty_print(val,options) << std::endl;
    }

    {
        json val = json::parse(R"({
           "verts" : [1, 2, 3],

           "normals" : [1, 0, 1],

           "uvs" : [ 0, 0, 1, 1 ]
        }
    )");
        std::cout << "Default print" << std::endl;
        std::cout << print(val) << std::endl;

        std::cout << "Default pretty print" << std::endl;
        std::cout << pretty_print(val) << std::endl;

        serialization_options options1;
        options1.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options1) << std::endl;

        serialization_options options;
        options.object_object_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val,options) << std::endl;
    }

    {
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
     
        std::cout << "array_array_split_lines(line_split_kind::new_line)" << std::endl;
        serialization_options options2 ;
        options2 .array_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val2,options2 ) << std::endl;

        std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
        serialization_options options4;
        options4.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val2, options4) << std::endl;

        std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
        serialization_options options5;
        options5.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val2, options5) << std::endl;
    }

    json val3 = json::parse(R"(
    {
   "data":
   {
       "item": [[2]]    //A two-dimensional array 
   }
    }
)");
    std::cout << "array_array_split_lines(line_split_kind::new_line)" << std::endl;
    serialization_options options6;
    options6.array_array_split_lines(line_split_kind::new_line);
    std::cout << pretty_print(val3,options6) << std::endl;
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

    std::cout << "Default array-array split line options" << std::endl;
    std::cout << pretty_print(val) << std::endl;

    std::cout << "Array-array same line options" << std::endl;
    serialization_options options1;
    options1.array_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val, options1) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
    serialization_options options2 ;
    options2 .object_array_split_lines(line_split_kind::new_line)
           .array_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val, options2 ) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    std::cout << "array_array_split_lines(line_split_kind::multi_line)" << std::endl;
    serialization_options options3;
    options3.object_array_split_lines(line_split_kind::new_line)
           .array_array_split_lines(line_split_kind::multi_line);
    std::cout << pretty_print(val, options3) << std::endl;

    {
        json val = json::parse(R"(
        {
       "header" : {"properties": {}},
       "data":
       {
           "tags" : [],
           "id" : [1,2,3],
           "item": [[1,2,3]]    
       }
        }
    )");
        std::cout << "Default" << std::endl;
        std::cout << pretty_print(val) << std::endl;

        std::string style1 = "array_array_split_lines(line_split_kind:same_line)";
        std::cout << style1 << std::endl;
        serialization_options options1;
        options1.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options1) << std::endl;

        std::string style2 = "array_array_split_lines(line_split_kind::new_line)";
        std::cout << style2 << std::endl;
        serialization_options options2 ;
        options2 .array_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val,options2 ) << std::endl;

        std::string style3 = "array_array_split_lines(line_split_kind::multi_line)";
        std::cout << style3 << std::endl;
        serialization_options options3;
        options3.array_array_split_lines(line_split_kind::multi_line);
        std::cout << pretty_print(val,options3) << std::endl;

        std::string style4 = "object_array_split_lines(line_split_kind:same_line)";
        std::cout << style4 << std::endl;
        serialization_options options4;
        options4.object_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options4) << std::endl;

        std::string style5 = "object_array_split_lines(line_split_kind::new_line)";
        std::cout << style5 << std::endl;
        serialization_options options5;
        options5.object_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val,options5) << std::endl;

        std::string style6 = "object_array_split_lines(line_split_kind::multi_line)";
        std::cout << style6 << std::endl;
        serialization_options options6;
        options6.object_array_split_lines(line_split_kind::multi_line);
        std::cout << pretty_print(val,options6) << std::endl;
    }
}

void dump_json_fragments()
{
    const json some_books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        }
    ]
    )");

    const json more_books = json::parse(R"(
    [
        {
            "title" : "A Wild Sheep Chase: A Novel",
            "author" : "Haruki Murakami",
            "price" : 9.01
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer",
            "price" : 8.00
        }
    ]
    )");

    json_serializer serializer(std::cout, jsoncons::pretty_printer()); // pretty print
    serializer.begin_json();
    serializer.begin_array();
    for (const auto& book : some_books.array_range())
    {
        book.dump_fragment(serializer);
    }
    for (const auto& book : more_books.array_range())
    {
        book.dump_fragment(serializer);
    }
    serializer.end_array();
    serializer.end_json();
}

void serialization_examples()
{
    std::cout << "\nSerialization examples\n\n";
    serialization_example1();
    serialization_example2();
    serialization_example3();
    serialization_example4();
    dump_json_fragments();
    std::cout << std::endl;
}

