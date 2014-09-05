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
  struct box_nbh2d_accessor
  {
    box_nbh2d_accessor()
    {
    }

    template <int r, int c>
    Const<V>& at()
    {
      return rows_[r + nrows / 2][c];
    }

    Const<V>& at(int r, int c)
    {
      return rows_[r + nrows / 2][c];
    }


    template <int r>
    Const<V>* row()
    {
      return rows_[r + nrows / 2];
    }

    template <typename F>
    void for_all(F f)
    {
      for (int r = -nrows/2; r <= nrows/2; r++)
        for (int c = -ncols/2; c <= ncols/2; c++)
          f(at(r, c));
    }

    Const<V>* rows_[nrows];
  };

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_;

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_row_iterator
  {
    typedef box_nbh2d_accessor<V, Const, nrows, ncols> accessor_type;

    box_nbh2d_row_iterator(vint2 p, const box_nbh2d_<V, Const, nrows, ncols>& box)
    {
      for (int r = -nrows / 2; r <= nrows / 2; r++)
        accessor_.rows_[r + nrows / 2] = (Const<V>*)((char*)box.begin_ + box.pitch_ * (p[0] + r));
    }

    void next()
    {
      for (int i = 0; i < nrows; i++)
        accessor_.rows_[i]++;
    }
    
    accessor_type& operator*()
    {
      return accessor_;
    }

    accessor_type accessor_;
  };

  template <typename V, template <class> class Const, int nrows, int ncols>
  struct box_nbh2d_
  {
    typedef box_nbh2d_row_iterator<V, Const, nrows, ncols> row_iterator;

    inline box_nbh2d_(Const<image2d<V>>& img)
      : pitch_(img.pitch()),
        begin_(&img(0,0))
    {
    }

    inline box_nbh2_runner<V, Const, nrows, ncols> operator()(Const<V>& pix)
    {
      return box_nbh2_runner<V, Const, nrows, ncols>(pitch_, pix);
    }

    int pitch_;
    Const<V>* begin_;
  };

  template <typename V, int nrows, int ncols>
  using box_nbh2d = box_nbh2d_<V, unconstify, nrows, ncols>;
  template <typename V, int nrows, int ncols>
  using const_box_nbh2d = box_nbh2d_<V, constify, nrows, ncols>;

}

#endif
