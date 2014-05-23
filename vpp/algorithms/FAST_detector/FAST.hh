#ifndef VPP_FAST_DETECTOR_HH_
# define VPP_FAST_DETECTOR_HH_

# include <vpp/vpp.hh>

namespace vpp
{
  struct fast_windows
  {
    template <typename I>
    static window<I> circle(const I& img)
    {
      return make_window(img, {
          { -3,-1}, {-3, 0}, {-3, 1}, {-2, 2}, { -1, 3},
          { 0, 3}, { 1, 3}, { 2, 2}, {  3, 1},
          { 3, 0}, { 3,-1}, { 2,-2}, {  1,-3},
          { 0,-3}, {-1,-3}, {-2,-2}
        });
    }

    template <typename I>
    static window<I> circle4(const I& img)
    {
      return make_window(img, {
          {-3, 0},
          { 0, 3},
          { 3, 0},
          { 0,-3}
        });
    }

  };

#pragma omp declare simd
  template <typename V, typename W>
  inline void fast_saliency(V& a, V& b, const W& circle4, int th)
  {
      int i = 0;
      typedef decltype(a - b) T;
      // Fast test on circle4
      i = 0;
      char values[16];
      int n_inf = 0;
      int n_sup = 0;
      circle4(a) < [&] (V& n)
      {
        if (n < a - th) n_inf++;
        if (n > a + th) n_sup++;
      };
      if (n_inf < 3 && n_sup < 3) { b = 0; return; }

  }

  template <unsigned N, typename V, typename U>
  std::enable_if_t<N == 9, void>
  fast_detector(image2d<V>& in, image2d<V>& out, U th)
  {
    const auto circle = fast_windows::circle(in);
    const auto circle4 = fast_windows::circle4(in);

    int nr = in.nrows();
    int nc = in.ncols();

    // First pass. Fast test.
// #pragma omp parallel for
//     for (int r = 0; r < nr; r++)
//     {
//       V* lprev = &in(r - 2, 0);
//       V* l = &in(r, 0);
//       V* lnext = &in(r + 2, 0);
//       V* out_row = &out(r, 0);

// #pragma omp simd aligned(l, lprev, lnext, out_row : sizeof(int) * 8)
//       for (int c = 0; c < nc; c++)
//       {
//         V a = l[c];
//         V v1 = lprev[c - 2];
//         V v2 = lnext[c + 2];
//         V v3 = lprev[c + 2];
//         V v4 = lnext[c - 2];

//         V as = a + th;
//         V ai = a - th;

//         int n_inf = (v1 < ai) +
//           (v2 < ai) +
//           (v3 < ai) +
//           (v4 < ai);

//         int n_sup = (v1 > as) +
//           (v2 > as) +
//           (v3 > as) +
//           (v4 > as);
//         out_row[c] = n_inf >= 3 || n_sup >= 3;
//       }

//     }


//#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      V* lprev = &in(r - 3, 0);
      V* l = &in(r, 0);
      V* lnext = &in(r + 3, 0);
      V* out_row = &out(r, 0);

#pragma omp simd aligned(l, lprev, lnext, out_row : sizeof(int) * 8)
      for (int c = 0; c < nc; c++)
      {
        V a = l[c];
        V v1 = l[c - 3];
        V v2 = l[c + 3];
        V v3 = lprev[c];
        V v4 = lnext[c];

        V as = a + th;
        V ai = a - th;

        int n_inf = (v1 < ai) +
          (v2 < ai) +
          (v3 < ai) +
          (v4 < ai);

        int n_sup = (v1 > as) +
          (v2 > as) +
          (v3 > as) +
          (v4 > as);
        //out_row[c] = out_row[c] && (n_inf >= 3 || n_sup >= 3);
        out_row[c] = (n_inf >= 3 || n_sup >= 3);
      }

    }

    pixel_wise(in, out)//.step(1)
      << [&] (V& a, V& b)
    {
      if (!b)
        // fast_saliency(a, b, circle4, th);
        return;

      int i = 0;
      int values[16];
      typedef decltype(a - b) T;

      // Test on the whole 16pixels circle.
      i = 0;
      circle(a) < [&] (V& n)
      {
        values[i] = n < a - th ? -1 : (n > a + th);
        ++i;
      };

      // Count the max number of consecutive -1 or 1;
      int n = 0;
      int max_n = 0;
      int cur = 0;
      for (i = 0; i < 16; i++)
      {
        if (values[i] == cur) n++;
        else if (values[i])
        {
          cur = values[i];
          max_n = std::max(n, max_n);
          n = 0;
        }
      }
      i = 0;
      if (n != 16 and cur == values[0])
        while (values[i] == cur) i++;

      n += i;
      max_n = std::max(n, max_n);

      b = max_n >= 9;
    };


  }
}

// SLOW
// #pragma omp parallel for
//     for (int r = 0; r < nr; r++)
//     {
//       V* lm3 = &in(r - 3, 0);
//       V* lm2 = &in(r - 2, 0);
//       V* lm1 = &in(r - 1, 0);
//       V* l = &in(r, 0);
//       V* lp1 = &in(r + 1, 0);
//       V* lp2 = &in(r + 2, 0);
//       V* lp3 = &in(r + 3, 0);

//       V* out_row = &out(r, 0);

//       #pragma omp simd// aligned(l, lprev, lnext, out_row : sizeof(int) * 8)
//       for (int c = 0; c < nc; c++) if (out_row[c])
//       {
//         V vs[16];
//         V a = l[c];
//         V as = a + th;
//         V ai = a - th;

//         auto f = [=] (V x) { return x < ai ? -1 : (x > as); };
//         vs[0] = f(lm3[c-1]);
//         vs[1] = f(lm3[c]);
//         vs[2] = f(lm3[c+1]);

//         vs[3] = f(lm2[c+2]);

//         vs[4] = f(lm1[c+3]);
//         vs[5] = f(l[c+3]);
//         vs[6] = f(lp1[c+3]);

//         vs[7] = f(lp2[c+2]);

//         vs[8] = f(lp3[c+1]);
//         vs[9] = f(lp3[c]);
//         vs[10] = f(lp3[c-1]);

//         vs[11] = f(lp2[c-2]);

//         vs[12] = f(lp1[c-3]);
//         vs[13] = f(l[c-3]);
//         vs[14] = f(lm1[c-3]);

//         vs[15] = f(lm2[c-2]);

//         // for (int i = 0; i < 16; i++)
//         //   vs[i] = vs[i] < a - th ? -1 : (vs[i] > a + th);
//         // Count the max number of consecutive -1 or 1;
//         int n = 0;
//         int max_n = 0;
//         int cur = 0;
//         int i;
//         for (i = 0; i < 16; i++)
//         {
//       if (vs[i] == cur) n++;
//       else if (vs[i])
//       {
//       cur = vs[i];
//       max_n = std::max(n, max_n);
//       n = 0;
//     }
//     }
//         i = 0;
//         if (N != 16 and cur == vs[0])
//           while (vs[i] == cur) i++;

//         n += i;
//         max_n = std::max(n, max_n);

//         out_row[c] = max_n >= 9;
//       }

//     }

// Old iteration.
// pixel_wise(in, out)//.step(1)
//   << [&] (V& a, V& b)
// {
//   if (!b)
//   // fast_saliency(a, b, circle4, th);
//   return;

//   int i = 0;
//   int values[16];
//   typedef decltype(a - b) T;
//   // Fast test on circle4
//   // i = 0;
//   // int n_inf = 0;
//   // int n_sup = 0;
//   // circle4(a) < [&] (V& n)
//   // {
//   //   if (n < a - th) n_inf++;
//   //   if (n > a + th) n_sup++;
//   // };
//   // if (n_inf < 3 && n_sup < 3) { b = 0; return; }

//   // return;
//   // Test on the whole 16pixels circle.
//   i = 0;
//   circle4(a) < [&] (V& n)
//   {
//     values[i] = n < a - th ? -1 : (n > a + th);
//     ++i;
//   };

//   // Count the max number of consecutive -1 or 1;
//   int n = 0;
//   int max_n = 0;
//   int cur = 0;
//   for (i = 0; i < 16; i++)
//   {
//     if (values[i] == cur) n++;
//     else if (values[i])
//     {
//       cur = values[i];
//       max_n = std::max(n, max_n);
//       n = 0;
//     }
//   }
//   i = 0;
//   if (N != 16 and cur == values[0])
//     while (values[i] == cur) i++;

//   n += i;
//   max_n = std::max(n, max_n);

//   b = max_n >= 9;
// };

#endif


