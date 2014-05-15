#ifndef VPP_BOX_NBH2D_HH__
# define VPP_BOX_NBH2D_HH__

# include <vpp/boxNd.hh>
# include <vpp/vector.hh>

namespace vpp
{

  template <int nrows, int ncols>
  struct box_nbh2_runner
  {
    inline box_nbh2_runner(int pitch, int& pix)
      : pitch_(pitch),
        pix_(&pix)
      {
      }

    template <typename F>
    inline void operator<<(F f)
      {
        for (int r = -nrows/2; r <= nrows/2; r++)
          for (int c = -ncols/2; c <= ncols/2; c++)
            f(*(int*)((char*)pix_ + r * pitch_ + c * sizeof(int)));
      }

    int pitch_;
    int* pix_;
  };

  template <typename V, int nrows, int ncols>
  struct box_nbh2d
  {
    inline box_nbh2d(const image2d<V>& img)
      : pitch_(img.pitch())
      {
      }

    inline box_nbh2_runner<nrows, ncols> operator()(V& pix)
    {
      return box_nbh2_runner<nrows, ncols>(pitch_, pix);
    }

    int pitch_;
  };

}

#endif
