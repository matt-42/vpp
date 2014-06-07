#ifndef VPP_PYRAMID_HH__
# define VPP_PYRAMID_HH__

# include <vpp/core/image2d.hh>
# include <vpp/core/box_nbh2d.hh>
# include <vpp/core/vector.hh>

namespace vpp
{

  template <typename V>
  void subsample2(const image2d<V>& in, image2d<V>& out)
  {
    int nr = out.nrows();
    int nc = out.ncols();

    typedef plus_promotion<V> S;
#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      V* out_row = &out(r, 0);
      const V* row1 = &in(r * 2, 0);
      const V* row2 = &in(r * 2 + 1, 0);
#pragma omp simd
      for (int c = 0; c < nc; c++)
      {
        out_row[c] = vpp::cast<V, S>((cast<S>(row1[c * 2]) + cast<S>(row1[c * 2 + 1]) +
                                      cast<S>(row2[c * 2]) + cast<S>(row2[c * 2 + 1])) / 4);
      }
    }
  }

  template <typename V, unsigned N>
  struct pyramid
  {
  public:

    typedef imageNd<V, N> image_type;

    pyramid(boxNd<N> d, int nlevels, int factor, border b = 0)
      : levels_(nlevels),
        factor_(factor)
    {
      for (int i = 0; i < nlevels; i++)
      {
        levels_[i] = imageNd<V, N>(d, b);
        d = make_box2d(d.nrows() / factor, d.ncols() / factor);
      }
    }

    image_type& operator[] (unsigned i)
    {
      return levels_[i];
    }

    const image_type& operator[] (unsigned i) const
    {
      return levels_[i];
    }

    void propagate_level0()
    {
      for (int i = 1; i < levels_.size(); i++)
      {
        if (factor_ == 2)
          subsample2(levels_[i - 1], levels_[i]);
        else
          assert(0); // Not implemented.
      }
    }

    void update(const imageNd<V, N>& in)
    {
      copy(in, levels_[0]);
      propagate_level0();
    }

    int factor() const { return factor_; }
    int size() const { return levels_.size(); }

    void swap(pyramid<V, N>& o)
    {
      levels_.swap(o.levels_);
      std::swap(factor_, o.factor_);
    }

    std::vector<image_type>& levels() { return levels_; };
    const std::vector<image_type>& levels() const { return levels_; };

  private:
    std::vector<image_type> levels_;
    int factor_;
  };

  template <typename V>
  using pyramid2d = pyramid<V, 2>;

  template <typename V>
  using pyramid3d = pyramid<V, 3>;

};

#endif
