### Eigen::Matrix examples

This example shows how to specialize [json_type_traits](json_type_traits.md) for an  [Eigen matrix class](https://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html).
It defines separate `json_type_traits` class templates for the dynamic and fixed sized row/column cases.

```cpp
#include <jsoncons/json.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cassert>

namespace jsoncons {
namespace reflect {

// fixed sized row/columns

template <typename Json, typename Scalar, std::size_t RowsAtCompileTime, std::size_t ColsAtCompileTime>
struct json_conv_traits<Json, Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime>>
{
    using allocator_type = typename Json::allocator_type;
    using matrix_type = Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime>;
    using result_type = conversion_result<matrix_type>;

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

    template <typename Alloc, typename TempAlloc>
    static result_type try_as(const allocator_set<Alloc, TempAlloc>& aset, const Json& jval)
    {
        if (!jval.is_array())
        {
            return result_type{jsoncons::unexpect, conv_errc::not_array};
        }
        if (jval.size() != RowsAtCompileTime)
        {
            return result_type{jsoncons::unexpect, conv_errc::conversion_failed, "Expected " + std::to_string(RowsAtCompileTime) + " rows, found " + std::to_string(jval.size())};
        }

        matrix_type m(RowsAtCompileTime, ColsAtCompileTime);

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != ColsAtCompileTime)
            {
                return result_type{jsoncons::unexpect, conv_errc::conversion_failed, "Expected " + std::to_string(ColsAtCompileTime) + " columns, found " + std::to_string(row.size())};
            }
            for (std::size_t j = 0; j < row.size(); ++j)
            {
                m(i, j) = row[j].as<Scalar>();
            }
        }

        return result_type{std::move(m)};
    }

    template <typename Alloc, typename TempAlloc>
    static Json to_json(const allocator_set<Alloc, TempAlloc>& aset, const matrix_type& m)
    {
        auto jval = jsoncons::make_obj_using_allocator<Json>(aset.get_allocator(), jsoncons::json_array_arg);
        for (Eigen::Index i = 0; i < m.rows(); ++i)
        {
            auto row = jsoncons::make_obj_using_allocator<Json>(aset.get_allocator(), jsoncons::json_array_arg);
            for (Eigen::Index j = 0; j < m.cols(); ++j)
            {
                row.push_back(m(i, j));
            }
            jval.push_back(std::move(row));
        }

        return jval;
    }
};

// dynamic sized row/columns

template <typename Json, typename Scalar>
struct json_conv_traits<Json, Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>>
{
    using allocator_type = typename Json::allocator_type;
    using matrix_type = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    using result_type = conversion_result<matrix_type>;

    static bool is(const Json& jval) noexcept
    {
        if (!jval.is_array())
            return false;

        std::size_t cols = jval[0].size();

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != cols)
            {
                return false;
            }
        }

        return true;
    }

    template <typename Alloc, typename TempAlloc>
    static result_type try_as(const allocator_set<Alloc, TempAlloc>&, const Json& jval)
    {
        if (!jval.is_array())
        {
            return result_type{jsoncons::unexpect, conv_errc::not_array};
        }
        if (jval.empty() || jval[0].size() == 0)
        {
            return result_type{jsoncons::unexpect, conv_errc::conversion_failed, "Invalid matrix"};
        }

        std::size_t cols = jval[0].size();
        matrix_type m(jval.size(), cols);

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != cols)
            {
                return result_type{jsoncons::unexpect, conv_errc::conversion_failed};
            }
            for (std::size_t j = 0; j < row.size(); ++j)
            {
                m(i, j) = row[j].template as<Scalar>();
            }
        }

        return result_type{std::move(m)};
    }

    template <typename Alloc, typename TempAlloc>
    static Json to_json(const allocator_set<Alloc, TempAlloc>& aset, const matrix_type& m)
    {
        auto jval = jsoncons::make_obj_using_allocator<Json>(aset.get_allocator(), jsoncons::json_array_arg);
        for (Eigen::Index i = 0; i < m.rows(); ++i)
        {
            auto row = jsoncons::make_obj_using_allocator<Json>(aset.get_allocator(), jsoncons::json_array_arg);
            for (Eigen::Index j = 0; j < m.cols(); ++j)
            {
                row.push_back(m(i, j));
            }
            jval.push_back(std::move(row));
        }

        return jval;
    }
};

} // namespace reflect
} // namespace jsoncons

#### Fixed size matrix example

using matrix_type = Eigen::Matrix<double, 3, 4>;

// Don't use auto here! (Random returns proxy)
matrix_type m1 = matrix_type::Random(3, 4);
std::cout << "(1) " << '\n' << m1 << "\n\n";

std::string buffer;
auto options = jsoncons::json_options{}.array_array_line_splits(jsoncons::line_split_kind::same_line);

auto wresult = jsoncons::try_encode_json_pretty(m1, buffer, options);
assert(wresult);

std::cout << "(2) " << '\n' << buffer << "\n\n";

auto rresult = jsoncons::try_decode_json<matrix_type>(buffer);
assert(rresult);
auto& m2(*rresult);
assert(m1 == m2);

// This should fail
auto r3by3 = jsoncons::try_decode_json<Eigen::Matrix<double, 3, 3>>(buffer);
if (!r3by3)
{
    std::cout << "(3) " << r3by3.error().message() << "\n\n";
}

auto j1 = jsoncons::json::parse(buffer);
auto mresult = j1.try_as<matrix_type>();
assert(mresult);
auto& m3(*mresult);
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

(3) Expected 3 columns, found 4: Unable to convert into the provided type at line 1 and column 2
```

#### Dynamic matrix example

using matrix_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

// Don't use auto here! (Random returns proxy)
matrix_type m1 = matrix_type::Random(3, 4);
std::cout << "(1) " << '\n' << m1 << "\n\n";

std::string buffer;
auto options = jsoncons::json_options{}.array_array_line_splits(jsoncons::line_split_kind::same_line);

auto wresult = jsoncons::try_encode_json_pretty(m1, buffer, options);
assert(wresult);

std::cout << "(2) " << '\n' << buffer << "\n\n";

auto rresult = jsoncons::try_decode_json<matrix_type>(buffer);
assert(rresult);
auto& m2(*rresult);

assert(m1 == m2);

auto j1 = jsoncons::json::parse(buffer);
auto mresult = j1.try_as<matrix_type>();
assert(mresult);
auto& m3(*mresult);
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
[decode_json, try_decode_json](../decode_json.md)  
[encode_json, encode_json_pretty, try_encode_json, try_encode_json_pretty](../encode_json.md)  
