#ifndef VPP_ALGORITHMS_FILTERS_SCHARR_HH_
# define VPP_ALGORITHMS_FILTERS_SCHARR_HH_

# include <vpp/core/image2d.hh>
# include <vpp/core/vector.hh>

namespace vpp
{

  template <typename U, typename V>
  void scharr(const image2d<vector<U, 1>>& in, image2d<vector<V, 2>>& out)
  {
    assert(in.border() >= 1);

    int nr = out.nrows();
    int nc = out.ncols();

#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      vector<V, 2>* out_row = &out(r, 0);
      const vector<U, 1>* row1 = &in(r - 1, 0);
      const vector<U, 1>* row2 = &in(r, 0);
      const vector<U, 1>* row3 = &in(r + 1, 0);

#pragma omp simd
      for (int c = 0; c < nc; c++)
      {
        out_row[c] = vector<V, 2>(

          (3 * int(row3[c - 1][0]) +
           10 * int(row3[c][0]) +
           3 * int(row3[c + 1][0])
           -
           3 * int(row1[c - 1][0]) -
           10 * int(row1[c ][0]) -
           3 * int(row1[c + 1][0])) / 32.f

          ,

          (3 * int(row1[c + 1][0]) +
           10 * int(row2[c + 1][0]) +
           3 * int(row3[c + 1][0])
           -
           3 * int(row1[c - 1][0]) -
           10 * int(row2[c - 1][0]) -
           3 * int(row3[c - 1][0])) / 32.f
          );
      }
    }

  }

};

#endif
