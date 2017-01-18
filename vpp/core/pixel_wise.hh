#pragma once

#include <iostream>
#include <tuple>
#include <utility>
#include <iod/sio.hh>
#include <iod/symbol.hh>

#include <vpp/core/imageNd.hh>
#include <vpp/core/image2d.hh>
#include <vpp/core/boxNd.hh>
#include <vpp/core/tuple_utils.hh>

#include <vpp/core/symbols.hh>

namespace vpp
{

  // pixel_wise is an abstraction that simplify writing of fast dense
  // local image processing. To accelerate the processing, it spans
  // the executions on multicore and let the compiler vectorise the
  // loops with SIMD vector instructions.
  //
  // Usage:
  // pixel_wise(img1, ..., imgN)(options...) | [] (auto pixel1, ... auto pixelN) { process the pixels here };
  //
  // Avalaible options:
  //
  // _right_to_left: forward iteration on rows.
  // _left_to_right (default): backward iteration on rows.
  // _top_to_bottom (default): forward iteration on rows.
  // _bottom_to_top: backward iteration on rows.
  // _no_threads: disable multithreading but still let the compiler vectorise the code if possible.
  //
  // Example: pixel_wise(img) | [] (int& i) { i += 1; } // Increment all pixels.
  //

  template <typename OPTS, typename... Params>
  struct pixel_wise_impl;
  
  struct pixel_wise_caller
  {
    template <typename... T>
    inline decltype(auto) operator()(T&&... t)
    {
      return pixel_wise_impl<iod::sio<>, T...>(std::forward_as_tuple(t...), iod::sio<>());
    }
  };

  static pixel_wise_caller pixel_wise;
}


#include <vpp/core/pixel_wise.hpp>
