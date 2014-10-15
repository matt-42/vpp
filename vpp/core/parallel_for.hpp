#pragma once

#include <vpp/core/parallel_for.hh>

namespace vpp
{
  namespace pixel_wise_internals
  {
    template <bool COL_REVERSE>
    struct loop;

    template <> struct loop<false>
    {
      template <typename F>
      static void run(F f, int row_start, int row_end, bool parallel)
      {
#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 1)
        for (int r = row_start; r <= row_end; r++)
          f(r);
      }
    };

    template <> struct loop<true>
    {
      template <typename F>
      static void run(F f, int row_start, int row_end, bool parallel)
      {
#pragma omp parallel for num_threads (parallel ? omp_get_num_procs() : 1)
        for (int r = row_start; r >= row_end; r--)
          f(r);
      }
    };

  }

  template <typename OPTS, typename... Params>
  template <typename F>
  void
  parallel_for_pixel_wise_runner<openmp, OPTS, Params...>::run_row_first(F fun)
  {
    auto p1 = std::get<0>(ranges_).first_point_coordinates();
    auto p2 = std::get<0>(ranges_).last_point_coordinates();

    int row_start = p1[0];
    int row_end = p2[0];

    constexpr OPTS opts = OPTS();
    constexpr bool row_reverse = opts.has(row_backward) || opts.has(mem_backward);
    constexpr bool col_reverse = opts.has(col_backward) || opts.has(mem_backward);
    const int config[4] = { options_.has(row_backward), options_.has(row_forward),
                            options_.has(col_backward), options_.has(col_forward) };
    const int config_sum = config[0] + config[1] + config[2] + config[3];
    const bool parallel =
      (config_sum == 0 || !((config[0] || config[1]) && 
                            (config[2] || config[3]))) && // no dependency or either row_* or col_* is activated (not both).
      !options_.has(serial); // user did not specify serial

    if (col_reverse)
      std::swap(row_start, row_end);

    auto process_row = [&] (int r)
    {
      int col_start = p1[1];
      int col_end = p2[1];

      if (row_reverse) std::swap(col_start, col_end);
      const int inc = row_reverse ? -1 : 1;
      auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
          typedef typename get_row_iterator<std::remove_reference_t<decltype(range)>>::type IT;
          //typedef get_row_iterator_t<decltype(range)> IT;
          return IT(vint2(r, col_start), range);
        });

      typedef typename get_row_iterator<std::remove_reference_t<decltype(std::get<0>(ranges_))>>::type IT1;
      //typedef get_row_iterator_t<decltype(std::get<0>(ranges_))> IT1;
      auto end0_ = IT1(vint2(r, col_end + inc), std::get<0>(ranges_));
      auto& cur0_ = std::get<0>(cur_);

      while (cur0_ != end0_)
      {
        internals::apply_args_star(cur_, fun);
        internals::tuple_map(cur_, [this, row_reverse] (auto& it) { row_reverse ? it.prev() : it.next(); });
      }
    };

    pixel_wise_internals::loop<OPTS().has(col_backward)>::run(process_row, row_start, row_end, parallel);

  }

  template <typename OPTS, typename... Params>
  template <typename F>
  void
  parallel_for_pixel_wise_runner<openmp, OPTS, Params...>::run_col_first(F fun)
  {
    auto p1 = std::get<0>(ranges_).first_point_coordinates();
    auto p2 = std::get<0>(ranges_).last_point_coordinates();

    int col_start = p1[1];
    int col_end = p2[1];

    const bool row_reverse = options_.has(row_backward) || options_.has(mem_backward);
    const bool col_reverse = options_.has(col_backward) || options_.has(mem_backward);
    const int config[4] = { options_.has(row_backward), options_.has(row_forward),
                            options_.has(col_backward), options_.has(col_forward) };
    const int config_sum = config[0] + config[1] + config[2] + config[3];
    const bool parallel =
      (config_sum == 0 || !((config[0] || config[1]) && 
                            (config[2] || config[3]))) && // no dependency or either row_* or col_* is activated (not both).
      !options_.has(serial); // user did not specify serial

    const int block_size = options_.get(block_size, 64);

    block_wise(vint2{p2[0] - p1[0], block_size}, ranges_)(s::tie_arguments)
      << [this, &fun] (auto& b)
    {
      if (options_.has(col_backward))
        pixel_wise(b)(col_backward) < fun;
      else
        pixel_wise(b) < fun;
    };

  }

}
