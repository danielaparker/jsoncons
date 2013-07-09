// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef MY_CUSTOM_DATA_HPP
#define MY_CUSTOM_DATA_HPP

#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace jsoncons {

template<> inline
void serialize(json_out_stream& os, const boost::numeric::ublas::matrix<double>& A)
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

}

#endif
