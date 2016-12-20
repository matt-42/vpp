#pragma once

#include <vpp/core/boxNd.hh>
#include <vpp/core/vector.hh>
#include <vpp/core/make_array.hh>

namespace vpp
{

  template <typename N>
  struct window
  {
    window(N n) : offsets(n) {}

    decltype(auto) operator()() { return offsets(); }
    N offsets;
  };

  // Apply a function on a set of relative coordinates.
  //
  // Example:
  //
  // pixel_wise(relative_access(img)) | [] (auto& a) {
  //     foreach(c4, [&] (vint2 n) { a(n) += a(0,0); });
  // };
  template <typename N, typename F>  
  auto foreach(window<N> n, F f)
  {
    for (int i = 0; i < n().size(); i++)
      f(vint2(n()[i][0], n()[i][1]));
  }

  template <typename F>  
  auto make_window(F f) { return window<F>(f); }

  // Define window with lambda function to allow inlining
  // of neighbor offsets.

  auto c9 = make_window
    ([] () { return vpp::make_array
             (vint2{-1,-1}, vint2{-1, 0}, vint2{-1, 1},
         vint2{ 0, -1}, vint2{ 0, 0}, vint2{0, 1},
         vint2{1,-1}, vint2{1, 0}, vint2{1, 1}); });
    
  auto c8 = make_window
    ([] { return make_array
        (vint2{-1,-1}, vint2{-1, 0}, vint2{-1, 1},
         vint2{ 0, -1}, vint2{0, 1},
         vint2{1,-1}, vint2{1, 0}, vint2{1, 1}); });

  auto c5 = make_window
    ([] { return make_array
        (vint2{-1, 0},
         vint2{ 0, -1}, vint2{ 0, 0}, vint2{0, 1},
         vint2{1, 0}); });

  auto c4 = make_window
    ([] { return make_array
        (vint2{-1, 0},
         vint2{ 0, -1}, vint2{0, 1},
         vint2{1, 0}); });

};
