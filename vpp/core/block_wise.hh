#ifndef VPP_CORE_BLOCK_WISE_HH_
# define VPP_CORE_BLOCK_WISE_HH_

# include <tuple>
# include <vpp/core/tuple_utils.hh>
# include <iod/iod.hh>
# include <iod/utils.hh>

namespace vpp
{

  template <typename OPTS, typename V, typename... Params>
  class block_wise_runner
  {
  public:
    typedef block_wise_runner<OPTS, V, Params...> self;

    block_wise_runner(V dims, std::tuple<Params...> t, OPTS options = iod::D())
      : dims_(dims), ranges_(t), options_(options) {}

    template <typename F>
    void run(F fun, bool parallel)
    {
      auto p1 = std::get<0>(ranges_).first_point_coordinates();
      auto p2 = std::get<0>(ranges_).last_point_coordinates();

      int rstart = p1[0];
      int rend = p2[0];

      int cstart = p1[1];
      int cend = p2[1];

      int nr = (rend - rstart) / dims_[0];
      int nc = (cend - cstart) / dims_[1];

#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 1)
      for (int r = 0; r <= nr; r++)
      {
        for (int c = 0; c <= nc; c++)
        {
          box2d b(vint2{rstart + r * dims_[0], cstart + c * dims_[1]},
                  vint2{std::min(rstart + (r + 1) * dims_[0] - 1, rend),
                      std::min(cstart + (c + 1) * dims_[1] - 1, cend)});

          iod::static_if<OPTS::has(s::tie_arguments)>
            ([this, b] (auto& fun) { // tie arguments into a tuple and pass it to fun.
              auto t = internals::tuple_transform(this->ranges_, [&b] (auto& i) { return i | b; });
              fun(t);
              return 0;
            },
              [this, b] (auto& fun) { // Directly apply arguments to fun.
                internals::apply_args_transform(this->ranges_, fun, [&b] (auto& i) { return i | b; });
                return 0;
              }, fun);
        }
      }

    }

    template <typename ...A>
    auto operator()(A... _options)
    {
      auto new_options = iod::D(_options...);
      return block_wise_runner<decltype(new_options), V, Params...>
        (dims_, ranges_, new_options);
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
    V dims_;
    std::tuple<Params...> ranges_;
    OPTS options_;
  };

  template <typename V, typename... PS>
  auto block_wise(V dims, PS&&... params)
  {
    return block_wise_runner<iod::iod_object<>, V, PS...>(dims, std::forward_as_tuple(params...));
  }

  template <typename V, typename... PS>
  auto block_wise(V dims, std::tuple<PS...> params)
  {
    return block_wise_runner<iod::iod_object<>, V, PS...>(dims, params, iod::D());
  }

};

#endif
