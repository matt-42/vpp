#ifndef VPP_PYRAMID_HH__
# define VPP_PYRAMID_HH__

# include <vpp/core/image2d.hh>
# include <vpp/core/vector.hh>
# include <vpp/core/fill.hh>

namespace vpp
{

  template <typename V>
  void antialiasing_lowpass_filter(const image2d<V>& in, image2d<V>& out)
  {
    image2d<V> tmp(in.domain(), _border = 2);
    int nr = in.nrows();
    int nc = in.ncols();
    typedef plus_promotion<V> S;

    #pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      const V* i = &in(r, 0);
      V* o = &tmp(r, 0);
#pragma omp simd
      for (int c = 0; c < nc; c++)
      {
        o[c] =
          cast<V>((1 * cast<S>(i[c - 2]) +
                   4 * cast<S>(i[c - 1]) +
                   6 * cast<S>(i[c]) +
                   4 * cast<S>(i[c + 1]) +
                   1 * cast<S>(i[c + 2])) / 16);
      }
    }

    fill_border_mirror(tmp);

    #pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      const V* r1 = &tmp(r - 2, 0);
      const V* r2 = &tmp(r - 1, 0);
      const V* r3 = &tmp(r, 0);
      const V* r4 = &tmp(r + 1, 0);
      const V* r5 = &tmp(r + 2, 0);
      V* o = &out(r, 0);
#pragma omp simd
      for (int c = 0; c < nc; c++)
      {
        o[c] =
          cast<V>((1 * cast<S>(r1[c]) +
                   4 * cast<S>(r2[c]) +
                   6 * cast<S>(r3[c]) +
                   4 * cast<S>(r4[c]) +
                   1 * cast<S>(r5[c])) / 16);
      }
    }

  }

  template <typename V>
  void subsample2(const image2d<V>& in, image2d<V>& out)
  {
    int nr = out.nrows();
    int nc = out.ncols();

#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      V* out_row = &out(r, 0);
      const V* row1 = &in(r * 2, 0);

#pragma omp simd
      for (int c = 0; c < nc; c++)
      {
        out_row[c] = row1[c * 2];
        // out_row[c] = vpp::cast<V, S>((cast<S>(row1[c * 2]) + cast<S>(row1[c * 2 + 1]) +
        //                               cast<S>(row2[c * 2]) + cast<S>(row2[c * 2 + 1])) / 4);
      }
    }
  }


  template <typename V>
  void subsample(const image2d<V>& in, image2d<V>& out, float factor)
  {
    int nr = out.nrows();
    int nc = out.ncols();


#pragma omp parallel for
    for (int r = 0; r < nr; r++)
    {
      V* out_row = &out(r, 0);
      const V* row1 = &in(int(r * factor), 0);
      for (int c = 0; c < nc; c++)
      {
        out_row[c] = row1[int(c * factor)];
        // out_row[c] = vpp::cast<V, S>((cast<S>(row1[c * 2]) + cast<S>(row1[c * 2 + 1]) +
        //                               cast<S>(row2[c * 2]) + cast<S>(row2[c * 2 + 1])) / 4);
      }
    }
  }
  
  template <typename V>
  image2d<V> antialias_subsample2(const image2d<V>& in)
  {
    auto tmp = clone(in, _border = std::max(in.border(), 1));
    fill_border_mirror(tmp);

    image2d<V> in2 = in;
    if (in2.border() < 2)
    {
      in2 = clone(in2, _border = 2);
      fill_border_mirror(in2);
    }

    antialiasing_lowpass_filter(in2, tmp);
    image2d<V> tmp2(1 + (in.nrows() / 2), 1 + (in.ncols() / 2), _border = std::max(in.border(), 0));
    subsample2(tmp, tmp2);
    fill_border_mirror(tmp2);
    return tmp2;
  }

  template <typename V, unsigned N>
  struct pyramid
  {
  public:

    typedef imageNd<V, N> image_type;

    template <typename... O>
    pyramid(boxNd<N> d, int nlevels, float factor, const O&... image_options)
      : levels_(nlevels),
        factor_(factor)
    {
      for (int i = 0; i < nlevels; i++)
      {
        levels_[i] = imageNd<V, N>(d, image_options...);
        d = make_box2d(1 + (d.nrows() / factor), 1 + (d.ncols() / factor));
      }
    }


    template <typename... O>
    pyramid(const imageNd<V, N>& img, int nlevels, float factor, const O&... image_options)
      : levels_(nlevels),
        factor_(factor)
    {
      boxNd<N> d = img.domain();
      for (int i = 0; i < nlevels; i++)
      {
        levels_[i] = imageNd<V, N>(d, image_options...);
        d = make_box2d(1 + (d.nrows() / factor), 1 + (d.ncols() / factor));
      }
      update(img);
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
      // image_type tmp_(levels_[0].domain(), _border = 3);
      fill_border_mirror(levels_[0]);

      for (int i = 1; i < levels_.size(); i++)
      {
        if (factor_ == 2.f)
        {
          //image_type tmp = tmp_.subimage(levels_[i - 1].domain());
          image_type tmp(levels_[i - 1].domain(), _border = 3);
          antialiasing_lowpass_filter(levels_[i - 1], tmp);
          subsample2(tmp, levels_[i]);
          fill_border_mirror(levels_[i]);
        }
        else
        {
          image_type tmp(levels_[i - 1].domain(), _border = 3);
          antialiasing_lowpass_filter(levels_[i - 1], tmp);
          subsample(tmp, levels_[i], factor_);
          fill_border_mirror(levels_[i]);
        }
      }
    }

    void update(const imageNd<V, N>& in)
    {
      copy(in, levels_[0]);
      propagate_level0();
    }

    float factor() const { return factor_; }
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
    float factor_;
  };

  template <typename V>
  using pyramid2d = pyramid<V, 2>;

  template <typename V>
  using pyramid3d = pyramid<V, 3>;

};

#endif
