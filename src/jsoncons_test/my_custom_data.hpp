// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef MY_CUSTOM_DATA_HPP
#define MY_CUSTOM_DATA_HPP

#include "jsoncons/json1.hpp"
#include "jsoncons/json_serializer.hpp"

namespace jsoncons {

template<>
struct custom_serialization<char, matrix<double>>
{
    static void serialize(json_out_stream& os,
                          const matrix<double>& A)
    {
        os.begin_array();
        for (size_t i = 0; i < A.size1(); ++i)
        {
            os.begin_array();
            for (size_t j = 0; j < A.size2(); ++j)
            {
                os.value(A(i, j));
            }
            os.end_array();
        }
        os.end_array();
    }
};

}

#endif
