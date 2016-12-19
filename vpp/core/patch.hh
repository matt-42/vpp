#pragma once

#include <memory>
#include <vpp/core/image2d.hh>
#include <vpp/core/relative_accessor.hh>

namespace vpp
{

  // Store a single patch of pixels.
  // Faster than imageNd because it uses unique_ptr instead of smart_ptr.
  // Non copyable.
  //
  // Note: image2d is still faster to store a set of patches because it
  // only call malloc once for all descritors.
  template <typename V>
  struct patch
  {
    patch(int nrows, int ncols) : size_(nrows * ncols), pixels(new V[size_]) {}
    patch(box2d d) : size_(d.nrows() * d.ncols()), pixels(new V[size_]) {}

    int size() { return size_; }
    decltype(auto) operator[] () { return pixels[i]; }
    
    // No copy allowed.
    int size_;
    std::unique_ptr<V[]> pixels;
  };

  template <typename V>
  auto extract_patches(const std::vector<vint2>& kps,
                       const image2d<V>& img, int winsize)
  {
    image2d<V> patches(kps.size(), winsize * winsize);
    for (vint2 pos : kps)
    {
      auto ra = relative_accessor(img, pos);
      const int h = winsize / 2;
      for (int r = -h; r < -h + winsize; r++)
        #pragma omp simd
        for (int c = -h; c < -h + winsize; c++)
          patches[ki][r * winsize + c] = ra(r, c);
    }

    return patches;
  }

  template <typename V>
  auto extract_patch(const vfloat2& p,
                     const image2d<V>& img, int winsize)
  {
    // Fixme add interpolation only if p has a floatting part.
    patch<V> patch(winsize, winsize);
    for (vint2 pos : kps)
    {
      auto ra = relative_accessor(img, pos);
      const int h = winsize / 2;
      for (int r = -h; r < -h + winsize; r++)
        #pragma omp simd
        for (int c = -h; c < -h + winsize; c++)
          patches[ki][r * winsize + c] = ra(r, c);
    }

    return patches;
  }


  template <typename V>
  auto extract_patch(const vint2& p,
                     const image2d<V>& img, int winsize)
  {
    patch<V> patch(winsize, winsize);
    for (vint2 pos : kps)
    {
      auto ra = relative_accessor(img, pos);
      const int h = winsize / 2;
      for (int r = -h; r < -h + winsize; r++)
        #pragma omp simd
        for (int c = -h; c < -h + winsize; c++)
          patches[ki][r * winsize + c] = ra(r, c);
    }

    return patches;
  }
  
}
