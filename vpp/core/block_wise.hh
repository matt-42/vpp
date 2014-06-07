#ifndef VPP_CORE_BLOCK_WISE_HH_
# define VPP_CORE_BLOCK_WISE_HH_

# include <tuple>
# include <vpp/core/tuple_utils.hh>

namespace vpp
{

  template <typename V, typename... Params>
  class block_wise_runner
  {
  public:
    typedef block_wise_runner<V, Params...> self;

    block_wise_runner(V dims, std::tuple<Params...> t) : dims_(dims), ranges_(t), step_(1) {}

    template <typename F>
    void run(F fun, bool parallel)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

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
          box2d b(vint2(rstart + r * dims_[0], cstart + c * dims_[1]),
                  vint2(rstart + (r + 1) * dims_[0] - 1, cstart + (c + 1) * dims_[1] - 1));

          internals::apply_args_transform(ranges_, fun, [&b] (auto& i) { return i | b; });
        }
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
    V dims_;
    std::tuple<Params...> ranges_;
    int step_;
  };

  template <typename V, typename... PS>
  block_wise_runner<V, PS...> block_wise(V dims, PS&&... params)
  {
    return block_wise_runner<V, PS...>(dims, std::forward_as_tuple(params...));
  }

};

#endif
