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
    inline void for_all(F f)
      {
        for (int r = -nrows/2; r <= nrows/2; r++)
          for (int c = -ncols/2; c <= ncols/2; c++)
            f(*(Const<V>*)((Const<char>*)pix_ + r * pitch_ + c * sizeof(V)));
      }

    int pitch_;
    Const<V>* pix_;
  };

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_row_iterator;

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_
  {
    typedef V value_type;
    typedef box_nbh2d_row_iterator<V, Const, nrows, ncols> row_iterator;

    inline box_nbh2d_(Const<image2d<V>>& img)
      : pitch_(img.pitch()),
        begin_(&img(0,0))
    {
    }

    inline box_nbh2d_(Const<image2d<V>>& img, vint2 p)
      : pitch_(img.pitch()),
        begin_(&img(0,0))
    {
      for (int r = -nrows / 2; r <= nrows / 2; r++)
        rows_[r + nrows / 2] = (Const<V>*)((char*)begin_ + pitch_ * (p[0] + r) + p[1] * sizeof(V));
    }

    Const<V>& operator()(int r, int c) const { return rows_[r + nrows / 2][c]; }
    Const<V>* row(int r) const { return rows_[r + nrows / 2]; }

    template <typename F>
    void for_all(F f) const
    {
      for (int r = -nrows/2; r <= nrows/2; r++)
        for (int c = -ncols/2; c <= ncols/2; c++)
          f(operator()(r, c));
    }

    void next()
    {
      for (int i = 0; i < nrows; i++)
        rows_[i]++;
    }

    int pitch_;
    Const<V>* begin_;
    Const<V>* rows_[nrows];
  };

  template <typename V, int nrows, int ncols>
  using box_nbh2d = box_nbh2d_<V, unconstify, nrows, ncols>;
  template <typename V, int nrows, int ncols>
  using const_box_nbh2d = box_nbh2d_<V, constify, nrows, ncols>;

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_row_iterator
  {
    typedef box_nbh2d_<V, Const, nrows, ncols> nbh_type;

    box_nbh2d_row_iterator(vint2 p, const nbh_type& _nbh)
      : nbh_(_nbh)
    {
      for (int r = -nrows / 2; r <= nrows / 2; r++)
        nbh_.rows_[r + nrows / 2] = (Const<V>*)((char*)nbh_.begin_ + nbh_.pitch_ * (p[0] + r));
    }

    void next()
    {
      nbh_.next();
    }
    
    nbh_type& operator*()
    {
      return nbh_;
    }

    nbh_type nbh_;
  };

}

#endif
