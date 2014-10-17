#pragma once

#include <iostream>
#include <iod/iod.hh>
#include <iod/symbol.hh>

#include <vpp/core/imageNd.hh>
#include <vpp/core/image2d.hh>
#include <vpp/core/boxNd.hh>
#include <vpp/core/tuple_utils.hh>

#include <vpp/core/symbols.hh>
#include <vpp/core/block_wise.hh>

namespace vpp
{
  // Backends
  struct openmp {};
  struct pthreads {}; // Todo
  struct cpp_amp {}; // Todo


  template <typename T>
  struct get_row_iterator
  {
    typedef typename T::row_iterator type;
  };

  template <typename T>
  struct get_row_iterator<const T>
  {
    typedef typename T::const_row_iterator type;
  };

  template <typename T>
  using get_row_iterator_t = typename get_row_iterator<std::remove_reference_t<T>>::type;

  template <typename B, typename... Params>
  class parallel_for_pixel_wise_runner;

  using s::row_forward;
  using s::row_backward;
  using s::col_forward;
  using s::col_backward;
  using s::mem_forward;
  using s::mem_backward;
  using s::serial;
  using s::block_size;
  using s::no_threads;

  // Pixel wise takes a variable number of 2d ranges. (Todo: implement Nd version).
  // A range should respect this interface:

  //   vint2 first_point_coordinates() -> the first point coordinates of the range.
  //   vint2 last_point_coordinates() -> the last point of the range.
  //   typedefs row_iterator, const_row_iterator -> a class to iterate on each row of the range.
  //       row_iterator(vint2 p, Range range) -> build an iterator an place it at position p.
  //       operator!=(row_iterator a, row_iterator b) -> return true if a != b.
  //       operator* -> return the current object (pixel, neighborhood access, point coordinates, ...)

  template <typename OPTS, typename... Params>
  class parallel_for_pixel_wise_runner<openmp, OPTS, Params...>
  {
  public:
    typedef parallel_for_pixel_wise_runner<openmp, OPTS, Params...> self;

    parallel_for_pixel_wise_runner(std::tuple<Params...> t, OPTS opts = iod::D())
      : ranges_(t), options_(opts) {}


    template <typename F>
    void run_row_first(F fun);

    template <typename F>
    void run_col_first_parallel(F fun);

    template <typename F>
    void run(F fun, bool parallel)
    {
      if (!options_.has(col_backward) and !options_.has(col_forward))
        run_row_first(fun);
      else
        if (parallel and !options_.has(row_backward) and !options_.has(row_forward))
          run_col_first_parallel(fun);
        else
          run_row_first(fun);
    }

    template <typename ...A>
    auto operator()(A... _options)
    {
      auto new_options = iod::D(_options...);
      return parallel_for_pixel_wise_runner<openmp, decltype(new_options), Params...>
        (ranges_, new_options);
    }


    template <typename F>
    void operator|(F fun) // if fun -> void.
    {
      run(fun, true);
    }

    template <typename F>
    auto operator|(F fun) // if fun -> something != void.
    {
      auto p1 = std::get<0>(ranges_).first_point_coordinates();
      auto p2 = std::get<0>(ranges_).last_point_coordinates();

      // fixme fun_return_type.
      image2d<fun_return_type> out(box2d(p1, p2));
      pixel_wise(std::tuple_cat(out, ranges_))(options_) | [] (auto& o, Params... ps)
      { o = fun(ps...); }

      return out;
    }

    template <typename F>
    void operator<<(F fun)
    {
      run(fun, true);
    }

    template <typename F>
    void operator<(F fun)
    {
      run(fun, false);
    }

  private:
    OPTS options_;
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  auto pixel_wise(PS&&... params)
  {
    return parallel_for_pixel_wise_runner<openmp, iod::iod_object<>, PS...>(std::forward_as_tuple(params...));
  }


  template <typename... PS>
  auto pixel_wise(std::tuple<PS...>& params)
  {
    return parallel_for_pixel_wise_runner<openmp, iod::iod_object<>, PS...>(params);
  }


  template <typename B, typename... Params>
  class parallel_for_row_wise_runner;

  template <typename... Params>
  class parallel_for_row_wise_runner<openmp, Params...>
  {
  public:

    parallel_for_row_wise_runner(std::tuple<Params...> t) : ranges_(t) {}

    template <typename F>
    void run(F fun, bool parallel)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int start = p1[0];
      int end = p2[0];
#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 0)
      for (int r = start; r <= end; r++)
      {
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef typename get_row_iterator<std::remove_reference_t<decltype(range)>>::type IT;
            return IT(vint2(r, p1[1]), range);
          });

        typedef typename get_row_iterator<std::remove_reference_t<decltype(std::get<0>(ranges_))>>::type IT1;
        auto end0_ = IT1(vint2(r, p2[1] + 1), std::get<0>(ranges_));
        auto& cur0_ = std::get<0>(cur_);

        while (cur0_ != end0_)
        {
          internals::apply_args_star(cur_, fun);
          internals::tuple_map(cur_, [this] (auto& it) { it.next(); });
        }

        internals::apply_args_star(cur_, fun);
      }
    }


    template <typename F>
    void operator<<(F fun)
    {
      run(fun, true);
    }

    template <typename F>
    void operator<(F fun)
    {
      run(fun, false);
    }


  private:
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  parallel_for_row_wise_runner<openmp, PS...> row_wise(PS&&... params)
  {
    return parallel_for_row_wise_runner<openmp, PS...>(std::forward_as_tuple(params...));
  }


  template <typename B, typename... Params>
  class parallel_for_col_wise_runner;

  template <typename... Params>
  class parallel_for_col_wise_runner<openmp, Params...>
  {
  public:

    parallel_for_col_wise_runner(std::tuple<Params...> t) : ranges_(t) {}

    template <typename F>
    void run(F fun, bool parallel)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int start = p1[1];
      int end = p2[1];
#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 0)
      for (int c = start; c <= end; c++)
      {
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef typename std::remove_reference_t<decltype(range)>::iterator IT;
            return IT(vint2(p1[0], c), range);
          });

        internals::apply_args_star(cur_, fun);
      }
    }

    template <typename F>
    void operator<<(F fun)
    {
      run(fun, true);
    }

    template <typename F>
    void operator<(F fun)
    {
      run(fun, false);
    }

  private:
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  parallel_for_col_wise_runner<openmp, PS...> col_wise(PS&&... params)
  {
    return parallel_for_col_wise_runner<openmp, PS...>(std::forward_as_tuple(params...));
  }

};

#include <vpp/core/pixel_wise.hpp>
