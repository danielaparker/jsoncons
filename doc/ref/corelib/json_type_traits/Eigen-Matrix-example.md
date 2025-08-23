### Eigen::Matrix examples

This example shows how to specialize [json_conv_traits](json_conv_traits.md) for an  [Eigen matrix class](https://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html).
It defines separate `json_type_traits` class templates for the dynamic and fixed sized row/column cases.

```cpp
#include <jsoncons/json.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cassert>

namespace jsoncons {

// fixed sized row/columns

template <typename Json, typename Scalar, std::size_t RowsAtCompileTime, std::size_t ColsAtCompileTime>
struct json_type_traits<Json, Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime>>
{
    using allocator_type = typename Json::allocator_type;
    using matrix_type = Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime>;

    static bool is(const Json& jval) noexcept
    {
        if (!jval.is_array() || jval.size() != RowsAtCompileTime)
            return false;

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != ColsAtCompileTime)
            {
                return false;
            }
        }

        return true;
    }

    static matrix_type as(const Json& jval)
    {
        // If error return zero initialized matrix 
        if (!jval.is_array() || jval.size() != RowsAtCompileTime)
        {
            return matrix_type::Zero();
        }

        matrix_type m(RowsAtCompileTime, ColsAtCompileTime);

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != ColsAtCompileTime)
            {
                return matrix_type::Zero();
            }
            for (std::size_t j = 0; j < row.size(); ++j)
            {
                m(i, j) = row[j].as<Scalar>();
            }
        }

        return m;
    }

    static Json to_json(const matrix_type& m, const allocator_type& alloc = allocator_type{})
    {
        Json val{jsoncons::json_array_arg, alloc};
        for (Eigen::Index i = 0; i < m.rows(); ++i)
        {
            Json row{jsoncons::json_array_arg, alloc};
            for (Eigen::Index j = 0; j < m.cols(); ++j)
            {
                row.push_back(m(i, j));
            }
            val.push_back(std::move(row));
        }

        return val;
    }
};

// dynamic sized row/columns

template <typename Json, typename Scalar>
struct json_type_traits<Json, Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>>
{
    using allocator_type = typename Json::allocator_type;
    using matrix_type = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;

    static bool is(const Json& val) noexcept
    {
        if (!val.is_array())
            return false;

        std::size_t cols = val[0].size();

        for (std::size_t i = 0; i < val.size(); ++i)
        {
            const Json& row = val[i];
            if (row.size() != cols)
            {
                return false;
            }
        }

        return true;
    }

    static matrix_type as(const Json& val)
    {
        // If error return default constructed matrix 
        if (!val.is_array() || val.size() == 0)
        {
            return matrix_type{};
        }

        std::size_t cols = val[0].size();
        matrix_type m(val.size(), cols);

        for (std::size_t i = 0; i < val.size(); ++i)
        {
            const Json& row = val[i];
            if (row.size() != cols)
            {
                return matrix_type{};
            }
            for (std::size_t j = 0; j < row.size(); ++j)
            {
                m(i, j) = row[j].template as<Scalar>();
            }
        }

        return m;
    }

    static Json to_json(const matrix_type& m, const allocator_type& alloc = allocator_type{})
    {
        Json val{jsoncons::json_array_arg, alloc};
        for (Eigen::Index i = 0; i < m.rows(); ++i)
        {
            Json row{jsoncons::json_array_arg, alloc};
            for (Eigen::Index j = 0; j < m.cols(); ++j)
            {
                row.push_back(m(i, j));
            }
            val.push_back(std::move(row));
        }

        return val;
    }
};

} // namespace jsoncons
```

#### Fixed-sized matrix example

```cpp
using matrix_type = Eigen::Matrix<double, 3, 4>;

// Don't use auto here! (Random returns proxy)
matrix_type m1 = matrix_type::Random(3, 4);

std::cout << "(1) " << '\n' << m1 << "\n\n";

std::string buffer;
auto options = jsoncons::json_options{}.array_array_line_splits(jsoncons::line_split_kind::same_line);

jsoncons::encode_json_pretty(m1, buffer, options);
std::cout << "(2) " << '\n' << buffer << "\n\n";

auto m2 = jsoncons::decode_json<matrix_type>(buffer);
assert(m1 == m2);

// This should fail, conversion returns a default constructed 3x3 matrix
auto m3by3 = jsoncons::decode_json<Eigen::Matrix<double, 3, 3>>(buffer);
std::cout << "(3)\n" << m3by3 << "\n\n";

auto j1 = jsoncons::json::parse(buffer);
auto m3 = j1.as<matrix_type>();
assert(m1 == m3);

jsoncons::json j2{m1};
assert(j1 == j2);
```
Output:
```
(1)
 -0.997497   0.617481  -0.299417    0.49321
  0.127171   0.170019   0.791925  -0.651784
 -0.613392 -0.0402539    0.64568   0.717887

(2)
[
    [-0.9974974822229682, 0.6174810022278512, -0.2994170964690085, 0.4932096316415906],
    [0.12717062898648024, 0.1700186162907804, 0.7919248023926511, -0.651783806878872],
    [-0.6133915219580676, -0.04025391399884026, 0.6456801049836727, 0.717886898403882]
]

(3)
0 0 0
0 0 0
0 0 0
```

#### Dynamic matrix example

```cpp
using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

// Don't use auto here! (Random returns proxy)
matrix_type m1 = matrix_type::Random(3, 4);
std::cout << "(1) " << '\n' << m1 << "\n\n";

std::string buffer;
auto options = jsoncons::json_options{}.array_array_line_splits(jsoncons::line_split_kind::same_line);

jsoncons::encode_json_pretty(m1, buffer, options);

std::cout << "(2) " << '\n' << buffer << "\n\n";

auto m2 = jsoncons::decode_json<matrix_type>(buffer);

assert(m1 == m2);

auto j1 = jsoncons::json::parse(buffer);
auto m3 = j1.as<matrix_type>();
assert(m1 == m3);

jsoncons::json j2{m1};
assert(j1 == j2);
```
Output:
```
(1)
 -0.997497   0.617481  -0.299417    0.49321
  0.127171   0.170019   0.791925  -0.651784
 -0.613392 -0.0402539    0.64568   0.717887

(2)
[
    [-0.9974974822229682, 0.6174810022278512, -0.2994170964690085, 0.4932096316415906],
    [0.12717062898648024, 0.1700186162907804, 0.7919248023926511, -0.651783806878872],
    [-0.6133915219580676, -0.04025391399884026, 0.6456801049836727, 0.717886898403882]
]
```

### See also

[basic_json](../basic_json.md)
[decode_json](../decode_json.md)  
[encode_json, encode_json_pretty](../encode_json.md)  
