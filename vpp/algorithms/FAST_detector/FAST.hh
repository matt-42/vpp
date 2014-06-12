#ifndef VPP_FAST3_DETECTOR_HH_
# define VPP_FAST3_DETECTOR_HH_

# include <thread>
# include <vpp/vpp.hh>

namespace vpp
{

  namespace FAST_internals
  {
    inline bool fast9_check_code(uint code32)
    {
      // Thanks Arkanosis (https://github.com/Arkanosis) for this implementation.
      uint64_t code48 = code32;
      code48 |= code48 << 32;
      code48 &= code48 << 8;
      code48 &= code48 << 4;
      code48 &= code48 << 2;
      return (code48 & (code48 << 2));
    }

    inline bool fast9_check_code_4(uint8_t code8)
    {
      uint16_t code16 = code8;
      code16 |= code16 << 8;
      return (code16 & (code16 << 2));
    }

    template <typename V>
    V* shift_row(V* p, int diff)
    {
      return (V*)(((char*)p) + diff);
    }

    template <typename V>
    inline
    int fast_score(V* row1,
                   V* row2,
                   V* row3,
                   V* row4,
                   V* row5,
                   V* row6,
                   V* row7,
                   int c,
                   int th)
    {
      V v = row4[c];
      int sum_inf = 0;
      int sum_sup = 0;

      auto f = [&] (V a) -> int {
        int diff = v - a;
        if (diff < -th) sum_inf -= diff;
        else if (diff > th) sum_sup += diff;
      };

      f(row1[c - 1]);
      f(row1[c]);
      f(row1[c + 1]);

      f(row2[c + 2]);

      f(row3[c + 3]);
      f(row4[c + 3]);
      f(row5[c + 3]);

      f(row6[c + 2]);

      f(row7[c + 1]);
      f(row7[c]);
      f(row7[c - 1]);

      f(row6[c - 2]);

      f(row5[c - 3]);
      f(row4[c - 3]);
      f(row3[c - 3]);

      f(row2[c - 2]);

      return std::max(sum_sup, sum_inf);
    }

    template <int fullcheck, typename V, typename U>
    void fast_detector9(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row2 = shift_row(a_row, -2*pitch);
        V* a_row3 = shift_row(a_row, -1*pitch);
        V* a_row4 = a_row;
        V* a_row5 = shift_row(a_row, 1*pitch);
        V* a_row6 = shift_row(a_row, 2*pitch);
        V* a_row7 = shift_row(a_row, 3*pitch);

        auto* b_row = &B(r, 0);

        #pragma omp simd aligned(a_row1, a_row2, a_row3, a_row4, a_row5, a_row6, a_row7, b_row:16 * sizeof(int))
        for (int c = 0; c < nc; c++) if (b_row[c] || fullcheck)
        {
          V v = a_row4[c];

          auto f = [&] (V a) -> int { return ((a > v + th) << 1) + (a < v - th); };

          uint x  =
            f(a_row1[c - 1]) +
            (f(a_row1[c]) << 2) +
            (f(a_row1[c + 1]) << 4) +

            (f(a_row2[c + 2]) << 6) +

            (f(a_row3[c + 3]) << 8) +
            (f(a_row4[c + 3]) << 10) +
            (f(a_row5[c + 3]) << 12) +

            (f(a_row6[c + 2]) << 14) +

            (f(a_row7[c + 1]) << 16) +
            (f(a_row7[c]) << 18) +
            (f(a_row7[c - 1]) << 20) +

            (f(a_row6[c - 2]) << 22) +

            (f(a_row5[c - 3]) << 24) +
            (f(a_row4[c - 3]) << 26) +
            (f(a_row3[c - 3]) << 28) +

            (f(a_row2[c - 2]) << 30);
          b_row[c] = fast9_check_code(x);
        }
      }
    }


    template <typename V, typename U>
    void fast_detector9_scores(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row2 = shift_row(a_row, -2*pitch);
        V* a_row3 = shift_row(a_row, -1*pitch);
        V* a_row4 = a_row;
        V* a_row5 = shift_row(a_row, 1*pitch);
        V* a_row6 = shift_row(a_row, 2*pitch);
        V* a_row7 = shift_row(a_row, 3*pitch);

        auto* b_row = &B(r, 0);

        #pragma omp simd aligned(a_row1, a_row2, a_row3, a_row4, a_row5, a_row6, a_row7, b_row:16 * sizeof(int))
        for (int c = 0; c < nc; c++) if (b_row[c])
        {
          b_row[c] = fast_score(a_row1,
                                a_row2,
                                a_row3,
                                a_row4,
                                a_row5,
                                a_row6,
                                a_row7,
                                c, th);
        }
      }
    }

    template <typename V, typename U>
    void fast_detector9_check4(image2d<V>& A, image2d<U>& B, int th)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -3*pitch);
        V* a_row4 = a_row;
        V* a_row7 = shift_row(a_row, 3*pitch);

        auto* b_row = &B(r, 0);

#pragma omp simd aligned(a_row1, a_row4, a_row7, b_row:16 * sizeof(int))
        for (int c = 0; c < nc; c++)
        {
          V v = a_row4[c];

          auto f = [&] (V a) -> int { return ((a > v + th) << 1) | (a < v - th); };

          uint8_t x  =
            (f(a_row1[c])) |
            (f(a_row4[c - 3]) << 2) |
            (f(a_row7[c]) << 4) |
            (f(a_row4[c + 3]) << 6);

          b_row[c] = fast9_check_code_4(x);
        }
      }
    }

    template <typename V>
    void fast_detector9_local_maxima(image2d<V>& A)
    {
      int nc = A.ncols();
      int nr = A.nrows();
      int pitch = A.pitch();

#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        V* a_row = &A(r, 0);
        V* a_row1 = shift_row(a_row, -1*pitch);
        V* a_row2 = a_row;
        V* a_row3 = shift_row(a_row, 1*pitch);

        #pragma omp simd aligned(a_row1, a_row2, a_row3:16 * sizeof(int))
        for (int c = 0; c < nc; c++)
          if (a_row2[c])
          {
            V max = 0;
            max = std::max(max, a_row1[c-1]);
            max = std::max(max, a_row1[c]);
            max = std::max(max, a_row1[c+1]);

            max = std::max(max, a_row2[c-1]);
            max = std::max(max, a_row2[c+1]);

            max = std::max(max, a_row3[c-1]);
            max = std::max(max, a_row3[c]);
            max = std::max(max, a_row3[c+1]);

            if (a_row2[c] < max)
              a_row2[c] = 0;
          }
      }
    }

  }

  void fast9_blockwise_maxima(image2d<int>& A, int block_size)
  {
    int nc = A.ncols();
    int nr = A.nrows();
    int pitch = A.pitch();

    #pragma omp parallel for
    for (int r = 0; r < nr; r += block_size)
    {
      int* rows[block_size];
      for (int i = 0; i < block_size; i++)
        rows[i] = &A(r + i, 0);

      for (int c = 0; c < nc; c += block_size)
      {
        // Maximum search.
        vint2 pmax;
        int vmax = 0;
        for (int br = 0; br < block_size; br++)
        for (int bc = c; bc < c + block_size; bc++)
        {
          int v = rows[br][bc];
          rows[br][bc] = 0;
          if (v > vmax)
          {
            vmax = v;
            pmax = vint2(br, bc);
          }
        }

        if (vmax > 0)
          rows[pmax[0]][pmax[1]] = vmax;
      }
    }
  }


  template <typename V>
  void fast_detector9(image2d<V>& A, image2d<int>& keypoints, int th, bool local_maxima = false,
                      bool low_density = false)
  {
    if (low_density)
    {
      FAST_internals::fast_detector9_check4(A, keypoints, th);
      FAST_internals::fast_detector9<false>(A, keypoints, th);
    }
    else
      FAST_internals::fast_detector9<true>(A, keypoints, th);

    if (local_maxima)
    {
      FAST_internals::fast_detector9_scores(A, keypoints, th);
      FAST_internals::fast_detector9_local_maxima(keypoints);
    }

  }

  template <typename V>
  void fast9_detect(image2d<V>& A, image2d<int>& keypoints, int th,
                    bool low_density = false)
  {
    if (low_density)
    {
      FAST_internals::fast_detector9_check4(A, keypoints, th);
      FAST_internals::fast_detector9<false>(A, keypoints, th);
    }
    else
      FAST_internals::fast_detector9<true>(A, keypoints, th);
  }

  template <typename V>
  void fast9_scores(image2d<V>& A, image2d<int>& keypoints, int th)
  {
    FAST_internals::fast_detector9_scores(A, keypoints, th);
  }

  inline void fast9_filter_localmaximas(image2d<int>& keypoints)
  {
    FAST_internals::fast_detector9_local_maxima(keypoints);
  }

  inline void make_keypoint_vector(image2d<int>& img, std::vector<vint2>& keypoints)
  {
    int n_threads = omp_get_num_threads();
    int nc = img.ncols();
    int nr = img.nrows();
#pragma omp parallel
    {
      std::vector<vint2> local;
#pragma omp for
      for (int r = 0; r < nr; r++)
      {
        auto* row = &img(r, 0);
        for (int c = 0; c < nc; c++)
          if (row[c])
            local.push_back(vint2(r, c));
      }
#pragma omp critical
        keypoints.insert(keypoints.end(), local.begin(), local.end());
    }
  }
}

#endif
