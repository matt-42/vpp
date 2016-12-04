#pragma once

#include <vpp/vpp.hh>
#include "gradient_descent.hh"

namespace vpp
{

  namespace ve_internals
  {

  template <typename F>
  int sad_distance(const image2d<F>& i1, const image2d<F>& i2,
                   vint2 a,
                   vint2 b,
                   const int winsize,
                   const int th)
  {
    //const int winsize = 5;
    int err = 0.f;

    const F* row1 = &i1(a - vint2(winsize/2, 0));
    const F* row2 = &i2(b - vint2(winsize/2, 0));

    const int skip = 1;
    for (int r = -winsize/2; r <= winsize/2 and err <= th; r+=skip)
    {
      int err2 = 0;
      for (int c = -winsize/2; c <= winsize/2; c+=skip)
        err2 += std::abs((row1[c]) - (row2[c])) * skip * skip;

      err += err2;
      row1 = (const F*)((const char*) row1 + i1.pitch());
      row2 = (const F*)((const char*) row2 + i2.pitch());
    }

    return err;
  }

  template <unsigned N, typename F>
  int sad_distance_static_size(const image2d<F>& i1, const image2d<F>& i2,
                   vint2 a,
                   vint2 b,
                   const int th)
  {
    const int winsize = N;
    int err = 0.f;

    const F* row1 = &i1(a - vint2(winsize/2, 0));
    const F* row2 = &i2(b - vint2(winsize/2, 0));

    const int skip = 1;
    for (int r = -winsize/2; r <= winsize/2 and err <= th; r+=skip)
    {
      int err2 = 0;
      for (int c = -winsize/2; c <= winsize/2; c+=skip)
        err2 += std::abs((row1[c]) - (row2[c])) * skip * skip;

      err += err2;
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
                          const image2d<unsigned char>& i2, const int winsize)
  {
  //    const int winsize = 7;
    const int nscales = 4;
    const int min_scale = 0;

    auto pf_domain = make_box2d(i1.domain().nrows() / winsize,
                                i1.domain().ncols() / winsize);
    pyramid2d<vint2> pyr_flow_map(pf_domain, nscales, 2, _border = nscales);
    pyramid2d<unsigned char> pyr_flow_map_mark(pf_domain, nscales, 2, _border = nscales);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = 2 * winsize);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = 2 * winsize);
    pyramid2d<int> distance_map(pf_domain, nscales, 2, _border = nscales);

    assert(pyr_flow_map_mark.size() == nscales);

    for (int scale = nscales - 1; scale >= min_scale; scale--)
    {
      int scale_div= std::pow(2, scale);
      image2d<unsigned char> i1 = pyr_i1[scale];
      image2d<unsigned char> i2 = pyr_i2[scale];

      // i1 = clone(i1, _border = 10);
      // i2 = clone(i2, _border = 10);
      fill_border_mirror(i1);
      fill_border_mirror(i2);

      
      float PC = 1.f;

      // Distance function
      auto distance = [&] (vint2 a, vint2 b, int max_distance)
        {
          if (i1.has(a) and i2.has(b))
          {
            //return ve_internals::sad_distance_static_size<winsize>(i1, i2, a, b, max_distance);
            //return ve_internals::sad_distance(i1, i2, a, b, winsize * PC, max_distance);
            switch (int(winsize * PC))
            {
            case 5: return ve_internals::sad_distance_static_size<5>(i1, i2, a, b, max_distance);
            case 7: return ve_internals::sad_distance_static_size<7>(i1, i2, a, b, max_distance);
            case 9: return ve_internals::sad_distance_static_size<9>(i1, i2, a, b, max_distance);
            case 11: return ve_internals::sad_distance_static_size<11>(i1, i2, a, b, max_distance);
            case 18: return ve_internals::sad_distance_static_size<18>(i1, i2, a, b, max_distance);
            default: return ve_internals::sad_distance(i1, i2, a, b, winsize * PC, max_distance);
            }
          }
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

        // Prediction
        vint2 pfm = p / (2 * winsize);
        vint2 prediction = p;
        if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](pfm))
          prediction = p + pyr_flow_map[scale + 1](pfm) * 2;

        // Descent
        if (!pyr_flow_map_mark[scale](p / winsize))
        {
          pyr_flow_map_mark[scale](p / winsize) = true;
          auto m = gradient_descent_match(p, prediction, distance, 5);

          // Register match.
          vint2 match = p + m.first;
  //        std::cout << (p / winsize).transpose() << " " << pyr_flow_map_mark[scale].domain().nrows() << "x" << pyr_flow_map_mark[scale].domain().ncols()
  //                  << " " << pyr_flow_map_mark[scale].has(p / winsize) << std::endl;
          assert(pyr_flow_map_mark[scale].domain_with_border().has(p / winsize));
          pyr_flow_map[scale](p / winsize) = match - p;
          distance_map[scale](p / winsize) = m.second;
        }
      }

      // Regularisation
      //if (scale != 0)
      const int regularization_niter = 4;
      for (int Ki = 0; Ki < regularization_niter; Ki++)
      {

        auto loop_body = [&] (int r, int c)
          {
            auto& flow_map = pyr_flow_map[scale];
            vint2 p(r, c);
            vint2 pf(r / winsize, c / winsize);

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
                    auto m = gradient_descent_match(p, p + flow_map(pfn), distance, 3);

                    // Register match.
                    vint2 match = p + m.first;
                    if (m.second < d1)
                    {
                      pyr_flow_map_mark[scale](pf) = true;
                      pyr_flow_map[scale](pf) = match - p;
                      distance_map[scale](pf) = m.second;
                    }
                
                  }
                }
              }
          };

        if (Ki % 2)
#pragma omp parallel for
          for (int r = 0; r < i1.nrows(); r+= winsize)
            for (int c = 0; c < i1.ncols(); c+= winsize)
              loop_body(r, c);
        else
#pragma omp parallel for
          for (int r = i1.nrows() - 1; r >= 0; r-= winsize)
            for (int c = i1.ncols() - 1; c >= 0; c-= winsize)
              loop_body(r, c);
      }
      
    }

    for (int pi = 0; pi < keypoints.size(); pi++)
    {
      vint2 pos = keypoints[pi];
      vint2 pos2 = pos / (winsize * std::pow(2, min_scale));
      if (pyr_flow_map_mark[min_scale](pos2))
        match_callback(pi, pos + pyr_flow_map[min_scale](pos2) * std::pow(2, min_scale),
                       distance_map[min_scale](pos2));
    }
    
  }

}
