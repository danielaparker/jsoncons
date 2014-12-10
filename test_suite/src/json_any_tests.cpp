// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "my_any_specializations.hpp"

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_exception;
using std::string;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_CASE(test_any)
{
    json obj;
    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;

    //std::cout << A << std::endl;

    std::cout << "Before set" << std::endl;
    obj.set("mydata",json::any(A));
    std::cout << "After set" << std::endl;

    obj.to_stream(std::cout);
    std::cout << "After to_stream" << std::endl;
    std::cout << obj << std::endl;

    matrix<double>& B = obj["mydata"].custom_data<matrix<double>>();
    BOOST_CHECK_EQUAL(A.size1(),B.size1());
    BOOST_CHECK_EQUAL(A.size2(),B.size2());

    //std::cout << B << std::endl;

    for (size_t i = 0; i < B.size1(); ++i)
    {
        for (size_t j = 0; j < B.size2(); ++j)
        {
            B(i,j) += 1;
        }
    }

    BOOST_CHECK_CLOSE(B(0,0),2.0,0.0000001);
    BOOST_CHECK_CLOSE(B(0,1),3.0,0.0000001);
    BOOST_CHECK_CLOSE(B(1,0),4.0,0.0000001);
    BOOST_CHECK_CLOSE(B(1,1),5.0,0.0000001);

    //std::cout << pretty_print(obj) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_any_array)
{
    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;
    matrix<double> B(2,2);
    B(0,0) = 5;
    B(0,1) = 6;
    B(1,0) = 7;
    B(1,1) = 8;

    json arr(json::an_array);
    arr.add(json::any(A));
    arr.add(json::any(B));

    BOOST_CHECK(!arr.is<json::any>());

    json::any a0 = arr[0].as<json::any>();
    BOOST_CHECK(arr[0].is<json::any>());
    matrix<double> C = a0.cast<matrix<double>>();

    BOOST_CHECK_EQUAL(C.size1(),A.size1());
    BOOST_CHECK_EQUAL(C.size2(),A.size2());
    BOOST_CHECK_CLOSE(C(0,0),A(0,0),0.0000001);
    BOOST_CHECK_CLOSE(C(0,1),A(0,1),0.0000001);
    BOOST_CHECK_CLOSE(C(1,0),A(1,0),0.0000001);
    BOOST_CHECK_CLOSE(C(1,1),A(1,1),0.0000001);

    matrix<double>& D = arr[0].any_cast<matrix<double>>();
    BOOST_CHECK_EQUAL(D.size1(),A.size1());
    BOOST_CHECK_EQUAL(D.size2(),A.size2());
    BOOST_CHECK_CLOSE(D(0,0),A(0,0),0.0000001);
    BOOST_CHECK_CLOSE(D(0,1),A(0,1),0.0000001);
    BOOST_CHECK_CLOSE(D(1,0),A(1,0),0.0000001);
    BOOST_CHECK_CLOSE(D(1,1),A(1,1),0.0000001);

    json::any a1 = arr[1].as<json::any>();
    BOOST_CHECK(arr[1].is<json::any>());

    matrix<double>& E = a1.cast<matrix<double>>();
    BOOST_CHECK_EQUAL(E.size1(),B.size1());
    BOOST_CHECK_EQUAL(E.size2(),B.size2());
    BOOST_CHECK_CLOSE(E(0,0),B(0,0),0.0000001);
    BOOST_CHECK_CLOSE(E(0,1),B(0,1),0.0000001);
    BOOST_CHECK_CLOSE(E(1,0),B(1,0),0.0000001);
    BOOST_CHECK_CLOSE(E(1,1),B(1,1),0.0000001);

    std::cout << pretty_print(arr) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_any_add_custom_data)
{
    json arr(json::an_array);
    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;
    matrix<double> B(2,2);
    B(0,0) = 5;
    B(0,1) = 6;
    B(1,0) = 7;
    B(1,1) = 8;

    arr.add_custom_data(A);
    arr.add_custom_data(B);

    std::cout << pretty_print(arr) << std::endl;
}

