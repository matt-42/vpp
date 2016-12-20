#pragma once

#include <memory>
#include <vpp/core/image2d.hh>
#include <vpp/core/vector.hh>
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
    decltype(auto) operator[](int i) { return pixels[i]; }
    
    // No copy allowed.
    int size_;
    std::unique_ptr<V[]> pixels;
  };

  template <typename V>
  auto extract_patches(const std::vector<vint2>& kps,
                       const image2d<V>& img, int winsize)
  {
    image2d<V> patches(kps.size(), winsize * winsize);
    for (int ki = 0; ki < kps.size(); ki++)
    {
      vint2 pos = kps[ki];
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
  auto extract_patch(const vint2& pos,
                     const image2d<V>& img, int winsize)
  {
    auto ra = relative_accessor(img, pos);
    patch<V> patch(winsize, winsize);
    const int h = winsize / 2;
    for (int r = -h; r < -h + winsize; r++)
#pragma omp simd
      for (int c = -h; c < -h + winsize; c++)
        patch[r * winsize + c] = ra(r, c);

    return patch;
  }
  
  template <typename V>
  auto extract_patch(const vpp::vfloat2& pos,
                     const image2d<V>& img, int winsize)
  {
    // Fixme add interpolation.
    auto ra = relative_accessor(img, pos.template cast<int>());
    patch<V> patch(winsize, winsize);
    const int h = winsize / 2;
    for (int r = -h; r < -h + winsize; r++)
#pragma omp simd
      for (int c = -h; c < -h + winsize; c++)
        patch[r * winsize + c] = ra(r, c);

    return patch;
  }
  
}
