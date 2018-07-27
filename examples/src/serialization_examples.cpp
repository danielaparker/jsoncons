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

    json_serializing_options options;

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
    json_serializing_options options1;
    options1.object_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val,options1) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    json_serializing_options options2 ;
    options2 .object_array_split_lines(line_split_kind::new_line);
    std::cout << pretty_print(val,options2 ) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::multi_line)" << std::endl;
    json_serializing_options options3;
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

        json_serializing_options options;
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

        json_serializing_options options1;
        options1.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options1) << std::endl;

        json_serializing_options options;
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
        json_serializing_options options2 ;
        options2 .array_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val2,options2 ) << std::endl;

        std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
        json_serializing_options options4;
        options4.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val2, options4) << std::endl;

        std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
        json_serializing_options options5;
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
    json_serializing_options options6;
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
    json_serializing_options options1;
    options1.array_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val, options1) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    std::cout << "array_array_split_lines(line_split_kind::same_line)" << std::endl;
    json_serializing_options options2 ;
    options2 .object_array_split_lines(line_split_kind::new_line)
           .array_array_split_lines(line_split_kind::same_line);
    std::cout << pretty_print(val, options2 ) << std::endl;

    std::cout << "object_array_split_lines(line_split_kind::new_line)" << std::endl;
    std::cout << "array_array_split_lines(line_split_kind::multi_line)" << std::endl;
    json_serializing_options options3;
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
        json_serializing_options options1;
        options1.array_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options1) << std::endl;

        std::string style2 = "array_array_split_lines(line_split_kind::new_line)";
        std::cout << style2 << std::endl;
        json_serializing_options options2 ;
        options2 .array_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val,options2 ) << std::endl;

        std::string style3 = "array_array_split_lines(line_split_kind::multi_line)";
        std::cout << style3 << std::endl;
        json_serializing_options options3;
        options3.array_array_split_lines(line_split_kind::multi_line);
        std::cout << pretty_print(val,options3) << std::endl;

        std::string style4 = "object_array_split_lines(line_split_kind:same_line)";
        std::cout << style4 << std::endl;
        json_serializing_options options4;
        options4.object_array_split_lines(line_split_kind::same_line);
        std::cout << pretty_print(val,options4) << std::endl;

        std::string style5 = "object_array_split_lines(line_split_kind::new_line)";
        std::cout << style5 << std::endl;
        json_serializing_options options5;
        options5.object_array_split_lines(line_split_kind::new_line);
        std::cout << pretty_print(val,options5) << std::endl;

        std::string style6 = "object_array_split_lines(line_split_kind::multi_line)";
        std::cout << style6 << std::endl;
        json_serializing_options options6;
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

    json_serializer serializer(std::cout, jsoncons::indenting::indent); // pretty print
    serializer.begin_document();
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
    serializer.end_document();
}

void nan_inf_replacement()
{
    json j;
    j["field1"] = std::sqrt(-1.0);
    j["field2"] = 1.79e308 * 1000;
    j["field3"] = -1.79e308 * 1000;

    json_serializing_options options;
    options.nan_replacement("\"NaN\"")
           .pos_inf_replacement("\"Inf\"")
           .neg_inf_replacement("\"-Inf\"");

    std::ostringstream os;
    os << pretty_print(j, options);

    std::cout << "(1)\n" << os.str() << std::endl;

    json j2 = json::parse(os.str(),options);

    std::cout << "\n(2) " << j2["field1"].as<double>() << std::endl;
    std::cout << "(3) " << j2["field2"].as<double>() << std::endl;
    std::cout << "(4) " << j2["field3"].as<double>() << std::endl;

    std::cout << "\n(5)\n" << pretty_print(j2,options) << std::endl;
}

void bignum_serialization_examples1()
{
    std::string s = "-18446744073709551617";

    json j(bignum(s.c_str()));

    std::cout << "(default) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(integer) ";
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(base64) ";
    json_serializing_options options3;
    options3.bignum_format(bignum_chars_format::base64);
    j.dump(std::cout, options3);
    std::cout << "\n\n";

    std::cout << "(base64url) ";
    json_serializing_options options4;
    options4.bignum_format(bignum_chars_format::base64url);
    j.dump(std::cout, options4);
    std::cout << "\n\n";
}

void bignum_serialization_examples2()
{
    std::string s = "-18446744073709551617";

    json j = json::parse(s);

    std::cout << "(1) ";
    j.dump(std::cout);
    std::cout << "\n\n";

    std::cout << "(2) ";
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    j.dump(std::cout, options1);
    std::cout << "\n\n";

    std::cout << "(3) ";
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::base64url);
    j.dump(std::cout, options2);
    std::cout << "\n\n";
}

void serialization_examples()
{
    std::cout << "\nSerialization examples\n\n";
    serialization_example1();
    serialization_example2();
    serialization_example3();
    serialization_example4();
    dump_json_fragments();
    nan_inf_replacement();
    bignum_serialization_examples2();
    bignum_serialization_examples1();
    std::cout << std::endl;
}

