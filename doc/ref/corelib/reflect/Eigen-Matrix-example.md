### Eigen::Matrix example

This example shows how to specialize [json_type_traits](json_type_traits.md) for an  [Eigen matrix class](https://eigen.tuxfamily.org/dox-devel/group__TutorialMatrixClass.html).
It defines separate `json_type_traits` class templates for the dynamic and fixed row/column cases.

```cpp
#include <jsoncons/json.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cassert>

namespace jsoncons {
namespace reflect {

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
        if (!jval.is_array() || jval.size() != RowsAtCompileTime || jval[0].size() != ColsAtCompileTime)
        {
            return result_type{jsoncons::unexpect, conv_errc::conversion_failed};
        }

        matrix_type m(RowsAtCompileTime, ColsAtCompileTime);

        for (std::size_t i = 0; i < jval.size(); ++i)
        {
            const Json& row = jval[i];
            if (row.size() != ColsAtCompileTime)
            {
                return result_type{jsoncons::unexpect, conv_errc::conversion_failed};
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
        if (!jval.is_array() || jval.empty() || jval[0].size() == 0)
        {
            return result_type{jsoncons::unexpect, conv_errc::conversion_failed};
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

int main() 
{
    auto m1 = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>::Random(3, 4);
    std::cout << "m1: " << '\n' << m1 << '\n';

    jsoncons::json j{m1};

    std::cout << jsoncons::pretty_print(j) << "\n";

    assert((j.is<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>>()));
    auto result = j.try_as<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>>();
    if (!result)
    {
        std::cout << result.error().message() << "\n";
        return 1;
    }
    auto& m2(*result);

    std::cout << "m2: " << '\n' << m2 << '\n';

    assert(m1 == m2);
}
```
Output:
```
m1:
 -0.997497   0.617481  -0.299417    0.49321
  0.127171   0.170019   0.791925  -0.651784
 -0.613392 -0.0402539    0.64568   0.717887
[
    [
        -0.9974974822229682,
        0.6174810022278512,
        -0.2994170964690085,
        0.4932096316415906
    ],
    [
        0.12717062898648024,
        0.1700186162907804,
        0.7919248023926511,
        -0.651783806878872
    ],
    [
        -0.6133915219580676,
        -0.04025391399884026,
        0.6456801049836727,
        0.717886898403882
    ]
]
m2:
 -0.997497   0.617481  -0.299417    0.49321
  0.127171   0.170019   0.791925  -0.651784
 -0.613392 -0.0402539    0.64568   0.717887
 ```
