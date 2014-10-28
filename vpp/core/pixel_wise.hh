#pragma once

#include <iostream>
#include <tuple>
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
  namespace liie {
    template <typename E, typename C>
    auto evaluate(E exp, C& ctx);

    template <typename E, typename C>
    auto evaluate_global_expressions(E exp, C&& ctx);    
  }

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
      : options_(opts), ranges_(t) {}


    template <typename F>
    void run_row_first(F fun);

    template <typename F>
    void run_col_first_parallel(F fun);

    template <typename F>
    void run(F fun, bool parallel)
    {
      if (!options_.has(_Col_backward) and !options_.has(_Col_forward))
        run_row_first(fun);
      else
        if (parallel and !options_.has(_Row_backward) and !options_.has(_Row_forward))
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

    template <typename ...A>
    auto operator()(iod::sio<A...> _options)
    {
      auto new_options = _options;
      return parallel_for_pixel_wise_runner<openmp, decltype(new_options), Params...>
        (ranges_, new_options);
    }

    // Compute the return type of a given kernel function.
    template <typename F, bool Tie>    
    struct kernel_return_type;

    template <typename F>    
    struct kernel_return_type<F, false>
    {
      typedef decltype(std::declval<F>()(*std::declval<get_row_iterator_t<Params>>()...)) type;
    };

    template <typename F>    
    struct kernel_return_type<F, true>
    {
      typedef decltype(std::make_tuple(*std::declval<get_row_iterator_t<Params>>()...))
        tuple_t;

      typedef decltype(std::declval<F>()(1)) type;
    };

    template <typename F>
    using kernel_return_type_t = typename kernel_return_type<F, OPTS::has(s::_Tie_arguments)>::type;

    template <typename P>
    using to_pixel_wise_kernel_argument = decltype(*std::declval<get_row_iterator_t<P>>());

    // template <typename F>
    // using kernel_return_type = decltype(std::declval<F>()(*std::declval<get_row_iterator_t<Params>>()...));

    // if fun -> void.
    template <typename F,
              typename X = std::enable_if_t<std::is_same<kernel_return_type_t<F>, void>::value>>
    void run_function(F fun) 
    {
      run(fun, !options_.has(_No_threads));
    }

    // if fun -> something != void.
    template <typename F,
              typename X = std::enable_if_t<!std::is_same<kernel_return_type_t<F>, void>::value>>
    auto run_function(F fun)
    {
      auto p1 = std::get<0>(ranges_).first_point_coordinates();
      auto p2 = std::get<0>(ranges_).last_point_coordinates();

      typedef kernel_return_type_t<F> fun_return_type;
      image2d<fun_return_type> out(box2d(p1, p2));
      auto ranges = std::tuple_cat(std::make_tuple(out), ranges_);
      pixel_wise(ranges)(options_) |
        [&fun] (auto& o,
                decltype(*std::declval<get_row_iterator_t<Params>>())... ps)
      { o = fun(ps...); };

      return out;
    }

    template <typename E>
    auto run_exp(E&& exp) 
    {
      auto exp2 = liie::evaluate_global_expressions(exp, ranges_);
      //void* x = exp2;
      return (*this) | [&] (decltype(*std::declval<get_row_iterator_t<Params>>())... ps) {
        auto tp = std::make_tuple(ps...);
        return liie::evaluate(exp2, tp);
      };
    }

    template <typename E,
              typename X = std::enable_if_t<std::is_base_of<iod::Exp<E>, E>::value>>
    auto operator|(E&& exp) { return run_exp(exp); }

    template <typename F,
              typename X = std::enable_if_t<!std::is_base_of<iod::Exp<F>, F>::value>>
    auto operator|(F f) { return run_function(f); }
      
  private:
    OPTS options_;
    std::tuple<Params...> ranges_;
  };

  template <typename P, typename... PS,
            typename X>
  auto pixel_wise(P&& p, PS&&... params)
  {
    return parallel_for_pixel_wise_runner<openmp, iod::sio<>, P, PS...>(std::forward_as_tuple(p, params...));
  }

  template <typename... PS>
  auto pixel_wise(std::tuple<PS...>& params)
  {
    return parallel_for_pixel_wise_runner<openmp, iod::sio<>, PS...>(params);
  }

};

#include <vpp/core/pixel_wise.hpp>
