#pragma once

#include <iod/callable_traits.hh>
#include <vpp/core/relative_accessor.hh>

namespace vpp
{
  using s::_left_to_right_t;
  using s::_right_to_left_t;
  using s::_top_to_bottom_t;
  using s::_bottom_to_top_t;
  
  template <typename I>
  struct relative_access_
  {
    auto first_point_coordinates() { return img.domain().p1(); }
    auto last_point_coordinates() { return img.domain().p2(); }
    I& img;
  };
  
  template <typename I>
  auto relative_access(I i)
  {
    return relative_access_<I>{i};
  }
    
  namespace pixel_wise_internals
  {
    struct iter_info
    {
      int r_start;
      int r_end;
      int c_start;
      int c_end;      
    };

    template <typename V, typename C>
    auto row_access(image2d<V>& img, C r)
    {
      return [row = img[r]] (int c) -> decltype(auto) { return row[c]; };
    }

    template <typename V, typename C>
    auto row_access(const image2d<V>& img, C r)
    {
      return [row = img[r]] (int c) -> decltype(auto) { return row[c]; };
    }

    template <typename C>
    inline decltype(auto) row_access(const box2d&, C r)
    {
      return [r] (int c) { return vint2(r, c); };
    }


    template <typename I, typename C>
    inline decltype(auto) row_access(const relative_access_<I>& ra, C r)
    {
      return [line=&ra.img[r]] (int col)
      {
        return relative_access_kernel<decltype(line)>{line, col};
      };
    }
    
  }

  // Apply \fun to all pixels of a row from left to right.
  template <typename F, typename... R>
  void process_row(_left_to_right_t, F&& fun, int start, int end, R&&... ranges)
  {
    for (int c = start; c <= end; c++)
      fun(ranges(c)...);
  }

  // Apply \fun to all pixels of a row from right to left.
  template <typename F, typename... R>
  void process_row(_right_to_left_t, F&& fun, int start, int end, R&&... ranges)
  {
    for (int c = end; c >= start; c--)
      fun(ranges(c)...);
  }

  // Multithread Top to bottom traversal.
  template <typename F, typename ROW_ORDER, typename... R>
  void pixel_wise_row_first_parallel_2d(_top_to_bottom_t,
                                        ROW_ORDER ro,
                                        pixel_wise_internals::iter_info i,
                                        F&& fun, R&&... ranges)
  {
#pragma omp parallel for
    for (int r = i.r_start; r <= i.r_end; r++)
      process_row(ro, std::forward<F>(fun), i.c_start, i.c_end, pixel_wise_internals::row_access(std::forward<R>(ranges), r)...);
  }

  // Multithread bottom to top traversal.
  template <typename F, typename ROW_ORDER, typename... R>
  void pixel_wise_row_first_parallel_2d(_bottom_to_top_t,
                                        ROW_ORDER ro,
                                        pixel_wise_internals::iter_info i,
                                        F&& fun, R&&... ranges)
  {
#pragma omp parallel for
    for (int r = i.r_end; r >= i.r_start; r--)
      process_row(ro, std::forward<F>(fun), i.c_start, i.c_end, pixel_wise_internals::row_access(std::forward<R>(ranges), r)...);
  }

  // Serial top to bottom traversal.
  template <typename F, typename ROW_ORDER, typename... R>
  void pixel_wise_row_first_serial_2d(_bottom_to_top_t,
                                        ROW_ORDER ro,
                                        pixel_wise_internals::iter_info i,
                                        F&& fun, R&&... ranges)
  {
    for (int r = i.r_end; r >= i.r_start; r--)
      process_row(ro, std::forward<F>(fun), i.c_start, i.c_end, pixel_wise_internals::row_access(std::forward<R>(ranges), r)...);
  }

  // Serial bottom to top traversal.
  template <typename F, typename ROW_ORDER, typename... R>
  void pixel_wise_row_first_serial_2d(_top_to_bottom_t,
                                      ROW_ORDER ro,
                                      pixel_wise_internals::iter_info i, F fun, R... ranges)
  {
    for (int r = i.r_start; r <= i.r_end; r++)
      process_row(ro, fun, i.c_start, i.c_end, pixel_wise_internals::row_access(std::forward<R>(ranges), r)...);
  }

  
  template <typename OPTS, typename... Params>
  struct pixel_wise_impl
  {
    pixel_wise_impl(std::tuple<Params...> t, OPTS opts)
      : ps(t), options(opts)  {}
    
    auto make_orders()
    {
      return D
        (_col_order = iod::static_if<iod::has_symbol<OPTS, _bottom_to_top_t>::value>
         ([] () { return _bottom_to_top; },
          [] () { return _top_to_bottom; }),
         _row_order = iod::static_if<iod::has_symbol<OPTS, _right_to_left_t>::value>
         ([] () { return _right_to_left; },
          [] () { return _left_to_right; }));
    }
    
    template <typename F, std::size_t... I>
    auto run(F fun, std::index_sequence<I...>)
    {
      auto p1 = std::get<0>(ps).first_point_coordinates();
      auto p2 = std::get<0>(ps).last_point_coordinates();

      auto ii = pixel_wise_internals::iter_info{p1[0], p2[0], p1[1], p2[1]};
      
      auto orders = make_orders();
      if (options.has(_no_threads))
        return pixel_wise_row_first_serial_2d(orders.col_order,
                                              orders.row_order,
                                              ii,
                                              fun, std::get<I>(ps)...);
      else
        return pixel_wise_row_first_parallel_2d(orders.col_order,
                                                orders.row_order,
                                                ii,
                                                fun, std::get<I>(ps)...);
    }

    
    template <typename ...A>
    auto operator()(A... options)
    {
      auto new_options = iod::D(options...);
      return pixel_wise_impl<decltype(new_options), Params...>
        (ps, new_options);
    }

    template <typename ...A>
    auto operator()(iod::sio<A...> new_options)
    {
      return pixel_wise_impl<decltype(new_options), Params...>
        (ps, new_options);
    }
    
    template <typename F>
    using kernel_return_type =
      decltype(std::declval<F>()(pixel_wise_internals::row_access(std::declval<Params>(), 0)(0)...));
    
    template <typename F>
    auto operator|(F fun)
    {
      return iod::static_if<std::is_same<kernel_return_type<F>, void>::value>
        (
         // Basic version: fun returns void.
         [this] (auto fun) { this->run(fun, std::make_index_sequence<sizeof...(Params)>()); },

         // Build a image: fun returns a pixel value.
         [this] (auto fun) {

           auto p1 = std::get<0>(ps).first_point_coordinates();
           auto p2 = std::get<0>(ps).last_point_coordinates();

           typedef kernel_return_type<decltype(fun)> value_type;
           image2d<value_type> out(box2d(p1, p2));

           auto ranges = std::tuple_cat(std::make_tuple(out), ps);
           make_pixel_wise_impl(ranges, this->options) |
             [&fun] (auto& o,
                     decltype(pixel_wise_internals::row_access(std::declval<Params>(), 0)(0))... pss)
           { o = fun(pss...); };

           return out;
         },
         fun);         
    }
    
    std::tuple<Params...> ps;
    OPTS options;
  };

  template <typename OPTS, typename... Params>
  auto make_pixel_wise_impl(std::tuple<Params...> ps, OPTS options)
  {
    return pixel_wise_impl<OPTS, Params...>(ps, options);
  }
  
}
