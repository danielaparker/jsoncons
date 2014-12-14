// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef MY_ANY_SPECIALIZATIONS_HPP
#define MY_ANY_SPECIALIZATIONS_HPP

#include "jsoncons/json.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace jsoncons {

template<> inline 
void serialize(json_output_handler& os, const boost::numeric::ublas::matrix<double>& A)
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
