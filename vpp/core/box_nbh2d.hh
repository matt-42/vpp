#ifndef VPP_BOX_NBH2D_HH__
# define VPP_BOX_NBH2D_HH__

# include <type_traits>
# include <vpp/core/boxNd.hh>
# include <vpp/core/vector.hh>
# include <vpp/core/image2d.hh>
# include <vpp/core/const.hh>

namespace vpp
{

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2_runner
  {
    inline box_nbh2_runner(int pitch, Const<V>& pix)
      : pitch_(pitch),
        pix_(&pix)
      {
      }

    template <typename F>
    inline void operator<(F f)
      {
        for (int r = -nrows/2; r <= nrows/2; r++)
          for (int c = -ncols/2; c <= ncols/2; c++)
            f(*(Const<V>*)((Const<char>*)pix_ + r * pitch_ + c * sizeof(V)));
      }

    int pitch_;
    Const<V>* pix_;
  };

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_
  {
    inline box_nbh2d_(Const<image2d<V>>& img)
      : pitch_(img.pitch())
      {
      }

    inline box_nbh2_runner<V, Const, nrows, ncols> operator()(Const<V>& pix)
    {
      return box_nbh2_runner<V, Const, nrows, ncols>(pitch_, pix);
    }

    int pitch_;
  };

  template <typename V, int nrows, int ncols>
  using box_nbh2d = box_nbh2d_<V, unconstify, nrows, ncols>;
  template <typename V, int nrows, int ncols>
  using const_box_nbh2d = box_nbh2d_<V, constify, nrows, ncols>;

}

#endif
