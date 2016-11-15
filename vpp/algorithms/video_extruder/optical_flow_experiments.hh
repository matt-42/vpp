#pragma once

#include <vpp/vpp.hh>
#include <vpp/utils/opencv_bridge.hh>
#include <vpp/algorithms/filters/scharr.hh>
#include "gradient_descent.hh"

namespace vpp
{

  template <typename F>
  int ssd_distance(const image2d<F>& i1, const image2d<F>& i2,
                   vint2 a,
                   vint2 b,
                   int winsize,
                   int th)
  {
    int err = 0.f;
    //const int winsize = 13;

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


  template <typename F>
  int ve_distance(const image2d<F>& i1, const image2d<F>& i2,
                  vint2 a,
                  vint2 b,
                  const int* offsets1,
                  const int* offsets2,
                  int offsets_len,
                  int th)
  {
    int err = 0.f;

    const F* row1 = &i1(a);
    const F* row2 = &i2(b);

    for (int i = 0; i < offsets_len and err <= th; i++)
      err += std::abs(row1[offsets1[i]] - row2[offsets2[i]]);

    return err;
  }
  
  void video_extruder_optical_flow(const image2d<unsigned char>& i1,
                                   const image2d<unsigned char>& i2,
                                   const std::vector<vint2>& pts,
                                   std::vector<vint2>& next_pts,
                                   const int nscales = 2,
                                   const int winsize = 11)
  {

    pyramid2d<vint2> pyr_flow_map(i1.domain(), nscales, 2);
    pyramid2d<unsigned char> pyr_flow_map_mark(i1.domain(), nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = winsize / 2);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = winsize / 2);

    
    next_pts.resize(pts.size());
    for (auto& p : next_pts) p = vint2(-1, -1);

    profiler prof;

    prof.begin("of_body");
    for (int scale = nscales - 1; scale >= 0; scale--)
    {
      auto& i1 = pyr_i1[scale];
      auto& i2 = pyr_i2[scale];

      const vint2 circle_r3[16] = {
        {-3, 0}, {-3, 1}, {-2, 2}, { -1, 3},
        { 0, 3}, { 1, 3}, { 2, 2}, {  3, 1},
        { 3, 0}, { 3,-1}, { 2,-2}, {  1,-3},
        { 0,-3}, {-1,-3}, {-2,-2}, { -3,-1}
      };

      int r3_offsets1[16];
      int r3_offsets2[16];

      for (int i = 0; i < 16; i++)
      {
        vint2 o = circle_r3[i];
        r3_offsets1[i] = (&i1(3,3)) - (&i1(vint2(3,3) + 2 * circle_r3[i]));
        r3_offsets2[i] = (&i2(3,3)) - (&i2(vint2(3,3) + 2 * circle_r3[i]));
      }
      
      image2d<unsigned char>& flow_map_mark = pyr_flow_map_mark[scale];      
      fill_with_border(flow_map_mark, 0);
      
      // Gradient descent
      #pragma omp parallel for
      for (int r = 0; r < i1.nrows(); r++)
      {
        for (int c = 0; c < i1.ncols(); c++)
        {
          vint2 p(r, c);

          // Distance function
          auto distance = [&] (vint2 a, vint2 b, int max_distance)
            {
              if (i1.has(a) and i2.has(b))
              {
                //return ssd_distance(i1, i2, a, b, winsize, max_distance);
                return ve_distance(i1, i2, a, b, r3_offsets1, r3_offsets2,
                                   sizeof(r3_offsets2) / sizeof(int), max_distance);
              }
              else
                return INT_MAX;
            };

          // Prediction
          vint2 prediction = p;
          if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](p / 2))
            prediction = p + pyr_flow_map[scale + 1](p / 2) * 2;

          // Descent
          auto m = gradient_descent_match(p, prediction, distance, 10);

          // Register match.
          vint2 match = p + m.first;
          if (m.second < (winsize * winsize * 15) and i1.domain().has(match))
            //if (m.second < (16 * 15) and i1.domain().has(match))
          {
            pyr_flow_map_mark[scale](p) = true;
            pyr_flow_map[scale](p) = match - p;
          }
        }
      }
      
    }
    prof.end("of_end");

    // std::cout << prof << std::endl;
#pragma omp parallel for
    for (int pi = 0; pi < next_pts.size(); pi++)
    {
      if (pyr_flow_map_mark[0](pts[pi]))
        next_pts[pi] = pts[pi] + pyr_flow_map[0](pts[pi]);
      else
        next_pts[pi] = vint2(-1, -1);
    }
    
  }
  

  void video_extruder_optical_flow2(const image2d<unsigned char>& i1,
                                    const image2d<unsigned char>& i2,
                                    const std::vector<vint2>& pts,
                                    std::vector<vint2>& next_pts,
                                    const int nscales = 2,
                                    const int winsize = 11,
                                    const int regularization_niter = 3)
  {
    auto pf_domain = make_box2d(i1.domain().nrows() / winsize,
                                i1.domain().ncols() / winsize);
    pyramid2d<vint2> pyr_flow_map(pf_domain, nscales, 2, _border = 1);
    pyramid2d<int> distance_map(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_flow_map_mark(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = 2 * winsize);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = 2 * winsize);

    next_pts.resize(pts.size());
    for (auto& p : next_pts) p = vint2(-1, -1);

    profiler prof;

    prof.begin("of_body");
    for (int scale = nscales - 1; scale >= 0; scale--)
    {
      int scale_power = std::pow(2, scale);
      auto& i1 = pyr_i1[scale];
      auto& i2 = pyr_i2[scale];


      // const vint2 circle_r3[16] = {
      //   {-3, 0}, {-3, 1}, {-2, 2}, { -1, 3},
      //   { 0, 3}, { 1, 3}, { 2, 2}, {  3, 1},
      //   { 3, 0}, { 3,-1}, { 2,-2}, {  1,-3},
      //   { 0,-3}, {-1,-3}, {-2,-2}, { -3,-1}
      // };

      // int r3_offsets1[16];
      // int r3_offsets2[16];

      // for (int i = 0; i < 16; i++)
      // {
      //   vint2 o = circle_r3[i];
      //   r3_offsets1[i] = (&i1(3,3)) - (&i1(vint2(3,3) + 2 * circle_r3[i]));
      //   r3_offsets2[i] = (&i2(3,3)) - (&i2(vint2(3,3) + 2 * circle_r3[i]));
      // }

      float PC = 2.1f;

      // Distance function
      auto distance = [&] (vint2 a, vint2 b, int max_distance)
        {
          if (i1.has(a) and i2.has(b))
          {
            return ssd_distance(i1, i2, a, b, winsize * PC, max_distance);
            // return ve_distance(i1, i2, a, b, r3_offsets1, r3_offsets2,
            //                    sizeof(r3_offsets2) / sizeof(int), max_distance);
          }
          else
            return INT_MAX;
        };
      
      image2d<unsigned char>& flow_map_mark = pyr_flow_map_mark[scale];      
      fill_with_border(flow_map_mark, 0);
      
      // Gradient descent
      #pragma omp parallel for
      for (int r = 0; r < i1.nrows(); r+=winsize)
      {
        for (int c = 0; c < i1.ncols(); c+=winsize)
        {
          vint2 p(r, c);

          // Prediction
          vint2 pfm = p / (2 * winsize);
          vint2 prediction = p;
          if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](pfm))
            prediction = p + pyr_flow_map[scale + 1](pfm) * 2;

          // Descent
          auto m = gradient_descent_match(p, prediction, distance, 3);

          // Register match.
          vint2 match = p + m.first;
          //if (m.second < (PC * PC * winsize * winsize * 40) and i1.domain().has(match))
          //if (m.second < (16 * 15) and i1.domain().has(match))
          {
            pyr_flow_map_mark[scale](p / winsize) = true;
            pyr_flow_map[scale](p / winsize) = match - p;
            distance_map[scale](p / winsize) = m.second;
          }
        }
      }

      // Regularisation
      //if (scale != 0)
      for (int K = 0; K < regularization_niter; K++)
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
                  //int dd2 = distance_map[scale](pfn);

                  if (d2 < d1
                      //dd2 <= d1
                      )
                  {
                    auto m = gradient_descent_match(p, p + flow_map(pfn), distance, 3);
                    //auto m = std::make_pair(flow_map(pfn), d2);
                    // Register match.
                    vint2 match = p + m.first;
                    if (m.second < d1 // and
                        // m.second < (PC * PC * winsize * winsize * 40) and i1.domain().has(match)
                        )
                      //if (m.second < (16 * 15) and i1.domain().has(match))
                    {
                      pyr_flow_map_mark[scale](pf) = true;
                      pyr_flow_map[scale](pf) = match - p;
                      distance_map[scale](pf) = m.second;
                    }
                
                  }
                }
              }
          };

        if (K % 2)
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
    prof.end("of_body");

    // std::cout << prof << std::endl;
#pragma omp parallel for
    for (int pi = 0; pi < next_pts.size(); pi++)
    {
      if (pyr_flow_map_mark[0](pts[pi] / winsize))
        next_pts[pi] = pts[pi] + pyr_flow_map[0](pts[pi] / winsize);
      else
        next_pts[pi] = vint2(-1, -1);
    }
    
  }

  
  void video_extruder_optical_flow3(const image2d<unsigned char>& i1,
                                    const image2d<unsigned char>& i2,
                                    const std::vector<vint2>& pts,
                                    std::vector<vint2>& next_pts,
                                    const int nscales = 2,
                                    const int winsize = 11,
                                    const int regularization_niter = 3)
  {
    auto pf_domain = make_box2d(i1.domain().nrows() / winsize,
                                i1.domain().ncols() / winsize);
    pyramid2d<vint2> pyr_flow_map(pf_domain, nscales, 2, _border = 1);
    pyramid2d<int> distance_map(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_flow_map_mark(pf_domain, nscales, 2, _border = 1);
    pyramid2d<unsigned char> pyr_i1(i1, nscales, 2, _border = 2 * winsize);
    pyramid2d<unsigned char> pyr_i2(i2, nscales, 2, _border = 2 * winsize);

    int winsize2 = winsize * winsize;
    
    next_pts.resize(pts.size());
    for (auto& p : next_pts) p = vint2(-1, -1);

    profiler prof;

    prof.begin("of_body");
    for (int scale = nscales - 1; scale >= 0; scale--)
    {
      int scale_power = std::pow(2, scale);
      auto& i1 = pyr_i1[scale];
      auto& i2 = pyr_i2[scale];


      // const vint2 circle_r3[16] = {
      //   {-3, 0}, {-3, 1}, {-2, 2}, { -1, 3},
      //   { 0, 3}, { 1, 3}, { 2, 2}, {  3, 1},
      //   { 3, 0}, { 3,-1}, { 2,-2}, {  1,-3},
      //   { 0,-3}, {-1,-3}, {-2,-2}, { -3,-1}
      // };

      // int r3_offsets1[16];
      // int r3_offsets2[16];

      // for (int i = 0; i < 16; i++)
      // {
      //   vint2 o = circle_r3[i];
      //   r3_offsets1[i] = (&i1(3,3)) - (&i1(vint2(3,3) + 2 * circle_r3[i]));
      //   r3_offsets2[i] = (&i2(3,3)) - (&i2(vint2(3,3) + 2 * circle_r3[i]));
      // }

      float PC = 2.1f;

      // Distance function
      auto distance = [&] (vint2 a, vint2 b, int max_distance)
        {
          if (i1.has(a) and i2.has(b))
          {
            return ssd_distance(i1, i2, a, b, winsize * PC, max_distance);
            // return ve_distance(i1, i2, a, b, r3_offsets1, r3_offsets2,
            //                    sizeof(r3_offsets2) / sizeof(int), max_distance);
          }
          else
            return INT_MAX;
        };
      
      image2d<unsigned char>& flow_map_mark = pyr_flow_map_mark[scale];      
      image2d<vint2>& flow_map = pyr_flow_map[scale];      
      fill_with_border(flow_map_mark, 0);
      
      // Gradient descent
      #pragma omp parallel for
      for (int r = 0; r < i1.nrows(); r+=winsize)
      {
        for (int c = 0; c < i1.ncols(); c+=winsize)
        {
          // if (scale == 0)
          //   if (r < 50 or r > i1.nrows() - 50 or
          //       c < 50 or c > i1.ncols() - 50)
          //     continue;

          vint2 p(r, c);

          // Prediction
          vint2 pfm = p / (2 * winsize);
          vint2 prediction = p;
          if (scale < (nscales - 1) and pyr_flow_map_mark[scale + 1](pfm))
            prediction = p + pyr_flow_map[scale + 1](pfm) * 2;

          // Descent
          auto m = gradient_descent_match(p, prediction, distance, 10);

          // Register match.
          vint2 match = p + m.first;
          //if (m.second < (PC * PC * winsize * winsize * 40) and i1.domain().has(match))
          //if (m.second < (16 * 15) and i1.domain().has(match))
          {
            pyr_flow_map_mark[scale](p / winsize) = true;
            pyr_flow_map[scale](p / winsize) = match - p;
            distance_map[scale](p / winsize) = m.second;
          }
        }
      }

      // Compute trust.

      // Motion histograms.
      image2d<int> trust_map(flow_map.domain(), _border = 1);
      fill_with_border(trust_map, 0);
      image2d<image2d<int>> histograms(5, 5, _border = 0);

      pixel_wise(histograms)(_no_threads) | [] (image2d<int>& i)
        {
          new (&i)image2d<int>(20, 20);
          fill(i, 0);
        };

      int Fc = 50;
      float Hr = 1.f * trust_map.nrows() / histograms.nrows();
      float Hc = 1.f * trust_map.ncols() / histograms.ncols();
      vint2 Hm(histograms(0,0).nrows() / 2, histograms(0,0).ncols() / 2);

      for (int r = 0; r < trust_map.nrows(); r++)
      for (int c = 0; c < trust_map.ncols(); c++)
      {
        assert(histograms.has(vint2(r / Hr, c / Hc)));
        // std::cout << vint2(r / Hr, c / Hc).transpose() << std::endl;
        // if (!histograms.has(vint2(r / Hr, c / Hc)))
        //   throw std::runtime_error("error!");
        auto& H = histograms(r / Hr, c / Hc);
        vint2 b = flow_map(r, c) / Fc + Hm;

        if (H.has(b)) H(b)++;
      }

      // Texture.
      image2d<vint2> grad(i1.domain(), _border = 1);
      scharr(i1, grad);
      
      pixel_wise(trust_map.domain()) | [&] (vint2 p)
        {
          auto& H = histograms(p[0] / Hr, p[1] / Hc);
          vint2 b = flow_map(p) / Fc + Hm;

          float gn = grad(p * winsize).norm();

          
          float support = 0;
          if (H.has(b))
            support += H(b) / (Hr * Hc * 0.1);
          if (gn < 10)
            support = 0;
          trust_map(p) = 100 * (//support // support of the neighbors
                                //);//
                                gn / 15.f); // Texture.

        };
      
      pixel_wise(histograms)(_no_threads) | [] (image2d<int>& i)
        {
          i.~image2d<int>();
        };
      
      // Regularisation
      //if (scale != 0)
      for (int K = 0; K < regularization_niter; K++)
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
                    // (flow_map(pf) - flow_map(pfn)).norm() > (1 / scale_power) and
                    // (prev_flow - flow_map(pfn)).norm() > (1 / scale_power))
                    (flow_map(pf) - flow_map(pfn)).norm() > 2 and
                    (prev_flow - flow_map(pfn)).norm() > 2)
                {
                  // Two neighbors with high divergence.
                  int d1 = distance_map[scale](pf);
                  int d2 = distance(p, p + flow_map(pfn), INT_MAX);

                  // std::cout << vint2(r, c).transpose() << std::endl;
                  int s1 = trust_map(pf);
                  int s2 = trust_map(pfn);
                  // std::cout << s1 << " " << s2 << std::endl;
                  //int dd2 = distance_map[scale](pfn);
 
                  // if (s1 < s2 and s1 < 50 // and d2 < (d1 + winsize2 * 200)
                  //     )
                  // {
                  //   trust_map(pf) = s2;
                  //   flow_map_mark(pf) = true;
                  //   flow_map(pf) = flow_map(pfn);
                  //   distance_map[scale](pf) = d2;
                  // }
                  
                  // else
                  if (d2 < d1 //or
                        //dd2 <= d1
                        //s1 < s2
                      )
                  {
                    auto m = gradient_descent_match(p, p + flow_map(pfn), distance, 10);
                    //auto m = std::make_pair(flow_map(pfn), d2);
                    // Register match.
                    vint2 match = p + m.first;
                    if (m.second < d1 // and
                        // m.second < (PC * PC * winsize * winsize * 40) and i1.domain().has(match)
                        )
                      //if (m.second < (16 * 15) and i1.domain().has(match))
                    {
                      trust_map(pf) = s2;
                      pyr_flow_map_mark[scale](pf) = true;
                      pyr_flow_map[scale](pf) = match - p;
                      distance_map[scale](pf) = m.second;
                    }
                
                  }
                }
              }
          };

        if (K % 2)
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
    prof.end("of_body");

    // std::cout << prof << std::endl;
#pragma omp parallel for
    for (int pi = 0; pi < next_pts.size(); pi++)
    {
      if (pyr_flow_map_mark[0](pts[pi] / winsize))
        next_pts[pi] = pts[pi] + pyr_flow_map[0](pts[pi] / winsize);
      else
        next_pts[pi] = vint2(-1, -1);
    }
    
  }
  
}
