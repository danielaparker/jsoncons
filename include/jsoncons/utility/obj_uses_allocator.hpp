#ifndef JSONCONS_UTILITY_OBJ_USES_ALLOCATOR
#define JSONCONS_UTILITY_OBJ_USES_ALLOCATOR 

#pragma GCC system_header

#include <new>			// for placement operator new
#include <tuple>		// for tuple, make_tuple, make_from_tuple
#include <pair>

namespace jsoncons
{
  template<typename _Tp>
    concept _Std_pair = __is_pair<remove_cv_t<_Tp>>;

  template<typename _Tp, typename _Alloc, typename... _Args>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a,
				     _Args&&... __args) noexcept
    requires (! _Std_pair<_Tp>)
    {
      if constexpr (uses_allocator_v<remove_cv_t<_Tp>, _Alloc>)
	{
	  if constexpr (is_constructible_v<_Tp, allocator_arg_t,
					   const _Alloc&, _Args...>)
	    {
	      return tuple<allocator_arg_t, const _Alloc&, _Args&&...>(
		  allocator_arg, __a, std::forward<_Args>(__args)...);
	    }
	  else
	    {
	      static_assert(is_constructible_v<_Tp, _Args..., const _Alloc&>,
		  "construction with an allocator must be possible"
		  " if uses_allocator is true");

	      return tuple<_Args&&..., const _Alloc&>(
		  std::forward<_Args>(__args)..., __a);
	    }
	}
      else
	{
	  static_assert(is_constructible_v<_Tp, _Args...>);

	  return tuple<_Args&&...>(std::forward<_Args>(__args)...);
	}
    }

  template<_Std_pair _Tp, typename _Alloc, typename _Tuple1, typename _Tuple2>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a, piecewise_construct_t,
				     _Tuple1&& __x, _Tuple2&& __y) noexcept;

  template<_Std_pair _Tp, typename _Alloc>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&) noexcept;

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&, _Up&&, _Vp&&) noexcept;

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&,
				     const pair<_Up, _Vp>&) noexcept;

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&, pair<_Up, _Vp>&&) noexcept;

#if __cplusplus > 202002L
  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&,
				     pair<_Up, _Vp>&) noexcept;

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc&, const pair<_Up, _Vp>&&) noexcept;
#endif // C++23

  template<_Std_pair _Tp, typename _Alloc, typename _Tuple1, typename _Tuple2>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a, piecewise_construct_t,
				     _Tuple1&& __x, _Tuple2&& __y) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::apply([&__a](auto&&... __args1) {
	      return std::uses_allocator_construction_args<_Tp1>(
		  __a, std::forward<decltype(__args1)>(__args1)...);
	  }, std::forward<_Tuple1>(__x)),
	  std::apply([&__a](auto&&... __args2) {
	      return std::uses_allocator_construction_args<_Tp2>(
		  __a, std::forward<decltype(__args2)>(__args2)...);
	  }, std::forward<_Tuple2>(__y)));
    }

  template<_Std_pair _Tp, typename _Alloc>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a),
	  std::uses_allocator_construction_args<_Tp2>(__a));
    }

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a, _Up&& __u, _Vp&& __v)
      noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a,
	    std::forward<_Up>(__u)),
	  std::uses_allocator_construction_args<_Tp2>(__a,
	    std::forward<_Vp>(__v)));
    }

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a,
				     const pair<_Up, _Vp>& __pr) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a, __pr.first),
	  std::uses_allocator_construction_args<_Tp2>(__a, __pr.second));
    }

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a,
				     pair<_Up, _Vp>&& __pr) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 3527. uses_allocator_construction_args handles rvalue pairs
      // of rvalue references incorrectly
      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a,
	    std::get<0>(std::move(__pr))),
	  std::uses_allocator_construction_args<_Tp2>(__a,
	    std::get<1>(std::move(__pr))));
    }

#if __cplusplus > 202002L
  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a,
				     pair<_Up, _Vp>& __pr) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a, __pr.first),
	  std::uses_allocator_construction_args<_Tp2>(__a, __pr.second));
    }

  template<_Std_pair _Tp, typename _Alloc, typename _Up, typename _Vp>
    constexpr auto
    uses_allocator_construction_args(const _Alloc& __a,
				     const pair<_Up, _Vp>&& __pr) noexcept
    {
      using _Tp1 = typename _Tp::first_type;
      using _Tp2 = typename _Tp::second_type;

      return std::make_tuple(piecewise_construct,
	  std::uses_allocator_construction_args<_Tp1>(__a,
	    std::get<0>(std::move(__pr))),
	  std::uses_allocator_construction_args<_Tp2>(__a,
	    std::get<1>(std::move(__pr))));
    }
#endif // C++23

  template<typename _Tp, typename _Alloc, typename... _Args>
    constexpr _Tp
    make_obj_using_allocator(const _Alloc& __a, _Args&&... __args)
    {
      return std::make_from_tuple<_Tp>(
	  std::uses_allocator_construction_args<_Tp>(__a,
	    std::forward<_Args>(__args)...));
    }

  template<typename _Tp, typename _Alloc, typename... _Args>
    constexpr _Tp*
    uninitialized_construct_using_allocator(_Tp* __p, const _Alloc& __a,
					    _Args&&... __args)
    {
      return std::apply([&](auto&&... __xs) {
	return std::construct_at(__p, std::forward<decltype(__xs)>(__xs)...);
      }, std::uses_allocator_construction_args<_Tp>(__a,
	std::forward<_Args>(__args)...));
    }

} // namespace jsoncons

#endif // JSONCONS_UTILITY_OBJ_USES_ALLOCATOR
