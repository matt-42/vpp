#pragma once

#include <vpp/core/pixel_wise.hh>

namespace vpp
{
  template <typename OPTS, typename... Params>
  class block_wise_runner
  {
  public:
    typedef block_wise_runner<OPTS, Params...> self;

    block_wise_runner(vint2 block_size, std::tuple<Params...> t, OPTS options = iod::D())
      : block_size_(block_size), ranges_(t), options_(options) {}


    template <typename ...A>
    auto operator()(A... _options)
    {
      auto new_options = iod::D(_options...);
      return block_wise_runner<decltype(new_options), Params...>
        (block_size_, ranges_, new_options);
    }

    template <typename F>
    void operator|(F fun)
    {
      auto p1 = std::get<0>(ranges_).first_point_coordinates();
      auto p2 = std::get<0>(ranges_).last_point_coordinates();
      
      int rstart = p1[0];
      int rend = p2[0];

      int cstart = p1[1];
      int cend = p2[1];
    
      int nr = std::ceil((1 + rend - rstart) / float(block_size_[0]));
      int nc = std::ceil((1 + cend - cstart) / float(block_size_[1]));

      box2d grid(vint2(0, 0),
                 vint2(nr - 1, nc - 1));

      pixel_wise(grid)(options_) | [&] (vint2 block)
      {
        int r1 = rstart + block[0] * block_size_[0];
        int r2 = std::min(rstart + (block[0] + 1) * block_size_[0] - 1, rend);
          
        int c1 = cstart + block[1] * block_size_[1];
        int c2 = std::min(cstart + (block[1] + 1) * block_size_[1] - 1, cend);

        box2d b(vint2{std::min(r1, r2), std::min(c1, c2)},
                vint2{std::max(r1, r2), std::max(c1, c2)});

        internals::apply_args_transform(this->ranges_, fun, [&b] (auto& i) { return i | b; });      
      };
    }

  private:
    vint2 block_size_;
    std::tuple<Params...> ranges_;
    OPTS options_;
  };
  
  template <typename... PS>
  auto block_wise(vint2 block_size, PS&&... params)
  {
    return block_wise_runner<iod::sio<>, PS...>(block_size, std::forward_as_tuple(params...));
  }

  template <typename P0, typename... PS>
  auto row_wise(P0&& a0, PS&&... params)
  {
    auto p1 = a0.first_point_coordinates();
    auto p2 = a0.last_point_coordinates();

    return block_wise(vint2(1, 1 + p2[1] - p1[1]),
                      std::forward<P0>(a0), std::forward<PS>(params)...);
  }
  
}
