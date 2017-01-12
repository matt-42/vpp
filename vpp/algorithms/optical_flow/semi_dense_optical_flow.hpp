#pragma once

#include <vpp/vpp.hh>
#include <vpp/algorithms/symbols.hh>
#include <vpp/algorithms/epipolar_geometry.hh>
#include <vpp/algorithms/optical_flow/epipolar_match.hh>
#include "gradient_descent.hh"

namespace vpp
{

  using namespace s;
  
  namespace of_internals
  {

    template <typename F>
    int sad_distance(const image2d<F>& i1, const image2d<F>& i2,
                     vint2 a,
                     vint2 b,
                     const int winsize,
                     const int th)
    {
      int err = 0.f;

      const F* row1 = &i1(a - vint2(winsize/2, winsize / 2));
      const F* row2 = &i2(b - vint2(winsize/2, winsize / 2));

      for (int r = 0; r < winsize and err <= th; r++)
      {
        int err2 = 0;
        #pragma omp simd
        for (int c = 0; c < winsize; c++)
          err2 += std::abs((row1[c]) - (row2[c]));

        err += err2;
        row1 = (const F*)((const char*) row1 + i1.pitch());
        row2 = (const F*)((const char*) row2 + i2.pitch());
      }

      return err;
    }

  }

  template <typename K, typename MC, typename... OPTS>
  inline void
  semi_dense_optical_flow(const K& keypoints,
                          MC match_callback,
                          const image2d<unsigned char>& i1,
                          const image2d<unsigned char>& i2,
                          OPTS... options)
  {
    using Eigen::Matrix3f;
    
    auto opts = D(options...);
    const int winsize = opts.get(_winsize, 7);
    const int nscales = opts.get(_nscales, 4);
    const int min_scale = opts.get(_min_scale, 0);
    const int propagation_niters = opts.get(_propagation, 2);
    const int patchsize = opts.get(_patchsize, 5);
    const bool f_provided = opts.has(_fundamental_matrix);
    const Matrix3f F = opts.get(_fundamental_matrix, Matrix3f::Zero());
    constexpr bool epipolar_flow = opts.has(_epipolar_flow);
    constexpr bool epipolar_filter = opts.has(_epipolar_filter);
    const bool epipolar_filter_th = opts.get(_epipolar_filter, 2);
      
    auto pf_domain = make_box2d(i1.domain().nrows() / patchsize,
                                i1.domain().ncols() / patchsize);
    pyramid2d<vint2> pyr_flow_map(pf_domain, nscales, 2, _border = nscales);
    pyramid2d<unsigned char> pyr_flow_map_mark(pf_domain, nscales, 2, _border = nscales);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = 2 * winsize);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = 2 * winsize);
    pyramid2d<int> distance_map(pf_domain, nscales, 2, _border = nscales);

    assert(pyr_flow_map_mark.size() == nscales);

    // Compute epipole for epipolar flow.
    vfloat2 epipole = epipole_right(F);
    // Scale fundamental matrix.
    std::vector<Matrix3f> Fs(nscales, F);
    for (int scale = nscales - 1; scale >= 0; scale--)
    {
      Matrix3f downscale;
      downscale <<
        2, 2, 1,
        2, 2, 1,
        1, 1, 0.5;
      Fs[scale] = Fs[scale + 1].cwiseProduct(downscale);
    }
    
    for (int scale = nscales - 1; scale >= min_scale; scale--)
    {
      int scale_div= std::pow(2, scale);
      image2d<unsigned char> i1 = pyr_i1[scale];
      image2d<unsigned char> i2 = pyr_i2[scale];

      fill_border_mirror(i1);
      fill_border_mirror(i2);
      
      // Distance function
      auto distance = [&] (vint2 a, vint2 b, int max_distance)
        {
          if (i1.has(a) and i2.has(b))
            return of_internals::sad_distance(i1, i2, a, b, winsize, max_distance);
          else
            return INT_MAX;
        };
      
      image2d<unsigned char>& flow_map_mark = pyr_flow_map_mark[scale];      
      fill_with_border(flow_map_mark, 0);
      
      // Gradient descent
      #pragma omp parallel for
      for (int i = 0; i < keypoints.size(); i++)
      {
        vint2 p = keypoints[i] / scale_div;
        
        // Descent
        if (!pyr_flow_map_mark[scale](p / patchsize))
        {
          vint2 pf = p / patchsize;
          vint2 pfm = p / (2 * patchsize);
          vint2 prediction = p;

          // Multiscale Prediction
          if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](pfm))
            prediction = p + pyr_flow_map[scale + 1](pfm) * 2;
          
          pyr_flow_map_mark[scale](p / patchsize) = 1;

          auto m = iod::static_if<epipolar_flow>
            ([&] { return epipolar_match(p, prediction, epipole, Fs[scale], distance); },
             [&] { return gradient_descent_match(p, prediction, distance, 5); });

          // Register match.
          vint2 match = p + m.flow;
          assert(pyr_flow_map_mark[scale].domain_with_border().has(p / patchsize));
          pyr_flow_map[scale](p / patchsize) = match - p;
          distance_map[scale](p / patchsize) = m.distance;
          pyr_flow_map_mark[scale](p / patchsize) = 2;
        }
      }

      // Propagation
      for (int Ki = 0; Ki < propagation_niters; Ki++)
      {

        auto loop_body = [&] (int r, int c)
          {
            auto& flow_map = pyr_flow_map[scale];
            vint2 p(r, c);
            vint2 pf(r / patchsize, c / patchsize);

            if (!pyr_flow_map_mark[scale](pf)) return;

            vint2 prev_flow = flow_map(pf);
            for (int dr = -1; dr <= 1; dr++)
              for (int dc = -1; dc <= 1; dc++)
              {
                if (!dr and !dc) continue;
                vint2 pfn(pf[0] + dr, pf[1] + dc);
                if (flow_map.has(pfn) and pyr_flow_map_mark[scale](pfn) and
                    (flow_map(pf) - flow_map(pfn)).norm() > 2 and
                    (prev_flow - flow_map(pfn)).norm() > 2)
                {
                  // Two neighbors with high divergence.
                  int d1 = distance_map[scale](pf);
                  int d2 = distance(p, p + flow_map(pfn), INT_MAX);

                  if (d2 < d1)
                  {
                    auto m = iod::static_if<epipolar_flow>
                      ([&] { return epipolar_match(p, vint2(p + flow_map(pfn)), epipole, Fs[scale], distance); },
                       [&] { return gradient_descent_match(p, p + flow_map(pfn), distance, 5); });

                    // Register match.
                    vint2 match = p + m.flow;
                    if (m.distance < d1)
                    {
                      pyr_flow_map_mark[scale](pf) = true;
                      pyr_flow_map[scale](pf) = match - p;
                      distance_map[scale](pf) = m.distance;
                    }
                
                  }
                }
              }
          };

        if (Ki % 2)
#pragma omp parallel for
          for (int r = 0; r < i1.nrows(); r+= patchsize)
            for (int c = 0; c < i1.ncols(); c+= patchsize)
              loop_body(r, c);
        else
#pragma omp parallel for
          for (int r = i1.nrows() - 1; r >= 0; r-= patchsize)
            for (int c = i1.ncols() - 1; c >= 0; c-= patchsize)
              loop_body(r, c);
      }
      
    }

    for (int pi = 0; pi < keypoints.size(); pi++)
    {
      vint2 pos = keypoints[pi];
      vint2 pos2 = pos / int(patchsize * std::pow(2, min_scale));
      if (pyr_flow_map_mark[min_scale].has(pos2) and pyr_flow_map_mark[min_scale](pos2))
        match_callback(pi, pos + pyr_flow_map[min_scale](pos2) * int(std::pow(2, min_scale)),
                       distance_map[min_scale](pos2));
    }
    
  }

}
