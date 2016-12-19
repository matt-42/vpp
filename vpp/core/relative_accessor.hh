#pragma once

#include <vpp/core/image2d.hh>

namespace vpp
{

  template <typename V>
  struct relative_access_kernel;

  // Provides a fast access to the neighborhood of a point.
  // Easily vectorized by the compiler if used inside a loop.
  //
  // Example:
  //   auto ra = relative_accessor(img, vint2(10, 10));
  //   auto nw = ra(-1, -1); // North west neighbor.
  template <typename V>
  auto relative_accessor(const image2d<V>& img, vint2 p)
  {
    auto line= &img[p[0]];
    int col = p[1];
    return relative_access_kernel<decltype(line)>{line, col};
  };

  template <typename V>
  struct relative_access_kernel
  {
    decltype(auto) operator() (int dr, int dc) { return line[dr][col+dc]; };
    decltype(auto) operator() (vint2 p) { return line[p[0]][col+p[1]]; };
    
    V line;
    int col;
  };
  
}
