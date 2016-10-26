// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <codecvt>
#include <sstream>
#include "jsoncons/json.hpp"

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(JSONTestSuite_test_suite)

BOOST_AUTO_TEST_CASE(test_json)
{
    boost::filesystem::path p("input/JSONTestSuite");

    if (exists(p) && is_directory(p))
    {
        ojson document;

        boost::filesystem::directory_iterator end_iter;
        for (boost::filesystem::directory_iterator dir_itr(p);
            dir_itr != end_iter;
            ++dir_itr)
        {
            if (is_regular_file(dir_itr->status()))
            {
                if (dir_itr->path().extension() == ".json")
                {
                    if (dir_itr->path().filename().c_str()[0] == 'y')
                    {
                        try
                        {
                            boost::filesystem::ifstream is(dir_itr->path());
                            is >> document;
                        }
                        catch (const parse_exception& e)
                        {
                            std::ostringstream os;
                            os << dir_itr->path().filename() << " should pass. " << e.what();
                            BOOST_CHECK_MESSAGE( false, os.str());                        
                        }
                    }
                    else if (dir_itr->path().filename().c_str()[0] == 'n')
                    {
                        boost::filesystem::ifstream is(dir_itr->path());
                        try
                        {
                            is >> document;
                            std::ostringstream os;
                            os << dir_itr->path().filename() << " should fail";
                            BOOST_CHECK_MESSAGE(false, os.str());                        
                        }
                        catch (const parse_exception&)
                        {
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()




