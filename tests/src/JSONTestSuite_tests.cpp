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
#include <sstream>
#include <jsoncons/json.hpp>
#include <fstream>
#include <iostream>
#include <locale>
#if defined(_MSC_VER)
#include <codecvt>
#endif
using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(JSONTestSuite_tests)

#if defined(_MSC_VER)
BOOST_AUTO_TEST_CASE(test_json)
{
    boost::filesystem::path p("./input/JSONTestSuite");

    if (exists(p) && is_directory(p))
    {

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
                            if (dir_itr->path().filename().string().find("utf16") == std::string::npos)
                            {
                                boost::filesystem::ifstream is(dir_itr->path());
                                json document;
                                is >> document;
                            }
                            else
                            {
                                std::wifstream fin(dir_itr->path().c_str(), std::ios::binary);
                                // apply BOM-sensitive UTF-16 facet
                                fin.imbue(std::locale(fin.getloc(),
                                                      new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
                                //boost::filesystem::wifstream is(dir_itr->path());
                                wjson document;
                                fin >> document;
                            }
                        }
                        catch (const parse_error& e)
                        {
                            std::ostringstream os;
                            os << dir_itr->path().filename() << " should pass. " << e.what();
                            BOOST_CHECK_MESSAGE( false, os.str());                        
                        }
                    }
                    else if (dir_itr->path().filename().c_str()[0] == 'n')
                    {
                        try
                        {
                            std::ifstream is(dir_itr->path().c_str());

                            json_decoder<ojson> decoder;
                            strict_parse_error_handler err_handler;
                            json_reader reader(is, decoder, err_handler);
                            reader.read();
                            std::ostringstream os;
                            os << dir_itr->path().filename() << " should fail";
                            BOOST_CHECK_MESSAGE(false, os.str());  
                        }
                        catch (const parse_error&)
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
#endif

BOOST_AUTO_TEST_SUITE_END()




