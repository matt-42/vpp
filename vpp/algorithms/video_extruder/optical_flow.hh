#pragma once

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include "gradient_descent.hh"

namespace vpp
{

  namespace ve_internals
  {

    template <typename F>
    int sad_distance(const image2d<F>& i1, const image2d<F>& i2,
                     vint2 a,
                     vint2 b,
                     int winsize,
                     int th)
    {
      int err = 0.f;

      const F* row1 = &i1(a - vint2(winsize/2, 0));
      const F* row2 = &i2(b - vint2(winsize/2, 0));

      const int skip = 1;
      for (int r = -winsize/2; r <= winsize/2 and err <= th; r+=skip)
      {
        for (int c = -winsize/2; c <= winsize/2; c+=skip)
          err += std::abs((row1[c]) - (row2[c])) * skip * skip;

        row1 = (const F*)((const char*) row1 + i1.pitch());
        row2 = (const F*)((const char*) row2 + i2.pitch());
      }

      return err;
    }
    
  }

  template <typename K, typename F>
  inline void
  semi_dense_optical_flow(const K& keypoints,
                          F match_callback,
                          const image2d<unsigned char>& i1,
                          const image2d<unsigned char>& i2)
  {
    const int winsize = 11;
    const int nscales = 4;
    
    auto pf_domain = make_box2d(i1.domain().nrows() / winsize,
                                i1.domain().ncols() / winsize);
    pyramid2d<vint2> pyr_flow_map(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_flow_map_mark(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = 2 * winsize);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = 2 * winsize);
    pyramid2d<int> distance_map(pf_domain, nscales, 2, _border = 1);

    for (int scale = nscales - 1; scale >= 0; scale--)
    {
      int scale_power = std::pow(2, scale);
      auto& i1 = pyr_i1[scale];
      auto& i2 = pyr_i2[scale];


      float PC = 2.1f;

      // Distance function
      auto distance = [&] (vint2 a, vint2 b, int max_distance)
        {
          if (i1.has(a) and i2.has(b))
            return ve_internals::sad_distance(i1, i2, a, b, winsize * PC, max_distance);
          else
            return INT_MAX;
        };
      
      image2d<unsigned char>& flow_map_mark = pyr_flow_map_mark[scale];      
      fill_with_border(flow_map_mark, 0);
      
      // Gradient descent
      #pragma omp parallel for
      for (int i = 0; i < keypoints.size(); i++)
      {
        vint2 p = keypoints[i];

        // Prediction
        vint2 pfm = p / (2 * winsize);
        vint2 prediction = p;
        if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](pfm))
          prediction = p + pyr_flow_map[scale + 1](pfm) * 2;

        // Descent
        auto m = gradient_descent_match(p, prediction, distance, 3);

        // Register match.
        vint2 match = p + m.first;
        pyr_flow_map_mark[scale](p / winsize) = true;
        pyr_flow_map[scale](p / winsize) = match - p;
        distance_map[scale](p / winsize) = m.second;
      }
    }

    for (int pi = 0; pi < keypoints.size(); pi++)
    {
      vint2 pos = keypoints[pi];
      if (pyr_flow_map_mark[0](pos / winsize))
        match_callback(pi, pos + pyr_flow_map[0](pos / winsize),
                       distance_map[0](pos / winsize));
    }
    
  }

}
