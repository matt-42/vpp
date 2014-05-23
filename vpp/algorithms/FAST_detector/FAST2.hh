#ifndef VPP_FAST2_DETECTOR_HH_
# define VPP_FAST2_DETECTOR_HH_

# include <thread>
# include <vpp/vpp.hh>

namespace vpp
{

  namespace FAST_internals
  {
    inline bool fast9_check_code(uint code32)
    {
      uint64_t code48 = code32;
      code48 |= code48 << 32;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      code48 &= code48 << 2;
      return (code48 & (code48 << 2)) ? 1 : 0;
    }

    template <typename V>
    V* shift_row(V* p, int diff)
    {
      return (V*)(((char*)p) + diff);
    }

    template <typename V, typename U>
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
        V* a_row8 = shift_row(a_row, 4*pitch);

        auto* b_row = &B(r, 0);
        auto* b_row2 = &B(r + 1, 0);

#pragma omp simd aligned(a_row1, a_row2, a_row3, a_row4, a_row5, a_row6, a_row7, b_row:4 * sizeof(int))
        for (int c = 0; c < nc; c++)
        {
          V v = a_row4[c];

          auto f = [&] (V a) -> int { return (a > v + th) ? 2 : (a < v - th); };

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

    void fast_count(image2d<int>& A, image2d<unsigned char>& B)
    {
      int nc = A.ncols();
      int nr = A.nrows();
#pragma omp parallel for
      for (int r = 0; r < nr; r++)
      {
        auto* a_row = &A(r, 0);
        auto* b_row = &B(r, 0);

#pragma omp simd aligned(b_row: 8 * sizeof(int))
        for (int c = 0; c < nc; c++)
        {
          b_row[c] = fast9_check_code(a_row[c]);
        }
      }
    }
  }

  template <typename V>
  void fast_detector9(image2d<V>& A, image2d<int>& keypoints, int th)
  {
    FAST_internals::fast_detector9(A, keypoints, th);
  }

  inline void keypoints_to_vector(image2d<int>& img, std::vector<vint2>& keypoints)
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



