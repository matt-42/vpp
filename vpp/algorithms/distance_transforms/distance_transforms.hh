#pragma once

#include <vector>
#include <tuple>
#include <array>
#include <Eigen/Dense>
#include <iod/symbols.hh>
#include <vpp/core/image2d.hh>
#include <vpp/core/vector.hh>
#include <vpp/core/make_array.hh>

namespace vpp
{
  
  template <typename T, typename U>
  void euclide_distance_transform(image2d<T>& input, image2d<U>& sedt)
  {
    image2d<vshort2> R(input.domain(), _border = 1);
    fill_with_border(R, vshort2{0,0});

    auto forward4 = [] () { return make_array(vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{0, -1}); };
    auto backward4 = [] () { return make_array(vint2{1, 1}, vint2{1, 0}, vint2{1, -1}, vint2{0, 1}); };

    fill_with_border(sedt, input.nrows() + input.ncols());
    pixel_wise(input, sedt) | [] (auto& i, auto& s) { if (i == 0) s = 0; };

    auto run = [&] (auto neighborhood, auto col_direction,
                    auto row_direction1, auto row_direction2, auto spn) {

      row_wise(sedt, R)(col_direction, _no_threads) | [&] (auto sedt_row, auto R_row)
      {
        // Forward pass
        pixel_wise(relative_access(sedt_row), relative_access(R_row))(row_direction1, _no_threads)
        | [&] (auto sedt_nbh, auto R_nbh)
        {
          vint2 min_rel_coord = neighborhood()[0];
          int min_dist = INT_MAX;
          for (vint2 nc : neighborhood())
          {
            int d = sedt_nbh(nc) + 2 * (std::abs(R_nbh(nc)[0] * nc[0]) +
                                        std::abs(R_nbh(nc)[1] * nc[1]))
              + nc.cwiseAbs().sum();
            
            if (d < min_dist)
            {
              min_dist = d;
              min_rel_coord = nc;
            }
          }

          if (min_dist < sedt_nbh(0, 0))
          {
            R_nbh(0, 0) = (R_nbh(min_rel_coord) + min_rel_coord.cast<short>()).template cast<short>();
            sedt_nbh(0, 0) = min_dist;
          }
        };

        // Backward pass
        pixel_wise(relative_access(sedt_row), relative_access(R_row))
        (row_direction2, _no_threads) | [&] (auto sedt_nbh, auto R_nbh)
        {
          int d = sedt_nbh(spn()) + 2 * std::abs(R_nbh(spn())[1]) + 1;
          if (d < sedt_nbh(0, 0))
          {
            sedt_nbh(0, 0) = d;
            R_nbh(0, 0) = (R_nbh(spn()) + spn().template cast<short>()).template cast<short>();
          }
        };

      };

    };

    run(forward4, _top_to_bottom, _left_to_right, _right_to_left, [] () { return vint2{0, 1}; });
    run(backward4, _bottom_to_top, _right_to_left, _left_to_right, [] () { return vint2{0, -1}; });

    // pixel_wise(sedt) | [] (auto& p) { p/=100; };
  }

  template <unsigned N, typename F>
  void loop_unroll(F f, std::enable_if_t<N == 0>* = 0) { f(N); }

  template <unsigned N, typename F>
  void loop_unroll(F f, std::enable_if_t<N != 0>* = 0) { f(N); loop_unroll<N-1>(f); }
  
  template <typename T, typename U, typename F, typename FW, typename B, typename BW, int WS = 3>
  void generic_incremental_distance_transform(image2d<T>& input, image2d<U>& sedt,
                                              F forward,
                                              FW forward_ws,
                                              B backward,
                                              BW backward_ws,
                                              std::integral_constant<int, WS> = std::integral_constant<int, WS>())
  {
    fill_with_border(sedt, input.nrows() + input.ncols());
    pixel_wise(input, sedt) | [] (auto& i, auto& s) { if (i == 0) s = 0; };
    
    auto run = [&] (auto neighb, auto ws,
                    auto col_direction,
                    auto row_direction) {
      pixel_wise(relative_access(sedt))(col_direction, row_direction, _no_threads) | [neighb, ws] (auto sn) {
        int min_dist = sn(0,0);

        auto nbh = neighb();
        
        // if (neighb().size() < 6)
        auto it = [&] (int i) {
          min_dist = std::min(min_dist, ws()[i] + sn(neighb()[i]));
        };

        typedef decltype(nbh) NBH;
        loop_unroll<std::tuple_size<NBH>::value - 1>(it);
        sn(0,0) = min_dist;
      };
    };

    run(forward, forward_ws, _top_to_bottom, _left_to_right);
    run(backward, backward_ws, _bottom_to_top, _right_to_left);
  }
  
  const auto d4_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform(a, b,
                                 [] () { return make_array(vint2{-1, 0}, vint2{0, -1}); },
                                 [] () { return make_array(1, 1); },
                                 [] () { return make_array(vint2{1, 0}, vint2{0, 1}); },
                                 [] () { return make_array(1, 1); });
  };

  const auto d8_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform(a, b,
                                 [] () { return make_array(vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{0, -1}); },
                                 [] () { return make_array(1,1,1,1); },
                                 [] () { return make_array(vint2{1, 1}, vint2{1, 0}, vint2{1, -1}, vint2{0, 1}); },
                                           [] () { return make_array(1, 1, 1, 1); });
  };

  const auto d3_4_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform(a, b,
                                           [] () { return make_array(vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{0, -1}); },
                                           [] () { return make_array(4,3,4,3); },
                                           [] () { return make_array(vint2{1, 1}, vint2{1, 0}, vint2{1, -1}, vint2{0, 1}); },
                                           [] () { return make_array(4, 3, 4, 3); }, std::integral_constant<int, 5>());
  };

  const auto d5_7_11_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform(a, b,
    
                                           [] () { return make_array(vint2{-2, -1}, vint2{-2, 1}, vint2{-1, -2}, vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{-1, 2}, vint2{0, -1}); },
                                           [] () { return make_array(11,11,11,7,5,7,11,5); },
                                           [] () { return make_array(vint2{0, 1}, vint2{1, -2}, vint2{1, -1}, vint2{1, 0}, vint2{1, 1}, vint2{1, 2}, vint2{2, -1}, vint2{2, 1}); },
                                           [] () { return make_array(5,11,7,5,7,11,11,11); },
                                           std::integral_constant<int, 5>());
  };
  
}
