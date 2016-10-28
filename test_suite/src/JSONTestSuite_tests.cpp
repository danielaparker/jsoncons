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
#include <fstream>
#include <iostream>
#include <locale>
#include <codecvt>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(JSONTestSuite_test_suite)

BOOST_AUTO_TEST_CASE(test_json)
{
    boost::filesystem::path p("input/JSONTestSuite");

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
                        catch (const parse_exception& e)
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
                            if (dir_itr->path().filename().string().find("n_string_UTF8_surrogate_U+D800.json") != std::string::npos)
                            {
                                /*boost::filesystem::ifstream is(dir_itr->path());
                                ojson document;
                                is >> document;
                                std::ostringstream os;
                                os << dir_itr->path().filename() << " should fail";
                                BOOST_CHECK_MESSAGE(false, os.str());*/
                            }
                            else if (dir_itr->path().filename().string().find("n_string_invalid_utf-8.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_iso_latin_1.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_lone_utf8_continuation_byte.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_overlong_sequence_2_bytes.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_overlong_sequence_6_bytes.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_overlong_sequence_6_bytes_null.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_string_UTF8_surrogate_U+D800.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_structure_object_with_comment.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_single_space.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_structure_no_data.json") == std::string::npos &&
                                dir_itr->path().filename().string().find("n_structure_UTF8_BOM_no_data.json") == std::string::npos) 
                            {
                                boost::filesystem::ifstream is(dir_itr->path());
                                ojson document;
                                is >> document;
                                std::ostringstream os;
                                os << dir_itr->path().filename() << " should fail";
                                BOOST_CHECK_MESSAGE(false, os.str());                        
                            }
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




