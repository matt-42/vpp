#pragma once

#include <vector>
#include <tuple>
#include <Eigen/Dense>
#include <iod/symbols.hh>
#include <vpp/core/image2d.hh>
#include <vpp/core/vector.hh>
#include <vpp/core/liie.hh>

namespace vpp
{

  template <typename T, typename U>
  void euclide_distance_transform(image2d<T>& input, image2d<U>& sedt)
  {
    image2d<vint2> R(input.domain(), _Border = 1);
    fill_with_border(R, vint2{0,0});

    std::vector<vint2> forward4 = {vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{0, -1}};
    std::vector<vint2> backward4 = {vint2{1, 1}, vint2{1, 0}, vint2{1, -1}, vint2{0, 1}};

    fill_with_border(sedt, INT_MAX / 2);

    pixel_wise(input, sedt).eval(_If(_1 > 0) (_2 = INT_MAX) (_2 = 0));

    auto run = [&] (auto neighborhood, auto col_direction,
                    auto row_direction1, auto row_direction2, auto spn) {

      row_wise(sedt, R)(col_direction) | [&] (auto sedt_row, auto R_row)
      {
        // Forward pass
        auto sedt_row_nbh = box_nbh2d<int, 3, 3>(sedt_row);
        auto R_row_nbh = box_nbh2d<vint2, 3, 3>(R_row);
      
        pixel_wise(sedt_row_nbh, R_row_nbh)(row_direction1)
        | [&] (auto& sedt_nbh, auto& R_nbh)
        {
          vint2 min_rel_coord = neighborhood[0];
          int min_dist = INT_MAX;
          for (vint2 nc : neighborhood)
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
            R_nbh(0, 0) = R_nbh(min_rel_coord) + min_rel_coord;
            sedt_nbh(0, 0) = min_dist;
          }
        };

        // Backward pass
        pixel_wise(sedt_row_nbh, R_row_nbh)(row_direction2) | [&] (auto& sedt_nbh, auto& R_nbh)
        {
          int d = sedt_nbh(spn) + 2 * std::abs(R_nbh(spn)[1]) + 1;
          if (d < sedt_nbh(0, 0))
          {
            sedt_nbh(0, 0) = d;
            R_nbh(0, 0) = R_nbh(spn) + spn;
          }
        };

      };

    };

    run(forward4, _Col_forward, _Row_forward, _Row_backward, vint2{0, 1});
    run(backward4, _Col_backward, _Row_backward, _Row_forward, vint2{0, -1});
  }


  template <typename D, typename T, typename U>
  void generic_incremental_distance_transform(image2d<T>& input, image2d<U>& sedt)
  {
    pixel_wise(input, sedt).eval(_If(_1 > 0) (_2 = INT_MAX) (_2 = 0));
    
    auto run = [&] (auto neighborhood, auto col_direction,
                    auto row_direction) {
      auto sedt_nbh = box_nbh2d<int, 3, 3>(sedt);
      pixel_wise(sedt_nbh)(col_direction, row_direction) | [&] (auto sn) {
        int min_dist = sn(0,0);
        for (auto nc : neighborhood) min_dist = sn(nc);
        sn(0,0) = min_dist;
      };
    };

    run(D::forward(), _Col_forward, _Row_forward);
    run(D::backward(), _Col_backward, _Row_backward);
  }

  struct d4_distance_transform_info
  {
    static const auto forward() { return std::vector<vint2>{vint2{-1, 0}, vint2{0, -1}}; };
    static const auto backward() { return std::vector<vint2>{vint2{1, 0}, vint2{0, 1}}; };
  };
  const auto d4_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform<d4_distance_transform_info>(a, b);
  };

  struct d8_distance_transform_info
  {
    static const auto forward() { return std::vector<vint2>{vint2{-1, -1}, vint2{-1, 0}, vint2{-1, 1}, vint2{0, -1}}; };
    static const auto backward() { return std::vector<vint2>{vint2{1, 1}, vint2{1, 0}, vint2{1, -1}, vint2{0, 1}}; };
  };
  
  const auto d8_distance_transform = [] (auto& a, auto& b) {
    generic_incremental_distance_transform<d8_distance_transform_info>(a, b);
  };
  
}
