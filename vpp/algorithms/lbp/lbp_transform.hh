

namespace vpp
{

  template <typename V, typename U>
  void lbp_transform(image2d<V>& A, image2d<U>& B)
  {
    int nr = A.nrows();
#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      U* curB = &B(vint2(r, 0));
      int nc = A.ncols();

      V* rows[3];
      for (int i = -1; i <= 1; i++)
        rows[i + 1] = (V*)&A(vint2(r + i, 0));

#pragma omp simd
      for (int i = 0; i < nc; i++)
      {
        curB[i] =
          ((rows[0][i - 1] > rows[1][i]) << 0) +
          ((rows[0][i    ] > rows[1][i]) << 1) +
          ((rows[0][i + 1] > rows[1][i]) << 2) +

          ((rows[1][i - 1] > rows[1][i]) << 3) +
          ((rows[1][i + 1] > rows[1][i]) << 4) +

          ((rows[2][i - 1] > rows[1][i]) << 5) +
          ((rows[2][i    ] > rows[1][i]) << 6) +
          ((rows[2][i + 1] > rows[1][i]) << 7);
        
      }
    }

  }
  
}
