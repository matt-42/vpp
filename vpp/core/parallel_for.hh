#ifndef VPP_PARALLEL_FOR_HH_
# define VPP_PARALLEL_FOR_HH_

# include <iostream>
# include <iod/iod2.hh>
# include <iod/symbol.hh>

# include <vpp/core/imageNd.hh>
# include <vpp/core/image2d.hh>
# include <vpp/core/boxNd.hh>
# include <vpp/core/tuple_utils.hh>

iod_define_symbol(s, row_forward);
iod_define_symbol(s, col_forward);
iod_define_symbol(s, row_backward);
iod_define_symbol(s, col_backward);

namespace vpp
{
  // Backends
  struct openmp {};
  struct pthreads {}; // Todo
  struct cpp_amp {}; // Todo

  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p1(imageNd<V, N>& img) { return img.domain().p1(); }
  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p1(const imageNd<V, N>& img) { return img.domain().p1(); }

  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p1(boxNd<N, C>& box) { return box.p1(); }
  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p1(const boxNd<N, C>& box) { return box.p1(); }

  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p2(imageNd<V, N>& img) { return img.domain().p2(); }
  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p2(const imageNd<V, N>& img) { return img.domain().p2(); }

  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p2(boxNd<N, C>& box) { return box.p2(); }
  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p2(const boxNd<N, C>& box) { return box.p2(); }


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

  template <typename OPTS, typename... Params>
  class parallel_for_pixel_wise_runner<openmp, OPTS, Params...>
  {
  public:
    typedef parallel_for_pixel_wise_runner<openmp, OPTS, Params...> self;

    parallel_for_pixel_wise_runner(std::tuple<Params...> t, OPTS opts = iod::iod()) : ranges_(t), options_(opts) {}

    template <typename F>
    void run_row_first(F fun, bool parallel)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int row_start = p1[0];
      int row_end = p2[0];

      if (options_.has(row_forward))
      {
        swap(row_start, row_end);
        std::cout << "Row forward" << std::endl;
      }

#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 1)
      for (int r = row_start;
           options_.has(row_backward) ? r >= row_end : r <= row_end;
           r += options_.has(row_backward) ? -1 : 1)
      {
        int col_start = p1[1];
        int col_end = p2[1];

        const int inc = options_.has(col_backward) ? -1 : 1;
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef get_row_iterator_t<decltype(range)> IT;
            return IT(vint2(r, col_start), range);
          });

        typedef get_row_iterator_t<decltype(std::get<0>(ranges_))> IT1;
        auto end0_ = IT1(vint2(r, col_end + inc), std::get<0>(ranges_));
        auto& cur0_ = std::get<0>(cur_);

        while (cur0_ != end0_)
        {
          internals::apply_args_star(cur_, fun);
          internals::tuple_map(cur_, [this] (auto& it) { options_.has(row_backward) ? it.prev() : it.next(); });
        }
      }

    }

    template <typename F>
    void run(F fun, bool parallel)
    {
      if (!options_.has(col_backward) and !options_.has(col_forward))
        run_row_first(fun, parallel);
      else
        if (!options_.has(row_backward) and !options_.has(row_forward))
          run_col_first(fun, parallel);
        else
        {

        }
    }

    template <typename ...A>
    auto operator()(A... _options)
    {
      auto new_options = iod::iod(_options...);
      return parallel_for_pixel_wise_runner<openmp, decltype(new_options), Params...>
        (ranges_, new_options);
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
  parallel_for_pixel_wise_runner<openmp, iod::iod_object<>, PS...> pixel_wise(PS&&... params)
  {
    return parallel_for_pixel_wise_runner<openmp, iod::iod_object<>, PS...>(std::forward_as_tuple(params...));
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


  using iod::iod;

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

#endif
