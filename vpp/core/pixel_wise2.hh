#pragma once

#include <iostream>
#include <tuple>
#include <utility>
#include <iod/sio.hh>
#include <iod/symbol.hh>

#include <vpp/core/imageNd.hh>
#include <vpp/core/image2d.hh>
#include <vpp/core/boxNd.hh>
#include <vpp/core/tuple_utils.hh>

#include <vpp/core/symbols.hh>
#include <vpp/core/block_wise.hh>


namespace vpp
{

  namespace pixel_wise_internals
  {
    struct iter_info
    {
      int r_start;
      int r_end;
      int c_start;
      int c_end;      
    };

    template <typename V, unsigned N, typename... C>
    decltype(auto) access(imageNd<V, N>& img, C... c)
    {
      return img(vint<N>(c...));
    }

    template <typename V, unsigned N, typename... C>
    decltype(auto) access(const imageNd<V, N>& img, C... c)
    {
      return img(vint<N>(c...));
    }

    template <unsigned N, typename... C>
    decltype(auto) access(const boxNd<N>&, C... c)
    {
      return vint<N>(c...);
    }
  }

  template <typename F, typename R0, typename... R>
  void pixel_wise_row_first_parallel_2d(pixel_wise_internals::iter_info i,
                                     F&& fun, R0&& range0, R&&... ranges)
  {

    if (i.c_start != 0 and i.r_start != 0)
#pragma omp parallel for
    for (int r = i.r_start; r < i.r_end; r++)
#pragma omp simd
      for (int c = i.c_start; c < i.c_end; c++)
        fun(pixel_wise_internals::access(range0, r, c),
            pixel_wise_internals::access(ranges, r, c)...);
    else
#pragma omp parallel for
    for (int r = 0; r < i.r_end; r++)
#pragma omp simd
      for (int c = 0; c < i.c_end; c++)
        fun(pixel_wise_internals::access(range0, r, c),
            pixel_wise_internals::access(ranges, r, c)...);
  }
  
  template <typename F, typename... R>
  void pixel_wise_row_first_serial_2d(pixel_wise_internals::iter_info i, F fun, R... ranges)
  {
    for (int r = i.r_start; r < i.r_end; r++)
    for (int c = i.c_start; c < i.c_end; c++)
      fun(pixel_wise_internals::access(ranges, r, c)...);
  }

  template <typename... Params>
  struct pixel_wise2_impl
  {
    pixel_wise2_impl(Params&&... t)
      : ps(std::forward_as_tuple(t...)) {}

    template <typename F, std::size_t... I>
    auto run(F fun, std::index_sequence<I...>)
    {
      auto A = std::get<0>(ps);
      return pixel_wise_row_first_parallel_2d(pixel_wise_internals::iter_info{0,A.nrows(), 0, A.ncols()},
                                              fun, std::get<I>(ps)...);
    }
    
    template <typename F>
    void operator|(F fun)
    {
      run(fun, std::make_index_sequence<sizeof...(Params)>());
    }

    std::tuple<Params...> ps;
  };

  template <typename... T>
  decltype(auto) pixel_wise2(T&&... t)
  {
    return pixel_wise2_impl<T...>(std::forward<T>(t)...);
  }
}
